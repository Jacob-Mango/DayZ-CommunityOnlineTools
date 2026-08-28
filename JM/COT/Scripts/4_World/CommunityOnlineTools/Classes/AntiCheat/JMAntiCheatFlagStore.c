// =============================================================================
//  JMAntiCheatFlagStore.c
//
//  On-disk shape of the flag table:
//  $profile:CommunityOnlineTools\AntiCheat\flags.json
//
//  A flat array rather than a map keyed by guid - the guid is already a field
//  on the flag, and an array survives a hand-edit by an admin far better than
//  a JSON object whose keys have to match a field inside each value.
//
//  Version exists so a later format change can migrate instead of silently
//  loading garbage into a score that gets people kicked.
// =============================================================================

class JMAntiCheatFlagStore
{
	static const int CURRENT_VERSION = 1;

	static const string DIR  = "$profile:CommunityOnlineTools\AntiCheat\\";
	static const string FILE = "$profile:CommunityOnlineTools\AntiCheat\flags.json";

	int Version;
	ref array< ref JMAntiCheatFlag > Entries;

	void JMAntiCheatFlagStore()
	{
		Version = CURRENT_VERSION;
		Entries = new array< ref JMAntiCheatFlag >;
	}
}
