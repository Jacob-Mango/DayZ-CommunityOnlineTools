class JMLoadoutForm: JMFormBase
{
	private UIActionScroller m_sclr_MainActions;
	private Widget m_ContentWrapper;
	private Widget m_ActionsWrapper;

	protected UIActionSelectBox m_SpawnModeSelect;
	protected ref array< string > m_SpawnModeText =
	{
		"#STR_COT_OBJECT_MODULE_CURSOR",
		"#STR_COT_OBJECT_MODULE_TARGET",
		"#STR_COT_OBJECT_MODULE_SELECTED_PLAYERS"
	};

	private JMLoadoutModule m_Module;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		m_ContentWrapper = m_sclr_MainActions.GetContentWidget();

		Widget buttonswrapper = UIActionManager.CreateGridSpacer( m_ContentWrapper, 1, 2 );

			UIActionManager.CreateButton( buttonswrapper, "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_VARIABLES_REFRESH", this, "OnClick_Refresh" );
			
			m_SpawnModeSelect = UIActionManager.CreateSelectionBox( buttonswrapper, "", m_SpawnModeText, this );
			m_SpawnModeSelect.SetSelectorWidth(1.0);
			m_SpawnModeSelect.SetSelection(0, false);
		
		m_sclr_MainActions.UpdateScroller();
	}

	override void OnShow()
	{
		if (m_Module.IsLoaded())
			OnSettingsUpdated();
		else
			m_Module.Load();
	}

	override void OnSettingsUpdated()
	{
		if (!m_Module.IsLoaded())
			return;
		
		if (m_ActionsWrapper)
			delete m_ActionsWrapper;

		m_ActionsWrapper = UIActionManager.CreateGridSpacer( m_ContentWrapper, 1, 1 );

		array< string > names = new array< string >;
		names.Copy( m_Module.GetLoadouts() );
		
		JMStatics.SortStringArray( names );

		foreach(string name: names)
		{
			if ( name == string.Empty )
				continue;

			Widget wrapper = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 2 );

			UIActionManager.CreateText( wrapper, name );
			
				Widget bttnwrapper = UIActionManager.CreateGridSpacer( wrapper, 1, 2 );

					UIActionButton buttn = UIActionManager.CreateButton( bttnwrapper, "#STR_COT_GENERIC_SPAWN", this, "OnClick_Spawn" );
					buttn.SetData( new JMLoadoutButtonData( name ) );
				
					UIActionManager.CreateButton( bttnwrapper, "#STR_COT_OBJECT_MODULE_DELETE", this, "OnClick_Delete" );
		}

		m_sclr_MainActions.UpdateScroller();
	}
	
	void OnClick_Delete( UIEvent eid, UIActionBase action ) 
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		JMLoadoutButtonData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Module.Delete( data.Filename );

		m_Module.Load();

		OnSettingsUpdated();
	}
	
	void OnClick_Refresh( UIEvent eid, UIActionBase action ) 
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.Load();

		OnSettingsUpdated();
	}

	void OnClick_Spawn( UIEvent eid, UIActionBase action ) 
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		JMLoadoutButtonData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		switch(m_SpawnModeSelect.GetSelection())
		{
			case COT_LoadoutSpawnMode.CURSOR:
				m_Module.SpawnCursor( data.Filename, GetCursorPos() );
			break;
			case COT_LoadoutSpawnMode.TARGET:
				//m_Module.SpawnOnTarget( data.Filename, GetCursorPos() );
			break;
			case COT_LoadoutSpawnMode.PLAYER:
				m_Module.SpawnPlayers( data.Filename, JM_GetSelected().GetPlayers() );
			break;
		}
	}
};
