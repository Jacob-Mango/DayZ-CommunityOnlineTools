class JMMapForm: JMFormBase 
{
	private MapWidget m_MapWidget;
	private Widget m_BackgroundWidget;

	private JMMapModule m_Module;
	
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
			int colors = ARGB( 255, 230, 20, 20 );
			string marker;

			//if ( player.PlayerObject && player.PlayerObject.GetParent() && player.PlayerObject.GetParent().IsInherited(CarScript) )
				//marker = JM_COT_ICON_CAR;
			//else
				marker = JM_COT_ICON_DOT;

			if ( player.PlayerObject )
			{
				if ( !player.PlayerObject.IsAlive() )
				{
					colors = ARGB( 255, 50, 0, 0 );
					//marker += "_dead";
				}
				else if ( player.PlayerObject.IsUnconscious() )
				{
					colors = ARGB( 255, 100, 0, 0 );
					//marker += "_uncon";
				}
			}
			
			if ( JM_GetSelected().IsSelected( player.GetGUID() ) )
				marker += "_selected";

			m_MapWidget.AddUserMark( player.GetPosition(), player.GetName(), colors, marker + ".paa" );
		}

		GetCommunityOnlineTools().RefreshClientPositions();
	}

	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}

		if ( w == m_MapWidget )
		{
			JMTeleportModule mod;
			if ( Class.CastTo( mod, GetModuleManager().GetModule( JMTeleportModule ) ) )
			{
				mod.Position( SnapToGround( m_MapWidget.ScreenToMap( Vector( x, y, 0 ) ) ) );
			}
			
			return true;
		}

		return false;
	}
};
