class JMMapForm extends JMFormBase 
{
	private MapWidget m_MapWidget;
	private Widget m_BackgroundWidget;

	private JMMapModule m_Module;
	
	override void OnInit()
	{
		#ifdef COT_DEBUGLOGS
		Print(( "+" + this + "::OnInit" ));
		#endif

		m_MapWidget = MapWidget.Cast( layoutRoot.FindAnyWidget( "map_widget" ) );

		m_BackgroundWidget = m_Window.GetWidgetRoot().FindAnyWidget( "background" );
		m_BackgroundWidget.Show( false );

		#ifdef COT_DEBUGLOGS
		Print(( "-" + this + "::OnInit" ));
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

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapMarkers, 1000, true );
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

		for ( int i = 0; i < players.Count(); ++i )
		{
			m_MapWidget.AddUserMark( players[i].GetPosition(), players[i].GetName(), ARGB( 255, 230, 20, 20 ), "\\JM\\COT\\GUI\\textures\\dot.paa" );
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
}