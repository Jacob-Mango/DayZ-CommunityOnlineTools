class ESPMenu extends Form
{
	protected ref UIActionEditableVector m_Position;

	protected ref UIActionText m_Name;

	protected ref UIActionSlider m_Pitch;
	protected ref UIActionSlider m_Yaw;
	protected ref UIActionSlider m_Roll;

	protected ref UIActionSlider m_Health;

	protected ref UIActionButton m_Set;
	protected ref UIActionButton m_Delete;

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

	void ESPMenu()
	{
	}

	void ~ESPMenu()
	{
	}

	override string GetTitle()
	{
		return "X-Ray Vision";
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
		m_TextMode = UIActionManager.CreateCheckbox( quadSpacer, "Text Mode", this, "Click_ChangeESPMode", ESPBox.ShowJustName );

		m_FullMapESP = UIActionManager.CreateButton( quadSpacer, "Enable Fullmap ESP", this, "Click_EnableFullMap" );
		UIActionManager.CreateCheckbox( quadSpacer, "Use Class Name", this, "Click_UseClassName", ESPBox.UseClassName );

		UIActionManager.CreateText( mainSpacer, "Information: ", "Enabling full map ESP requires you to be in Free Cam." );
		UIActionManager.CreateText( mainSpacer, "Warning: ", "Enabling full map ESP removes your character from the server, relog to fix." );

		m_RangeSlider = UIActionManager.CreateSlider( mainSpacer, "Radius", 0, 1000, this, "Change_Range" );
		m_RangeSlider.SetValue( ESPModule.Cast( module ).ESPRadius );
		m_RangeSlider.SetAppend(" metre(s)");
		m_RangeSlider.SetStepValue( 10 );

		Widget duoSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		m_ESPIsUpdating = UIActionManager.CreateCheckbox( duoSpacer, "Should Update Continuously", this, "Click_UpdateAtRate", ESPModule.Cast( module ).ESPIsUpdating );

		m_UpdateRate = UIActionManager.CreateSlider( duoSpacer, "At Rate", 0.5, 10, this, "Change_UpdateRate" );
		m_UpdateRate.SetValue( ESPModule.Cast( module ).ESPUpdateTime );
		m_UpdateRate.SetAppend(" second(s)");
		m_UpdateRate.SetStepValue( 0.5 );
	}

	void ESPFilters( Widget mainSpacer )
	{
		m_ViewEverything = UIActionManager.CreateCheckbox( mainSpacer, "Include Everything", this, "Click_AllESP", ESPModule.Cast( module ).ViewEverything );

		UIActionManager.CreateEditableText( mainSpacer, "Class Filter: ", this, "Change_Filter", ESPModule.Cast( module ).Filter );

		m_ViewPlayers = UIActionManager.CreateCheckbox( mainSpacer, "Include Players", this, "Click_PlayerESP", ESPModule.Cast( module ).ViewPlayers );
		m_ViewBaseBuilding = UIActionManager.CreateCheckbox( mainSpacer, "Include Base Building", this, "Click_BaseBuildingESP", ESPModule.Cast( module ).ViewBaseBuilding );
		m_ViewVehicles = UIActionManager.CreateCheckbox( mainSpacer, "Include Vehicles", this, "Click_VehicleESP", ESPModule.Cast( module ).ViewVehicles );
		m_ViewItems = UIActionManager.CreateCheckbox( mainSpacer, "Include Items", this, "Click_ItemESP", ESPModule.Cast( module ).ViewItems );
		m_ViewInfected = UIActionManager.CreateCheckbox( mainSpacer, "Include Infected", this, "Click_InfectedESP", ESPModule.Cast( module ).ViewInfected );
		m_ViewCreature = UIActionManager.CreateCheckbox( mainSpacer, "Include Animals", this, "Click_CreatureESP", ESPModule.Cast( module ).ViewCreature );
	}

	void ESPObjectControl( Widget mainSpacer )
	{
		m_Name = UIActionManager.CreateText( mainSpacer, "Name: ", "" );

		m_Position = UIActionManager.CreateEditableVector( mainSpacer, "Position: ", this, "Change_Position" );
		
		m_Pitch = UIActionManager.CreateSlider( mainSpacer, "Pitch", -180, 180, this, "Change_Pitch" );
		m_Pitch.SetValue( 0 );
		m_Pitch.SetAppend("°");
		m_Pitch.SetStepValue( 0.5 );

		m_Yaw = UIActionManager.CreateSlider( mainSpacer, "Yaw", -180, 180, this, "Change_Yaw" );
		m_Yaw.SetValue( 0 );
		m_Yaw.SetAppend("°");
		m_Yaw.SetStepValue( 0.5 );

		m_Roll = UIActionManager.CreateSlider( mainSpacer, "Roll", -180, 180, this, "Change_Roll" );
		m_Roll.SetValue( 0 );
		m_Roll.SetAppend("°");
		m_Roll.SetStepValue( 0.5 );

		m_Health = UIActionManager.CreateSlider( mainSpacer, "Health", 0, 100, this, "Change_Health" );
		m_Health.SetValue( 0 );
		m_Health.SetAppend(" HP");
		m_Health.SetStepValue( 1 );

		m_Set = UIActionManager.CreateButton( mainSpacer, "Set", this, "Click_Set" );
		m_Delete = UIActionManager.CreateButton( mainSpacer, "Delete", this, "Click_Delete" );
	}

	override void OnInit( bool fromMenu )
	{
		ESPBox.espMenu = this;

		Widget mainSpacer = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 3, 1 );

		ESPControls( UIActionManager.CreateGridSpacer( mainSpacer, 3, 1 ) );

		UIActionManager.CreateSpacer( mainSpacer );

		Widget splitSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		ESPFilters( UIActionManager.CreateGridSpacer( splitSpacer, 8, 1 ) );

		ESPObjectControl( UIActionManager.CreateGridSpacer( splitSpacer, 8, 1 ) );
	}

	override void OnShow()
	{
		super.OnShow();

		OnSelect();

		m_Name.SetText( ESPModule.Cast( module ).Name );
		m_Position.SetValue( ESPModule.Cast( module ).Position );
		m_Pitch.SetValue( ESPModule.Cast( module ).Rotation[1] );
		m_Yaw.SetValue( ESPModule.Cast( module ).Rotation[0] );
		m_Roll.SetValue( ESPModule.Cast( module ).Rotation[2] );
		m_Health.SetMax( ESPModule.Cast( module ).MaxHealth );
		m_Health.SetValue( ESPModule.Cast( module ).Health );
		m_RangeSlider.SetValue( ESPModule.Cast( module ).ESPRadius );
		m_UpdateRate.SetValue( ESPModule.Cast( module ).ESPUpdateTime );
		m_ESPIsUpdating.SetChecked( ESPModule.Cast( module ).ESPIsUpdating );

		if ( COTPlayerIsRemoved )
		{
			m_FullMapESP.Disable();
		}

		// ESPModule.Cast( module ).Filter

		UpdateESPButtonName();

		UpdateCheckboxStates();
	}

	override void OnHide()
	{
		super.OnHide();
	}

	void OnSelect()
	{
		if ( ESPModule.Cast( module ).GetSelectedCount() != 1 || ESPModule.Cast( module ).ESPIsUpdating )
		{
			m_Name.Disable();
			m_Position.Disable();
			m_Pitch.Disable();
			m_Yaw.Disable();
			m_Roll.Disable();
			m_Health.Disable();
			m_Set.Disable();
		} else
		{
			m_Name.Enable();
			m_Position.Enable();
			m_Pitch.Enable();
			m_Yaw.Enable();
			m_Roll.Enable();
			m_Health.Enable();
			m_Set.Enable();

			m_Name.SetText( ESPModule.Cast( module ).Name );
			m_Position.SetValue( ESPModule.Cast( module ).Position );
			m_Pitch.SetValue( ESPModule.Cast( module ).Rotation[1] );
			m_Yaw.SetValue( ESPModule.Cast( module ).Rotation[0] );
			m_Roll.SetValue( ESPModule.Cast( module ).Rotation[2] );
			m_Health.SetMax( ESPModule.Cast( module ).MaxHealth );
			m_Health.SetValue( ESPModule.Cast( module ).Health );
		}
	}

	void UpdateESPButtonName()
	{
		bool isShowing = ESPModule.Cast( module ).IsShowing;

		if ( isShowing )
		{
			m_UpdateShow.SetButton( "Clear ESP" );
		} else
		{
			m_UpdateShow.SetButton( "Enable ESP" );
		}
	}

	void UpdateCheckboxStates()
	{
		if ( ESPModule.Cast( module ).ViewEverything )
		{
			m_ViewPlayers.Disable();
			m_ViewBaseBuilding.Disable();
			m_ViewVehicles.Disable();
			m_ViewItems.Disable();
			m_ViewInfected.Disable();
			m_ViewCreature.Disable();
		} else
		{
			m_ViewPlayers.Enable();
			m_ViewBaseBuilding.Enable();
			m_ViewVehicles.Enable();
			m_ViewItems.Enable();
			m_ViewInfected.Enable();
			m_ViewCreature.Enable();
		}
	}

	void Click_UpdateESP( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK ) return;

		if ( ESPModule.Cast( module ).IsShowing )
		{
			ESPModule.Cast( module ).ESPIsUpdating = false;
			
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( ESPModule.Cast( module ).UpdateESP );

			ESPModule.Cast( module ).HideESP();
		} else
		{
			ESPModule.Cast( module ).ESPIsUpdating = m_ESPIsUpdating.IsChecked();

			ESPModule.Cast( module ).UpdateESP();
		}

		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "ESP Showing " + ESPModule.Cast( module ).IsShowing ) );

		UpdateESPButtonName();

		UpdateCheckboxStates();
	}

	// Removed.
	void Click_HideESP( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).HideESP();
	}

	void Click_ChangeESPMode( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPBox.ShowJustName = action.IsChecked();
	}
	
	void Change_Filter( UIEvent eid, ref UIActionEditableText action )
	{
		if ( eid != UIEvent.CHANGE ) return;

		ESPModule.Cast( module ).Filter = action.GetText();
	}

	void Click_PlayerESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewPlayers = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Players [" + ESPModule.Cast( module ).ViewPlayers + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_BaseBuildingESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewBaseBuilding = action.IsChecked();

		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Base Building [" + ESPModule.Cast( module ).ViewBaseBuilding + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_VehicleESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewVehicles = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Vehicles [" + ESPModule.Cast( module ).ViewVehicles + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_ItemESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewItems = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Items [" + ESPModule.Cast( module ).ViewItems + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_InfectedESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewInfected = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Infected [" + ESPModule.Cast( module ).ViewInfected + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_CreatureESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewCreature = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View Creature [" + ESPModule.Cast( module ).ViewCreature + "]" ) );

		UpdateCheckboxStates();
	}

	void Click_AllESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewEverything = action.IsChecked();
		
		GetRPCManager().SendRPC( "COT_ESP", "ESPLog", new Param1< string >( "View All [" + ESPModule.Cast( module ).ViewEverything + "]" ) );

		UpdateCheckboxStates();
	}

	void Change_Position( UIEvent eid, ref UIActionEditableVector action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).Position = action.GetValue();
	}

	void Change_Pitch( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).Rotation[1] = action.GetValue();
	}

	void Change_Yaw( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).Rotation[0] = action.GetValue();
	}

	void Change_Roll( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).Rotation[2] = action.GetValue();
	}

	void Change_Health( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).Health = action.GetValue();
	}

	void Click_Set( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).SetSelected();
	}

	void Click_Delete( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).DeleteSelected();
	}

	void Change_UpdateRate( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).ESPUpdateTime = action.GetValue();
	}

	void Change_Range( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).ESPRadius = action.GetValue();
	}

	void Click_UseClassName( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPBox.UseClassName = action.IsChecked();
	}

	void Click_EnableFullMap( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).EnableFullMap();
		
		m_FullMapESP.Disable();
	}

	void Click_UpdateAtRate( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ESPUpdateLoop( action.IsChecked() );

		if ( ESPModule.Cast( module ).ESPIsUpdating )
		{
			m_TextMode.Disable();

			ESPBox.ShowJustName = true;
		} else
		{
			m_TextMode.Enable();

			ESPBox.ShowJustName = m_TextMode.IsChecked();
		}
	}
}