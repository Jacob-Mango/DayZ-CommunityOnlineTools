class JMLoadoutForm: JMFormBase
{
	private UIActionScroller m_sclr_MainActions;
	private Widget m_ContentWrapper;
	private Widget m_ActionsWrapper;

	protected UIActionDropdown m_SpawnModeDropdown;
	protected ref array< string > m_SpawnModeText =
	{
		"#STR_COT_OBJECT_MODULE_CROSSHAIR",
		"#STR_COT_OBJECT_MODULE_TARGET",
		"#STR_COT_OBJECT_MODULE_SELECTED_PLAYERS"
	};

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMLoadoutModule m_Module;

	JMLoadoutButtonData m_TempData;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_sclr_MainActions = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		m_ContentWrapper = m_sclr_MainActions.GetContentWidget();

		// Toolbar: 32x32 refresh icon on the left, dropdown fills the remaining row width.
		// WrapSpacer, not GridSpacer - GridSpacer(1,2) splits the row into equal halves.
		// Fractional < 1.0 on the dropdown keeps the row tight at narrow panel widths.
		Widget toolbar = UIActionManager.CreateWrapSpacer( m_ContentWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		UIActionImageButton refreshBtn = UIActionManager.CreateRefreshButton( toolbar, this, "OnClick_Refresh", "#STR_COT_GENERIC_REFRESH" );
		refreshBtn.SetFixedSize( ICON_BUTTON_PX, ICON_BUTTON_PX );

		m_SpawnModeDropdown = UIActionManager.CreateDropdown( toolbar, "", layoutRoot, this, "OnClick_SpawnMode", m_SpawnModeText );
		m_SpawnModeDropdown.SetWidth( 0.85 );
		m_SpawnModeDropdown.SetSelection( 0, false );
		RegisterOverlay( m_SpawnModeDropdown );

		m_sclr_MainActions.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_sclr_MainActions )
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

			// WrapSpacer so all three siblings pack on the same row.
			Widget row = UIActionManager.CreateWrapSpacer( m_ActionsWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			// Icon-only delete (32x32). Compact confirm labels because the button
			// is narrow - full "Confirm"/"Cancel" would clip on small widths.
			UIActionConfirmInline delbttn = UIActionManager.CreateConfirmInline( row, "", this, "OnClick_Delete" );
			UIActionIconGrid.ApplyDeletePreset( delbttn );
			delbttn.SetButton( "" );
			delbttn.SetFixedSize( ICON_BUTTON_PX, ICON_BUTTON_PX );
			delbttn.CenterIcon( ICON_BUTTON_PX, 16 );
			delbttn.SetConfirmLabel( "O" );
			delbttn.SetCancelLabel( "X" );
			delbttn.SetTooltip( "Delete this loadout" );
			delbttn.SetData( new JMLoadoutButtonData( name ) );

			// Fractional Spawn directly after the delete icon.
			UIActionButton spwnbttn = UIActionManager.CreateButton( row, "Spawn", this, "OnClick_Spawn" );
			spwnbttn.SetWidth( 0.20 );
			spwnbttn.SetData( new JMLoadoutButtonData( name ) );

			// Loadout name fills the rest of the row.
			UIActionText nameText = UIActionManager.CreateText( row, "", name );
			nameText.SetWidth( 0.74 );
			nameText.SetTextVAlign( UIActionVAlign.CENTER );
		}

		m_sclr_MainActions.UpdateScroller();
	}
	
	void OnClick_Delete( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
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

		UIActionButton btn;
		if ( Class.CastTo( btn, action ) )
			btn.TriggerSpin( 2 );

		m_Module.Load();

		OnSettingsUpdated();
	}

	void OnClick_SpawnMode( UIEvent eid, UIActionBase action )
	{
	}

	void OnClick_Spawn( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMLoadoutButtonData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		int spawnMode = m_SpawnModeDropdown.GetSelection();
		if ( spawnMode < 0 )
			spawnMode = COT_LoadoutSpawnMode.CURSOR;

		switch ( spawnMode )
		{
			case COT_LoadoutSpawnMode.CURSOR:
				m_Module.SpawnCursor( data.Filename, GetCursorPos() );
			break;
			case COT_LoadoutSpawnMode.TARGET:
				EntityAI ent;
				if ( Class.CastTo( ent, CF_Modules<JMObjectSpawnerModule>.Get().GetObjectAtCursor() ) )
					m_Module.SpawnTarget( data.Filename, ent );
			break;
			case COT_LoadoutSpawnMode.PLAYER:
				m_Module.SpawnPlayers( data.Filename, JM_GetSelected().GetPlayersOrSelf() );
			break;
			default:
				CreateConfirmation_One( JMConfirmationType.INFO, "Unknown spawn mode", "Selected spawn mode is not handled. Re-select the mode and try again.", "#STR_COT_GENERIC_OK", "" );
			break;
		}
	}
}
