#ifdef JM_CommunityOnlineTools
// Example: a checkbox in the Teleport map's filter menu that shows/hides a custom map marker layer.
// Same Register() call, Teleport scope; the marker is added in RefreshMapMarkers only while it is on.
modded class JMTeleportForm
{
	protected bool m_ExShowOutposts = true;

	bool IsExShowOutposts()
	{
		return m_ExShowOutposts;
	}

	override void OnCreate()
	{
		super.OnCreate();

		JMFilterRegistry.Register( JMFilterRegistry.TELEPORT, "ex_outposts", "Sub-Mod Outposts", "", 0, this, "OnExFilterOutposts", "IsExShowOutposts" );
	}

	void OnExFilterOutposts( string id )
	{
		m_ExShowOutposts = !m_ExShowOutposts;
		RefreshMapMarkers();
	}

	override protected void RefreshMapMarkers()
	{
		super.RefreshMapMarkers();

		if ( m_Map && m_ExShowOutposts )
			m_Map.AddMarker( "ex_outpost_1", Vector( 7500, 0, 7500 ), "Sub-Mod Outpost Alpha", 0xFF00E5FF, JMConstants.Lucide( "tent" ), "ex_markers" );
	}
}
#endif
