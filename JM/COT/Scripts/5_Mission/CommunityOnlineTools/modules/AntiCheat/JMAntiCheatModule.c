// =============================================================================
//  JMAntiCheatModule.c
//
//  Server-side cheat detection. Runs a periodic poll (default 1 Hz) that
//  evaluates every connected player against the rules in JMAntiCheatDetector
//  and accumulates flags. When the score crosses the configured threshold the
//  module fires a webhook (severity-based colour) and optionally auto-kicks.
//
//  RPCs (JMAntiCheatModuleRPC @ 10920):
//    RequestFlags   client -> server
//    Flags          server -> client
//
//  Client RPC traffic only happens when an admin opens the form (one-shot
//  fetch). Per-frame server polling never produces client RPCs.
// =============================================================================

class JMAntiCheatModule : JMRenderableModuleBase
{
	protected ref JMAntiCheatConfig m_Config;

	// Server-side state
	protected ref map< string, ref JMAntiCheatFlag > m_Flags;
	protected ref map< string, vector > m_LastPosition;
	protected ref map< string, int > m_LastPositionMs;
	protected ref map< string, float > m_LastHealth;
	protected ref map< string, int > m_ItemsAddedWindow;
	protected ref map< string, int > m_LastWindowReset;

	//! Per-detector cooldown: "<guid>|<detector>" -> last flag time in ms.
	//! Without this a speedhacker scores once a poll and the total measures how
	//! long they were online rather than how bad the reading is.
	protected ref map< string, int > m_LastFlagMs;

	//! Persistence bookkeeping. Writes are skipped entirely while nothing has
	//! changed, so an idle server does not rewrite the same file every minute.
	protected bool  m_FlagsDirty;
	protected float m_SaveAccumulator;
	protected float m_DecayAccumulator;

	//! Decay is a few points an HOUR. Charging for it every poll would be all
	//! truncation and no movement, and it costs the same to charge for it in
	//! half-minute lumps because ApplyDecay bills real elapsed time.
	static const float DECAY_INTERVAL_S = 30.0;

	//! Damage immunity: consecutive damaging hits with no health drop.
	protected ref map< string, int > m_NoDropHits;
	protected ref map< string, float > m_LastHitHealth;

	//! Infinite ammo: consecutive shots with no cartridge decrement, and the
	//! count seen after the previous shot.
	protected ref map< string, int > m_ShotsNoDecrement;
	protected ref map< string, int > m_LastAmmoCount;

	//! Denied RPCs inside the rolling window, and when that window opened.
	protected ref map< string, int > m_DeniedRpcCount;
	protected ref map< string, int > m_DeniedRpcWindowMs;
	protected ref map< string, string > m_DeniedRpcLast;

	//! Kill timestamps for the rapid-kill rule, oldest first, capped.
	protected ref map< string, ref array< int > > m_RecentKillMs;

	//! Inventory sampling runs on its own slower cadence than the main poll.
	protected ref map< string, int > m_LastItemCount;
	protected float m_InventoryAccumulator;

	// Kill-stats state
	protected ref map< string, ref JMAntiCheatKillStats > m_KillStats;
	protected ref map< string, vector > m_LastAlivePosition;

	// Admin-client mirror (sent on RequestFlags)
	protected ref array< string > m_ClientGuidOrder;
	protected ref map< string, ref JMAntiCheatFlag > m_ClientFlags;
	protected ref array< string > m_ClientKillGuidOrder;
	protected ref map< string, ref JMAntiCheatKillStats > m_ClientKillStats;

	void JMAntiCheatModule()
	{
		m_Flags           = new map< string, ref JMAntiCheatFlag >;
		m_LastPosition    = new map< string, vector >;
		m_LastPositionMs  = new map< string, int >;
		m_LastHealth      = new map< string, float >;
		m_ItemsAddedWindow = new map< string, int >;
		m_LastWindowReset = new map< string, int >;

		m_LastFlagMs           = new map< string, int >;
		m_NoDropHits           = new map< string, int >;
		m_LastHitHealth        = new map< string, float >;
		m_ShotsNoDecrement     = new map< string, int >;
		m_LastAmmoCount        = new map< string, int >;
		m_DeniedRpcCount       = new map< string, int >;
		m_DeniedRpcWindowMs    = new map< string, int >;
		m_DeniedRpcLast        = new map< string, string >;
		m_RecentKillMs         = new map< string, ref array< int > >;
		m_LastItemCount        = new map< string, int >;

		m_KillStats         = new map< string, ref JMAntiCheatKillStats >;
		m_LastAlivePosition = new map< string, vector >;

		m_ClientGuidOrder    = new array< string >;
		m_ClientFlags        = new map< string, ref JMAntiCheatFlag >;
		m_ClientKillGuidOrder = new array< string >;
		m_ClientKillStats     = new map< string, ref JMAntiCheatKillStats >;

		GetPermissionsManager().RegisterPermission( "Admin.AntiCheat.View" );
		GetPermissionsManager().RegisterPermission( "Admin.AntiCheat.Clear" );
		GetPermissionsManager().RegisterPermission( "COT" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Admin.AntiCheat.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/anticheat_form.layout";
	}

	override string GetCategory()
	{
		return "Server";
	}

	override string GetTitle()
	{
		return "#STR_COT_ANTICHEAT_MODULE_NAME";
	}

	override string GetIconName()
	{
		return JMConstants.Lucide( "shield-alert" );
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	override string GetWebhookTitle()
	{
		return "Anti-Cheat Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Detection" );
		types.Insert( "Action" );
	}

	override int GetRPCMin()
	{
		return JMAntiCheatModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMAntiCheatModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMAntiCheatModuleRPC.RequestFlags:
			RPC_RequestFlags( ctx, sender, target );
			break;
		case JMAntiCheatModuleRPC.Flags:
			RPC_Flags( ctx, sender, target );
			break;
		}
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if ( IsMissionHost() )
		{
			m_Config = JMAntiCheatConfig.Load();

			//! The 4_World callers that grant movement sanctions cannot reach
			//! a 5_Mission config, so the number is pushed down to them here.
			JMAntiCheatSanction.MoveGraceSeconds = m_Config.PositionGraceS;

			LoadFlags();

			Print( "[COT AntiCheat] loaded: enabled=" + m_Config.Enabled.ToString() + " poll=" + m_Config.PollInterval.ToString() + "s decay=" + m_Config.FlagDecayPerHour.ToString() + "/h" );
		}
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		//! Last chance to persist. A clean shutdown should never lose the
		//! window since the last periodic write.
		SaveFlags( true );
	}

