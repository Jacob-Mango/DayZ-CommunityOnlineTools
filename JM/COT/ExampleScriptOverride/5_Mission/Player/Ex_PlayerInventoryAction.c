#ifdef JM_CommunityOnlineTools
// Example: adding custom right-click actions to items in the player inventory tab (JMPlayerFormTabInventory).
modded class JMPlayerFormTabInventory
{
	override void ShowInventoryContextMenu()
	{
		super.ShowInventoryContextMenu();

		if ( !m_InventoryMenu )
			return;

		m_InventoryMenu.AddItem( "ex_disinfect_item", "Sub-Mod Disinfect Item", JMConstants.Lucide( "sparkles" ), 0, false, this, "OnExDisinfectItem" );
	}

	void OnExDisinfectItem( string itemId )
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "Item disinfected via sub-mod" ), JMConstants.Lucide( "sparkles" ) );
	}
}
#endif
