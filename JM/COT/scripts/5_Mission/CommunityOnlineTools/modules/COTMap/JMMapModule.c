class JMMapModule: JMRenderableModuleBase
{
	protected ref array< Man > m_ServerPlayers;
	
	void JMMapModule() 
	{
		m_ServerPlayers = new array< Man >;

		GetPermissionsManager().RegisterPermission( "Map.View" );
		GetPermissionsManager().RegisterPermission( "Map.Players" );

		GetPermissionsManager().RegisterPermission( "Teleport.Position" );
	}
	
	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/mapview_form.layout";
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Map.View" );
	}
}