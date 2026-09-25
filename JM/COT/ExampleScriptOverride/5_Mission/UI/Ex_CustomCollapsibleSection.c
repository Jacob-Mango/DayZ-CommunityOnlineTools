#ifdef JM_CommunityOnlineTools
// Example: creating collapsible section cards using UIActionManager.
modded class JMCustomExampleForm
{
	protected ref UIActionCollapsibleSection m_ExCollapsible;

	override protected void BuildCustomSection( Widget parent )
	{
		if ( !parent )
			return;

		m_ExCollapsible = UIActionManager.CreateCollapsibleSection( parent, "Sub-Mod Advanced Options" );
		if ( m_ExCollapsible )
		{
			UIActionManager.CreateButton( m_ExCollapsible.GetContainer(), "Sub-Mod Action", this, "OnExSubAction" );
		}
	}

	void OnExSubAction( UIActionBase action )
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "Collapsible section action executed" ), JMConstants.Lucide( "check" ) );
	}
}
#endif
