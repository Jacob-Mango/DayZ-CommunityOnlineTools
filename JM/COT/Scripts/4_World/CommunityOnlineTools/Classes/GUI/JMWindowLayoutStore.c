//  JMWindowLayoutStore.c - per-client window rect keyed by module name, stored in
//  $profile:CommunityOnlineTools/GUI/window_layout.json. A flat array, not a map:
//  the name is already a field on the entry and an array survives hand-edits
//  better (same reasoning as JMAntiCheatFlagStore).

class JMWindowLayoutEntry
{
	string ModuleName;
	string WindowName;
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

	JMWindowLayoutEntry FindEntry( string moduleName, string windowName )
	{
		foreach ( JMWindowLayoutEntry entry: Entries )
		{
			if ( entry.ModuleName == moduleName && entry.WindowName == windowName )
				return entry;
		}
		return NULL;
	}

	void SetEntry( string moduleName, string windowName, float x, float y, float w, float h )
	{
		JMWindowLayoutEntry entry = FindEntry( moduleName, windowName );
		if ( !entry )
		{
			entry = new JMWindowLayoutEntry();
			entry.ModuleName = moduleName;
			entry.WindowName = windowName;
			Entries.Insert( entry );
		}

		entry.X = x;
		entry.Y = y;
		entry.W = w;
		entry.H = h;
	}
}
