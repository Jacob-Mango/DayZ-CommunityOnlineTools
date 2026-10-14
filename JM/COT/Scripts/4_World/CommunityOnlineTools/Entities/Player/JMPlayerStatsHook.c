//  JMPlayerStatsHook - forwards the engine death event to per-player session stats.
//  Static forwarder like JMAntiCheatKillHook, so the World-layer PlayerBase never references
//  the Mission-layer module.
//  Server-side only: a client would count deaths into a player file it does not have.

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
	}
}
