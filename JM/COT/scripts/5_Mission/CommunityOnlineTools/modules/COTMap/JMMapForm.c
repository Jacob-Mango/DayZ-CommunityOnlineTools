class JMMapForm extends JMFormBase 
{
	private MapWidget m_MapWidget;
	private Widget m_BackgroundWidget;
	
	override void OnInit()
	{
		m_MapWidget = MapWidget.Cast( layoutRoot.FindAnyWidget( "JM_COT_GUI_Map" ) );

		m_BackgroundWidget = window.GetLayoutRoot().FindAnyWidget( "background" );
		m_BackgroundWidget.Show( false );
	}
	
	override void OnShow()
	{
		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( UpdatePlayers, 1000, true );
	}

	override void OnHide() 
	{
		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( UpdatePlayers );
	}

	void UpdatePlayers() 
	{
		m_MapWidget.ClearUserMarks();

		if ( !GetPermissionsManager().HasPermission( "Admin.Players.Map" ) )
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