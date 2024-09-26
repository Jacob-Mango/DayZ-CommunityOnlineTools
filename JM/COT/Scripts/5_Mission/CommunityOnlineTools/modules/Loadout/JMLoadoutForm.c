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
	
	JMLoadoutButtonData m_TempData;

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

		if (names.Count() < 1)
		{
			UIActionManager.CreateText( m_ActionsWrapper, "Use the ESP to create new Sets from the quick action tab (right side)" );
			m_sclr_MainActions.UpdateScroller();
			return;
		}
		
		JMStatics.SortStringArray( names );

		foreach(string name: names)
		{
			if ( name == string.Empty )
				continue;

			Widget wrapper = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 2 );

			UIActionManager.CreateText( wrapper, name );
			
				Widget bttnwrapper = UIActionManager.CreateGridSpacer( wrapper, 1, 2 );

					UIActionButton spwnbttn = UIActionManager.CreateButton( bttnwrapper, "#STR_COT_GENERIC_SPAWN", this, "OnClick_Spawn" );
					spwnbttn.SetData( new JMLoadoutButtonData( name ) );
				
					UIActionButton delbttn = UIActionManager.CreateButton( bttnwrapper, "#STR_COT_OBJECT_MODULE_DELETE", this, "OnClick_Delete" );
					delbttn.SetData( new JMLoadoutButtonData( name ) );
					delbttn.SetColor(COLOR_RED);
		}

		m_sclr_MainActions.UpdateScroller();
	}
	
	void OnClick_Delete( UIEvent eid, UIActionBase action ) 
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( !Class.CastTo( m_TempData, action.GetData() ) )
			return;
	
		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_GENERIC_CONFIRM", string.Format( Widget.TranslateString( "#STR_COT_LOADOUT_MODULE_LOADOUT_CONFIRMATION_BODY" ), m_TempData.Filename ), "#STR_COT_GENERIC_NO", "", "#STR_COT_GENERIC_YES", "OnClick_DeleteConfirm" );
	}

	void OnClick_DeleteConfirm(JMConfirmation confirmation) 
	{
		m_Module.Delete( m_TempData.Filename );

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
				EntityAI ent = m_Module.GetObjectAtCursor();
				if (ent)
					m_Module.SpawnTarget( data.Filename, ent );
			break;
			case COT_LoadoutSpawnMode.PLAYER:
				m_Module.SpawnPlayers( data.Filename, JM_GetSelected().GetPlayersOrSelf() );
			break;
		}
	}
};
