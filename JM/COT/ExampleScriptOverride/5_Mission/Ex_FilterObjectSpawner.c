#ifdef JM_CommunityOnlineTools
// Example: a custom row in the Object Spawner's category menu - the same Register() call as the
// Player list, with the Object Spawner's scope. The callback receives the clicked id.
modded class JMObjectSpawnerForm
{
	override void OnCreate()
	{
		super.OnCreate();

		JMFilterRegistry.Register( JMFilterRegistry.OBJECTS, "ex_custom_items", "Sub-Mod Custom Items", "box", 0, this, "OnExCustomItems" );
	}

	void OnExCustomItems( string id )
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "Custom category picked" ), JMConstants.Lucide( "box" ) );
	}
}
#endif
