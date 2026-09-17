#ifdef JM_CommunityOnlineTools
// Example: Adding custom map markers for the Teleport Menu map (JMTeleportForm)
modded class JMTeleportForm
{
	override protected void RefreshMapMarkers()
	{
		super.RefreshMapMarkers();

		if ( !m_Map )
			return;

		// Add custom map marker layer with a distinct icon and color
		m_Map.AddMarker( "submod_outpost_1", Vector( 7500, 0, 7500 ), "Sub-Mod Outpost Alpha", 0xFF00E5FF, JMConstants.Lucide( "shield" ), "submod_markers" );
	}
}
#endif
