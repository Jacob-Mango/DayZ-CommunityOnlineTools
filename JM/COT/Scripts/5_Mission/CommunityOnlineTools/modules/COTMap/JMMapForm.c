class JMMapForm: JMFormBase
{
	private MapWidget m_MapWidget;
	private Widget m_BackgroundWidget;

	private JMMapModule m_Module;

	private vector m_TeleportPosition;

	private ref array<string> m_LootMarkerNames;
	private ref array<vector> m_LootMarkerPositions;
	
	override void OnInit()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::OnInit" );
		#endif

		m_MapWidget = MapWidget.Cast( layoutRoot.FindAnyWidget( "map_widget" ) );

		m_BackgroundWidget = m_Window.GetWidgetRoot().FindAnyWidget( "background" );
		m_BackgroundWidget.Show( false );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::OnInit" );
		#endif
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}
	
	override void OnShow()
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Map.View" ) )
			return;

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapPosition, 34, false, true, vector.Zero );

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapMarkers, 1000, true );
	}

	void UpdateMapPosition( bool usePlayerPosition, vector mapPosition = vector.Zero )
	{
		if ( usePlayerPosition )
		{
			PlayerBase player;
			float scale;
			if ( Class.CastTo( player, g_Game.GetPlayer() ) && !player.GetLastMapInfo( scale, mapPosition ) )
			{
				scale = 0.33;
				mapPosition = player.GetWorldPosition();
			}

			m_MapWidget.SetScale( scale );
		}

		m_MapWidget.SetMapPos( mapPosition );
	}

	override void OnHide() 
	{
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( UpdateMapMarkers );
	}

	void UpdateMapMarkers()
	{
		m_MapWidget.ClearUserMarks();

		UpdatePlayers();
		UpdateLootMarkers();

#ifdef EXPANSIONMODNAVIGATION
		UpdateExpansionMarkers();
		UpdateExpansionSafezones();
#endif
	}

	void SetLootMarkers(array<string> names, array<vector> positions)
	{
		m_LootMarkerNames = names;
		m_LootMarkerPositions = positions;
		// Full redraw so existing marks (players + old loot) are cleared first
		UpdateMapMarkers();
	}

	void ClearLootMarkers()
	{
		m_LootMarkerNames = null;
		m_LootMarkerPositions = null;
		UpdateMapMarkers();
	}

	private void UpdateLootMarkers()
	{
		if (!m_LootMarkerNames || !m_LootMarkerPositions)
			return;

		if (m_LootMarkerNames.Count() != m_LootMarkerPositions.Count())
			return;

		for (int i = 0; i < m_LootMarkerNames.Count(); i++)
		{
			m_MapWidget.AddUserMark(m_LootMarkerPositions[i], m_LootMarkerNames[i], ARGB(255, 255, 165, 0), JM_COT_ICON_DOT + ".paa");
		}
	}

	private void UpdatePlayers() 
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.Map.Players" ) )
			return;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();
		foreach (JMPlayerInstance player: players)
		{
			int colors;
			if ( JM_GetSelected().IsSelected( player.GetGUID() ) )
				colors = ARGB( 255, 255, 255, 0 ); // Yellow
			else if (player.HasPermission( "COT" ))
				colors = ARGB( 255, 255, 0, 0 ); 	// Red
			else
				colors = ARGB( 255, 0, 255, 0 ); 	// Green

			m_MapWidget.AddUserMark( player.GetPosition(), player.GetName(), colors, JM_COT_ICON_DOT + ".paa" );
		}

		GetCommunityOnlineTools().RefreshClientPositions();
	}

	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
			return false;

		if ( w == m_MapWidget )
		{
			m_TeleportPosition = SnapToGround(m_MapWidget.ScreenToMap(Vector( x, y, 0 )));
			CreateAdvancedPlayerConfirm("#STR_COT_TELEPORT_MODULE_TELEPORT_OFFLINE", "TeleportPlayerMulti", "TeleportPlayerSingle", "TeleportPlayerSelf", false);
			
			return true;
		}

		return false;
	}

	void TeleportPlayerMulti(JMConfirmation confirmation = NULL)
	{
		CF_Modules<JMTeleportModule>.Get().Position( m_TeleportPosition, JM_GetSelected().GetPlayers() );
	}

	void TeleportPlayerSingle(JMConfirmation confirmation = NULL)
	{
		CF_Modules<JMTeleportModule>.Get().Position( m_TeleportPosition, {JM_GetSelected().GetPlayers()[0]} );
	}

	void TeleportPlayerSelf(JMConfirmation confirmation = NULL)
	{
		CF_Modules<JMTeleportModule>.Get().Position( m_TeleportPosition, {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

#ifdef EXPANSIONMODNAVIGATION
	private void UpdateExpansionMarkers()
	{
		ExpansionMapSettings mapSettings = GetExpansionSettings().GetMap();
		if ( !mapSettings || !mapSettings.ServerMarkers )
			return;

		for ( int i = 0; i < mapSettings.ServerMarkers.Count(); i++ )
		{
			ExpansionMarkerData marker = mapSettings.ServerMarkers[i];
			if ( !marker )
				continue;

			string name = marker.GetName();
			if ( name == "" )
				name = "Marker";

			m_MapWidget.AddUserMark( marker.GetPosition(), name, marker.GetColor(), JM_COT_ICON_DOT + ".paa" );
		}
	}

	private void UpdateExpansionSafezones()
	{
		ExpansionSafeZoneSettings szSettings = GetExpansionSettings().GetSafeZone();
		if ( !szSettings || !szSettings.Enabled )
			return;

		int szColor = ARGB( 200, 50, 220, 50 );

		// Circle zones
		for ( int ci = 0; ci < szSettings.CircleZones.Count(); ci++ )
		{
			ExpansionSafeZoneCircle circleZone = szSettings.CircleZones[ci];
			if ( !circleZone )
				continue;
			DrawMapCircle( circleZone.Center, circleZone.Radius, szColor );
		}

		// Cylinder zones (same footprint as circle on a 2D map)
		for ( int cy = 0; cy < szSettings.CylinderZones.Count(); cy++ )
		{
			ExpansionSafeZoneCylinder cylinderZone = szSettings.CylinderZones[cy];
			if ( !cylinderZone )
				continue;
			DrawMapCircle( cylinderZone.Center, cylinderZone.Radius, szColor );
		}
	}

	// Approximates a circle on the map by placing dot markers around the perimeter.
	private void DrawMapCircle( vector center, float radius, int color )
	{
		int segments = 12;
		float step = Math.PI2 / segments;

		for ( int i = 0; i < segments; i++ )
		{
			float angle = step * i;
			vector pt;
			pt[0] = center[0] + radius * Math.Sin( angle );
			pt[1] = center[1];
			pt[2] = center[2] + radius * Math.Cos( angle );
			m_MapWidget.AddUserMark( pt, "", color, JM_COT_ICON_DOT + ".paa" );
		}

		// Centre pin so the zone is identifiable when zoomed out.
		m_MapWidget.AddUserMark( center, "Safe Zone", color, JM_COT_ICON_DOT + ".paa" );
	}
#endif
}
