#ifdef JM_CommunityOnlineTools
// Example: Adding custom filter entry for Object Spawner (JMObjectSpawnerForm)
modded class JMObjectSpawnerForm
{
	override protected void RebuildCategoryMenu()
	{
		super.RebuildCategoryMenu();

		// Custom filter category for object spawner list
		if ( m_CategoryMenu )
		{
			m_CategoryMenu.AddItem( "submod_custom_items", "Sub-Mod Custom Items", JMConstants.Lucide( "box" ) );
		}
	}
}
#endif
