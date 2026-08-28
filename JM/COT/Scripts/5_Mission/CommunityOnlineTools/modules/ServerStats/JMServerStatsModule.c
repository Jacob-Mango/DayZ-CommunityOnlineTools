// =============================================================================
//  JMServerStatsModule.c
//
//  Measures server frame rate and pushes it to admin clients.
//
//  A dedicated server renders nothing, so its "fps" is its simulation tick
//  rate - the same number the RPT reports as "Average server FPS". A module
//  OnUpdate runs once per tick, so counting OnUpdate calls over a known window
//  measures exactly that, and the per-call timeslice gives the per-frame times
//  the one-percent figures are computed from.
//
//  Why one-percent lows matter more than the average: a server holding a 50 fps
//  average while 1 percent of its ticks take 200ms is the server whose players
//  report rubber-banding. The average hides that; the low does not.
//
//  RPCs (JMServerStatsModuleRPC @ 10940):
//    Stats   server -> client, broadcast on a timer to COT.View holders only
//
//  This module has no UI of its own. The sidebar footer renders what it
//  publishes, via the JMServerStats holder in 3_Game.
//
//  It also carries the anti-cheat flag count. That is not this module's data,
//  but the footer badge that shows it needs to be live before anyone opens the
//  AntiCheat form, and this is already a timer broadcasting to exactly the
//  admins who would care. A second module-wide timer and a second RPC to move
//  one int is the worse trade.
// =============================================================================

class JMServerStatsModule : JMModuleBase
{
	//! Seconds between broadcasts. Short enough that the average tracks a live
	//! server, long enough that the packet is noise next to normal traffic.
	private static const float BROADCAST_INTERVAL = 2.0;

	//! Frame times kept for the percentile window. At a 45 fps server this is
	//! about 23 seconds of history, which is the timescale a one-percent figure
	//! is meaningful over - a 2 second window would hold one frame in its tail.
	private static const int SAMPLE_WINDOW = 1024;

	//! Upper bound on how many frames the one-percent tails average over. Caps
	//! the per-broadcast selection cost, and covers windows up to 3200 frames.
	private static const int TAIL_MAX = 32;

	//! Ignore absurd deltas. The first tick after mission load, and the tick
	//! after the server has been paused by the debugger, are not stutters worth
	//! reporting - they would pin the one-percent low at ~0 for a whole window.
	private static const float FRAME_TIME_MAX = 1.0;

	//! Server-side ring buffer of frame times in seconds.
	private ref array< float > m_Samples;
	private int m_SampleNext;
	private bool m_SampleWrapped;

	private float m_BroadcastAccumulator;

	//! Frame cap read once from serverDZ.cfg, 0 when the server is uncapped.
	private int m_MaxFPS;

	#ifdef DIAG
	//! Publishes between diagnostic log lines - see Publish.
	private static const int DIAG_PRINT_EVERY = 10;
	private int m_DiagCounter;
	#endif

	void JMServerStatsModule()
	{
		m_Samples = new array< float >;
	}

	override int GetRPCMin()
	{
		return JMServerStatsModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMServerStatsModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMServerStatsModuleRPC.Stats:
			RPC_Stats( ctx, sender, target );
			break;
		}
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		JMServerStats.Clear();
		JMAntiCheatStatus.Clear();

		if ( !IsMissionHost() )
			return;

		ResetSamples();

		m_BroadcastAccumulator = 0;

