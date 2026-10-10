#ifdef DIAG_DEVELOPER
// Example: adding entries to the player list right-click menu (JMPlayerForm).
// Uses JMContextMenuRegistry for declarative 1-line context menu item registration.
modded class JMPlayerForm
{
	void JMPlayerForm()
	{
		//! Register sub-mod player roster action items
		JMContextMenuRegistry.Register( "PlayerRoster", "ex_quick", "Sub-Mod Quick Action", JMConstants.Lucide( "sparkles" ), 0, false, this, "OnExQuickAction" );
		JMContextMenuRegistry.Register( "PlayerRoster", "ex_danger", "Sub-Mod Danger Action", JMConstants.Lucide( "ban" ), JMTheme.DANGER, false, this, "OnExDangerAction" );
	}

	override void OnPlayerRow_RightClick( string guid, int x, int y )
	{
		super.OnPlayerRow_RightClick( guid, x, y );

		if ( m_PlayerMenu )
			m_PlayerMenu.SetItemEnabled( "ex_danger", JMPermissions.Has( JMConstants.PERM_PLAYER_INJECTED_PANEL ) );
	}

	void OnExQuickAction( string itemId )
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "Quick action on " + m_PlayerMenuGUID ), JMConstants.Lucide( "sparkles" ) );
	}

	void OnExDangerAction( string itemId )
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "Danger action on " + m_PlayerMenuGUID ), JMConstants.Lucide( "ban" ) );
	}
}
#endif
