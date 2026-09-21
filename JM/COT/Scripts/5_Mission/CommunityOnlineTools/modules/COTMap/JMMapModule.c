class JMMapModule: JMRenderableModuleBase
{
	protected ref array< Man > m_ServerPlayers;

	void JMMapModule()
	{
		m_ServerPlayers = new array< Man >;

	}

	void SetLootMarkers(array<string> names, array<vector> positions)
	{
		JMMapForm form;
		if (Class.CastTo(form, GetForm()))
		{
			form.SetLootMarkers(names, positions);
		}
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "#STR_COT_MAP_MODULE_NAME";
		info.Icon = "map";
		info.Layout = "JM/COT/GUI/layouts/mapview_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_WORLD;
		info.ViewPermission = JMConstants.PERM_MAP_VIEW;
		info.InputToggle = "UACOTToggleMap";
		// Temporarily hidden from the sidebar.
		info.HasButton = false;

		//! Called on both client and server as the module registers, before the mission loads.
		info.AddPermission( JMConstants.PERM_MAP );
		info.AddPermission( JMConstants.PERM_MAP_PLAYERS );
		info.AddPermission( JMConstants.PERM_PLAYER_TELEPORT_POSITION );
	}

	override void EnableUpdate()
	{
	}

	void ClearLootMarkers()
	{
		JMMapForm form;
		if (Class.CastTo(form, GetForm()))
		{
			form.ClearLootMarkers();
		}
	}
}
