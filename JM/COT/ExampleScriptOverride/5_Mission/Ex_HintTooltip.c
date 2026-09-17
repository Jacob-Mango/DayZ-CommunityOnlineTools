#ifdef JM_CommunityOnlineTools
// Example: Adding tooltips and hints to UI controls
modded class JMPlayerForm
{
	protected void InitControlsWithTooltips( Widget parent )
	{
		UIActionButton button = UIActionManager.CreateButton( parent, "Action Button", this, "OnClick_DemoTooltip" );

		// 1. Static tooltip string
		button.SetTooltip( "Click this button to trigger a sub-mod action." );

		UIActionText textControl = UIActionManager.CreateText( parent, "Info Label", "Hover over me" );

		// 2. Localization stringtable key for tooltip
		textControl.SetTooltip( "#STR_COT_GENERIC_SEARCH" );
	}

	void OnClick_DemoTooltip( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		action.AnimateFeedback();
	}
}
#endif
