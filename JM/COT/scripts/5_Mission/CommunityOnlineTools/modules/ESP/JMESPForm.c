class JMESPForm extends JMFormBase
{
	private autoptr array< ref JMESPViewTypeWidget > m_ESPTypeList;

	private UIActionScroller m_ESPListScroller;
	private Widget m_ESPListRows;

	private UIActionScroller m_ESPSelectedObjects;

	private UIActionButton m_btn_FullMap;

	private UIActionButton m_btn_Toggle;
	
	private UIActionCheckbox m_chkbx_Refresh;
	private UIActionSlider m_sldr_Refresh;

	private UIActionSlider m_sldr_Radius;

	private JMESPModule m_Module;

	void JMESPForm()
	{
		m_ESPTypeList = new array< ref JMESPViewTypeWidget >;
	}

	void ~JMESPForm()
	{
	}

	protected override bool SetModule( ref JMRenderableModuleBase mdl )
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

		m_chkbx_Refresh = UIActionManager.CreateCheckbox( quadSpacer, "#STR_COT_ESP_MODULE_TOGGLE_AUTO_REFRESH", this, "Click_UpdateAtRate" );
		m_sldr_Refresh = UIActionManager.CreateSlider( quadSpacer, "", 1.0, 10.0, this, "Change_UpdateRate" );
		m_sldr_Refresh.SetCurrent( m_Module.ESPUpdateTime );
		m_sldr_Refresh.SetFormat("#STR_COT_FORMAT_SECOND_LONG");
		m_sldr_Refresh.SetStepValue( 1.0 );

		Widget fullMapSpacer = UIActionManager.CreatePanel( mainSpacer, 0x00000000, 30 );

		m_btn_FullMap = UIActionManager.CreateButton( fullMapSpacer, "#STR_COT_ESP_MODULE_ACTION_FULLMAP_BUTTON", this, "Click_EnableFullMap" );
		m_btn_FullMap.SetPosition( 0 );
		m_btn_FullMap.SetWidth( 0.3 );

		UIActionText fmHeading = UIActionManager.CreateText( fullMapSpacer, "#STR_COT_ESP_MODULE_ACTION_FULLMAP_WARNING_HEADER", "" );
		fmHeading.SetPosition( 0.3 );
		fmHeading.SetWidth( 0.15 );
		UIActionText fmText = UIActionManager.CreateText( fullMapSpacer, "", "#STR_COT_ESP_MODULE_ACTION_FULLMAP_WARNING_DESCRIPTION" );
		fmText.SetPosition( 0.45 );
		fmText.SetWidth( 0.55 );

		Widget filterSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		m_sldr_Radius = UIActionManager.CreateSlider( filterSpacer, "#STR_COT_ESP_MODULE_RADIUS", 0, 1000, this, "Change_Range" );
		m_sldr_Radius.SetCurrent( m_Module.ESPRadius );
		m_sldr_Radius.SetFormat("#STR_COT_FORMAT_METRE_LONG");
		m_sldr_Radius.SetStepValue( 10.0 );

		UIActionManager.CreateEditableText( filterSpacer, "#STR_COT_ESP_MODULE_CLASS_FILTER", this, "Change_Filter", m_Module.Filter );
	
		UIActionManager.CreatePanel( mainSpacer, 0xFF000000, 3 );

		Widget headingSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );
		UIActionManager.CreateText( headingSpacer, "#STR_COT_ESP_MODULE_FILTERS_HEADER", "" );
		UIActionManager.CreateText( headingSpacer, "#STR_COT_ESP_MODULE_ACTIONS_HEADER", "" );
	}

	private void ESPFilters( Widget parent )
	{
		//Print( "+" + this + "::ESPFilters" );

		m_ESPListScroller = UIActionManager.CreateScroller( parent );
		m_ESPListRows = UIActionManager.CreateActionRows( m_ESPListScroller.GetContentWidget() );

		int totalInContentRow = 100;
		int currentContentRow = 0;

		GridSpacerWidget gsw;

		for ( int i = 0; i < m_Module.GetViewTypes().Count(); i++ )
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

			rScript.Set( m_Module.GetViewTypes()[i] );

			m_ESPTypeList.Insert( rScript );
		}

		m_ESPListScroller.UpdateScroller();

		//Print( "-" + this + "::ESPFilters" );
	}

	private void ESPSelectedObjects( Widget parent )
	{
		m_ESPSelectedObjects = UIActionManager.CreateScroller( parent );
		Widget container = m_ESPSelectedObjects.GetContentWidget();

		//UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_MAKE_ITEM_SET", this, "Click_MakeItemSet" );
		//UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_DUPLICATE_ALL", this, "Click_DuplicateAll" );
		UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_DELETE_ALL", this, "Click_DeleteAll" );
		//UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_MOVE_TO_CURSOR_RELATIVE", this, "Click_MoveToCursorRelative" );
		//UIActionManager.CreateButton( container, "#STR_COT_ESP_MODULE_ACTION_MOVE_TO_CURSOR_ABSOLUTE", this, "Click_MoveToCursorAbsolute" );

		m_ESPSelectedObjects.UpdateScroller();
	}

	override void OnInit()
	{
		JMESPWidgetHandler.espMenu = this;

		ESPControls( layoutRoot.FindAnyWidget( "panel_top" ) );

		Widget left_bottom = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIPanel.layout", layoutRoot.FindAnyWidget( "panel_bottom" ) );
		Widget right_bottom = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIPanel.layout", layoutRoot.FindAnyWidget( "panel_bottom" ) );

		//left_bottom.SetSize( 1.0, 1.0 );
		//right_bottom.SetSize( 0.0, 1.0 );
		//left_bottom.SetPos( 0.0, 0.0 );
		//right_bottom.SetPos( 1.0, 0.0 );

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

			m_sldr_Refresh.Disable();
		} else
		{
			if ( m_chkbx_Refresh.IsChecked() )
			{
				m_btn_Toggle.SetButton( "#STR_COT_ESP_MODULE_ACTION_SHOW_ESP" );

				m_sldr_Refresh.Enable();
			} else
			{
				m_btn_Toggle.SetButton( "#STR_COT_ESP_MODULE_ACTION_SHOW_ESP" );

				m_sldr_Refresh.Disable();
			}
		}

		if ( COTPlayerIsRemoved )
		{	
			m_btn_FullMap.Disable();
		}
	}

	void Click_UpdateESP( UIEvent eid, ref UIActionBase action )
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
			} else
			{
				m_Module.UpdateState( JMESPState.View );

				m_Module.Log( "Viewing ESP" );
			}
		}

		UpdateUI();
	}
	
	void Change_Filter( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Module.Filter = action.GetText();
	}

	void Change_UpdateRate( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		m_Module.ESPUpdateTime = action.GetCurrent();
	}

	void Change_Range( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;
		
		m_Module.ESPRadius = action.GetCurrent();
	}

	void Click_UseClassName( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		JMESPWidgetHandler.UseClassName = action.IsChecked();
	}
	
	void Click_EnableFullMap( UIEvent eid, ref UIActionBase action )	
	{	
		if ( eid != UIEvent.CLICK ) return;	

		m_Module.EnableFullMap();	

		// TODO: Send RPC back to disable this	
		// m_FullMapESP.Disable();	
	}

	void Click_UpdateAtRate( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_Module.GetState() != JMESPState.Remove )
		{
			if ( action.IsChecked() )
			{
				m_Module.UpdateState( JMESPState.Update );
			} else
			{
				m_Module.UpdateState( JMESPState.Remove );
			}
		}
		
		UpdateUI();
	}
	
	void Click_MakeItemSet( UIEvent eid, ref UIActionBase action )
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
	
	void Click_DuplicateAll( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.DuplicateAll();
	}
	
	void Click_DeleteAll( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.DeleteAll();
	}
	
	void Click_MoveToCursorRelative( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.MoveToCursorRelative( "0 0 0" );
	}
	
	void Click_MoveToCursorAbsolute( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_Module.MoveToCursorAbsolute( "0 0 0" );
	}
}