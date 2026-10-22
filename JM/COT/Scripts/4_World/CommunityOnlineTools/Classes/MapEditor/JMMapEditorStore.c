// JMMapEditorStore.c - JSON-serialisable container for persisted Map Editor objects.
// Per-mission save: $profile:CommunityOnlineTools/MapEditor/map.json

class JMMapEditorStore
{
	ref array< ref JMMapEditorObject > Objects = new array< ref JMMapEditorObject >;
	static const string FILE = "$profile:CommunityOnlineTools\\MapEditor\\map.json";

	static JMMapEditorStore Load()
	{
		if ( !FileExist( FILE ) )
			return new JMMapEditorStore();

		JMMapEditorStore store = new JMMapEditorStore();
		JMJsonFile<JMMapEditorStore>.Load( FILE, store );
		return store;
	}

	static void Save( JMMapEditorStore store )
	{
		JMJsonFile<JMMapEditorStore>.Save( FILE, store );
	}
}