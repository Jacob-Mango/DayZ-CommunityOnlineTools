// =============================================================================
//  JMAntiCheatSignals.c
//
//  Static 4_World-side queues for events the anti-cheat module cannot hook
//  directly. Same pattern, and the same reason, as JMAntiCheatKillHook: the
//  producers here are 4_World types (the permission manager, Weapon_Base) and
//  4_World compiles before 5_Mission, so they cannot name JMAntiCheatModule.
//  They post here instead and the module drains the queues in its OnUpdate.
//
//  Everything is bounded. A disabled or crashed anti-cheat module must not
//  turn a queue into a memory leak, and a client spamming denied RPCs must not
//  be able to make the server allocate on its behalf - which would turn a
//  detector into the very thing it is there to stop.
// =============================================================================

class JMAntiCheatSignals
{
	//! Guid of a player whose RPC was refused for want of a permission.
	static ref array< string > DeniedRpcGuids = new array< string >;
	//! The permission each denial asked for, index-matched to the above.
	static ref array< string > DeniedRpcPermissions = new array< string >;

	//! Guid of a player who fired a shot, plus the cartridge count left in the
	//! weapon immediately afterwards. The module compares consecutive samples:
	//! a count that never falls across many shots is not a real magazine.
	static ref array< string > ShotGuids = new array< string >;
	static ref array< int >    ShotAmmoCounts = new array< int >;

	//! Deliberately small. These are drained every server frame, so a backlog
	//! this deep already means the module is not running - in which case
	//! keeping the events is pointless and dropping them is free.
	static const int MAX_QUEUE = 256;

	// -------------------------------------------------------------------------
	//  Producers
	// -------------------------------------------------------------------------

	static void ReportDeniedRpc( string guid, string permission )
	{
		if ( guid == "" )
			return;

		if ( DeniedRpcGuids.Count() >= MAX_QUEUE )
		{
			DeniedRpcGuids.Remove( 0 );
			DeniedRpcPermissions.Remove( 0 );
		}

		DeniedRpcGuids.Insert( guid );
		DeniedRpcPermissions.Insert( permission );
	}

	//! Kept as the name every caller already uses. Engine-initiated moves are
	//! no longer queued: they grant a movement sanction directly, which takes
	//! effect on the same frame instead of on the module's next drain. A poll
	//! landing between the teleport and the drain used to flag it.
	static void ReportEngineMove( string guid )
	{
		JMAntiCheatSanction.Grant( guid, JMAntiCheatSanction.MOVEMENT, JMAntiCheatSanction.MoveGraceSeconds );
	}

	static void ReportShot( string guid, int ammoCountAfter )
	{
		if ( guid == "" )
			return;

		if ( ShotGuids.Count() >= MAX_QUEUE )
		{
			ShotGuids.Remove( 0 );
			ShotAmmoCounts.Remove( 0 );
		}

		ShotGuids.Insert( guid );
		ShotAmmoCounts.Insert( ammoCountAfter );
	}

	// -------------------------------------------------------------------------
	//  Consumer helpers - the module drains, it does not read in place, so a
	//  producer firing mid-drain lands in the next tick rather than being lost.
	// -------------------------------------------------------------------------

	static void DrainDeniedRpc( out array< string > guids, out array< string > permissions )
	{
		guids.Clear();
		permissions.Clear();

		int count = DeniedRpcGuids.Count();

		for ( int i = 0; i < count; i++ )
		{
			guids.Insert( DeniedRpcGuids.Get( i ) );
			permissions.Insert( DeniedRpcPermissions.Get( i ) );
		}

		for ( int j = count - 1; j >= 0; j-- )
		{
			DeniedRpcGuids.Remove( j );
			DeniedRpcPermissions.Remove( j );
		}
	}

	static void DrainShots( out array< string > guids, out array< int > ammoCounts )
	{
		guids.Clear();
		ammoCounts.Clear();

		int count = ShotGuids.Count();

		for ( int i = 0; i < count; i++ )
		{
			guids.Insert( ShotGuids.Get( i ) );
			ammoCounts.Insert( ShotAmmoCounts.Get( i ) );
		}

		for ( int j = count - 1; j >= 0; j-- )
		{
			ShotGuids.Remove( j );
			ShotAmmoCounts.Remove( j );
		}
	}
}
