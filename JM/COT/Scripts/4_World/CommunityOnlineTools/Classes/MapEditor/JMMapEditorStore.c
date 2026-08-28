// =============================================================================
//  JMMapEditorStore.c
//
//  JSON-serialisable container for persisted Map Editor objects.
//  Per-mission save lives at $profile:CommunityOnlineTools/MapEditor/map.json
// =============================================================================

class JMMapEditorStore
{
	ref array< ref JMMapEditorObject > Objects = new array< ref JMMapEditorObject >;

	static const string DIR  = "$profile:CommunityOnlineTools\\MapEditor\\";
	static const string FILE = "$profile:CommunityOnlineTools\\MapEditor\\map.json";

	static JMMapEditorStore Load()
	{
		if ( !FileExist( FILE ) )
			return new JMMapEditorStore();

		JMMapEditorStore store = new JMMapEditorStore();
		JsonFileLoader<JMMapEditorStore>.JsonLoadFile( FILE, store );
		return store;
	}

	static void Save( JMMapEditorStore store )
	{
		if ( !FileExist( DIR ) )
			MakeDirectory( DIR );

		JsonFileLoader<JMMapEditorStore>.JsonSaveFile( FILE, store );
	}
}