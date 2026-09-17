#ifdef JM_CommunityOnlineTools
// Example: Creating context menu actions and context sub-menus (like the ESP Color Context Menu)
modded class JMPlayerForm
{
	protected UIActionColorPicker m_SubModColorPicker;
	protected UIActionContextMenu m_SubModContextMenu;

	override void OnInit()
	{
		super.OnInit();

		// Create a color picker control host for popup editing
		m_SubModColorPicker = UIActionManager.CreateColorPicker( layoutRoot, "Sub-Mod Palette", this, "OnChange_SubModColor" );
		m_SubModColorPicker.Hide(); // Popup host runs hidden
		RegisterOverlay( m_SubModColorPicker );
	}

	override void OnPlayerRow_RightClick( string guid, int x, int y )
	{
		super.OnPlayerRow_RightClick( guid, x, y );

		if ( !m_PlayerMenu )
			return;

		// 1. Direct Context Action (triggers immediate action logic)
		m_PlayerMenu.AddItem( "act_direct", "Heal & Clear Statuses", JMConstants.Lucide( "heart-pulse" ), 0, false, this, "OnContextAction_Direct" );

		// 2. Action opening a Color Context Editor (like ESP category color editor)
		m_PlayerMenu.AddItem( "act_color", "Change Tag Color", JMConstants.Lucide( "palette" ), 0, false, this, "OnContextAction_OpenColor" );

		// 3. Action opening a nested Context Sub-Menu (indicated by trailing chevron)
		m_PlayerMenu.AddItem( "act_submenu", "Advanced Role Actions...", JMConstants.Lucide( "layers" ), 0, true, this, "OnContextAction_OpenSubmenu" );
	}

	// 1. Handler for direct context action
	void OnContextAction_Direct( string itemId )
	{
		if ( m_Module )
			COTCreateLocalAdminNotification( new StringLocaliser( "Direct context action executed for player: " + m_PlayerMenuGUID ), JMConstants.Lucide( "heart-pulse" ) );
	}

	// 2. Handler opening Color Context Editor on row/popup target
	void OnContextAction_OpenColor( string itemId )
	{
		if ( !m_SubModColorPicker )
			return;

		m_SubModColorPicker.SetColor( ARGB( 255, 0, 229, 255 ) );
		m_SubModColorPicker.Open();
	}

	void OnChange_SubModColor( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || !m_SubModColorPicker )
			return;

		int selectedColor = m_SubModColorPicker.GetColor();
		if ( m_Module )
			COTCreateLocalAdminNotification( new StringLocaliser( "New color selected: " + selectedColor.ToString() ), JMConstants.Lucide( "palette" ) );
	}

	// 3. Handler opening a nested Sub-Menu overlay
	void OnContextAction_OpenSubmenu( string itemId )
	{
		if ( !m_SubModContextMenu )
		{
			m_SubModContextMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "" );
			RegisterOverlay( m_SubModContextMenu );
		}

		m_SubModContextMenu.ClearItems();
		m_SubModContextMenu.AddItem( "sub_grant", "Grant Temporary VIP", JMConstants.Lucide( "star" ), 0, false, this, "OnSubmenu_GrantVip" );
		m_SubModContextMenu.AddItem( "sub_revoke", "Revoke Temporary VIP", JMConstants.Lucide( "ban" ), JMTheme.DANGER, false, this, "OnSubmenu_RevokeVip" );

		int mx, my;
		GetMousePos( mx, my );
		m_SubModContextMenu.ShowAt( mx, my );
	}

	void OnSubmenu_GrantVip( string itemId )
	{
		if ( m_Module )
			COTCreateLocalAdminNotification( new StringLocaliser( "Granted VIP in sub-menu!" ), JMConstants.Lucide( "star" ) );
	}

	void OnSubmenu_RevokeVip( string itemId )
	{
		if ( m_Module )
			COTCreateLocalAdminNotification( new StringLocaliser( "Revoked VIP in sub-menu!" ), JMConstants.Lucide( "ban" ) );
	}
}
#endif
