//  JMAntiCheatFlagStore.c - on-disk flag table: $profile:CommunityOnlineTools\AntiCheat\flags.json
//  Flat array, not a guid-keyed map: survives admin hand-edits better.
//  Version lets a format change migrate instead of loading garbage into scores.

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
