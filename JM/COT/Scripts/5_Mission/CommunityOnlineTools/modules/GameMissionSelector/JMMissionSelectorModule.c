class JMMissionSelectorModule extends JMRenderableModuleBase
{
	void JMMissionSelectorModule()
	{
		GetPermissionsManager().RegisterPermission( "GameMission" );
		GetPermissionsManager().RegisterPermission( "GameMission.View" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "GameMission.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/mission_selector_form.layout";
	}

	override string GetTitle()
	{
		return "Offline Mission";
	}
	
	override string GetIconName()
	{
		return "OM";
	}

	override bool ImageIsIcon()
	{
		return false;
	}
};