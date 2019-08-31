class JMESPForm extends JMFormBase
{
	protected ref UIActionSlider m_RangeSlider;

	protected ref UIActionButton m_UpdateShow;
	protected ref UIActionCheckbox m_TextMode;
	protected ref UIActionSlider m_UpdateRate;
	protected ref UIActionCheckbox m_ESPIsUpdating;

	protected ref UIActionButton m_FullMapESP;

	protected ref UIActionCheckbox m_ViewEverything;
	protected ref UIActionCheckbox m_ViewPlayers;
	protected ref UIActionCheckbox m_ViewBaseBuilding;
	protected ref UIActionCheckbox m_ViewVehicles;
	protected ref UIActionCheckbox m_ViewItems;
	protected ref UIActionCheckbox m_ViewInfected;
	protected ref UIActionCheckbox m_ViewCreature;

	void JMESPForm()
	{
	}

	void ~JMESPForm()
	{
	}

	override string GetTitle()
	{
		return "ESP Viewer";
	}
	
	override string GetIconName()
	{
		return "X";
	}

	override bool ImageIsIcon()
	{
		return false;
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
		m_RangeSlider.SetValue( JMESPModule.Cast( module ).ESPRadius );
		m_RangeSlider.SetAppend(" metre(s)");
		m_RangeSlider.SetStepValue( 10 );

		Widget duoSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		m_ESPIsUpdating = UIActionManager.CreateCheckbox( duoSpacer, "Should Update Continuously", this, "Click_UpdateAtRate", JMESPModule.Cast( module ).ESPIsUpdating );

		m_UpdateRate = UIActionManager.CreateSlider( duoSpacer, "At Rate", 0.5, 10, this, "Change_UpdateRate" );
		m_UpdateRate.SetValue( JMESPModule.Cast( module ).ESPUpdateTime );
		m_UpdateRate.SetAppend(" second(s)");
		m_UpdateRate.SetStepValue( 0.5 );
	}

	void ESPFilters( Widget mainSpacer )
	{
		m_ViewEverything = UIActionManager.CreateCheckbox( mainSpacer, "Include Everything", this, "Click_AllESP", JMESPModule.Cast( module ).ViewEverything );

		UIActionManager.CreateEditableText( mainSpacer, "Class Filter: ", this, "Change_Filter", JMESPModule.Cast( module ).Filter );

		m_ViewPlayers = UIActionManager.CreateCheckbox( mainSpacer, "Include Players", this, "Click_PlayerESP", JMESPModule.Cast( module ).ViewPlayers );
		m_ViewBaseBuilding = UIActionManager.CreateCheckbox( mainSpacer, "Include Base Building", this, "Click_BaseBuildingESP", JMESPModule.Cast( module ).ViewBaseBuilding );
		m_ViewVehicles = UIActionManager.CreateCheckbox( mainSpacer, "Include Vehicles", this, "Click_VehicleESP", JMESPModule.Cast( module ).ViewVehicles );
		m_ViewItems = UIActionManager.CreateCheckbox( mainSpacer, "Include Items", this, "Click_ItemESP", JMESPModule.Cast( module ).ViewItems );
		m_ViewInfected = UIActionManager.CreateCheckbox( mainSpacer, "Include Infected", this, "Click_InfectedESP", JMESPModule.Cast( module ).ViewInfected );
		m_ViewCreature = UIActionManager.CreateCheckbox( mainSpacer, "Include Animals", this, "Click_CreatureESP", JMESPModule.Cast( module ).ViewCreature );
	}


	override void OnInit( bool fromMenu )
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

		m_RangeSlider.SetValue( JMESPModule.Cast( module ).ESPRadius );
		m_UpdateRate.SetValue( JMESPModule.Cast( module ).ESPUpdateTime );
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
		if ( JMESPModule.Cast( module ).CanViewPlayers )
		{
			m_ViewPlayers.Enable();
		} else
		{
			m_ViewPlayers.Disable();
		}

		if ( JMESPModule.Cast( module ).CanViewBaseBuilding )
		{
			m_ViewBaseBuilding.Enable();
		} else
		{
			m_ViewBaseBuilding.Disable();
		}

		if ( JMESPModule.Cast( module ).CanViewVehicles )
		{
			m_ViewVehicles.Enable();
		} else
		{
			m_ViewVehicles.Disable();
		}

		if ( JMESPModule.Cast( module ).CanViewItems )
		{
			m_ViewItems.Enable();
		} else
		{
			m_ViewItems.Disable();
		}

		if ( JMESPModule.Cast( module ).CanViewInfected )
		{
			m_ViewInfected.Enable();
		} else
		{
			m_ViewInfected.Disable();
		}

		if ( JMESPModule.Cast( module ).CanViewCreature )
		{
			m_ViewCreature.Enable();
		} else
		{
			m_ViewCreature.Disable();
		}

		if ( JMESPModule.Cast( module ).CanViewEverything )
		{
			m_ViewEverything.Enable();
		} else
		{
			m_ViewEverything.Disable();
		}
		
	}

	void UpdateCheckboxStates()
	{
		if ( JMESPModule.Cast( module ).ViewEverything )
		{
			m_ViewPlayers.Disable();
			m_ViewBaseBuilding.Disable();
			m_ViewVehicles.Disable();
			m_ViewItems.Disable();
			m_ViewInfected.Disable();
			m_ViewCreature.Disable();
		} else
		{
			DisableToggleableOptions();
		}
	}

	void Click_UpdateESP( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK ) return;

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
	void Click_HideESP( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).HideESP();
	}

	void Click_ChangeESPMode( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPWidget.ShowJustName = action.IsChecked();
	}
	
	void Change_Filter( UIEvent eid, ref UIActionEditableText action )
	{
		if ( eid != UIEvent.CHANGE ) return;

		JMESPModule.Cast( module ).Filter = action.GetText();
	}

	void Click_PlayerESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).ViewPlayers = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Players [" + JMESPModule.Cast( module ).ViewPlayers + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_BaseBuildingESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).ViewBaseBuilding = action.IsChecked();

		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Base Building [" + JMESPModule.Cast( module ).ViewBaseBuilding + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_VehicleESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).ViewVehicles = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Vehicles [" + JMESPModule.Cast( module ).ViewVehicles + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_ItemESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).ViewItems = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Items [" + JMESPModule.Cast( module ).ViewItems + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_InfectedESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).ViewInfected = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Infected [" + JMESPModule.Cast( module ).ViewInfected + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_CreatureESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).ViewCreature = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Creature [" + JMESPModule.Cast( module ).ViewCreature + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_AllESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).ViewEverything = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View All [" + JMESPModule.Cast( module ).ViewEverything + "]" ) );

		UpdateCheckboxStates();
	}

	void Change_UpdateRate( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		JMESPModule.Cast( module ).ESPUpdateTime = action.GetValue();
	}

	void Change_Range( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		JMESPModule.Cast( module ).ESPRadius = action.GetValue();
	}

	void Click_UseClassName( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPWidget.UseClassName = action.IsChecked();
	}

	void Click_EnableFullMap( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).EnableFullMap();
		
		m_FullMapESP.Disable();
	}

	void Click_UpdateAtRate( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		JMESPModule.Cast( module ).ESPUpdateLoop( action.IsChecked() );

		if ( JMESPModule.Cast( module ).ESPIsUpdating )
		{
			m_TextMode.Disable();

			JMESPWidget.ShowJustName = true;
		} else
		{
			m_TextMode.Enable();

			JMESPWidget.ShowJustName = m_TextMode.IsChecked();
		}
	}
}