		//! serverDZ.cfg is the only place a frame cap is visible to script - the
		//! -limitFPS startup parameter is not readable from here. Absent key
		//! returns 0, which is exactly the "uncapped" value the UI wants.
		m_MaxFPS = g_Game.ServerConfigGetInt( "limitFPS" );
	}

	private void ResetSamples()
	{
		m_Samples.Clear();
		m_SampleNext = 0;
		m_SampleWrapped = false;

		for ( int i = 0; i < SAMPLE_WINDOW; i++ )
		{
			m_Samples.Insert( 0 );
		}
	}

	// -------------------------------------------------------------------------
	//  Sampling
	// -------------------------------------------------------------------------
	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		if ( !IsMissionHost() )
			return;

		RecordFrame( timeslice );

		m_BroadcastAccumulator += timeslice;

		if ( m_BroadcastAccumulator < BROADCAST_INTERVAL )
			return;

		m_BroadcastAccumulator = 0;

		Publish();
	}

	private void RecordFrame( float timeslice )
	{
		if ( timeslice <= 0 || timeslice > FRAME_TIME_MAX )
			return;

		//! The ring is sized here rather than trusting OnMissionLoaded to have
		//! run first. Set() past the end of an Enforce array is not a script
		//! error, it is a native out-of-bounds write, so the one ordering
		//! assumption worth not making is this one.
		if ( m_Samples.Count() != SAMPLE_WINDOW )
			ResetSamples();

		m_Samples.Set( m_SampleNext, timeslice );

		m_SampleNext++;

		if ( m_SampleNext < SAMPLE_WINDOW )
			return;

		m_SampleNext = 0;
		m_SampleWrapped = true;
	}

	//! How many samples the ring currently holds.
	private int GetSampleCount()
	{
		if ( m_SampleWrapped )
			return SAMPLE_WINDOW;

		return m_SampleNext;
	}

	// -------------------------------------------------------------------------
	//  Percentiles
	//
	//  A full sort of the window every broadcast would be wasted work: only the
	//  two tails are wanted. Instead each tail is kept as a small sorted array
	//  and every sample is tested against that array worst entry first - one
	//  compare for the overwhelming majority of samples, an O(TAIL) insert for
	//  the few that make the cut.
	// -------------------------------------------------------------------------
	private void Publish()
	{
		int count = GetSampleCount();

		//! Nothing meaningful to say from a handful of frames.
		if ( count < 32 )
			return;

		//! How many frames one percent of the window actually is, clamped so a
		//! long window cannot blow past the tail arrays.
		int tail = (int)Math.Round( count / 100.0 );
		if ( tail < 1 )
			tail = 1;
		if ( tail > TAIL_MAX )
			tail = TAIL_MAX;

		float total = 0;

		//! Slowest frames (largest frame times) and fastest frames (smallest),
		//! each kept sorted worst-first / best-first.
		array< float > slowest = new array< float >;
		array< float > fastest = new array< float >;

		for ( int i = 0; i < count; i++ )
		{
			float sample = m_Samples.Get( i );
			total += sample;

			InsertDescending( slowest, sample, tail );
			InsertAscending( fastest, sample, tail );
		}

		float average = 0;
		if ( total > 0 )
			average = count / total;

		float low = FrameTimesToFPS( slowest );
		float high = FrameTimesToFPS( fastest );

		#ifdef DIAG
		//! Cross-check against the "Average server FPS" line the engine writes to
		//! the RPT - the two measure the same tick rate and should agree. Every
		//! tenth publish rather than every one: at a 2s broadcast this is a line
		//! every 20s, which is a diagnostic rather than a log flood.
		m_DiagCounter++;
		if ( m_DiagCounter >= DIAG_PRINT_EVERY )
		{
			m_DiagCounter = 0;
			Print( "[COT ServerStats] frames=" + count.ToString() + " tail=" + tail.ToString() + " avg=" + average.ToString() + " low1=" + low.ToString() + " high1=" + high.ToString() );
		}
		#endif

		Broadcast( average, low, high );
	}

	//! Keep the `limit` largest values, largest first.
	private void InsertDescending( array< float > list, float value, int limit )
	{
		int count = list.Count();

		if ( count >= limit && value <= list.Get( count - 1 ) )
			return;

		int at = count;
		for ( int i = 0; i < count; i++ )
		{
			if ( value > list.Get( i ) )
			{
				at = i;
				break;
			}
		}

		InsertSorted( list, value, at );

		if ( list.Count() > limit )
			list.Remove( list.Count() - 1 );
	}

	//! Keep the `limit` smallest values, smallest first.
	private void InsertAscending( array< float > list, float value, int limit )
	{
		int count = list.Count();

		if ( count >= limit && value >= list.Get( count - 1 ) )
			return;

		int at = count;
		for ( int i = 0; i < count; i++ )
		{
			if ( value < list.Get( i ) )
			{
				at = i;
				break;
			}
		}

		InsertSorted( list, value, at );

		if ( list.Count() > limit )
			list.Remove( list.Count() - 1 );
	}

	//! InsertAt requires index < Count() - appending through it is a native
	//! out-of-bounds write, not a script error. Both callers compute "at" as a
	//! sorted position that is legitimately Count() when the value belongs on
	//! the end, so that case has to go through Insert instead.
	private void InsertSorted( array< float > list, float value, int at )
	{
		if ( at >= list.Count() )
		{
			list.Insert( value );
			return;
		}

		list.InsertAt( value, at );
	}

	//! Mean frame rate of a set of frame times. Averaging the times and then
	//! inverting is the correct order - averaging the individual rates would
	//! weight a single fast frame the same as a single 200ms hitch.
	private float FrameTimesToFPS( array< float > frameTimes )
	{
		int count = frameTimes.Count();
		if ( count == 0 )
			return 0;

		float total = 0;
		for ( int i = 0; i < count; i++ )
		{
			total += frameTimes.Get( i );
		}

		if ( total <= 0 )
			return 0;

		return count / total;
	}

	// -------------------------------------------------------------------------
	//  Transport
	// -------------------------------------------------------------------------

	//! Sent only to clients that can open COT. Everyone else has no use for it,
	//! and server performance is not something to hand every connected player.
	private void Broadcast( float average, float low, float high )
	{
		int maxFPS = m_MaxFPS;

		array< string > flaggedGuids = new array< string >;
		GetAntiCheatFlagged( flaggedGuids );
		int flagged = flaggedGuids.Count();

		//! An offline / listen host is its own client: no packet, write through.
		if ( !IsMissionOffline() )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

			foreach ( JMPlayerInstance player: players )
			{
				//! JMPlayerInstance has no identity of its own - it caches the
				//! guid/name at connect time and keeps the entity, so the live
				//! identity has to come back off the player object.
				if ( !player.PlayerObject )
					continue;

				PlayerIdentity identity = player.PlayerObject.GetIdentity();
				if ( !identity )
					continue;

				if ( !GetPermissionsManager().HasPermission( "COT.View", identity ) )
					continue;

				//! Seeing COT is not the same as being allowed to know the
				//! server has flagged someone, so this field is gated on its
				//! own permission and sent as 0 to everyone else.
				int flaggedForThem = 0;
				if ( GetPermissionsManager().HasPermission( "Admin.AntiCheat.View", identity ) )
					flaggedForThem = flagged;

				ScriptRPC rpc = new ScriptRPC();
				rpc.Write( average );
				rpc.Write( low );
				rpc.Write( high );
				rpc.Write( maxFPS );
				rpc.Write( flaggedForThem );

				//! The GUIDs only go to whoever the count went to. Written one
				//! by one rather than as an array so the reader can stop at the
				//! count it already has, and so an unauthorised client is sent
				//! nothing at all rather than an empty container.
				for ( int g = 0; g < flaggedForThem; g++ )
				{
					rpc.Write( flaggedGuids.Get( g ) );
				}

				rpc.Send( NULL, JMServerStatsModuleRPC.Stats, false, identity );
			}
		}

		//! An offline / listen host is its own admin, so it writes the real
		//! count through rather than the gated copy.
		if ( IsMissionClient() )
		{
			JMServerStats.Set( average, low, high, maxFPS );
			JMAntiCheatStatus.Set( flagged, flaggedGuids );
		}
	}

	//! 0 when the anti-cheat module is absent or this is not the host - the
	//! count only exists server-side, and a client reading its own empty table
	//! would publish a badge that says "all clear" on no evidence.
	private void GetAntiCheatFlagged( out array< string > guids )
	{
		guids.Clear();

		if ( !IsMissionHost() )
			return;

		JMAntiCheatModule antiCheat = CF_Modules<JMAntiCheatModule>.Get();
		if ( !antiCheat )
			return;

		antiCheat.GetFlaggedGuids( guids );
	}

	private void RPC_Stats( ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !IsMissionClient() )
			return;

		float average;
		if ( !ctx.Read( average ) )
			return;

		float low;
		if ( !ctx.Read( low ) )
			return;

		float high;
		if ( !ctx.Read( high ) )
			return;

		int maxFPS;
		if ( !ctx.Read( maxFPS ) )
			return;

		JMServerStats.Set( average, low, high, maxFPS );

		int flagged;
		if ( !ctx.Read( flagged ) )
			return;

		array< string > flaggedGuids = new array< string >;

		for ( int g = 0; g < flagged; g++ )
		{
			string flaggedGuid;
			if ( !ctx.Read( flaggedGuid ) )
				break;

			flaggedGuids.Insert( flaggedGuid );
		}

		JMAntiCheatStatus.Set( flagged, flaggedGuids );
	}
}
