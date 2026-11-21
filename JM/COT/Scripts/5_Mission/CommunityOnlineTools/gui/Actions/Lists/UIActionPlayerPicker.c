class UIActionPlayerPicker
{
	protected ref COTFilteredListController m_ListController;
	protected ref array<JMPlayerInstance> m_AllPlayers;
	protected ref array<JMPlayerInstance> m_SelectedPlayers;
	protected Class m_CallbackInstance;
	protected string m_CallbackFunction;

	void UIActionPlayerPicker()
	{
		m_ListController = new COTPlayerListController();
		m_ListController.SetMultiSelect( true );
		m_ListController.SetCallback( this, "OnListSelectionChanged" );

		m_AllPlayers = new array<JMPlayerInstance>();
		m_SelectedPlayers = new array<JMPlayerInstance>();
	}

	COTFilteredListController GetListController()
	{
		return m_ListController;
	}

	array<string> GetSelectedGUIDs()
	{
		array<string> guids = new array<string>();
		array<JMPlayerInstance> players = GetSelectedPlayers();

		foreach ( JMPlayerInstance player : players )
		{
			if ( player )
				guids.Insert( player.GetGUID() );
		}

		return guids;
	}

	array<JMPlayerInstance> GetSelectedPlayers()
	{
		m_SelectedPlayers.Clear();
		array<Class> selectedClasses = m_ListController.GetSelectedItems();

		foreach ( Class item : selectedClasses )
		{
			JMPlayerInstance inst;
			if ( Class.CastTo( inst, item ) && inst )
			{
				m_SelectedPlayers.Insert( inst );
			}
		}

		return m_SelectedPlayers;
	}

	bool IsMultiSelect()
	{
		return m_ListController.IsMultiSelect();
	}

	void SetCallback( Class instance, string functionName )
	{
		m_CallbackInstance = instance;
		m_CallbackFunction = functionName;
	}

	void SetMultiSelect( bool enable )
	{
		m_ListController.SetMultiSelect( enable );
	}

	void SetSearchQuery( string query )
	{
		m_ListController.SetSearchQuery( query );
	}

	void RefreshPlayers()
	{
		m_AllPlayers.Clear();

		if ( GetPermissionsManager() )
		{
			array<JMPlayerInstance> permissionsPlayers = GetPermissionsManager().GetPlayers();
			if ( permissionsPlayers )
			{
				foreach ( JMPlayerInstance p : permissionsPlayers )
				{
					if ( p )
						m_AllPlayers.Insert( p );
				}
			}
		}

		array<Class> classItems = new array<Class>();
		foreach ( JMPlayerInstance playerInst : m_AllPlayers )
		{
			classItems.Insert( playerInst );
		}

		m_ListController.SetItems( classItems );
	}

	void SelectAll()
	{
		m_ListController.SelectAllFiltered();
	}

	void DeselectAll()
	{
		m_ListController.ClearSelection();
	}

	void SelectSelf()
	{
		m_ListController.ClearSelection();

		if ( !g_Game || !g_Game.GetPlayer() )
			return;

		PlayerIdentity identity = g_Game.GetPlayer().GetIdentity();
		if ( !identity )
			return;

		string localGuid = identity.GetId();

		foreach ( JMPlayerInstance player : m_AllPlayers )
		{
			if ( player && player.GetGUID() == localGuid )
			{
				m_ListController.SelectItem( player, true );
				break;
			}
		}
	}

	void SelectOnlineOnly()
	{
		m_ListController.ClearSelection();

		foreach ( JMPlayerInstance onlinePlayer : m_AllPlayers )
		{
			if ( onlinePlayer && onlinePlayer.PlayerObject )
			{
				m_ListController.SelectItem( onlinePlayer, true );
			}
		}
	}

	protected void OnListSelectionChanged( COTFilteredListController controller )
	{
		if ( m_CallbackInstance && m_CallbackFunction != "" )
		{
			g_Game.GameScript.CallFunction( m_CallbackInstance, m_CallbackFunction, null, this );
		}
	}
}