	override void OnUpdate( float timeslice )
	{
		if ( !IsMissionHost() || !m_Config || !m_Config.Enabled )
			return;

		// Drain the static 4_World-side kill-event queue written by
		// JMAntiCheatKillHook.  The 4_World module can't see us directly,
		// so we poll the queue each tick.  Bounded at 200 entries by
		// the producer (Enqueue), so a single tick is bounded.
		DrainKillEventQueue();

		// Signals posted from 4_World: refused RPCs and fired shots. Drained
		// every frame rather than on the poll cadence, because both queues are
		// bounded and a backlog would start dropping the oldest evidence.
		DrainDeniedRpcQueue();
		DrainShotQueue();

		// Manual throttle - OnUpdate is per-frame
		static float accumulator = 0;
		accumulator += timeslice;

		m_InventoryAccumulator += timeslice;
		m_DecayAccumulator     += timeslice;
		m_SaveAccumulator      += timeslice;

		if ( accumulator < m_Config.PollInterval )
			return;
		accumulator = 0;

		PollAllPlayers();

		if ( m_DecayAccumulator >= DECAY_INTERVAL_S )
		{
			m_DecayAccumulator = 0;
			DecayFlags();
		}

		if ( m_SaveAccumulator >= m_Config.FlagSaveIntervalS )
		{
			m_SaveAccumulator = 0;
			SaveFlags( false );
		}
	}

	// -----------------------------------------------------------------------
	//  Unauthorized RPC
	// -----------------------------------------------------------------------

	private void DrainDeniedRpcQueue()
	{
		array< string > guids = new array< string >;
		array< string > permissions = new array< string >;

		JMAntiCheatSignals.DrainDeniedRpc( guids, permissions );

		int nowMs = g_Game.GetTime();
		int windowMs = (int)( m_Config.RpcDeniedWindowS * 1000 );

		for ( int i = 0; i < guids.Count(); i++ )
		{
			string guid = guids.Get( i );

			if ( guid == "" )
				continue;

			if ( JMAntiCheatSanction.IsActive( guid, JMAntiCheatSanction.RPC ) )
				continue;

			//! Staff are counted, not skipped. The one allowance they get is a
			//! higher burst: a COT client shows an admin every control their
			//! role grants, so clicking one whose permission changed
			//! mid-session is a denial that means nothing. A cheat menu
			//! probing the mod clears either number in seconds.
			int burst = m_Config.RpcDeniedBurstCount;
			if ( HasCOTAccess( guid ) )
				burst = m_Config.RpcDeniedBurstCountStaff;

			int windowStart = 0;
			if ( m_DeniedRpcWindowMs.Contains( guid ) )
				windowStart = m_DeniedRpcWindowMs.Get( guid );

			if ( windowStart == 0 || ( nowMs - windowStart ) > windowMs )
			{
				m_DeniedRpcWindowMs.Set( guid, nowMs );
				m_DeniedRpcCount.Set( guid, 0 );
			}

			int count = 0;
			if ( m_DeniedRpcCount.Contains( guid ) )
				count = m_DeniedRpcCount.Get( guid );

			count++;
			m_DeniedRpcCount.Set( guid, count );
			m_DeniedRpcLast.Set( guid, permissions.Get( i ) );

			array< ref JMAntiCheatHit > hits = new array< ref JMAntiCheatHit >;
			JMAntiCheatDetector.DetectDeniedRpcBurst( count, burst, m_Config.RpcDeniedWindowS, permissions.Get( i ), m_Config.RpcDeniedFlagWeight, hits );

			if ( hits.Count() > 0 )
			{
				//! Reset the window on a flag so the same burst is not scored
				//! again on every further denial inside it.
				m_DeniedRpcCount.Set( guid, 0 );
				m_DeniedRpcWindowMs.Set( guid, nowMs );
				ApplyHitsForGuid( guid, "rpc", hits, nowMs );
			}
		}
	}

	// -----------------------------------------------------------------------
	//  Infinite ammo
	// -----------------------------------------------------------------------

	private void DrainShotQueue()
	{
		array< string > guids = new array< string >;
		array< int > ammoCounts = new array< int >;

		JMAntiCheatSignals.DrainShots( guids, ammoCounts );

		int nowMs = g_Game.GetTime();

		for ( int i = 0; i < guids.Count(); i++ )
		{
			string guid = guids.Get( i );
			int ammo = ammoCounts.Get( i );

			if ( guid == "" )
				continue;

			if ( JMAntiCheatSanction.IsActive( guid, JMAntiCheatSanction.AMMO ) )
				continue;

			int previous = -1;
			if ( m_LastAmmoCount.Contains( guid ) )
				previous = m_LastAmmoCount.Get( guid );

			m_LastAmmoCount.Set( guid, ammo );

			//! First shot seen for this player, or a reload / weapon swap put
			//! more rounds in than there were. Either way the run restarts.
			if ( previous < 0 || ammo > previous )
			{
				m_ShotsNoDecrement.Set( guid, 0 );
				continue;
			}

			if ( ammo < previous )
			{
				m_ShotsNoDecrement.Set( guid, 0 );
				continue;
			}

			int run = 0;
			if ( m_ShotsNoDecrement.Contains( guid ) )
				run = m_ShotsNoDecrement.Get( guid );

			run++;
			m_ShotsNoDecrement.Set( guid, run );

			array< ref JMAntiCheatHit > hits = new array< ref JMAntiCheatHit >;
			JMAntiCheatDetector.DetectInfiniteAmmo( run, m_Config.AmmoShotsWithoutDecrement, m_Config.AmmoFlagWeight, hits );

			if ( hits.Count() > 0 )
			{
				m_ShotsNoDecrement.Set( guid, 0 );
				ApplyHitsForGuid( guid, "ammo", hits, nowMs );
			}
		}
	}

