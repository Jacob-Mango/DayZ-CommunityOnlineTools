#ifdef JM_CommunityOnlineTools
// Example: Adding custom filter entry for Player List (JMPlayerForm)
modded class JMPlayerForm
{
	protected bool m_FilterShowSubModVip = true;

	override protected void OnClick_PlayerListFilters( UIEvent eid, UIActionBase action )
	{
		super.OnClick_PlayerListFilters( eid, action );

		if ( !m_FilterMenu )
			return;

		// Append custom filter toggle to player list filter dropdown menu
		m_FilterMenu.AddItem( "f_vip", "Show VIP Players Only", JMConstants.Lucide( "star" ) );
		
		int color = JMTheme.TEXT_DISABLED;
		if ( m_FilterShowSubModVip )
			color = JMTheme.TEXT_PRIMARY;

		m_FilterMenu.SetItemTextColor( "f_vip", color );

	}

	override protected bool PassesListFilters( JMPlayerInstance player )
	{
		if ( !super.PassesListFilters( player ) )
			return false;

		// Apply custom filter condition
		if ( !m_FilterShowSubModVip )
		{
			// Example filter logic: exclude specific player group
		}

		return true;
	}
}
#endif
