#ifdef JM_CommunityOnlineTools
// Example: Adding a custom tab with its own lazily-built panels and content
modded class JMPlayerForm
{
	protected UIActionCard m_CustomTabCard;

	override void OnInit()
	{
		super.OnInit();

		// Clean 1-line tab registration: Label, Icon, Callback function name
		AddTab( "SubMod", JMConstants.Lucide( "star" ), "BuildCustomTabContent" );
	}

	// Invoked lazily the FIRST time the user clicks on the "SubMod" tab
	void BuildCustomTabContent( Widget parentPanel )
	{
		if ( !parentPanel )
			return;

		UIActionManager.CreatePanel( parentPanel, 0x00000000, 10 );

		m_CustomTabCard = UIActionManager.CreateCard( parentPanel, "Custom Tab Settings" );
		if ( !m_CustomTabCard )
			return;

		Widget body = m_CustomTabCard.GetContent();
		if ( !body )
			return;

		UIActionManager.CreateText( body, "Status:", "This tab and its widgets were created lazily!" );
		UIActionButton actionBtn = UIActionManager.CreateButton( body, "Tab Action", this, "OnClick_CustomTabAction" );

		// Bind permission constant
		BindPermission( actionBtn, JMConstants.PERM_PLAYER_INJECTED_PANEL );
	}

	void OnClick_CustomTabAction( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateFeedback();
	}
}
#endif