	private void DrainKillEventQueue()
	{
		if ( !JMAntiCheatKillHook.Pending )
			return;

		// Iterate a copy so the producer can keep adding during a single
		// tick.  Bounded by the producer's 200-entry cap.
		int count = JMAntiCheatKillHook.Pending.Count();
		for ( int i = 0; i < count; i++ )
		{
			JMAntiCheatKillEvent evt = JMAntiCheatKillHook.Pending.Get( i );
			if ( !evt )
				continue;
			if ( !evt.Killer || !evt.Victim )
				continue;

			//! The victim rule runs on every hit, kill or not: immunity shows
			//! up in the hits that did NOT kill anybody.
			EvaluateVictimImmunity( evt, g_Game.GetTime() );

			if ( evt.IsKill )
				RecordKillFromEvent( evt );
			else
				RecordHitFromEvent( evt );
		}

		// Clear the drained events
		for ( int j = count - 1; j >= 0; j-- )
			JMAntiCheatKillHook.Pending.Remove( j );
	}

	// Same logic as RecordKill but reads the body-part from the event
	// instead of looking up the most recent hit (this version is correct
	// when the hook fires both events in the same tick).
	private void RecordKillFromEvent( JMAntiCheatKillEvent evt )
	{
		if ( !IsMissionHost() || !evt.Killer )
			return;

		PlayerIdentity id = evt.Killer.GetIdentity();
		if ( !id )
			return;
		string guid = id.GetId();
		if ( guid == "" )
			return;

		JMAntiCheatKillStats stats = m_KillStats.Get( guid );
		if ( !stats )
		{
			stats = new JMAntiCheatKillStats();
			stats.PlayerName = id.GetName();
			stats.Guid       = guid;
			m_KillStats.Set( guid, stats );
		}

		string victimName = "?";
		if ( evt.Victim && evt.Victim.GetIdentity() )
			victimName = evt.Victim.GetIdentity().GetName();

		int nowMs = g_Game.GetTime();
		stats.RecordHit( victimName, evt.BodyPart, evt.Distance, nowMs );

		//! Per-event rules first, then the ones that read the running record -
		//! the record has to include this kill for the ratio to be current.
		EvaluateAttacker( guid, evt, nowMs );
		EvaluateKillPatterns( guid, stats, nowMs );
	}

	//! Non-fatal hit taken straight from the event, so the aim direction and
	//! delivery class captured at impact are still available. RecordHit below
	//! is kept for callers that only have the two players.
	private void RecordHitFromEvent( JMAntiCheatKillEvent evt )
	{
		if ( !IsMissionHost() || !evt.Killer )
			return;

		PlayerIdentity id = evt.Killer.GetIdentity();
		if ( !id )
			return;

		string guid = id.GetId();
		if ( guid == "" )
			return;

		EvaluateAttacker( guid, evt, g_Game.GetTime() );
	}

	// -----------------------------------------------------------------------
	//  Detection loop
	// -----------------------------------------------------------------------

	private void PollAllPlayers()
	{
		array< Man > players = new array< Man >;
		g_Game.GetPlayers( players );

		int nowMs = g_Game.GetTime();

		foreach ( Man m : players )
		{
			PlayerBase pb = PlayerBase.Cast( m );
			if ( !pb || !pb.IsAlive() )
				continue;

			string guid = pb.GetIdentity().GetId();
			if ( guid == "" )
				continue;

			PollMovement( guid, pb, nowMs );
		}

		if ( m_InventoryAccumulator >= m_Config.InventorySampleIntervalS )
		{
			float inventoryWindow = m_InventoryAccumulator;
			m_InventoryAccumulator = 0;
			PollInventories( players, inventoryWindow, nowMs );
		}
	}

	// -----------------------------------------------------------------------
	//  Movement
	//
	//  Speed and teleport are two rules over the same two samples, and both
	//  have to know about vehicles. A player in a car is legitimately doing
	//  30 m/s; without the exemption every vehicle on the server trips the
	//  speed rule on every poll and the flag list stops meaning anything.
	// -----------------------------------------------------------------------
	private void PollMovement( string guid, PlayerBase pb, int nowMs )
	{
		vector pos = pb.GetPosition();

		bool hadPrevious = m_LastPosition.Contains( guid );

		vector prevPos = pos;
		int prevMs = nowMs;

		if ( hadPrevious )
		{
			prevPos = m_LastPosition.Get( guid );
			prevMs = m_LastPositionMs.Get( guid );
		}

		m_LastPosition.Set( guid, pos );
		m_LastPositionMs.Set( guid, nowMs );
		m_LastAlivePosition.Set( guid, pos );

		if ( !hadPrevious )
		{
			//! First sample for this player. Nothing to compare against, and
			//! the next one lands inside the grace below anyway.
			JMAntiCheatSanction.Grant( guid, JMAntiCheatSanction.MOVEMENT, m_Config.PositionGraceS );
			return;
		}

		//! Somebody moved this player rather than the player moving: an admin
		//! teleport, a respawn, a vehicle unstuck, another mod's warp. The code
		//! that did it said so, which is the whole difference between a
		//! scripted relocation and a teleport cheat - the cheat announces
		//! nothing, because the client is what moved.
		if ( JMAntiCheatSanction.IsActive( guid, JMAntiCheatSanction.MOVEMENT ) )
			return;

		array< ref JMAntiCheatHit > hits = new array< ref JMAntiCheatHit >;

		//! Teleport first: a jump big enough to be a teleport would also read
		//! as an enormous speed, and reporting both for one event says the same
		//! thing twice at double the weight.
		JMAntiCheatDetector.DetectTeleport( prevPos, pos, m_Config.TeleportDistance, m_Config.TeleportFlagWeight, hits );

		if ( hits.Count() > 0 )
		{
			ApplyHitsForGuid( guid, "teleport", hits, nowMs );
			return;
		}

		float speedCeiling = m_Config.MaxSpeed;
		if ( IsInVehicle( pb ) )
			speedCeiling = m_Config.MaxVehicleSpeed;

		JMAntiCheatDetector.DetectSpeed( prevPos, prevMs, pos, nowMs, speedCeiling, m_Config.SpeedFlagWeight, hits );

		if ( hits.Count() > 0 )
			ApplyHitsForGuid( guid, "speed", hits, nowMs );
	}

