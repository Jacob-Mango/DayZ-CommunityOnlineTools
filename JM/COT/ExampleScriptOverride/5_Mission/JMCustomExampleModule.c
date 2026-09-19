#ifdef JM_CommunityOnlineTools
// Example: Creating a standalone module and registering custom permissions
class JMCustomExampleModule: JMRenderableModuleBase
{
	void JMCustomExampleModule()
	{
		// Always use static string constants from JMConstants for permission registration
		GetPermissionsManager().RegisterPermission( JMConstants.PERM_CUSTOM_VIEW );
		GetPermissionsManager().RegisterPermission( JMConstants.PERM_CUSTOM_ACTION );
		GetPermissionsManager().RegisterPermission( JMConstants.PERM_RPC_EXECUTE );
		GetPermissionsManager().RegisterPermission( JMConstants.PERM_PLAYER_INJECTED_PANEL );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( JMConstants.PERM_CUSTOM_VIEW );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/Example_form.layout";
	}

	override string GetTitle()
	{
		return "Custom Sub-Mod Module";
	}

	// Module Category in the COT Sidebar navigation
	override string GetCategory()
	{
		return "Custom Addons";
	}

	override string GetIconName()
	{
		return JMConstants.Lucide( "sparkles" );
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	override string GetWebhookTitle()
	{
		return "Custom Sub-Mod Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "CustomAction" );
	}
}
#endif
