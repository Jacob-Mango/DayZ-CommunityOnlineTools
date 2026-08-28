// =============================================================================
//  JMAntiCheatKillHook.c
//
//  Static forwarder for the kill-tracking hooks wired in PlayerBase.c.
//  Lives at 4_World so it's compiled in the same module that owns
//  PlayerBase.  The 4_World module compiles BEFORE 5_Mission so we
//  can't reference the 5_Mission `JMAntiCheatModule` class directly.
//
//  Pattern: store the events in a static 4_World-side cache.  The
//  anti-cheat module polls the cache in its `OnUpdate` tick (which
//  already runs in 5_Mission, where it can resolve its own type).
// =============================================================================

class JMAntiCheatKillEvent
{
	PlayerBase Killer;
	PlayerBase Victim;
	string      BodyPart;
	float       Distance;
	bool        IsKill;

	//! Ammo classname from EEHitBy, "" for melee and fists.
	string      HitAmmo;

	//! How the hit was delivered. Only one of these can be true, and all three
	//! can be false - a zombie or a fall is neither.
	//!
	//! The distinction matters because the distance rules differ per class:
	//! a melee hit at 40m is impossible, a rifle hit at 40m is a Tuesday, and
	//! a grenade "hit" at 40m is just blast radius and must not be judged at
	//! all.
	bool        IsRanged;
	bool        IsMelee;
	bool        IsExplosive;

	//! Where the attacker was and which way they were facing when the hit
	//! landed. Captured here because by the time a kill resolves - possibly
	//! minutes later through bleed-out - both are long gone.
	vector      AttackerPos;
	vector      AttackerDir;
	vector      VictimPos;

	//! Victim state at the moment of the hit, for the damage-immunity rule.
	float       VictimHealth;
	float       DamageDealt;
	//! COT's own godmode. An admin decision, not a cheat, and it has to be
	//! carried here because the module sees the event, not the player.
	bool        VictimGodMode;
}

class JMAntiCheatKillHook
{
	// FIFO queue of pending events, drained by the anti-cheat module's
	// OnUpdate.  Bounded so the queue can't grow unbounded if the
	// anti-cheat module is disabled.
	static ref array< ref JMAntiCheatKillEvent > Pending = new array< ref JMAntiCheatKillEvent >;

	//! A hit snapshot is only trusted as the source of a kill for this long.
	//! Covers bleed-out/shock deaths that resolve well after the final hit.
	static const int SNAPSHOT_WINDOW_MS = 120000;

	//! Resolve the player responsible for damage dealt by `source`.
	//!
	//! `source` is rarely the killer:
	//!   - shot      -> source is the *weapon*, owned by the shooter
	//!   - explosion -> source is the grenade/mine, whose owner we tagged in
	//!                  ItemBase.OnInventoryExit / OnPlacementComplete
	//!   - trap      -> source is the trap, tagged in TrapBase.StartActivate
	//!   - roadkill  -> source is the vehicle; blame the driver
	//!
	//! A bare PlayerBase.Cast(source) catches only melee/fists, which is why
	//! every explosive, trap and vehicle kill used to be dropped outright.
	static PlayerBase ResolveKiller( Object source )
	{
		if ( !source )
			return null;

		//! Melee / direct: the player themselves.
		PlayerBase direct = PlayerBase.Cast( source );
		if ( direct )
			return direct;

		EntityAI sourceEnt = EntityAI.Cast( source );
		if ( !sourceEnt )
			return null;

		//! Explosive or trap we tagged at throw / place / arm time. Checked
		//! before the hierarchy walk: a mine has no parent once placed.
		ItemBase sourceItem = ItemBase.Cast( sourceEnt );
		if ( sourceItem && sourceItem.COT_IsKillTrackable() )
		{
			PlayerBase tagged = sourceItem.COT_GetKillOwner();
			if ( tagged )
				return tagged;
		}

		//! Held weapon: walk up to the player holding it.
		Man rootPlayer = sourceEnt.GetHierarchyRootPlayer();
		PlayerBase rootPb = PlayerBase.Cast( rootPlayer );
		if ( rootPb )
			return rootPb;

		//! Direct parent, for sources one level down that aren't in a
		//! player's inventory hierarchy.
		EntityAI parent = sourceEnt.GetHierarchyParent();
		PlayerBase parentPb = PlayerBase.Cast( parent );
		if ( parentPb )
			return parentPb;

		//! Roadkill: blame whoever is driving.
		Transport transport = Transport.Cast( sourceEnt );
		if ( transport )
		{
			Human driver = transport.CrewMember( DayZPlayerConstants.VEHICLESEAT_DRIVER );
			PlayerBase driverPb = PlayerBase.Cast( driver );
			if ( driverPb )
				return driverPb;
		}

		return null;
	}

