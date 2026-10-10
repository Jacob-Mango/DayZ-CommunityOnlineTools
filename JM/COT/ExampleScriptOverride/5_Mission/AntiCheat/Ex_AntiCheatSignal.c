#ifdef DIAG_DEVELOPER
// Example: feeding a custom anti-cheat detection signal into JMAntiCheatModule.
// Real detectors (JMAntiCheatDetector) build a JMAntiCheatHit array and hand it
// to ApplyHitsForGuid under their own detector name; a sub-mod plugs in the
// same way instead of maintaining a separate flag list.
modded class JMAntiCheatModule
{
	//! Call this from wherever your sub-mod observes the suspicious behavior.
	void FlagCustomSignal( string guid, string reason, int weight )
	{
		array< ref JMAntiCheatHit > hits = new array< ref JMAntiCheatHit >;
		hits.Insert( new JMAntiCheatHit( weight, reason ) );

		ApplyHitsForGuid( guid, "submod_custom", hits, g_Game.GetTickTime() );
	}
}
#endif
