#ifdef JM_CommunityOnlineTools
// Example: adding custom admin toggles to the player general tab (JMPlayerFormTabGeneral).
modded class JMPlayerFormTabGeneral
{
	override protected void BuildToggleRows()
	{
		super.BuildToggleRows();

		if ( !m_ActionsWrapper )
			return;

		UIActionToggle toggle = UIActionManager.CreateToggle( m_ActionsWrapper, "Sub-Mod Invisibility", this, "OnExToggleInvisibility" );
	}

	void OnExToggleInvisibility( UIActionBase action )
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "Toggled Sub-Mod Invisibility" ), JMConstants.Lucide( "eye-off" ) );
	}
}
#endif
