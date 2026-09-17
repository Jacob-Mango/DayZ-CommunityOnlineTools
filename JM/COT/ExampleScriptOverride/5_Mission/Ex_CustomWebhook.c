#ifdef JM_CommunityOnlineTools
// Example: Creating custom webhook types and logging actions to Discord
modded class JMPlayerModule
{
	override void GetWebhookTypes( out array< string > types )
	{
		super.GetWebhookTypes( types );

		// Register custom webhook type keyword
		types.Insert( "SubModAction" );
	}

	void LogSubModWebhookAction( string details )
	{
		// Dispatch webhook notification via COT webhook system
		SendWebhook( "SubModAction", GetPermissionsManager().GetClientPlayer(), details );
	}
}
#endif
