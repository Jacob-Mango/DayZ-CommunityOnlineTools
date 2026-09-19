#ifdef JM_CommunityOnlineTools
// Example: Injecting a custom panel/card into an ALREADY EXISTING tab (General tab on JMPlayerForm)
modded class JMPlayerForm
{
	protected UIActionCard m_InjectedCard;

	override Widget InitActionWidgetsIdentity( Widget actionsParent )
	{
		Widget parent = super.InitActionWidgetsIdentity( actionsParent );

		if ( !parent )
			return parent;

		// Inject a new card into the General/Identity tab's actions wrapper
		m_InjectedCard = UIActionManager.CreateCard( parent, "Injected Sub-Mod Panel" );
		if ( !m_InjectedCard )
			return parent;

		// Add header action buttons directly to the card title bar
		m_InjectedCard.AddRefreshButton( this, "OnClick_InjectedRefresh", "Refresh injected data" );

		Widget cardBody = m_InjectedCard.GetContent();
		if ( !cardBody )
			return parent;

		UIActionText text = UIActionManager.CreateText( cardBody, "Injected Section:", "Panel added to existing tab without replacing layout." );
		UIActionButton button = UIActionManager.CreateButton( cardBody, "Execute Panel Action", this, "OnClick_InjectedButton" );

		// Bind custom permission constant to control
		BindPermission( button, JMConstants.PERM_PLAYER_INJECTED_PANEL );

		return parent;
	}

	void OnClick_InjectedRefresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateSpin();
	}

	void OnClick_InjectedButton( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateFeedback();
	}
}
#endif
