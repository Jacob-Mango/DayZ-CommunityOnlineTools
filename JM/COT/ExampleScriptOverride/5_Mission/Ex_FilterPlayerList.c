#ifdef JM_CommunityOnlineTools
// Example: a checkbox row in the Player list's filter menu. One Register() call adds the row
// (re-registering the same id replaces it, so OnCreate can run more than once); the state
// callback makes it a checkbox, PassesListFilters decides which players the list shows.
// The same call works for every filter menu - only the scope changes (see JMFilterRegistry).
modded class JMPlayerForm
{
	protected bool m_ExOnlyAdmins;

	// Drives the row's checkbox.
	bool IsExFilterAdmins()
	{
		return m_ExOnlyAdmins;
	}

	override void OnCreate()
	{
		super.OnCreate();

		JMFilterRegistry.Register( JMFilterRegistry.PLAYERS, "ex_only_admins", "Only Admins (example)", "", 0, this, "OnExFilterAdmins", "IsExFilterAdmins" );
	}

	// Called with the id of the clicked row.
	void OnExFilterAdmins( string id )
	{
		m_ExOnlyAdmins = !m_ExOnlyAdmins;
		UpdatePlayerList( true );
	}

	override protected bool PassesListFilters( JMPlayerInstance player )
	{
		if ( m_ExOnlyAdmins && !player.HasPermission( "COT" ) )
			return false;

		return super.PassesListFilters( player );
	}
}
#endif
