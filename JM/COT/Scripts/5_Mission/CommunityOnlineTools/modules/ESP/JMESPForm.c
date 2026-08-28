class JMESPForm: JMFormBase
{
	private autoptr array< ref JMESPViewTypeWidget > m_ESPTypeList;
	private ref map<typename, JMESPViewTypeWidget> m_ESPTypeWidgetsByType;

	private UIActionScroller m_ESPListScroller;
	private Widget m_ESPListRows;

	private UIActionScroller m_ESPSelectedObjects;

	private UIActionDropdown m_slbx_PlayerSkeletons;
	private UIActionDropdown m_slbx_Skeletons_LineThickness;

	private UIActionButton m_btn_Toggle;
	
	private UIActionCheckbox m_chkbx_Refresh;
	private UIActionSlider m_sldr_Refresh;

	private UIActionSlider m_sldr_Radius;

	private UIActionCheckbox m_DisableSafetyCheckbox;

	private UIActionButton m_ExportButton;
	private UIActionDropdown m_ExportTypeList;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMESPModule m_Module;
	protected JMLoadoutModule m_LoadoutModule;

	protected UIActionTabs m_Tabs;
	protected Widget m_TabFiltersPanel;
	protected Widget m_TabSelectedPanel;

	static const int TAB_FILTERS  = 0;
	static const int TAB_SELECTED = 1;

	private UIActionSearchBox m_SearchBox;

	void JMESPForm()
	{
		m_ESPTypeList = new array< ref JMESPViewTypeWidget >;
		m_ESPTypeWidgetsByType = new map<typename, JMESPViewTypeWidget>;

		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Insert(OnESPViewTypeChanged);
	}

	void ~JMESPForm()
	{
		JMScriptInvokers.ESP_VIEWTYPE_CHANGED.Remove(OnESPViewTypeChanged);
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	private void ESPControls( Widget parent )
	{
		Widget mainSpacer = UIActionManager.CreateGridSpacer( parent, 5, 1 );

		// 4 cells: [Toggle btn] [2-row checkbox column]
		//          [AutoRefresh checkbox] [Refresh-rate slider]
		Widget quadSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 2, 2 );

		m_btn_Toggle = UIActionManager.CreateButton( quadSpacer, "#STR_COT_ESP_MODULE_TOGGLE", this, "OnClick_UpdateESP" );
		m_btn_Toggle.SetTooltip( "Toggle the ESP overlay on/off" );

		Widget checkboxesSpacer = UIActionManager.CreateGridSpacer( quadSpacer, 2, 1 );

		UIActionCheckbox chkClassName = UIActionManager.CreateCheckbox( checkboxesSpacer, "#STR_COT_ESP_MODULE_TOGGLE_CLASS_NAME", this, "OnClick_UseClassName", JMESPWidgetHandler.UseClassName );
		chkClassName.SetTooltip( "Show each object's config classname on the overlay" );
		m_DisableSafetyCheckbox = UIActionManager.CreateCheckbox( checkboxesSpacer, "#STR_COT_ESP_MODULE_TOGGLE_SAFETY", this, "OnClick_DisableSafety", m_Module.GetFilterSafetyState() );
		m_DisableSafetyCheckbox.SetTooltip( "Bypass the 'too many results' safety limit on filters" );

		m_chkbx_Refresh = UIActionManager.CreateCheckbox( quadSpacer, "#STR_COT_ESP_MODULE_TOGGLE_AUTO_REFRESH", this, "OnClick_UpdateAtRate", m_Module.GetState() == JMESPState.Update );
		m_chkbx_Refresh.SetTooltip( "Continuously refresh the tracked objects at the rate below" );
		m_sldr_Refresh = UIActionManager.CreateSlider( quadSpacer, "", 1.0, 10.0, this, "OnChange_UpdateRate" );
		m_sldr_Refresh.SetCurrent( m_Module.ESPUpdateTime );
		m_sldr_Refresh.SetFormat("#STR_COT_FORMAT_SECOND_LONG");
		m_sldr_Refresh.SetStepValue( 1.0 );

		Widget skeletonSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		array<string> skeletonOpts = {"#STR_COT_GENERIC_NONE", "#STR_COT_GENERIC_OTHERS", "#STR_COT_GENERIC_ALL"};
		m_slbx_PlayerSkeletons = UIActionManager.CreateDropdown( skeletonSpacer, "#STR_COT_ESP_MODULE_DRAW_PLAYER_SKELETONS", parent, this, "OnChange_PlayerSkeletons", skeletonOpts );
		RegisterOverlay( m_slbx_PlayerSkeletons );
		int idx = m_Module.GetDrawPlayerSkeletonsEnabled();
		if (idx)
			idx += m_Module.DrawPlayerSkeletonsIncludingMyself;
		m_slbx_PlayerSkeletons.SetSelection(idx, false);
		array<string> thickOpts = {"1", "2", "3", "4"};
		m_slbx_Skeletons_LineThickness = UIActionManager.CreateDropdown( skeletonSpacer, "#STR_COT_GENERIC_LINE_THICKNESS", parent, this, "OnChange_Skeleton_LineThickness", thickOpts );
		RegisterOverlay( m_slbx_Skeletons_LineThickness );
		m_slbx_Skeletons_LineThickness.SetSelection( m_Module.SkeletonLineThickness - 1 );

		Widget filterSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		m_sldr_Radius = UIActionManager.CreateSlider( filterSpacer, "#STR_COT_ESP_MODULE_RADIUS", 0, m_Module.GetMaxRadius(), this, "OnChange_Range" );
		m_sldr_Radius.SetCurrent( m_Module.ESPRadius );
		m_sldr_Radius.SetFormat("#STR_COT_FORMAT_METRE_LONG");
		m_sldr_Radius.SetStepValue( 10.0 );

		Widget searchSpacer = UIActionManager.CreateWrapSpacer( filterSpacer, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		m_SearchBox = UIActionManager.CreateSearchBox( searchSpacer, this, "OnChange_Filter", "#STR_COT_ESP_MODULE_CLASS_FILTER", m_Module.Filter );
		m_SearchBox.SetWidth( 1.0 );
		UIActionImageButton button = UIActionManager.CreateImageButton( searchSpacer, "set:dayz_gui image:icon_x", this, "Reset_Filter" );
		button.SetFixedSize( ICON_BUTTON_PX, ICON_BUTTON_PX );
	
		UIActionManager.CreatePanel( mainSpacer, 0xFF000000, 3 );

		Widget headingSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );
		UIActionManager.CreateText( headingSpacer, "#STR_COT_ESP_MODULE_FILTERS_HEADER", "" );
		UIActionManager.CreateText( headingSpacer, "#STR_COT_ESP_MODULE_ACTIONS_HEADER", "" );
	}

	private void ESPFilters( Widget parent )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::ESPFilters" );
		#endif

		m_ESPListScroller = UIActionManager.CreateScroller( parent );
		Widget container = m_ESPListScroller.GetContentWidget();

		Widget rowSelectors = UIActionManager.CreateGridSpacer( container, 1, 2 );
		UIActionButton btnEspSelAll   = UIActionManager.CreateButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL", this, "OnClick_ESPSelectAll" );
		btnEspSelAll.SetTooltip( "Enable every ESP category filter" );
		UIActionButton btnEspDeselAll = UIActionManager.CreateButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL", this, "OnClick_ESPDeselectAll" );
		btnEspDeselAll.SetTooltip( "Disable every ESP category filter" );

		m_ESPListRows = UIActionManager.CreateActionRows( container );

		int totalInContentRow = 100;
		int currentContentRow = 0;

		GridSpacerWidget gsw;
		auto viewTypes = m_Module.GetViewTypes();

		foreach (auto viewType: viewTypes)
		{
			if ( totalInContentRow >= 100 )
			{
				Class.CastTo( gsw, m_ESPListRows.FindAnyWidget( "Content_Row_0" + currentContentRow ) );
				gsw.Show( true );
				currentContentRow++;
				totalInContentRow = 0;
			}

			Widget rWidget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp_type_widget.layout", gsw );
			
			if ( !rWidget )
				continue;

			JMESPViewTypeWidget rScript;
			rWidget.GetScript( rScript );

			if ( !rScript )
				continue;

			rScript.Set( viewType );

			m_ESPTypeList.Insert( rScript );
			m_ESPTypeWidgetsByType[viewType.Type()] = rScript;
		}

		m_ESPListScroller.UpdateScroller();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::ESPFilters" );
		#endif
	}

	private void ESPSelectedObjects( Widget parent )
	{
		m_ESPSelectedObjects = UIActionManager.CreateScroller( parent );
		Widget container = m_ESPSelectedObjects.GetContentWidget();
		
		UIActionManager.CreateText(container,"Visible Items");
		Widget rowSelectors = UIActionManager.CreateGridSpacer( container, 1, 2 );
		UIActionButton btnSelAll   = UIActionManager.CreateButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL", this, "OnClick_SelectAll" );
		btnSelAll.SetTooltip( "Select every visible object in the list" );
		UIActionButton btnDeselAll = UIActionManager.CreateButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL", this, "OnClick_DeselectAll" );
		btnDeselAll.SetTooltip( "Clear the current selection" );

		UIActionManager.CreateText(container,"Selected Items");
	
		Widget rowExports = UIActionManager.CreateGridSpacer( container, 1, 2 );
		m_ExportButton = UIActionManager.CreateButton( rowExports, "#STR_COT_TO_CLIPBOARD", this, "OnClick_CopyToClipboard" );
		m_ExportButton.SetIcon( JMConstants.ICON_STACK );
		m_ExportButton.SetTooltip( "Copy the selected objects to the clipboard in the chosen format" );
		TStringArray exportChoices = {
			"Raw",
			"SpawnableTypes",
		#ifdef DZ_Expansion_Market
			"Exp Market",
		#endif
		#ifdef DZ_Expansion_Core
			"Exp Loadout",
		#endif
			"#STR_COT_LOADOUT_MODULE_NAME"
		};
		m_ExportTypeList = UIActionManager.CreateDropdown( rowExports, "", parent, this, "OnClick_ExportType", exportChoices );
		RegisterOverlay( m_ExportTypeList );

		Widget rowMisc = UIActionManager.CreateWrapSpacerFit( container );
		UIActionButton btnMove = UIActionManager.CreateButton( rowMisc, "Move To Cursor", this, "OnClick_MoveToCursor" );
		btnMove.SetTooltip( "Move every selected object to your cursor position" );
		UIActionConfirmInline delbtn = UIActionManager.CreateConfirmInline( rowMisc, "Delete", this, "OnClick_DeleteSelected" );
		UIActionIconGrid.ApplyDeletePreset( delbtn );
		delbtn.SetTooltip( "Delete every selected object from the world" );

		m_ESPSelectedObjects.UpdateScroller();
	}

	override void OnInit()
	{
		JMESPWidgetHandler.espMenu = this;

		ESPControls( layoutRoot.FindAnyWidget( "panel_top" ) );

		InitWidgetsBottom();

		Class.CastTo(m_LoadoutModule, GetModuleManager().GetModule(JMLoadoutModule));
	}

	//! Archetype B: the controls stay pinned on top, the two lists below become
	//! tabs. They used to be two UIPanel layouts created at runtime and
	//! hand-positioned to 50/50 - at 700px wide that left each of them 350px for
	//! a filter tree and an object list that both want the full width.
	protected void InitWidgetsBottom()
	{
		m_TabFiltersPanel  = layoutRoot.FindAnyWidget( "esp_filters_panel" );
		m_TabSelectedPanel = layoutRoot.FindAnyWidget( "esp_selected_panel" );

		ref array<string> tabLabels = { "#STR_COT_ESP_TAB_FILTERS", "#STR_COT_ESP_TAB_VISIBLE" };
		ref array<string> tabIcons  = { JMConstants.Lucide( "filter" ), JMConstants.Lucide( "eye" ) };

		m_Tabs = UIActionManager.CreateTabs( layoutRoot.FindAnyWidget( "panel_bottom_tabs" ), tabLabels, tabIcons, this, "OnChange_Tab" );

		m_Tabs.AddContent( m_TabFiltersPanel );
		m_Tabs.AddContent( m_TabSelectedPanel );

		InitTabState( 2 );

		m_Tabs.SetSelection( TAB_FILTERS, false );

		BuildTabIfNeeded( TAB_FILTERS );
	}

	private void BuildTabIfNeeded( int tabIdx )
	{
		if ( !ShouldBuildTab( tabIdx ) )
			return;

		switch ( tabIdx )
		{
			case TAB_FILTERS:  ESPFilters( m_TabFiltersPanel );          break;
			case TAB_SELECTED: ESPSelectedObjects( m_TabSelectedPanel ); break;
		}

		UpdateUI();
	}

	override int GetActiveTabIndex()
	{
		if ( !m_Tabs )
			return -1;

		return m_Tabs.GetSelection();
	}

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CloseAllOverlays();

		BuildTabIfNeeded( GetActiveTabIndex() );
	}

	override void OnResize( float w, float h )
	{
		if ( m_ESPListScroller )    m_ESPListScroller.UpdateScroller();
		if ( m_ESPSelectedObjects ) m_ESPSelectedObjects.UpdateScroller();
	}

	override void OnShow()
	{
		super.OnShow();

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateUI, 500, true );

		UpdateUI();
	}

	override void OnHide()
	{
		super.OnHide();

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( UpdateUI );
	}

	void DisableToggleableOptions()
	{
	}

	void OnESPViewTypeChanged(JMESPViewType viewType)
	{
		UpdateMaxRange();
	}

	void UpdateUI()
	{
		m_sldr_Radius.SetCurrent( m_Module.ESPRadius );
		m_sldr_Refresh.SetCurrent( m_Module.ESPUpdateTime );

		if ( m_Module.GetState() != JMESPState.Remove )
		{
			m_btn_Toggle.SetButton( "#STR_COT_ESP_MODULE_ACTION_CLEAR_ESP" );
			m_btn_Toggle.SetColor(JMTheme.DANGER_FILL);
		}
		else
		{
			m_btn_Toggle.SetColor(JMTheme.SUCCESS_FILL);
			if ( m_chkbx_Refresh.IsChecked() )
			{
				m_btn_Toggle.SetButton( "#STR_COT_ESP_MODULE_ACTION_SHOW_ESP" );

				m_sldr_Refresh.Enable();
			}
			else
			{
				m_btn_Toggle.SetButton( "#STR_COT_ESP_MODULE_ACTION_SHOW_ESP" );

				m_sldr_Refresh.SetColor(JMTheme.TEXT_PRIMARY);
				m_sldr_Refresh.Disable();
			}
		}
	}

	void UpdateList()
	{
		string closestMatch;

		string strSearch = m_SearchBox.GetText();

		if ( strSearch != "" )
		{
			TStringArray configs = new TStringArray;
			configs.Insert( CFG_VEHICLESPATH );
			configs.Insert( CFG_WEAPONSPATH );
			configs.Insert( CFG_MAGAZINESPATH );
			configs.Insert( CFG_NONAI_VEHICLES );

			strSearch.ToLower();

			for ( int nConfig = 0; nConfig < configs.Count(); nConfig++ )
			{
				string strConfigPath = configs.Get( nConfig );

				int nClasses = g_Game.ConfigGetChildrenCount( strConfigPath );

				int nClassStart = 0;
				if (nConfig == 0) nClassStart = 20;

				for ( int nClass = nClassStart; nClass < nClasses; nClass++ )
				{
					string strName;

					g_Game.ConfigGetChildName( strConfigPath, nClass, strName );

					int scope = g_Game.ConfigGetInt( strConfigPath + " " + strName + " scope" );

					if ( scope == 0 )
						continue;

					if ( !g_Game.ConfigIsExisting( strConfigPath + " " + strName + " model" ) )
						continue;

					string strNameLower = strName;

					strNameLower.ToLower();

					if ( strNameLower == strSearch )
					{
						closestMatch = strNameLower;
						break;  //! We can end the search here because we got a perfect match
					}
					else if ( strNameLower.IndexOf(strSearch) == 0 )
					{
						if (!closestMatch || strNameLower.Length() < closestMatch.Length())
							closestMatch = strNameLower;
					}
				}
			}
		}

		m_SearchBox.SetTextPreview(closestMatch);
	}

	void OnClick_UpdateESP( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_Module.GetState() != JMESPState.Remove )
		{
			m_Module.UpdateState( JMESPState.Remove );

			m_Module.Log( "Clearing ESP" );
		} else
		{
			if ( m_chkbx_Refresh.IsChecked() )
			{
				m_Module.UpdateState( JMESPState.Update );

				m_Module.Log( "ESP updating" );
			}
			else
			{
				m_Module.UpdateState( JMESPState.View );

				m_Module.Log( "Viewing ESP" );
			}
		}

		UpdateUI();
	}
	
	void OnChange_Filter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Module.Filter = action.GetText();
		UpdateList();
	}

	void Reset_Filter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_SearchBox.SetText("");
		m_Module.Filter = "";
		UpdateList();
	}

	void OnChange_UpdateRate( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		if ( m_sldr_Refresh.GetCurrent() > 0 )
			m_sldr_Refresh.SetColor(JMTheme.TEXT_PRIMARY);
		else
			m_sldr_Refresh.SetColor(JMTheme.DANGER);

		m_Module.ESPUpdateTime = action.GetCurrent();
	}

	void OnChange_Range( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		m_Module.ESPRadius = action.GetCurrent();
	}

	void OnClick_DisableSafety( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		m_Module.SetFilterSafetyState(action.IsChecked());
	}

	void UpdateMaxRange()
	{
		float maxRadius = m_Module.GetMaxRadius();
		if (m_Module.ESPRadius > maxRadius)
			m_Module.ESPRadius = maxRadius;
		m_sldr_Radius.SetMinMax(m_sldr_Radius.GetMin(), maxRadius);
		m_sldr_Radius.SetCurrent(m_Module.ESPRadius);
	}	

	void OnClick_UseClassName( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		JMESPWidgetHandler.UseClassName = action.IsChecked();
	}

	void OnClick_UpdateAtRate( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_Module.GetState() != JMESPState.Remove )
		{
			if ( action.IsChecked() )
			{
				m_sldr_Refresh.Enable();
				m_Module.UpdateState( JMESPState.Update );
			}
			else
			{
				m_sldr_Refresh.Disable();
				m_Module.UpdateState( JMESPState.View );
			}
		}
		
		UpdateUI();
	}
	
	void OnChange_PlayerSkeletons( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		int idx = action.GetSelection();
		if (idx > 0 && !m_ESPTypeWidgetsByType[JMESPViewTypePlayerAI].IsChecked())
			m_ESPTypeWidgetsByType[JMESPViewTypePlayer].SetChecked(true);

		m_Module.SetDrawPlayerSkeletonsEnabled(idx > 0);
		m_Module.DrawPlayerSkeletonsIncludingMyself = idx > 1;
	}

	void OnClick_ExportType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if (m_ExportTypeList.GetSelection() == COT_ESPMode.CREATELOADOUT)
			m_ExportButton.SetButton("#STR_COT_SAVE_AS");
		else
			m_ExportButton.SetButton("#STR_COT_TO_CLIPBOARD");
	}	

	void OnChange_Skeleton_LineThickness( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		m_Module.SkeletonLineThickness = action.GetSelection() + 1;
	}
	
	void OnClick_DuplicateSelected( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.DuplicateSelected();
	}
	
	void OnClick_ESPDeselectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		foreach(JMESPViewTypeWidget espType: m_ESPTypeList)
			espType.SetChecked(false);
	}
	
	void OnClick_ESPSelectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		foreach(JMESPViewTypeWidget espType: m_ESPTypeList)
			espType.SetChecked(true);
	}
	
	void OnClick_DeleteSelected( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CreateAdvancedObjectConfirm("DeleteSelected", "DeleteSelected");
	}

	void DeleteSelected()
	{
		m_Module.DeleteSelected();
	}
	
	void OnClick_SelectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		auto node = JMESPMeta.s_JM_All.m_Head;
		while ( node )
		{
			if ( node.m_Value.widgetRoot.IsVisible() )
				node.m_Value.widgetHandler.Select();
			
			node = node.m_Next;
		}
	}
	
	void OnClick_DeselectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		auto node = JMESPMeta.s_JM_All.m_Head;
		while ( node )
		{
			node.m_Value.widgetHandler.Deselect();
			node = node.m_Next;
		}

		// Some scenarios require this to prevent unexpected results for the end user
		JM_GetSelected().ClearObjects();
	}
	
	void OnClick_MoveToCursor( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedObjectConfirm("MoveToCursor", "MoveToCursor");
	}

	void MoveToCursor()
	{
		vector dir = g_Game.GetCurrentCameraDirection();
		vector from = g_Game.GetCurrentCameraPosition(); 
		vector to = from + ( dir * 1000 );   
		vector contact_pos;
		vector contact_dir;
		int contact_component;
		
		if ( DayZPhysics.RaycastRV(from, to, contact_pos, contact_dir, contact_component, NULL, NULL, NULL, false, true) )
			m_Module.MoveToCursor( contact_pos );
	}
	
	void OnClick_CopyToClipboard( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		switch(m_ExportTypeList.GetSelection())
		{
			case COT_ESPMode.COPYLISTRAW:
				m_Module.CopyToClipboardRaw();
			break;
			case COT_ESPMode.COPYLISTSPAWNABLETYPES:
				m_Module.CopyToClipboardSpawnableTypes();
			break;
		#ifdef DZ_Expansion_Market
			case COT_ESPMode.COPYLISTEXPMARKET:
				m_Module.CopyToClipboardMarket();
			break;
		#endif
		#ifdef DZ_Expansion_Core
			case COT_ESPMode.COPYLISTEXPLOADOUT:
				m_Module.CopyToClipboardExpLoadout(JMSelectedObject);
			break;
		#endif
			case COT_ESPMode.CREATELOADOUT:
				CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_ESP_MODULE_LOADOUT_MESSAGE_HEADER", "#STR_COT_ESP_MODULE_LOADOUT_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CREATE", "CreateLoadout_Confirm" );
			break;
		}
	}

	void CreateLoadout_Confirm(JMConfirmation confirmation)
	{
		string name = confirmation.GetEditBoxValue();
		if (name == string.Empty)
			return;

		if (!m_LoadoutModule)
			Class.CastTo(m_LoadoutModule, GetModuleManager().GetModule(JMLoadoutModule));
		
		m_LoadoutModule.Create(name);
	}
}
