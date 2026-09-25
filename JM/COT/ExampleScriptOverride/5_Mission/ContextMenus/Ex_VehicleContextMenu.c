#ifdef JM_CommunityOnlineTools
// Example: adding entries to the vehicle list & map marker right-click menu (JMVehiclesFormTabMap).
modded class JMVehiclesFormTabMap
{
	override void OnMarkerRow_RightClick( string id, int x, int y )
	{
		super.OnMarkerRow_RightClick( id, x, y );

		if ( !m_MarkerMenu )
			return;

		m_MarkerMenu.AddItem( "ex_refuel_veh", "Sub-Mod Refuel Vehicle", JMConstants.Lucide( "fuel" ), 0, false, this, "OnExRefuelVehicle" );
	}

	void OnExRefuelVehicle( string itemId )
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "Refuel action triggered on vehicle" ), JMConstants.Lucide( "fuel" ) );
	}
}
#endif
