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

	private JMESPModule m_Module;

	private UIActionEditableTextPreview m_SearchBox;

	private bool m_UseFilter;

	void JMESPForm()
	{
		m_ESPTypeList = new array< ref JMESPViewTypeWidget >;
		m_ESPTypeWidgetsByType = new map<typename, JMESPViewTypeWidget>;
	}

	void ~JMESPForm()
	{
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

		Widget checkboxesSpacer = UIActionManager.CreateGridSpacer( quadSpacer, 1, 2 );

		UIActionManager.CreateCheckbox( checkboxesSpacer, "#STR_COT_ESP_MODULE_TOGGLE_CLASS_NAME", this, "Click_UseClassName", JMESPWidgetHandler.UseClassName );
		UIActionManager.CreateCheckbox( checkboxesSpacer, "#STR_COT_ESP_MODULE_TOGGLE_SAFETY", this, "Click_DisableSafety", m_Module.GetFilterSafetyState() );

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

		m_sldr_Radius = UIActionManager.CreateSlider( filterSpacer, "#STR_COT_ESP_MODULE_RADIUS", 0, 1000, this, "Change_Range" );
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
		m_ESPListRows = UIActionManager.CreateActionRows( m_ESPListScroller.GetContentWidget() );

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

		//UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_MAKE_ITEM_SET", this, "Click_MakeItemSet" );
		//UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_MAKE_LOADOUT", this, "Click_MakeLoadout" );
		
		UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_DELETE_ALL", this, "Click_DeleteAll" );
		UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_MOVE_TO_CURSOR", this, "Click_MoveToCursor" );

		UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_COPY_TO_CLIPBOARD_RAW", this, "Click_CopyToClipboardRaw" );
		UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_COPY_TO_CLIPBOARD_MARKET", this, "Click_CopyToClipboardMarket" );
		UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_COPY_TO_CLIPBOARD_SPAWNABLETYPES", this, "Click_CopyToClipboardSpawnableTypes" );

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
	}

	override void OnShow()
	{
		super.OnShow();

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateUI, 500, true );

		UpdateUI();

		GetCommunityOnlineTools().RefreshClients();
	}

	override void OnHide()
	{
		super.OnHide();

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).Remove( UpdateUI );
	}

	void DisableToggleableOptions()
	{
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
		TStringArray classnamelist = new TStringArray;

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

					GetGame().ConfigGetChildName( strConfigPath, nClass, strName );

					int scope = GetGame().ConfigGetInt( strConfigPath + " " + strName + " scope" );

					if ( scope == 0 )
						continue;

					if ( !GetGame().ConfigIsExisting( strConfigPath + " " + strName + " model" ) )
						continue;

					string strNameLower = strName;

					strNameLower.ToLower();

					if ( strNameLower.Contains( strSearch ) )
						classnamelist.Insert(strNameLower);
				}
			}
		}

		if (classnamelist.Count())
		{
			string wordPreview = FindClosestWord(classnamelist, strSearch);
			m_SearchBox.SetTextPreview(wordPreview);
			m_UseFilter = true;
		}
		else
		{
			m_SearchBox.SetTextPreview("");
			m_UseFilter = false;
		}
	}

    static string FindClosestWord(TStringArray words, string inputWord)
    {
        TStringArray suggestions = new TStringArray;

        foreach (string word : words)
        {
            if ( word == inputWord )
                return word;

            if ( word.IndexOf(inputWord) == 0 )
                suggestions.Insert(word);
        }

		suggestions.Sort();

        return suggestions[0];
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

	void Change_Skeleton_LineThickness( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		m_Module.SkeletonLineThickness = action.GetSelection() + 1;
	}

	void Click_MakeItemSet( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Two( JMConfirmationType.EDIT, "#STR_COT_ESP_MODULE_ACTION_MAKE_ITEM_SET_CONFIRMATION_HEADER", "#STR_COT_ESP_MODULE_ACTION_MAKE_ITEM_SET_CONFIRMATION_DESCRIPTION", "#STR_COT_GENERIC_CANCEL", "MakeItemSet_Cancel", "#STR_COT_GENERIC_CREATE", "MakeItemSet_Create" );
	}

	void MakeItemSet_Cancel( JMConfirmation confirmation )
	{
		// do nothing
	}

	void MakeItemSet_Create( JMConfirmation confirmation )
	{
		m_Module.MakeItemSet( confirmation.GetEditBoxValue() );
	}
	
	void Click_DuplicateAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.DuplicateAll();
	}
	
	void Click_DeleteAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.DeleteAll();
	}
	
	void Click_MoveToCursor( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		vector dir = GetGame().GetCurrentCameraDirection();
		vector from = GetGame().GetCurrentCameraPosition(); 
		vector to = from + ( dir * 1000 );   
		vector contact_pos;
		vector contact_dir;
		int contact_component;
		
		if ( DayZPhysics.RaycastRV(from, to, contact_pos, contact_dir, contact_component, NULL, NULL, NULL, false, true) )
			m_Module.MoveToCursor( contact_pos );
	}
	
	void Click_CopyToClipboardRaw( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.CopyToClipboardRaw();
	}
	
	void Click_CopyToClipboardMarket( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.CopyToClipboardMarket();
	}
	
	void Click_CopyToClipboardSpawnableTypes( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.CopyToClipboardSpawnableTypes();
	}
};
