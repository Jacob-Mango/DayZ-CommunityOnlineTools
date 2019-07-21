class JMMapForm extends JMFormBase 
{
	protected ref MapWidget map_widget;
	protected ref Widget basewindow_background;
	
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
		basewindow_background = window.GetLayoutRoot().FindAnyWidget("background");
		basewindow_background.Show(false);
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
		GetRPCManager().SendRPC( "COT_Map", "Request_Map_PlayerPositions", new Param );
	}
	
	void ShowPlayers( ref array<ref JMPlayerInformation> data ) 
	{
		map_widget.ClearUserMarks();
		
		foreach( ref JMPlayerInformation playerData : data ) 
		{
			map_widget.AddUserMark(playerData.VPosition, playerData.SName, ARGB(255, 230, 20, 20), "\\JM\\COT\\GUI\\textures\\dot.paa");
		}
	}
	
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		GetRPCManager().SendRPC( "COT_Map", "MapTeleport", new Param1< vector >( SnapToGround(map_widget.ScreenToMap( Vector( x, y, 0 ) ) ) ), true );

		return true;
	}
}