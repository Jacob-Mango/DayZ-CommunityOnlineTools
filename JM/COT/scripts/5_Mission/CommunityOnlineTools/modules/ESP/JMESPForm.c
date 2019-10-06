class JMESPForm extends JMFormBase
{
	protected UIActionSlider m_RangeSlider;

	protected UIActionSlider m_UpdateRate;

	protected UIActionButton m_FullMapESP;

	void JMESPForm()
	{
	}

	void ~JMESPForm()
	{
	}

	void ESPControls( Widget parent )
	{
		Widget mainSpacer = UIActionManager.CreateGridSpacer( parent, 3, 1 );

		Widget quadSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 3, 2 );
		
		UIActionManager.CreateButton( quadSpacer, "Toggle", this, "Click_UpdateESP" );

		Widget checkboxesSpacer = UIActionManager.CreateGridSpacer( quadSpacer, 1, 2 );

		UIActionManager.CreateCheckbox( checkboxesSpacer, "Simplified", this, "Click_ChangeESPMode", JMESPWidget.Simplified );
		UIActionManager.CreateCheckbox( checkboxesSpacer, "Use Class Name", this, "Click_UseClassName", JMESPWidget.UseClassName );

		UIActionManager.CreateButton( quadSpacer, "Update Interval", this, "Click_UpdateAtRate" );
		m_UpdateRate = UIActionManager.CreateSlider( quadSpacer, "", 0.5, 10, this, "Change_UpdateRate" );
		m_UpdateRate.SetCurrent( JMESPModule.Cast( module ).ESPUpdateTime );
		m_UpdateRate.SetAppend(" second(s)");
		m_UpdateRate.SetStepValue( 0.5 );

		Widget fullMapSpacer = UIActionManager.CreatePanel( parent, 0x00000000, 30 );
		m_FullMapESP = UIActionManager.CreateButton( fullMapSpacer, "Enable Fullmap ESP", this, "Click_EnableFullMap" );
		m_FullMapESP.SetPosition( 0 );
		m_FullMapESP.SetWidth( 0.2 );
		UIActionText fmHeading = UIActionManager.CreateText( fullMapSpacer, "Warning: ", "" );
		fmHeading.SetPosition( 0.2 );
		fmHeading.SetWidth( 0.15 );
		UIActionText fmText = UIActionManager.CreateText( fullMapSpacer, "", "Entering deletes your character and exiting fullmap ESP requires a relog" );
		fmText.SetPosition( 0.35 );
		fmText.SetWidth( 0.65 );

		Widget otherStuffSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		m_RangeSlider = UIActionManager.CreateSlider( otherStuffSpacer, "Radius", 0, 1000, this, "Change_Range" );
		m_RangeSlider.SetCurrent( JMESPModule.Cast( module ).ESPRadius );
		m_RangeSlider.SetAppend(" metre(s)");
		m_RangeSlider.SetStepValue( 10 );

		UIActionManager.CreateEditableText( otherStuffSpacer, "Class Filter: ", this, "Change_Filter", JMESPModule.Cast( module ).Filter );
	}

	void ESPFilters( Widget parent )
	{
		Widget mainSpacer = UIActionManager.CreateGridSpacer( parent, 8, 1 );
	}

	override void OnInit()
	{
		JMESPWidget.espMenu = this;

		Widget mainSpacer = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 3, 1 );

		ESPControls( mainSpacer );

		UIActionManager.CreateSpacer( mainSpacer );

		ESPFilters( mainSpacer );
	}

	override void OnShow()
	{
		super.OnShow();

		m_RangeSlider.SetCurrent( JMESPModule.Cast( module ).ESPRadius );

		UpdateRefreshRateSlider();

		if ( COTPlayerIsRemoved )
		{
			m_FullMapESP.Disable();
		}

		UpdateESPButtonName();

		UpdateCheckboxStates();
	}

	override void OnHide()
	{
		super.OnHide();
	}

	void UpdateRefreshRateSlider()
	{
		m_UpdateRate.SetCurrent( JMESPModule.Cast( module ).ESPUpdateTime );

		if ( JMESPModule.Cast( module ).ESPIsUpdating  )
		{
			m_UpdateRate.Enable();
		} else
		{
			m_UpdateRate.Disable();
		}
	}

	void UpdateESPButtonName()
	{
	}

	void DisableToggleableOptions()
	{
	}

	void UpdateCheckboxStates()
	{
	}

	void Click_UpdateESP( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( JMESPModule.Cast( module ).IsShowing )
		{			
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( JMESPModule.Cast( module ).UpdateESP );

			JMESPModule.Cast( module ).HideESP();
		} else
		{
			JMESPModule.Cast( module ).UpdateESP();
		}

		// GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "ESP Showing " + JMESPModule.Cast( module ).IsShowing ) );

		UpdateESPButtonName();

		UpdateCheckboxStates();
	}

	// Removed.
	void Click_HideESP( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).HideESP();
	}

	void Click_ChangeESPMode( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPWidget.Simplified = action.IsChecked();
	}
	
	void Change_Filter( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;

		JMESPModule.Cast( module ).Filter = action.GetText();
	}

	void Change_UpdateRate( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		JMESPModule.Cast( module ).ESPUpdateTime = action.GetCurrent();
	}

	void Change_Range( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		JMESPModule.Cast( module ).ESPRadius = action.GetCurrent();
	}

	void Click_UseClassName( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPWidget.UseClassName = action.IsChecked();
	}

	void Click_EnableFullMap( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).EnableFullMap();
		
		// TODO: Send RPC back to disable this
		// m_FullMapESP.Disable();
	}

	void Click_UpdateAtRate( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		JMESPModule.Cast( module ).ESPUpdateLoop( !JMESPModule.Cast( module ).ESPIsUpdating );

		UpdateRefreshRateSlider();
	}
}