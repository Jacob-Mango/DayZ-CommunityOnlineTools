#ifdef JM_CommunityOnlineTools
// Example: a card with action buttons in its title bar. Premade Refresh / Apply / Delete
// buttons pack right-to-left, and AddCardHeaderAction takes any Lucide icon.
modded class JMPlayerForm
{
	override void OnCreate()
	{
		super.OnCreate();

		AddTab( "Header", JMConstants.Lucide( "panel-top" ), "BuildHeaderButtonsTab" );
	}

	void BuildHeaderButtonsTab( Widget parentPanel )
	{
		UIActionCard card = UIActionManager.CreateCard( parentPanel, "Card Title Bar Buttons" );
		if ( !card )
			return;

		card.AddRefreshButton( this, "OnClick_DemoRefresh", "Refresh section data" );
		card.AddApplyButton( this, "OnClick_DemoApply", "Apply pending changes" );
		card.AddDeleteButton( this, "OnClick_DemoDelete", "Wipe section data" );
		card.AddCardHeaderAction( JMConstants.Lucide( "download" ), this, "OnClick_DemoExport", "Export configuration" );

		UIActionManager.CreateText( card.GetContent(), "Notice:", "The buttons sit in the card's title bar." );
	}

	void OnClick_DemoRefresh( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
			action.AnimateSpin(); // 360 degree spin
	}

	void OnClick_DemoApply( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
			action.AnimateFeedback(); // green check flash
	}

	void OnClick_DemoDelete( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
			action.AnimateFeedback();
	}

	void OnClick_DemoExport( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
			action.AnimateFeedback();
	}
}
#endif
