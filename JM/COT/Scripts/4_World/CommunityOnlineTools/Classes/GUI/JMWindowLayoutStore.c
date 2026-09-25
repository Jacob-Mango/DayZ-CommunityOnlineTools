// =============================================================================
//  JMWindowLayoutStore.c
//
//  Per-client persisted window position/size, keyed by module name so a
//  module's window reopens at its last rect regardless of which instance
//  opened it. Lives at $profile:CommunityOnlineTools/GUI/window_layout.json
//
//  A flat array rather than a map keyed by module name - the module name is
//  already a field on the entry, and an array survives a hand-edit far
//  better than a JSON object whose keys have to match a field inside each
//  value (same reasoning as JMAntiCheatFlagStore).
// =============================================================================

class JMWindowLayoutEntry
{
	string ModuleName;
	float X, Y, W, H;
}

class JMWindowLayoutStore
{
	static const string FILE = "$profile:CommunityOnlineTools\\GUI\\window_layout.json";

	ref array< ref JMWindowLayoutEntry > Entries = new array< ref JMWindowLayoutEntry >;

	static JMWindowLayoutStore Load()
	{
		JMWindowLayoutStore store = new JMWindowLayoutStore();
		JMJsonFile<JMWindowLayoutStore>.Load( FILE, store );
		return store;
	}

	static void Save( JMWindowLayoutStore store )
	{
		JMJsonFile<JMWindowLayoutStore>.Save( FILE, store );
	}

	JMWindowLayoutEntry FindEntry( string moduleName )
	{
		foreach ( JMWindowLayoutEntry entry: Entries )
		{
			if ( entry.ModuleName == moduleName )
				return entry;
		}
		return NULL;
	}

	void SetEntry( string moduleName, float x, float y, float w, float h )
	{
		JMWindowLayoutEntry entry = FindEntry( moduleName );
		if ( !entry )
		{
			entry = new JMWindowLayoutEntry();
			entry.ModuleName = moduleName;
			Entries.Insert( entry );
		}

		entry.X = x;
		entry.Y = y;
		entry.W = w;
		entry.H = h;
	}
}