	//! Kept for callers that already use it. Prefer JMAntiCheatSanction.Grant
	//! directly - it works from 4_World and does not need this module to exist.
	void COT_GrantPositionGrace( string guid )
	{
		if ( !m_Config )
			return;

		JMAntiCheatSanction.Grant( guid, JMAntiCheatSanction.MOVEMENT, m_Config.PositionGraceS );
	}

	private bool IsInVehicle( PlayerBase pb )
	{
		if ( !pb )
			return false;

		//! Covers cars, boats and anything else deriving from Transport. A
		//! player in a seat has the vehicle as their hierarchy parent.
		if ( pb.GetParent() && Transport.Cast( pb.GetParent() ) )
			return true;

		HumanCommandVehicle cmdVehicle = pb.GetCommand_Vehicle();
		if ( cmdVehicle )
			return true;

		return false;
	}

	// -----------------------------------------------------------------------
	//  Inventory
	//
	//  Sampled on its own slower cadence: walking the inventory of every
	//  player once a second is real server cost for a signal that does not
	//  move that fast, and the rule is a rate over a window anyway.
	// -----------------------------------------------------------------------
	private void PollInventories( array< Man > players, float windowS, int nowMs )
	{
		foreach ( Man m : players )
		{
			PlayerBase pb = PlayerBase.Cast( m );
			if ( !pb || !pb.IsAlive() || !pb.GetIdentity() )
				continue;

			string guid = pb.GetIdentity().GetId();
			if ( guid == "" )
				continue;

			//! An admin handing out a loadout announced itself before the first
			//! item landed. Anything that did not announce itself is judged.
			if ( JMAntiCheatSanction.IsActive( guid, JMAntiCheatSanction.INVENTORY ) )
			{
				//! Drop the baseline too, or the count taken before the spawn
				//! is compared against the count after it once the window
				//! closes, and the whole loadout is charged in one sample.
				m_LastItemCount.Remove( guid );
				continue;
			}

			int count = CountInventory( pb );

			bool hadPrevious = m_LastItemCount.Contains( guid );
			int previous = count;

			if ( hadPrevious )
				previous = m_LastItemCount.Get( guid );

			m_LastItemCount.Set( guid, count );

			if ( !hadPrevious )
				continue;

			int added = count - previous;
			if ( added <= 0 )
				continue;

			array< ref JMAntiCheatHit > hits = new array< ref JMAntiCheatHit >;
			JMAntiCheatDetector.DetectItemAddRate( added, windowS, m_Config.MaxItemsAddedPerSecond, m_Config.InventoryFlagWeight, hits );

			if ( hits.Count() > 0 )
				ApplyHitsForGuid( guid, "inventory", hits, nowMs );
		}
	}

	private int CountInventory( PlayerBase pb )
	{
		if ( !pb || !pb.GetInventory() )
			return 0;

		array< EntityAI > items = new array< EntityAI >;
		pb.GetInventory().EnumerateInventory( InventoryTraversalType.PREORDER, items );

		return items.Count();
	}

	// -----------------------------------------------------------------------
	//  Shared helpers
	// -----------------------------------------------------------------------

	//! Does this player hold COT access at all.
	//!
	//! This used to be an exemption: anyone with COT was skipped by every rule,
	//! which meant a stolen admin account was invisible to the whole module.
	//! It is not an exemption any more. Admins are scanned like everybody else,
	//! and the things an admin legitimately does that look like cheats are
	//! excused per ACTION instead - see JMAntiCheatSanction. The one place this
	//! is still consulted is the denied-RPC burst, where holding COT genuinely
	//! changes what a denial means.
	private bool HasCOTAccess( string guid )
	{
		if ( guid == "" )
			return false;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );
		if ( !instance )
			return false;