	static void OnPlayerKilled( PlayerBase victim, Object killer )
	{
		if ( !victim )
			return;

		PlayerBase killerPb = ResolveKiller( killer );

		//! Prefer the snapshot taken at the last hit. At death the explosive is
		//! already deleted and a bleed-out victim may have walked hundreds of
		//! metres, so distance computed here would be meaningless.
		JMAntiCheatKillEvent snapshot = victim.COT_GetLastHitSnapshot();
		if ( snapshot && ( g_Game.GetTime() - victim.COT_GetLastHitTime() ) <= SNAPSHOT_WINDOW_MS )
		{
			JMAntiCheatKillEvent fromHit = new JMAntiCheatKillEvent();
			fromHit.Killer   = snapshot.Killer;
			fromHit.Victim   = victim;
			fromHit.IsKill   = true;
			fromHit.BodyPart = snapshot.BodyPart;
			fromHit.Distance = snapshot.Distance;

			//! Only trust the snapshot's killer if we couldn't resolve one now.
			if ( killerPb )
				fromHit.Killer = killerPb;

			if ( fromHit.Killer )
				Enqueue( fromHit );

			return;
		}

		if ( !killerPb )
			return; // environmental death (fall, zombie, fire) -- skip

		JMAntiCheatKillEvent evt = new JMAntiCheatKillEvent();
		evt.Killer   = killerPb;
		evt.Victim   = victim;
		evt.IsKill   = true;
		evt.BodyPart = "Torso"; // default; module adjusts on first hit data
		evt.Distance = vector.Distance( killerPb.GetPosition(), victim.GetPosition() );
		Enqueue( evt );
	}

	//! Which of the three delivery classes this source belongs to. Checked in
	//! this order on purpose: a thrown grenade is an ItemBase held moments ago,
	//! so testing "is it a weapon / is it held" first would call it melee.
	static void ClassifySource( Object source, out bool isRanged, out bool isMelee, out bool isExplosive )
	{
		isRanged = false;
		isMelee = false;
		isExplosive = false;

		if ( !source )
			return;

		EntityAI sourceEnt = EntityAI.Cast( source );

		ItemBase sourceItem = ItemBase.Cast( sourceEnt );
		if ( sourceItem && sourceItem.COT_IsKillTrackable() )
		{
			isExplosive = true;
			return;
		}

		if ( Transport.Cast( sourceEnt ) )
		{
			isExplosive = true;
			return;
		}

		if ( Weapon_Base.Cast( sourceEnt ) )
		{
			isRanged = true;
			return;
		}

		//! Fists report the player as the source; a melee weapon reports the
		//! item in their hands. Neither is a firearm, and both have a reach of
		//! about two metres.
		if ( PlayerBase.Cast( source ) )
		{
			isMelee = true;
			return;
		}

		if ( sourceEnt && PlayerBase.Cast( sourceEnt.GetHierarchyRootPlayer() ) )
			isMelee = true;
	}

	static void OnPlayerHit( PlayerBase victim, EntityAI source, string dmgZone, TotalDamageResult dmg, string ammo = "" )
	{
		if ( !victim || !source )
			return;

		PlayerBase attacker = ResolveKiller( source );
		if ( !attacker )
			return;

		// Player-vs-player hit recorded as a hit, distance based on
		// current positions.  We keep these as separate events from
		// kills so the anti-cheat module can update body-part stats
		// continuously.
		JMAntiCheatKillEvent evt = new JMAntiCheatKillEvent();
		evt.Killer   = attacker;
		evt.Victim   = victim;
		evt.IsKill   = false;
		evt.BodyPart = dmgZone;
		evt.Distance = vector.Distance( attacker.GetPosition(), victim.GetPosition() );
		evt.HitAmmo = ammo;

		bool isRanged, isMelee, isExplosive;
		ClassifySource( source, isRanged, isMelee, isExplosive );
		evt.IsRanged    = isRanged;
		evt.IsMelee     = isMelee;
		evt.IsExplosive = isExplosive;

		evt.AttackerPos = attacker.GetPosition();
		evt.AttackerDir = attacker.GetDirection();
		evt.VictimPos   = victim.GetPosition();

		evt.VictimHealth  = victim.GetHealth( "", "Health" );
		evt.VictimGodMode = victim.COTHasGodMode();

		if ( dmg )
			evt.DamageDealt = dmg.GetHighestDamage( "Health" );

		Enqueue( evt );

		//! Snapshot for OnPlayerKilled: at death the weapon/explosive may be
		//! gone and the positions stale, but right now they are true.
		victim.COT_SetLastHitSnapshot( evt, g_Game.GetTime() );
	}

	private static void Enqueue( JMAntiCheatKillEvent evt )
	{
		if ( Pending.Count() > 200 )
			Pending.Remove( 0 ); // drop oldest
		Pending.Insert( evt );
	}
}