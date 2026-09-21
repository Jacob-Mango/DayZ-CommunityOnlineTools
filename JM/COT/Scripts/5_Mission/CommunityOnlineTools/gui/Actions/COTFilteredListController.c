class COTFilteredListController
{
	protected ref array<Class> m_AllItems;
	protected ref array<Class> m_FilteredItems;
	protected ref array<Class> m_SelectedItems;
	protected TextWidget m_CountLabel;
	protected string m_SearchQuery;
	protected bool m_MultiSelect;
	protected Class m_CallbackInstance;
	protected string m_CallbackFunction;

	void COTFilteredListController()
	{
		m_AllItems = new array<Class>();
		m_FilteredItems = new array<Class>();
		m_SelectedItems = new array<Class>();
		m_SearchQuery = "";
		m_MultiSelect = false;
	}

	void ~COTFilteredListController()
	{
		delete m_AllItems;
		delete m_FilteredItems;
		delete m_SelectedItems;
	}

	array<Class> GetAllItems()
	{
		return m_AllItems;
	}

	array<Class> GetFilteredItems()
	{
		return m_FilteredItems;
	}

	string GetSearchQuery()
	{
		return m_SearchQuery;
	}

	array<Class> GetSelectedItems()
	{
		return m_SelectedItems;
	}

	Class GetSingleSelected()
	{
		if ( m_SelectedItems.Count() > 0 )
			return m_SelectedItems.Get( 0 );

		return null;
	}

	bool IsMultiSelect()
	{
		return m_MultiSelect;
	}

	bool IsSelected( Class item )
	{
		if ( !item )
			return false;

		int idx = m_SelectedItems.Find( item );
		if ( idx != -1 )
			return true;

		return false;
	}

	void SetCallback( Class instance, string functionName )
	{
		m_CallbackInstance = instance;
		m_CallbackFunction = functionName;
	}

	void SetCountLabel( TextWidget label )
	{
		m_CountLabel = label;
		UpdateCountLabel();
	}

	void SetItems( array<Class> items )
	{
		m_AllItems.Clear();
		if ( items )
		{
			foreach ( Class item : items )
			{
				if ( item )
					m_AllItems.Insert( item );
			}
		}

		ApplyFilter();
	}

	void SetMultiSelect( bool enable )
	{
		m_MultiSelect = enable;
	}

	void SetSearchQuery( string query )
	{
		query.ToLower();
		m_SearchQuery = query;
		ApplyFilter();
	}

	void AddItem( Class item )
	{
		if ( !item )
			return;

		m_AllItems.Insert( item );
		ApplyFilter();
	}

	void ClearItems()
	{
		m_AllItems.Clear();
		m_FilteredItems.Clear();
		m_SelectedItems.Clear();
		UpdateCountLabel();
	}

	void ApplyFilter()
	{
		m_FilteredItems.Clear();

		if ( m_SearchQuery == "" )
		{
			foreach ( Class allItem : m_AllItems )
			{
				m_FilteredItems.Insert( allItem );
			}
		}
		else
		{
			foreach ( Class itemToFilter : m_AllItems )
			{
				if ( MatchesFilter( itemToFilter, m_SearchQuery ) )
					m_FilteredItems.Insert( itemToFilter );
			}
		}

		UpdateCountLabel();
		NotifyCallback();
	}

	protected bool MatchesFilter( Class item, string query )
	{
		if ( !item )
			return false;

		string itemStr = item.ToString();
		itemStr.ToLower();

		if ( itemStr.Contains( query ) )
			return true;

		return false;
	}

	void SelectItem( Class item, bool selected = true )
	{
		if ( !item )
			return;

		if ( !m_MultiSelect )
		{
			m_SelectedItems.Clear();
			if ( selected )
				m_SelectedItems.Insert( item );
		}
		else
		{
			int foundIdx = m_SelectedItems.Find( item );
			if ( selected )
			{
				if ( foundIdx == -1 )
					m_SelectedItems.Insert( item );
			}
			else
			{
				if ( foundIdx != -1 )
					m_SelectedItems.Remove( foundIdx );
			}
		}

		NotifyCallback();
	}

	void SelectAllFiltered()
	{
		if ( !m_MultiSelect )
			return;

		foreach ( Class filteredItem : m_FilteredItems )
		{
			if ( m_SelectedItems.Find( filteredItem ) == -1 )
				m_SelectedItems.Insert( filteredItem );
		}

		NotifyCallback();
	}

	void ClearSelection()
	{
		m_SelectedItems.Clear();
		NotifyCallback();
	}

	protected void UpdateCountLabel()
	{
		if ( !m_CountLabel )
			return;

		string countText = "" + m_FilteredItems.Count() + " / " + m_AllItems.Count();
		m_CountLabel.SetText( countText );
	}

	protected void NotifyCallback()
	{
		if ( m_CallbackInstance && m_CallbackFunction != "" )
		{
			g_Game.GameScript.CallFunction( m_CallbackInstance, m_CallbackFunction, null, this );
		}
	}
}
