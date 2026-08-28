// =============================================================================
//  JMPlayerStatsHook
//
//  Forwards the engine death event to the per-player session stats.
//
//  A static forwarder rather than another override inside PlayerBase, matching
//  JMAntiCheatKillHook next door: PlayerBase.EEKilled already calls out to one
//  of these, and keeping the pattern means the Mission-layer module never has to
//  be referenced from the World-layer entity.
//
//  Server-side only. A client running this would count deaths into a player
//  file it does not have.
// =============================================================================

class JMPlayerStatsHook
{
	static void OnPlayerKilled( PlayerBase victim, Object killer )
	{
		if ( !victim || !g_Game.IsServer() )
			return;

		PlayerIdentity identity = victim.GetIdentity();
		if ( !identity )
			return;

		JMPlayerStatsHook.RecordDeath( identity.GetId() );
	}

	static void RecordDeath( string guid )
	{
	#ifndef CF_MODULE_PERMISSIONS
		if ( guid == "" )
			return;

		JMPlayerInstance instance = GetPermissionsManager().GetPlayer( guid );
		if ( !instance )
			return;

		JMPlayerStats stats = instance.GetStats();
		if ( !stats )
			return;

		stats.OnDeath();

		// Written through immediately: a death is exactly the kind of event a
		// server crash would otherwise swallow, and it happens rarely enough
		// that the extra file write costs nothing.
		instance.Save();
	#endif
	}
}
