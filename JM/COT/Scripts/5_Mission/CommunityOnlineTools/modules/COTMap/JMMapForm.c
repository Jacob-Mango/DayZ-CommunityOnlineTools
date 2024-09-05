class JMMapForm: JMFormBase 
{
	private MapWidget m_MapWidget;
	private Widget m_BackgroundWidget;

	private JMMapModule m_Module;

	private vector m_TeleportPosition;
	
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

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapPosition, 34, false, true, vector.Zero );

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapMarkers, 1000, true );
	}

	void UpdateMapPosition( bool usePlayerPosition, vector mapPosition = vector.Zero )
	{
		if ( usePlayerPosition )
		{
			PlayerBase player;
			float scale;
			if ( Class.CastTo( player, GetGame().GetPlayer() ) && !player.GetLastMapInfo( scale, mapPosition ) )
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
		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).Remove( UpdateMapMarkers );
	}

	void UpdateMapMarkers()
	{
		m_MapWidget.ClearUserMarks();

		UpdatePlayers();
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
			if (!HasTooManyPlayers("TeleportSelected","TeleportPlayers"))
				TeleportSelected(NULL);
			
			return true;
		}

		return false;
	}

	void TeleportPlayers(JMConfirmation confirmation)
	{
		JMTeleportModule mod;
		if ( Class.CastTo( mod, GetModuleManager().GetModule( JMTeleportModule ) ) )
			mod.Position( m_TeleportPosition, JM_GetSelected().GetPlayers() );
	}

	void TeleportSelected(JMConfirmation confirmation)
	{
		JMTeleportModule mod;
		if ( Class.CastTo( mod, GetModuleManager().GetModule( JMTeleportModule ) ) )
			mod.Position( m_TeleportPosition, {JM_GetSelected().GetPlayers()[0]} );
	}

	bool HasTooManyPlayers(string funcOnlyName, string funcName)
	{
		int count = JM_GetSelected().GetPlayers().Count();
		if (count > 1)
		{
			JMPlayerInstance inst = GetPermissionsManager().GetPlayer( JM_GetSelected().GetPlayers()[0] );
			CreateConfirmation_Three( JMConfirmationType.INFO, "#STR_COT_WARNING_PLAYERS_MESSAGE_HEADER", string.Format(Widget.TranslateString("#STR_COT_WARNING_PLAYERS_MESSAGE_BODY"), count.ToString()), "#STR_COT_GENERIC_CANCEL", "", inst.GetName(), funcOnlyName, "#STR_COT_GENERIC_CONFIRM", funcName );
			return true;
		}

		return false;
	}
};
