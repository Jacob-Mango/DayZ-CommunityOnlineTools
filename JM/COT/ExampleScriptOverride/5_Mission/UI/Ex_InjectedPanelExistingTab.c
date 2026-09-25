#ifdef JM_CommunityOnlineTools
// Example: injecting a card into a tab that already exists (Player > General), with a
// header refresh button and a permission-gated action.
modded class JMPlayerForm
{
	protected UIActionCard m_InjectedCard;

	override protected Widget InitActionWidgetsIdentity( Widget actionsParent )
	{
		Widget parent = super.InitActionWidgetsIdentity( actionsParent );

		m_InjectedCard = UIActionManager.CreateCard( parent, "Injected Sub-Mod Panel" );
		if ( !m_InjectedCard )
			return parent;

		m_InjectedCard.AddRefreshButton( this, "OnClick_ExInjectedRefresh", "Refresh injected data" );

		Widget body = m_InjectedCard.GetContent();

		UIActionManager.CreateText( body, "Injected:", "Added without replacing the layout." );

		UIActionButton button = UIActionManager.CreateButton( body, "Execute Panel Action", null, "" );
		button.SetOnClick( this, "OnExInjectedAction" );
		BindPermission( button, JMConstants.PERM_PLAYER_INJECTED_PANEL );

		return parent;
	}

	void OnClick_ExInjectedRefresh( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
			action.AnimateSpin();
	}

	void OnExInjectedAction( UIActionBase action )
	{
		action.AnimateFeedback();
	}
}
#endif
