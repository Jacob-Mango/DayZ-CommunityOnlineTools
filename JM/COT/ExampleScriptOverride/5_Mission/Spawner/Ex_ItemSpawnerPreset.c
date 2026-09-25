#ifdef JM_CommunityOnlineTools
// Example: registering custom item quick-spawn presets in the object spawner (JMObjectSpawnerForm).
modded class JMObjectSpawnerForm
{
	override protected void InitActionWidgetsIdentity( Widget actionsParent )
	{
		super.InitActionWidgetsIdentity( actionsParent );

		UIActionCard card = UIActionManager.CreateCard( actionsParent, "Sub-Mod Quick Presets" );
		UIActionButton btn = UIActionManager.CreateButton( card, "Spawn Medical Kit", this, "OnExSpawnMedicalKit" );
	}

	void OnExSpawnMedicalKit( UIActionBase action )
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "Spawning Medical Kit Preset" ), JMConstants.Lucide( "package-plus" ) );
	}
}
#endif
