class JMMapForm extends JMFormBase 
{
	private MapWidget map_widget;
	private Widget basewindow_background;
	
	void JMMapForm()
	{
	}

	void ~JMMapForm()
	{
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
	
	override void OnInit( bool fromMenu )
	{
		map_widget = MapWidget.Cast( layoutRoot.FindAnyWidget( "map_widget" ) );
		basewindow_background = window.GetLayoutRoot().FindAnyWidget( "background" );
		basewindow_background.Show( false );
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
		map_widget.ClearUserMarks();

		foreach( JMPlayerInstance player : GetPermissionsManager().GetPlayers() ) 
		{
			map_widget.AddUserMark( player.Data.VPosition, player.Data.SName, ARGB( 255, 230, 20, 20 ), "\\JM\\COT\\GUI\\textures\\dot.paa" );
		}

		GetCommunityOnlineTools().RefreshClients();
	}

	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		if ( w == NULL )
		{
			return false;
		}

		if ( w == map_widget )
		{
			JMMapModule mod;
			if ( Class.CastTo( mod, module ) )
			{
				mod.Teleport( SnapToGround( map_widget.ScreenToMap( Vector( x, y, 0 ) ) ) );
			}
			
			return true;
		}

		return false;
	}
}