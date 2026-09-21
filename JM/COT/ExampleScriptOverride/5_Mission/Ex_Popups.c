#ifdef JM_CommunityOnlineTools
// Example: popups owned by a form - a slider prompt and a menu opened at the cursor.
// CreateOverlayPrompt / CreateOverlayMenu build the popup anchored to the window and register
// it as an overlay, so switching tab or closing the form dismisses it. Never call InitPrompt().
modded class JMPlayerForm
{
	protected UIActionValuePrompt m_ExPrompt;
	protected UIActionContextMenu m_ExMenu;

	override void OnCreate()
	{
		super.OnCreate();

		AddTab( "Popups", JMConstants.Lucide( "message-square" ), "BuildPopupsTab" );
	}

	void BuildPopupsTab( Widget parentPanel )
	{
		Widget body = UIActionManager.CreateSection( parentPanel, "Popups" );
		if ( !body )
			return;

		UIActionManager.CreateButton( body, "Open slider prompt", null, "" ).SetOnClick( this, "OnExOpenPrompt" );
		UIActionManager.CreateButton( body, "Open menu", null, "" ).SetOnClick( this, "OnExOpenMenu" );

		m_ExPrompt = UIActionManager.CreateOverlayPrompt( this, this, "OnExPromptConfirm" );
		m_ExMenu = UIActionManager.CreateOverlayMenu( this, this, "OnExMenuPick" );
	}

	void OnExOpenPrompt( UIActionBase action )
	{
		if ( m_ExPrompt )
			m_ExPrompt.OpenSlider( "ex_value", "Set Custom Value", "Value", 1, 100, 50 );
	}

	void OnExPromptConfirm( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || m_ExPrompt.GetPromptId() != "ex_value" )
			return;

		COTCreateLocalAdminNotification( new StringLocaliser( "Chosen value: " + m_ExPrompt.GetSliderValue() ), JMConstants.Lucide( "check" ) );
	}

	void OnExOpenMenu( UIActionBase action )
	{
		if ( !m_ExMenu )
			return;

		m_ExMenu.ClearItems();
		m_ExMenu.AddItem( "grant", "Grant", JMConstants.Lucide( "star" ) );
		m_ExMenu.AddItem( "revoke", "Revoke", JMConstants.Lucide( "ban" ), JMTheme.DANGER );
		m_ExMenu.OpenAtMouse();
	}

	void OnExMenuPick( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
			COTCreateLocalAdminNotification( new StringLocaliser( "Picked: " + m_ExMenu.GetLastClickedId() ), JMConstants.Lucide( "check" ) );
	}
}
#endif
