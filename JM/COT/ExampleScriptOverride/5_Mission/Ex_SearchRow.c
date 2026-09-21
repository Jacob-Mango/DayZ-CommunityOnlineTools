#ifdef JM_CommunityOnlineTools
// Example: a search toolbar over a list. CreateSearchRow builds [refresh] [search box] in one
// call and JMSearchMatcher does the matching (case-insensitive, all words must match, `|` or
// `OR` for any). Prepare the matcher once per rebuild, then ask it about each row.
modded class JMPlayerForm
{
	protected UIActionSearchBox m_ExSearch;
	protected Widget m_ExSearchBody;
	protected Widget m_ExSearchList;

	override void OnCreate()
	{
		super.OnCreate();

		AddTab( "Search", JMConstants.Lucide( "search" ), "BuildSearchTab" );
	}

	void BuildSearchTab( Widget parentPanel )
	{
		m_ExSearchBody = UIActionManager.CreateSection( parentPanel, "Search" );
		if ( !m_ExSearchBody )
			return;

		JMSearchRow bar = UIActionManager.CreateSearchRow( m_ExSearchBody, "Search fruit", this, "OnExSearch", "", "", "OnExSearchRefresh" );
		m_ExSearch = bar.Search;

		RebuildExSearchList();
	}

	void OnExSearch( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CHANGE )
			RebuildExSearchList();
	}

	void OnExSearchRefresh( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
			RebuildExSearchList();
	}

	protected void RebuildExSearchList()
	{
		if ( m_ExSearchList )
			delete m_ExSearchList;

		m_ExSearchList = UIActionManager.CreateGridSpacer( m_ExSearchBody, 1, 1 );

		JMSearchMatcher matcher = new JMSearchMatcher( m_ExSearch.GetText() );

		array<string> fruit = { "Apple", "Banana", "Cherry", "Green apple", "Pineapple" };
		foreach ( string name : fruit )
		{
			if ( matcher.Matches( name ) )
				UIActionManager.CreateText( m_ExSearchList, name );
		}
	}
}
#endif
