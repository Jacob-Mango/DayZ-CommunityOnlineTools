class JMMapModule: JMRenderableModuleBase
{
	protected ref array< Man > m_ServerPlayers;
	
	void JMMapModule() 
	{
		m_ServerPlayers = new array< Man >;

		GetPermissionsManager().RegisterPermission( "Admin.Player.Map" );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Map.SeePlayers" );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Teleport.Position" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Admin.Player.Map" );
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
		return "Map";
	}
	
	override string GetIconName()
	{
		return "M";
	}
	
	override bool ImageIsIcon()
	{
		return false;
	}
}