		return instance.HasPermission( "COT" );
	}

	//! Score a set of hits against a player found by guid, honouring the
	//! per-detector cooldown. The detector argument is the cooldown bucket,
	//! not the text shown to an admin.
	private void ApplyHitsForGuid( string guid, string detector, array< ref JMAntiCheatHit > hits, int nowMs )
	{
		if ( hits.Count() == 0 )
			return;

		if ( IsOnCooldown( guid, detector, nowMs ) )
			return;

		m_LastFlagMs.Set( guid + "|" + detector, nowMs );

		PlayerBase pb = null;
		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );
		if ( instance )
			pb = PlayerBase.Cast( instance.PlayerObject );

		ApplyHits( guid, pb, hits, nowMs );
	}

	private bool IsOnCooldown( string guid, string detector, int nowMs )
	{
		string key = guid + "|" + detector;

		if ( !m_LastFlagMs.Contains( key ) )
			return false;

		int elapsed = nowMs - m_LastFlagMs.Get( key );

		return elapsed < (int)( m_Config.DetectorCooldownS * 1000 );
	}

	private void ApplyHits( string guid, PlayerBase player, array< ref JMAntiCheatHit > hits, int nowMs )
	{
		if ( hits.Count() == 0 )
			return;

		//! The player entity is not required. A denied RPC can arrive from a
		//! client whose character has not spawned yet, and a flag on a name we
		//! have to look up is still a flag worth keeping.
		PlayerIdentity identity = null;
		if ( player )
			identity = player.GetIdentity();

		JMAntiCheatFlag flag = m_Flags.Get( guid );
		if ( !flag )
		{
			flag = new JMAntiCheatFlag();
			flag.PlayerName = ResolvePlayerName( guid, identity );
			flag.Guid = guid;
			m_Flags.Set( guid, flag );
		}

		int addedWeight = 0;
		string summary = "";
		foreach ( JMAntiCheatHit h : hits )
		{
			addedWeight += h.Weight;
			if ( summary != "" )
				summary = summary + "; ";
			summary = summary + h.Text;
		}

		flag.AddEvent( addedWeight, summary, nowMs );
		m_FlagsDirty = true;

		GetCommunityOnlineToolsBase().Log( identity, "AntiCheat flags +" + addedWeight.ToString() + " for " + flag.PlayerName + ": " + summary );

		// Threshold breach?
		int total = flag.TotalScore;
		string title = "Detection";
		int colour = JMConstants.WEBHOOK_COLOR_WARNING;

		if ( total >= m_Config.FlagThresholdCritical )
		{
			title = "Detection Critical";
			colour = JMConstants.WEBHOOK_COLOR_DANGER;
		}
		else if ( total >= m_Config.FlagThresholdHigh )
		{
			title = "Detection";
			colour = JMConstants.WEBHOOK_COLOR_WARNING;
		}

		SendWebhookColored( title, NULL, flag.PlayerName + " (score=" + total.ToString() + "): " + summary, colour );

		// Auto-action
		if ( m_Config.AutoKickOnCritical && total >= m_Config.FlagThresholdCritical )
		{
			int elapsedSec = ( nowMs - flag.LastActionTimeMs ) / 1000;
			if ( identity && ( flag.LastActionTimeMs == 0 || elapsedSec >= m_Config.AutoActionCooldownS ) )
			{
				GetGame().DisconnectPlayer( identity, "Auto anti-cheat action (score=" + total.ToString() + ")" );
				flag.LastActionTimeMs = nowMs;

				GetCommunityOnlineToolsBase().Log( identity, "AntiCheat auto-disconnected " + flag.PlayerName );
				SendWebhookColored( "Action", NULL, "Auto-disconnected " + flag.PlayerName + " (score=" + total.ToString() + ")", JMConstants.WEBHOOK_COLOR_DANGER );
			}
		}
	}

	// -----------------------------------------------------------------------
	//  Decay and persistence
	//
	//  Both exist for the same reason: a score is only useful if it describes
	//  the present. A number that only ever rises ends up measuring how long
	//  somebody has played here, and a number that resets on every restart
	//  lets anyone launder a flag by waiting for the nightly reboot.
	//
	//  Real UTC seconds throughout, never g_Game.GetTime() - that clock starts
	//  again at zero with the mission, which is exactly the gap being closed.
	// -----------------------------------------------------------------------

	//! Bleed every score down, and forget entries that have been at zero for
	//! long enough that keeping them is just file growth.
	private void DecayFlags()
	{
		if ( !m_Config || m_Config.FlagDecayPerHour <= 0 )
			return;

		int nowUnix = JMAntiCheatClock.UtcNow();

		//! A host with no usable clock gets no decay at all. Guessing here
		//! would either freeze every score or wipe them, and both are worse
		//! than leaving them where they are.
		if ( nowUnix <= 0 )
			return;

		int forgetAfter = m_Config.FlagForgetAfterDays * 86400;

		array< string > doomed = new array< string >;

		foreach ( string guid, JMAntiCheatFlag flag : m_Flags )
		{
			if ( flag.ApplyDecay( nowUnix, m_Config.FlagDecayPerHour ) )
				m_FlagsDirty = true;

			if ( flag.TotalScore > 0 )
				continue;

			if ( forgetAfter <= 0 )
				continue;

			if ( flag.AgeSeconds( nowUnix ) < forgetAfter )
				continue;

			doomed.Insert( guid );
		}

		//! Collected first: removing from a map mid-iteration is how you get a
		//! crash that only shows up on a busy server.
		foreach ( string dead : doomed )
		{
			m_Flags.Remove( dead );
			m_FlagsDirty = true;
		}
	}

	private void LoadFlags()
	{
		if ( !IsMissionHost() || !m_Config || !m_Config.PersistFlags )
			return;

		if ( !FileExist( JMAntiCheatFlagStore.FILE ) )
			return;

		JMAntiCheatFlagStore store = new JMAntiCheatFlagStore();
		JsonFileLoader<JMAntiCheatFlagStore>.JsonLoadFile( JMAntiCheatFlagStore.FILE, store );

		if ( !store.Entries )
			return;

		//! Refuse a file this build does not understand rather than loading it
		//! field-by-field into a score that gets people auto-kicked.
		if ( store.Version != JMAntiCheatFlagStore.CURRENT_VERSION )
		{
			Print( "[COT AntiCheat] flags.json is version " + store.Version.ToString() + ", expected " + JMAntiCheatFlagStore.CURRENT_VERSION.ToString() + " - ignored" );
			return;
		}

		int nowUnix = JMAntiCheatClock.UtcNow();
		int restored = 0;

		foreach ( JMAntiCheatFlag flag : store.Entries )
		{
			if ( !flag || flag.Guid == "" )
				continue;

			//! A hand-edited file can be missing the array entirely.
			if ( !flag.RecentEvents )
				flag.RecentEvents = new array< string >;

			//! Session state, not history: an auto-kick cooldown from before
			//! the restart is not holding anything back now.
			flag.LastActionTimeMs = 0;

			//! Charge for the downtime. A server that was off for a day should
			//! come back with a day's worth of decay already applied, or the
			//! decay rate silently means "per hour of uptime".
			flag.ApplyDecay( nowUnix, m_Config.FlagDecayPerHour );

			m_Flags.Set( flag.Guid, flag );
			restored++;
		}

		Print( "[COT AntiCheat] restored " + restored.ToString() + " flag(s) from disk" );
	}

	//! Write the table out. A no-op unless something actually changed, so the
	//! periodic call costs nothing on a quiet server. Pass force on shutdown
	//! and after an admin action, where the next periodic write may not come.
	private void SaveFlags( bool force )
	{
		if ( !IsMissionHost() || !m_Config || !m_Config.PersistFlags )
			return;

		if ( !force && !m_FlagsDirty )
			return;

		MakeDirectory( JMAntiCheatFlagStore.DIR );

		JMAntiCheatFlagStore store = new JMAntiCheatFlagStore();

		foreach ( string guid, JMAntiCheatFlag flag : m_Flags )
			store.Entries.Insert( flag );

		JsonFileLoader<JMAntiCheatFlagStore>.JsonSaveFile( JMAntiCheatFlagStore.FILE, store );

		m_FlagsDirty = false;
	}

	// -----------------------------------------------------------------------
	//  Admin client RPC
	// -----------------------------------------------------------------------

	void RequestFlags()
	{
		if ( IsMissionHost() )
		{
			// Send local mirror to local form (server-side admin)
			// form reads m_Flags directly via GetServerFlags
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send( NULL, JMAntiCheatModuleRPC.RequestFlags, true, NULL );
	}

	private void RPC_RequestFlags( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionHost() )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.AntiCheat.View", senderRPC, instance ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( m_Flags.Count() );
		foreach ( string guid, JMAntiCheatFlag flag : m_Flags )
		{
			rpc.Write( guid );
			rpc.Write( flag.PlayerName );
			rpc.Write( flag.TotalScore );
			rpc.Write( flag.RecentEvents.Count() );
			foreach ( string evt : flag.RecentEvents )
				rpc.Write( evt );
		}

		// Kill-stats payload: count + per-player entry
		rpc.Write( m_KillStats.Count() );
		foreach ( string kStatGuid, JMAntiCheatKillStats stats : m_KillStats )
		{
			rpc.Write( kStatGuid );
			rpc.Write( stats.PlayerName );
			rpc.Write( stats.TotalKills );
			rpc.Write( stats.TotalMisses );
			rpc.Write( stats.BodyHead );
			rpc.Write( stats.BodyTorso );
			rpc.Write( stats.BodyLeftArm );
			rpc.Write( stats.BodyRightArm );
			rpc.Write( stats.BodyLeftLeg );
			rpc.Write( stats.BodyRightLeg );
			rpc.Write( stats.BodyFeet );
			rpc.Write( stats.DistanceSum );
			rpc.Write( stats.DistanceCount );
			rpc.Write( stats.DistanceMax );
			rpc.Write( stats.RecentKills.Count() );
			foreach ( string kKillEvt : stats.RecentKills )
				rpc.Write( kKillEvt );
		}

		rpc.Send( NULL, JMAntiCheatModuleRPC.Flags, true, senderRPC );
	}

	private void RPC_Flags( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
			return;

		int count;
		if ( !ctx.Read( count ) )
			return;

		m_ClientFlags.Clear();
		m_ClientGuidOrder.Clear();

		for ( int fIdx = 0; fIdx < count; fIdx++ )
		{
			string fGuid;
			string fName;
			int fScore;
			int fEvtCount;

			if ( !ctx.Read( fGuid ) )    break;
			if ( !ctx.Read( fName ) )    break;
			if ( !ctx.Read( fScore ) )   break;
			if ( !ctx.Read( fEvtCount ) ) break;

			JMAntiCheatFlag flag = new JMAntiCheatFlag();
			flag.PlayerName = fName;
			flag.Guid = fGuid;
			flag.TotalScore = fScore;

			for ( int fEvt = 0; fEvt < fEvtCount; fEvt++ )
			{
				string fEvent;
				if ( !ctx.Read( fEvent ) )
					break;
				flag.RecentEvents.Insert( fEvent );
			}

			m_ClientFlags.Set( fGuid, flag );
			m_ClientGuidOrder.Insert( fGuid );
		}

		// Kill-stats payload
		int kCount;
		if ( !ctx.Read( kCount ) )
		{
			JMAntiCheatForm earlyForm;
			if ( Class.CastTo( earlyForm, GetForm() ) )
				earlyForm.OnFlagsUpdated( m_ClientGuidOrder, m_ClientFlags );
			return;
		}

		m_ClientKillStats.Clear();
		m_ClientKillGuidOrder.Clear();

		for ( int kIdx = 0; kIdx < kCount; kIdx++ )
		{
			string kGuid, kName;
			int kKills, kMisses, kHead, kTorso, kLA, kRA, kLL, kRL, kFeet;
			float dSum;
			int dCount;
			float dMax;
			int kEvtCount;

			if ( !ctx.Read( kGuid ) )     break;
			if ( !ctx.Read( kName ) )     break;
			if ( !ctx.Read( kKills ) )    break;
			if ( !ctx.Read( kMisses ) )   break;
			if ( !ctx.Read( kHead ) )     break;
			if ( !ctx.Read( kTorso ) )    break;
			if ( !ctx.Read( kLA ) )       break;
			if ( !ctx.Read( kRA ) )       break;
			if ( !ctx.Read( kLL ) )       break;
			if ( !ctx.Read( kRL ) )       break;
			if ( !ctx.Read( kFeet ) )     break;
			if ( !ctx.Read( dSum ) )      break;
			if ( !ctx.Read( dCount ) )    break;
			if ( !ctx.Read( dMax ) )      break;
			if ( !ctx.Read( kEvtCount ) ) break;

			JMAntiCheatKillStats stats = new JMAntiCheatKillStats();
			stats.Guid          = kGuid;
			stats.PlayerName    = kName;
			stats.TotalKills    = kKills;
			stats.TotalMisses   = kMisses;
			stats.BodyHead      = kHead;
			stats.BodyTorso     = kTorso;
			stats.BodyLeftArm   = kLA;
			stats.BodyRightArm  = kRA;
			stats.BodyLeftLeg   = kLL;
			stats.BodyRightLeg  = kRL;
			stats.BodyFeet      = kFeet;
			stats.DistanceSum   = dSum;
			stats.DistanceCount = dCount;
			stats.DistanceMax   = dMax;

			for ( int kEvt = 0; kEvt < kEvtCount; kEvt++ )
			{
				string kEvent;
				if ( !ctx.Read( kEvent ) )
					break;
				stats.RecentKills.Insert( kEvent );
			}

			m_ClientKillStats.Set( kGuid, stats );
			m_ClientKillGuidOrder.Insert( kGuid );
		}

		JMAntiCheatForm form;
		if ( Class.CastTo( form, GetForm() ) )
		{
			form.OnFlagsUpdated( m_ClientGuidOrder, m_ClientFlags );
			form.OnKillStatsUpdated( m_ClientKillGuidOrder, m_ClientKillStats );
		}
	}

	// -----------------------------------------------------------------------
	//  Admin actions (server-side, called from form via RPC if needed)
	// -----------------------------------------------------------------------

	void ClearFlag( string guid )
	{
		if ( !IsMissionHost() )
			return;

		if ( m_Flags.Contains( guid ) )
		{
			JMAntiCheatFlag flag = m_Flags.Get( guid );
			Print( "[COT AntiCheat] cleared flags for " + flag.PlayerName );
			SendWebhookColored( "Action", NULL, "Cleared flags for " + flag.PlayerName, JMConstants.WEBHOOK_COLOR_NEUTRAL );
			m_Flags.Remove( guid );

			//! Written through immediately. An admin who clears a flag and then
			//! restarts the server should not find it back.
			m_FlagsDirty = true;
			SaveFlags( true );
		}
	}

	// -----------------------------------------------------------------------
	//  Client accessors for the form
	// -----------------------------------------------------------------------

	array< string > GetClientGuidOrder()
	{
		return m_ClientGuidOrder;
	}

	map< string, ref JMAntiCheatFlag > GetClientFlags()
	{
		return m_ClientFlags;
	}

	//! Best name available for a guid. The live identity when there is one, the
	//! permission roster otherwise, and the guid itself as a last resort - a
	//! flag with no name attached is nearly useless to an admin.
	private string ResolvePlayerName( string guid, PlayerIdentity identity )
	{
		if ( identity )
			return identity.GetName();

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );
		if ( instance )
			return instance.GetName();

		return guid;
	}

	// -----------------------------------------------------------------------
	//  Rules over combat events
	//
	//  Run from the kill/hit drain rather than the poll: these are judgements
	//  about single events, and the event carries context - aim direction,
	//  delivery class, victim health - that is gone a frame later.
	// -----------------------------------------------------------------------

	//! Rules that judge the ATTACKER, on every hit and kill.
	private void EvaluateAttacker( string guid, JMAntiCheatKillEvent evt, int nowMs )
	{
		if ( guid == "" )
			return;

		if ( JMAntiCheatSanction.IsActive( guid, JMAntiCheatSanction.COMBAT ) )
			return;

		array< ref JMAntiCheatHit > hits = new array< ref JMAntiCheatHit >;

		JMAntiCheatDetector.DetectSilentAim( evt.AttackerPos, evt.AttackerDir, evt.VictimPos, evt.IsRanged, m_Config.SilentAimMinDistance, m_Config.SilentAimConeDeg, m_Config.SilentAimFlagWeight, hits );

		if ( hits.Count() > 0 )
		{
			ApplyHitsForGuid( guid, "silentaim", hits, nowMs );
			hits.Clear();
		}

		JMAntiCheatDetector.DetectKillDistance( evt.Distance, evt.IsMelee, evt.IsExplosive, m_Config.MaxMeleeDistance, m_Config.MaxKillDistance, m_Config.DistanceFlagWeight, hits );

		if ( hits.Count() > 0 )
			ApplyHitsForGuid( guid, "distance", hits, nowMs );
	}

	//! Rules that need the accumulated kill record, so they run on kills only.
	private void EvaluateKillPatterns( string guid, JMAntiCheatKillStats stats, int nowMs )
	{
		if ( !stats || guid == "" )
			return;

		if ( JMAntiCheatSanction.IsActive( guid, JMAntiCheatSanction.COMBAT ) )
			return;

		array< ref JMAntiCheatHit > hits = new array< ref JMAntiCheatHit >;

		JMAntiCheatDetector.DetectHeadshotRatio( stats.BodyHead, stats.TotalKills, m_Config.HeadshotMinKills, m_Config.HeadshotRatioMax, m_Config.HeadshotFlagWeight, hits );

		if ( hits.Count() > 0 )
		{
			ApplyHitsForGuid( guid, "headshot", hits, nowMs );
			hits.Clear();
		}

		int killsInWindow = RecordAndCountRecentKills( guid, nowMs );

		JMAntiCheatDetector.DetectRapidKills( killsInWindow, m_Config.RapidKillCount, m_Config.RapidKillWindowS, m_Config.RapidKillFlagWeight, hits );

		if ( hits.Count() > 0 )
			ApplyHitsForGuid( guid, "rapidkill", hits, nowMs );
	}

	//! Append this kill and return how many fall inside the window. The list is
	//! pruned from the front every call, so it stays the size of the window
	//! rather than the size of the session.
	private int RecordAndCountRecentKills( string guid, int nowMs )
	{
		array< int > stamps = m_RecentKillMs.Get( guid );
		if ( !stamps )
		{
			stamps = new array< int >;
			m_RecentKillMs.Set( guid, stamps );
		}

		stamps.Insert( nowMs );

		int windowMs = (int)( m_Config.RapidKillWindowS * 1000 );

		while ( stamps.Count() > 0 && ( nowMs - stamps.Get( 0 ) ) > windowMs )
			stamps.Remove( 0 );

		return stamps.Count();
	}

	//! The damage-immunity rule judges the VICTIM: the cheat that matters here
	//! is one that keeps its own health pinned while real damage lands on it.
	private void EvaluateVictimImmunity( JMAntiCheatKillEvent evt, int nowMs )
	{
		if ( !evt.Victim || !evt.Victim.GetIdentity() )
			return;

		//! COT godmode is an admin decision, not a cheat, and the whole point
		//! of it is that damage stops mattering.
		if ( evt.VictimGodMode )
			return;

		string guid = evt.Victim.GetIdentity().GetId();
		if ( guid == "" )
			return;

		if ( JMAntiCheatSanction.IsActive( guid, JMAntiCheatSanction.DAMAGE ) )
			return;

		//! Only hits that actually computed damage say anything. A glancing
		//! hit for zero is not evidence of immunity.
		if ( evt.DamageDealt <= 0 )
			return;

		bool hadPrevious = m_LastHitHealth.Contains( guid );
		float previous = evt.VictimHealth;

		if ( hadPrevious )
			previous = m_LastHitHealth.Get( guid );

		m_LastHitHealth.Set( guid, evt.VictimHealth );

		if ( !hadPrevious )
			return;

		if ( evt.VictimHealth < previous )
		{
			m_NoDropHits.Set( guid, 0 );
			return;
		}

		int run = 0;
		if ( m_NoDropHits.Contains( guid ) )
			run = m_NoDropHits.Get( guid );

		run++;
		m_NoDropHits.Set( guid, run );

		array< ref JMAntiCheatHit > hits = new array< ref JMAntiCheatHit >;
		JMAntiCheatDetector.DetectDamageImmunity( run, m_Config.GodModeHitsBeforeFlag, m_Config.GodModeFlagWeight, hits );

		if ( hits.Count() > 0 )
		{
			m_NoDropHits.Set( guid, 0 );
			ApplyHitsForGuid( guid, "godmode", hits, nowMs );
		}
	}

	// Server-side helper for testing
	map< string, ref JMAntiCheatFlag > GetServerFlags()
	{
		return m_Flags;
	}

	//! How many players are flagged hard enough to be worth an admin's time.
	//!
	//! A flag entry exists the moment a player trips anything at all, and on a
	//! busy server that is most of them - counting entries would light a footer
	//! badge permanently and teach everyone to ignore it. The bar used here is
	//! the module's own FlagThresholdHigh, the score at which it already
	//! considers the detection worth a WARNING webhook.
	//!
	//! Flags decay, so this clears itself if nothing else happens - but only
	//! slowly, and a certain detection outlives an admin's whole shift. The
	//! intended reading is unchanged: while it is raised, somebody has to look.
	int GetFlaggedCount()
	{
		array< string > guids = new array< string >;
		GetFlaggedGuids( guids );
		return guids.Count();
	}

	//! The GUIDs behind GetFlaggedCount, for consumers that need to ask about
	//! one player - the player list draws a badge per row from this.
	void GetFlaggedGuids( out array< string > guids )
	{
		guids.Clear();

		if ( !IsMissionHost() )
			return;

		int threshold = 15;
		if ( m_Config )
			threshold = m_Config.FlagThresholdHigh;

		foreach ( string guid, JMAntiCheatFlag flag : m_Flags )
		{
			if ( flag.TotalScore >= threshold )
				guids.Insert( guid );
		}
	}

	// -----------------------------------------------------------------------
	//  Kill-stats integration
	//
	//  Called from the static forwarder in 4_World/Entities/Player/
	//  JMAntiCheatKillHook.c which is wired to PlayerBase.EEKilled / EEHitBy.
	//  Both run on the server; on a dedicated server this is the only path
	//  that fires, on a listen server the server side is what we use.
	// -----------------------------------------------------------------------

	void RecordHit( PlayerBase attacker, PlayerBase victim, string dmgZone )
	{
		if ( !IsMissionHost() || !attacker || !victim )
			return;

		PlayerIdentity id = attacker.GetIdentity();
		if ( !id )
			return;
		string guid = id.GetId();
		if ( guid == "" )
			return;

		JMAntiCheatKillStats stats = m_KillStats.Get( guid );
		if ( !stats )
		{
			stats = new JMAntiCheatKillStats();
			stats.PlayerName = id.GetName();
			stats.Guid       = guid;
			m_KillStats.Set( guid, stats );
		}

		// Distance from the killer's last-alive position to the victim's
		// last-alive position at the moment of impact.  Falls back to 0 if
		// either is unknown (early frames).
		vector killerPos = attacker.GetPosition();
		vector victimPos = victim.GetPosition();
		float distance = vector.Distance( killerPos, victimPos );

		string victimName = "?";
		if ( victim.GetIdentity() )
			victimName = victim.GetIdentity().GetName();
		stats.RecordHit( victimName, dmgZone, distance, g_Game.GetTime() );
	}

	void RecordKill( PlayerBase killer, PlayerBase victim )
	{
		if ( !IsMissionHost() || !killer || !victim )
			return;

		PlayerIdentity id = killer.GetIdentity();
		if ( !id )
			return;
		string guid = id.GetId();
		if ( guid == "" )
			return;

		JMAntiCheatKillStats stats = m_KillStats.Get( guid );
		if ( !stats )
		{
			stats = new JMAntiCheatKillStats();
			stats.PlayerName = id.GetName();
			stats.Guid       = guid;
			m_KillStats.Set( guid, stats );
		}

		// Use the killer's last-alive position to compute distance (the
		// victim is dead by the time RecordKill fires, so its last position
		// is also the death point).
		vector killerPos = killer.GetPosition();
		vector victimPos = victim.GetPosition();
		float distance = vector.Distance( killerPos, victimPos );

		// The body-part that dealt the killing blow isn't surfaced by the
		// EEKilled event in vanilla; the EEHitBy hook recorded the most-recent
		// hit, so we pull it from the most recent RecordHit.  As a
		// approximation we look at the last entry's body-part.
		string lastBodyPart = "Torso";
		if ( stats.RecentKills.Count() > 0 )
		{
			string last = stats.RecentKills.Get( stats.RecentKills.Count() - 1 );
			int idx = last.IndexOf( "[" );
			if ( idx > 0 )
			{
				// Parse out the bracketed body-part at the end
				int end = last.LastIndexOf( "]" );
				if ( end > idx )
					lastBodyPart = last.Substring( idx + 1, end - idx - 1 );
			}
		}

		string victimName = "?";
		if ( victim.GetIdentity() )
			victimName = victim.GetIdentity().GetName();
		stats.RecordHit( victimName, lastBodyPart, distance, g_Game.GetTime() );
	}

	// -----------------------------------------------------------------------
	//  Client accessors for the form
	// -----------------------------------------------------------------------

	array< string > GetClientKillGuidOrder()
	{
		return m_ClientKillGuidOrder;
	}

	map< string, ref JMAntiCheatKillStats > GetClientKillStats()
	{
		return m_ClientKillStats;
	}

	// Server-side helper
	map< string, ref JMAntiCheatKillStats > GetServerKillStats()
	{
		return m_KillStats;
	}
}