class JMMapModule: JMRenderableModuleBase
{
	protected ref array< Man > m_ServerPlayers;
	
	void JMMapModule() 
	{
		m_ServerPlayers = new array< Man >;

		GetPermissionsManager().RegisterPermission( "Admin.Map" );

		GetPermissionsManager().RegisterPermission( "Admin.Map.View" );
		GetPermissionsManager().RegisterPermission( "Admin.Map.Players" );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Position" );
	}

	override void EnableUpdate()
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Admin.Map.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleMap";
	}
	
	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/mapview_form.layout";
	}

	override string GetTitle()
	{
		return "#STR_COT_MAP_MODULE_NAME";
	}
	
	override string GetIconName()
	{
		return "JM\\COT\\GUI\\textures\\modules\\COTMap.paa";
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}
};
