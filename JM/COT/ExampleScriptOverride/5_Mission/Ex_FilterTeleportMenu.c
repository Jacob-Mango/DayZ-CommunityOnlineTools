#ifdef JM_CommunityOnlineTools
// Example: Adding custom filter entry for Teleport Menu (JMTeleportForm)
modded class JMTeleportForm
{
	override protected void RefreshCategories()
	{
		super.RefreshCategories();

		// Add custom category filter entry to teleport saved locations
		if ( m_CategoryMenu )
		{
			m_CategoryMenu.AddItem( "submod_outposts", "Sub-Mod Outposts", JMConstants.Lucide( "tent" ) );
		}
	}
}
#endif
