#ifdef JM_CommunityOnlineTools
// Example: handling right-click or double-click events on the COT teleport map (JMTeleportForm).
modded class JMTeleportForm
{
	override void OnMapClick( int x, int y, int button )
	{
		super.OnMapClick( x, y, button );

		if ( button != MouseState.RIGHT )
			return;

		vector pos = m_Map.ScreenToMap( Vector( x, y, 0 ) );
		COTCreateLocalAdminNotification( new StringLocaliser( "Map right-clicked at " + pos.ToString() ), JMConstants.Lucide( "map-pin" ) );
	}
}
#endif
