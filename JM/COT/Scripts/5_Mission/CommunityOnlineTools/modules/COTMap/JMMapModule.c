class JMMapModule: JMRenderableModuleBase
{
	protected ref array< Man > m_ServerPlayers;

	void JMMapModule()
	{
		m_ServerPlayers = new array< Man >;

		GetPermissionsManager().RegisterPermission( "Admin.Map" );

		JMPermissions.Register( JMConstants.PERM_MAP_VIEW );
		JMPermissions.Register( JMConstants.PERM_MAP_PLAYERS );

		JMPermissions.Register( JMConstants.PERM_PLAYER_TELEPORT_POSITION );
	}

	override void EnableUpdate()
	{
	}

	override bool HasAccess()
	{
		return JMPermissions.Has( JMConstants.PERM_MAP_VIEW );
	}

	// Temporarily hidden from the sidebar.
	override bool HasButton() { return false; }

	override string GetInputToggle()
	{
		return "UACOTToggleMap";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/mapview_form.layout";
	}

	override string GetCategory()
	{
		return "World";
	}

	override string GetTitle()
	{
		return "#STR_COT_MAP_MODULE_NAME";
	}

	override string GetIconName()
	{
		return JMConstants.Lucide( "map" );
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	void SetLootMarkers(array<string> names, array<vector> positions)
	{
		JMMapForm form;
		if (Class.CastTo(form, GetForm()))
		{
			form.SetLootMarkers(names, positions);
		}
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
