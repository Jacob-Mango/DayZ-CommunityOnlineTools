#ifdef JM_CommunityOnlineTools
// Example: tooltips. SetTooltip takes plain text or a #STR_ stringtable key,
// on any control.
modded class JMPlayerForm
{
	override void OnCreate()
	{
		super.OnCreate();

		AddTab( "Tips", JMConstants.Lucide( "info" ), "BuildTooltipTab" );
	}

	void BuildTooltipTab( Widget parentPanel )
	{
		Widget body = UIActionManager.CreateSection( parentPanel, "Tooltips" );
		if ( !body )
			return;

		UIActionButton button = UIActionManager.CreateButton( body, "Hover me", null, "" );
		button.SetTooltip( "Plain text tooltip." );

		UIActionText text = UIActionManager.CreateText( body, "Info:", "Hover me too" );
		text.SetTooltip( "#STR_COT_GENERIC_SEARCH" );
	}
}
#endif
