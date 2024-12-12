class JMESPForm: JMFormBase
{
	private autoptr array< ref JMESPViewTypeWidget > m_ESPTypeList;
	private ref map<typename, JMESPViewTypeWidget> m_ESPTypeWidgetsByType;

	private UIActionScroller m_ESPListScroller;
	private Widget m_ESPListRows;

	private UIActionScroller m_ESPSelectedObjects;

	private UIActionSelectBox m_slbx_PlayerSkeletons;
	private UIActionSelectBox m_slbx_Skeletons_LineThickness;

	private UIActionButton m_btn_Toggle;
	
	private UIActionCheckbox m_chkbx_Refresh;
	private UIActionSlider m_sldr_Refresh;

	private UIActionSlider m_sldr_Radius;

	private UIActionCheckbox m_DisableSafetyCheckbox;

	private UIActionButton m_ExportButton;
	private UIActionSelectBox m_ExportTypeList;

	private JMESPModule m_Module;
	private JMLoadoutModule m_LoadoutModule;

	private UIActionEditableTextPreview m_SearchBox;

	private bool m_UseFilter;

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

		Widget quadSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 3, 2 );
		
		m_btn_Toggle = UIActionManager.CreateButton( quadSpacer, "#STR_COT_ESP_MODULE_TOGGLE", this, "Click_UpdateESP" );

		Widget checkboxesSpacer = UIActionManager.CreateGridSpacer( quadSpacer, 2, 1 );

		UIActionManager.CreateCheckbox( checkboxesSpacer, "#STR_COT_ESP_MODULE_TOGGLE_CLASS_NAME", this, "Click_UseClassName", JMESPWidgetHandler.UseClassName );
		m_DisableSafetyCheckbox = UIActionManager.CreateCheckbox( checkboxesSpacer, "#STR_COT_ESP_MODULE_TOGGLE_SAFETY", this, "Click_DisableSafety", m_Module.GetFilterSafetyState() );
		UpdateDisableSafetyCheckbox();

		m_chkbx_Refresh = UIActionManager.CreateCheckbox( quadSpacer, "#STR_COT_ESP_MODULE_TOGGLE_AUTO_REFRESH", this, "Click_UpdateAtRate", m_Module.GetState() == JMESPState.Update );
		m_sldr_Refresh = UIActionManager.CreateSlider( quadSpacer, "", 1.0, 10.0, this, "Change_UpdateRate" );
		m_sldr_Refresh.SetCurrent( m_Module.ESPUpdateTime );
		m_sldr_Refresh.SetFormat("#STR_COT_FORMAT_SECOND_LONG");
		m_sldr_Refresh.SetStepValue( 1.0 );

		Widget skeletonSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		m_slbx_PlayerSkeletons = UIActionManager.CreateSelectionBox( skeletonSpacer, "#STR_COT_ESP_MODULE_DRAW_PLAYER_SKELETONS", {"#STR_COT_GENERIC_NONE", "#STR_COT_GENERIC_OTHERS", "#STR_COT_GENERIC_ALL"}, this, "Change_PlayerSkeletons" );
		m_slbx_PlayerSkeletons.SetSelectorWidth(0.4);
		int idx = m_Module.GetDrawPlayerSkeletonsEnabled();
		if (idx)
			idx += m_Module.DrawPlayerSkeletonsIncludingMyself;
		m_slbx_PlayerSkeletons.SetSelection(idx, false);
		m_slbx_Skeletons_LineThickness = UIActionManager.CreateSelectionBox( skeletonSpacer, "#STR_COT_GENERIC_LINE_THICKNESS", {"1", "2", "3", "4"}, this, "Change_Skeleton_LineThickness" );
		m_slbx_Skeletons_LineThickness.SetSelectorWidth(0.4);
		m_slbx_Skeletons_LineThickness.SetSelection( m_Module.SkeletonLineThickness - 1 );

		Widget filterSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		m_sldr_Radius = UIActionManager.CreateSlider( filterSpacer, "#STR_COT_ESP_MODULE_RADIUS", 0, m_Module.GetMaxRadius(), this, "Change_Range" );
		m_sldr_Radius.SetCurrent( m_Module.ESPRadius );
		m_sldr_Radius.SetFormat("#STR_COT_FORMAT_METRE_LONG");
		m_sldr_Radius.SetStepValue( 10.0 );

		m_SearchBox = UIActionManager.CreateEditableTextPreview( filterSpacer, "#STR_COT_ESP_MODULE_CLASS_FILTER", this, "Change_Filter", m_Module.Filter );
	
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
		UIActionManager.CreateButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL", this, "Click_ESPSelectAll" );
		UIActionManager.CreateButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL", this, "Click_ESPDeselectAll" );

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

			Widget rWidget = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp_type_widget.layout", gsw );
			
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
		UIActionManager.CreateButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL", this, "Click_SelectAll" );
		UIActionManager.CreateButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL", this, "Click_DeselectAll" );

		UIActionManager.CreateText(container,"Selected Items");
	
		Widget rowExports = UIActionManager.CreateGridSpacer( container, 1, 2 );
		m_ExportButton = UIActionManager.CreateButton( rowExports, "Copy to Clipboard", this, "Click_CopyToClipboard" );
		m_ExportTypeList = UIActionManager.CreateSelectionBox( rowExports, "", {"Raw", "SpawnableTypes", "Exp Market", "#STR_COT_LOADOUT_MODULE_NAME"}, this, "Click_ExportType" );
		m_ExportTypeList.SetSelectorWidth(1.0);

		Widget rowMisc = UIActionManager.CreateGridSpacer( container, 1, 2 );
		UIActionManager.CreateButton( rowMisc, "#STR_COT_ESP_MODULE_ACTION_MOVE_TO_CURSOR", this, "Click_MoveToCursor" );
		UIActionButton delbtn = UIActionManager.CreateButton( rowMisc, "#STR_COT_ESP_MODULE_ACTION_DELETE_SELECTED", this, "Click_DeleteSelected" );
		delbtn.SetColor(COLOR_RED);

		m_ESPSelectedObjects.UpdateScroller();
	}

	override void OnInit()
	{
		JMESPWidgetHandler.espMenu = this;

		ESPControls( layoutRoot.FindAnyWidget( "panel_top" ) );

		Widget left_bottom = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIPanel.layout", layoutRoot.FindAnyWidget( "panel_bottom" ) );
		Widget right_bottom = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIPanel.layout", layoutRoot.FindAnyWidget( "panel_bottom" ) );

		left_bottom.SetSize( 0.5, 1.0 );
		right_bottom.SetSize( 0.5, 1.0 );
		left_bottom.SetPos( 0.0, 0.0 );
		right_bottom.SetPos( 0.5, 0.0 );

		ESPFilters( left_bottom );
		ESPSelectedObjects( right_bottom );
		
		Class.CastTo(m_LoadoutModule, GetModuleManager().GetModule(JMLoadoutModule));
	}

	override void OnShow()
	{
		super.OnShow();

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateUI, 500, true );

		UpdateUI();
	}

	override void OnHide()
	{
		super.OnHide();

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).Remove( UpdateUI );
	}

	void DisableToggleableOptions()
	{
	}

	void OnESPViewTypeChanged(JMESPViewType viewType)
	{
		UpdateMaxRange();

		switch (viewType.Type())
		{
			case JMESPViewTypeBuilding:
				UpdateDisableSafetyCheckbox();
				break;
		}
	}

	void UpdateDisableSafetyCheckbox()
	{
		if (m_Module.GetViewType(JMESPViewTypeBuilding).View)
			m_DisableSafetyCheckbox.Enable();
		else
			m_DisableSafetyCheckbox.Disable();
	}

	void UpdateUI()
	{
		m_sldr_Radius.SetCurrent( m_Module.ESPRadius );
		m_sldr_Refresh.SetCurrent( m_Module.ESPUpdateTime );

		if ( m_Module.GetState() != JMESPState.Remove )
		{
			m_btn_Toggle.SetButton( "#STR_COT_ESP_MODULE_ACTION_CLEAR_ESP" );
			m_btn_Toggle.SetColor(COLOR_RED_A);
		}
		else
		{
			m_btn_Toggle.SetColor(COLOR_GREEN_A);
			if ( m_chkbx_Refresh.IsChecked() )
			{
				m_btn_Toggle.SetButton( "#STR_COT_ESP_MODULE_ACTION_SHOW_ESP" );

				m_sldr_Refresh.Enable();
			}
			else
			{
				m_btn_Toggle.SetButton( "#STR_COT_ESP_MODULE_ACTION_SHOW_ESP" );

				m_sldr_Refresh.SetColor(COLOR_WHITE);
				m_sldr_Refresh.Disable();
			}
		}
	}

	void UpdateList()
	{
		string closestMatch;

		string strSearch = m_SearchBox.GetText();
		
		m_UseFilter = false;

		if ( strSearch != "" )
		{
			TStringArray suggestions = new TStringArray;

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

					GetGame().ConfigGetChildName( strConfigPath, nClass, strName );

					int scope = GetGame().ConfigGetInt( strConfigPath + " " + strName + " scope" );

					if ( scope == 0 )
						continue;

					if ( !GetGame().ConfigIsExisting( strConfigPath + " " + strName + " model" ) )
						continue;

					string strNameLower = strName;

					strNameLower.ToLower();

					if ( strNameLower == strSearch )
					{
						suggestions.Clear();
						closestMatch = strNameLower;
						m_UseFilter = true;
						break;  //! We can end the search here because we got a perfect match
					}
					else if ( strNameLower.IndexOf(strSearch) == 0 )
					{
						suggestions.Insert(strNameLower);
					}
					else if ( strNameLower.Contains(strSearch) )
					{
						m_UseFilter = true;
					}
				}
			}

			if (suggestions.Count())
			{
				suggestions.Sort();
				closestMatch = suggestions[0];
				m_UseFilter = true;
			}
		}

		m_SearchBox.SetTextPreview(closestMatch);
	}

	void Click_UpdateESP( UIEvent eid, UIActionBase action )
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
	
	void Change_Filter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateList();

		if ( m_UseFilter )
			m_Module.Filter = action.GetText();
		else
			m_Module.Filter = "";
	}

	void Change_UpdateRate( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		if ( m_sldr_Refresh.GetCurrent() > 0 )
			m_sldr_Refresh.SetColor(COLOR_WHITE);
		else
			m_sldr_Refresh.SetColor(COLOR_RED_A);

		m_Module.ESPUpdateTime = action.GetCurrent();
	}

	void Change_Range( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		m_Module.ESPRadius = action.GetCurrent();
	}

	void Click_DisableSafety( UIEvent eid, UIActionBase action )
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
		m_sldr_Radius.SetMax(maxRadius);
		m_sldr_Radius.SetCurrent(m_Module.ESPRadius);
	}	

	void Click_UseClassName( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		JMESPWidgetHandler.UseClassName = action.IsChecked();
	}

	void Click_UpdateAtRate( UIEvent eid, UIActionBase action )
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
	
	void Change_PlayerSkeletons( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		int idx = action.GetSelection();
		if (idx > 0 && !m_ESPTypeWidgetsByType[JMESPViewTypePlayerAI].IsChecked())
			m_ESPTypeWidgetsByType[JMESPViewTypePlayer].SetChecked(true);

		m_Module.SetDrawPlayerSkeletonsEnabled(idx > 0);
		m_Module.DrawPlayerSkeletonsIncludingMyself = idx > 1;
	}

	void Click_ExportType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if (m_ExportTypeList.GetSelection() == COT_ESPMode.CREATELOADOUT)
			m_ExportButton.SetButton("Save as");
		else
			m_ExportButton.SetButton("Copy to Clipboard");
	}	

	void Change_Skeleton_LineThickness( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		m_Module.SkeletonLineThickness = action.GetSelection() + 1;
	}
	
	void Click_DuplicateSelected( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.DuplicateSelected();
	}
	
	void Click_ESPDeselectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		foreach(JMESPViewTypeWidget espType: m_ESPTypeList)
			espType.SetChecked(false);
	}
	
	void Click_ESPSelectAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		foreach(JMESPViewTypeWidget espType: m_ESPTypeList)
			espType.SetChecked(true);
	}
	
	void Click_DeleteSelected( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedObjectConfirm("DeleteSelected");
	}

	void DeleteSelected()
	{
		m_Module.DeleteSelected();
	}
	
	void Click_SelectAll( UIEvent eid, UIActionBase action )
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
	
	void Click_DeselectAll( UIEvent eid, UIActionBase action )
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
	
	void Click_MoveToCursor( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedObjectConfirm("MoveToCursor");
	}

	void MoveToCursor()
	{
		vector dir = GetGame().GetCurrentCameraDirection();
		vector from = GetGame().GetCurrentCameraPosition(); 
		vector to = from + ( dir * 1000 );   
		vector contact_pos;
		vector contact_dir;
		int contact_component;
		
		if ( DayZPhysics.RaycastRV(from, to, contact_pos, contact_dir, contact_component, NULL, NULL, NULL, false, true) )
			m_Module.MoveToCursor( contact_pos );
	}
	
	void Click_CopyToClipboard( UIEvent eid, UIActionBase action )
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
			case COT_ESPMode.COPYLISTEXPMARKET:
				m_Module.CopyToClipboardMarket();
			break;
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
};
