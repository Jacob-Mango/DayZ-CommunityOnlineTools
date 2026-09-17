#ifdef JM_CommunityOnlineTools
// Example: Creating a UIActionCard panel with header action buttons (Refresh / Apply / Delete / Save)
modded class JMPlayerForm
{
	protected UIActionCard m_HeaderButtonDemoCard;
	protected UIActionImageButton m_CardRefreshBtn;
	protected UIActionImageButton m_CardApplyBtn;
	protected UIActionImageButton m_CardDeleteBtn;

	protected void InitHeaderButtonsCardDemo( Widget parent )
	{
		// 1. Create UIActionCard container
		m_HeaderButtonDemoCard = UIActionManager.CreateCard( parent, "Card Title Bar Action Buttons" );
		if ( !m_HeaderButtonDemoCard )
			return;

		// 2. Add premade action buttons directly into card header strip (packed right-to-left)
		m_CardRefreshBtn = m_HeaderButtonDemoCard.AddRefreshButton( this, "OnClick_DemoCardRefresh", "Refresh section data" );
		m_CardApplyBtn   = m_HeaderButtonDemoCard.AddApplyButton( this, "OnClick_DemoCardApply", "Apply pending changes" );
		m_CardDeleteBtn  = m_HeaderButtonDemoCard.AddDeleteButton( this, "OnClick_DemoCardDelete", "Wipe section data" );

		// 3. Add custom header action button using Lucide icon
		UIActionImageButton customHeaderBtn = m_HeaderButtonDemoCard.AddCardHeaderAction( JMConstants.Lucide( "download" ), this, "OnClick_DemoCardExport", "Export configuration" );

		Widget cardContent = m_HeaderButtonDemoCard.GetContent();
		if ( cardContent )
		{
			UIActionManager.CreateText( cardContent, "Notice:", "Header action buttons sit inside the card title bar." );
		}
	}

	void OnClick_DemoCardRefresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateSpin(); // 360 degree spin animation
	}

	void OnClick_DemoCardApply( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateFeedback(); // Green checkmark flash
	}

	void OnClick_DemoCardDelete( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateFeedback();
	}

	void OnClick_DemoCardExport( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateFeedback();
	}
}
#endif
