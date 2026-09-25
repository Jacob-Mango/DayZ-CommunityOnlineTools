#ifdef JM_CommunityOnlineTools
// Example: registering custom object prefab groups in MapEditorModule.
modded class MapEditorModule
{
	override void RegisterPrefabs()
	{
		super.RegisterPrefabs();

		#ifdef COT_DEBUGLOGS
		Print( "[COT_DBG] Ex_MapEditorObjectGroup: Registered sub-mod prefabs" );
		#endif
	}
}
#endif
