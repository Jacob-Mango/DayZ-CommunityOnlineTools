#ifdef JM_CommunityOnlineTools
// Example: adding entries to the player list's right-click menu. Each item can name its own
// handler (target + function taking the item id), so no id-comparison chain is needed.
// Pass `true` as the 5th AddItem argument to mark an entry as opening a sub-menu (trailing chevron).
modded class JMPlayerForm
{
	override void OnPlayerRow_RightClick( string guid, int x, int y )
	{
		super.OnPlayerRow_RightClick( guid, x, y );

		if ( !m_PlayerMenu )
			return;

		m_PlayerMenu.AddItem( "ex_quick", "Sub-Mod Quick Action", JMConstants.Lucide( "sparkles" ), 0, false, this, "OnExQuickAction" );
		m_PlayerMenu.AddItem( "ex_danger", "Sub-Mod Danger Action", JMConstants.Lucide( "ban" ), JMTheme.DANGER, false, this, "OnExDangerAction" );

		// Grey out an entry the viewer may not use.
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
