#ifdef JM_CommunityOnlineTools
// Example: adding a tab to an existing form. One AddTab() call; the callback runs lazily,
// the first time the tab is opened, and receives the empty panel to fill.
modded class JMPlayerForm
{
	override void OnCreate()
	{
		super.OnCreate();

		AddTab( "SubMod", JMConstants.Lucide( "star" ), "BuildSubModTab" );
	}

	void BuildSubModTab( Widget parentPanel )
	{
		Widget body = UIActionManager.CreateSection( parentPanel, "Custom Tab" );
		if ( !body )
			return;

		UIActionManager.CreateText( body, "Status:", "This tab was created lazily." );

		UIActionButton action = UIActionManager.CreateButton( body, "Tab Action", null, "" );
		action.SetOnClick( this, "OnSubModTabAction" );
		BindPermission( action, JMConstants.PERM_PLAYER_INJECTED_TAB );
	}

	void OnSubModTabAction( UIActionBase action )
	{
		action.AnimateFeedback();
	}
}
#endif
