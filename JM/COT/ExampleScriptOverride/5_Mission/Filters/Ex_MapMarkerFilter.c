#ifdef JM_CommunityOnlineTools
// Example: adding custom map marker layers to the map view (JMTeleportForm).
modded class JMTeleportForm
{
	override void BuildMapMarkers()
	{
		super.BuildMapMarkers();

		if ( !m_Map )
			return;

		m_Map.AddMarker( "ex_outpost_alpha", Vector( 7500, 0, 7500 ), "Outpost Alpha", 0xFF00E5FF, JMConstants.Lucide( "tent" ), "ex_markers" );
	}
}
#endif
