class JMESPForm extends JMFormBase
{
	protected ref UIActionSlider m_RangeSlider;

	protected ref UIActionButton m_UpdateShow;
	protected ref UIActionCheckbox m_TextMode;
	protected ref UIActionSlider m_UpdateRate;
	protected ref UIActionCheckbox m_ESPIsUpdating;

	protected ref UIActionButton m_FullMapESP;

	void JMESPForm()
	{
	}

	void ~JMESPForm()
	{
	}

	void ESPControls( Widget mainSpacer )
	{
		Widget quadSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 2, 2 );
		
		m_UpdateShow = UIActionManager.CreateButton( quadSpacer, "Show ESP", this, "Click_UpdateESP" );
		m_TextMode = UIActionManager.CreateCheckbox( quadSpacer, "Text Mode", this, "Click_ChangeESPMode", JMESPWidget.ShowJustName );

		m_FullMapESP = UIActionManager.CreateButton( quadSpacer, "Enable Fullmap ESP", this, "Click_EnableFullMap" );
		UIActionManager.CreateCheckbox( quadSpacer, "Use Class Name", this, "Click_UseClassName", JMESPWidget.UseClassName );

		UIActionManager.CreateText( mainSpacer, "Information: ", "Enabling full map ESP requires you to be in Free Cam." );
		UIActionManager.CreateText( mainSpacer, "Warning: ", "Enabling full map ESP removes your character from the server, relog to fix." );

		m_RangeSlider = UIActionManager.CreateSlider( mainSpacer, "Radius", 0, 1000, this, "Change_Range" );
		m_RangeSlider.SetCurrent( JMESPModule.Cast( module ).ESPRadius );
		m_RangeSlider.SetAppend(" metre(s)");
		m_RangeSlider.SetStepValue( 10 );

		Widget duoSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		m_ESPIsUpdating = UIActionManager.CreateCheckbox( duoSpacer, "Should Update Continuously", this, "Click_UpdateAtRate", JMESPModule.Cast( module ).ESPIsUpdating );

		m_UpdateRate = UIActionManager.CreateSlider( duoSpacer, "At Rate", 0.5, 10, this, "Change_UpdateRate" );
		m_UpdateRate.SetCurrent( JMESPModule.Cast( module ).ESPUpdateTime );
		m_UpdateRate.SetAppend(" second(s)");
		m_UpdateRate.SetStepValue( 0.5 );
	}

	void ESPFilters( Widget mainSpacer )
	{
		UIActionManager.CreateEditableText( mainSpacer, "Class Filter: ", this, "Change_Filter", JMESPModule.Cast( module ).Filter );
	}

	override void OnInit()
	{
		JMESPWidget.espMenu = this;

		Widget mainSpacer = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 3, 1 );

		ESPControls( UIActionManager.CreateGridSpacer( mainSpacer, 3, 1 ) );

		UIActionManager.CreateSpacer( mainSpacer );

		ESPFilters( UIActionManager.CreateGridSpacer( mainSpacer, 8, 1 ) );
	}

	override void OnShow()
	{
		super.OnShow();

		m_RangeSlider.SetCurrent( JMESPModule.Cast( module ).ESPRadius );
		m_UpdateRate.SetCurrent( JMESPModule.Cast( module ).ESPUpdateTime );
		m_ESPIsUpdating.SetChecked( JMESPModule.Cast( module ).ESPIsUpdating );

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

	void UpdateESPButtonName()
	{
		bool isShowing = JMESPModule.Cast( module ).IsShowing;

		if ( isShowing )
		{
			m_UpdateShow.SetButton( "Clear ESP" );
		} else
		{
			m_UpdateShow.SetButton( "Enable ESP" );
		}
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
			JMESPModule.Cast( module ).ESPIsUpdating = false;
			
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( JMESPModule.Cast( module ).UpdateESP );

			JMESPModule.Cast( module ).HideESP();
		} else
		{
			JMESPModule.Cast( module ).ESPIsUpdating = m_ESPIsUpdating.IsChecked();

			JMESPModule.Cast( module ).UpdateESP();
		}

		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "ESP Showing " + JMESPModule.Cast( module ).IsShowing ) );

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
		
		JMESPWidget.ShowJustName = action.IsChecked();
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
		
		m_FullMapESP.Disable();
	}

	void Click_UpdateAtRate( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).ESPUpdateLoop( action.IsChecked() );
	}
}