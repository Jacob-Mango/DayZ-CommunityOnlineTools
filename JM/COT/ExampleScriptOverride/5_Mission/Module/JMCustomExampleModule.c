#ifdef JM_CommunityOnlineTools
// Example: a sidebar module. Everything a module needs is one small override each;
// the form it opens is JMCustomExampleForm.c and it is loaded by Ex_ModuleRegistration.c.
class JMCustomExampleModule: JMRenderableModuleBase
{
	// Called on client AND server before the mission loads - no constructor needed.
	override void DeclarePermissions()
	{
		JMPermissions.Register( JMConstants.PERM_CUSTOM_VIEW );
		JMPermissions.Register( JMConstants.PERM_CUSTOM_ACTION );
		JMPermissions.Register( JMConstants.PERM_PLAYER_INJECTED_TAB );
		JMPermissions.Register( JMConstants.PERM_PLAYER_INJECTED_PANEL );
	}

	override bool HasAccess()
	{
		return JMPermissions.Has( JMConstants.PERM_CUSTOM_VIEW );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/ExampleScriptOverride/GUI/layouts/JMCustomExampleForm.layout";
	}

	override string GetTitle()
	{
		return "Custom Sub-Mod Module";
	}

	override string GetCategory()
	{
		return "Custom Addons";
	}

	// One Lucide icon name replaces GetIconName + ImageIsIcon + ImageHasPath.
	override string GetLucideIcon()
	{
		return "sparkles";
	}

	override string GetWebhookTitle()
	{
		return "Custom Sub-Mod Module";
	}
}
#endif
