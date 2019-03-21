class ESPMenu extends Form
{
	protected ref UIActionEditableVector m_Position;

	protected ref UIActionSlider m_Pitch;
	protected ref UIActionSlider m_Yaw;
	protected ref UIActionSlider m_Roll;

	protected ref UIActionSlider m_Health;

	protected ref UIActionButton m_Set;
	protected ref UIActionButton m_Delete;

	protected ref UIActionSlider m_RangeSlider;

	protected ref UIActionButton m_UpdateShow;
	protected ref UIActionSlider m_UpdateRate;

	void ESPMenu()
	{
	}

	void ~ESPMenu()
	{
	}

	override string GetTitle()
	{
		return "ESP";
	}
	
	override string GetIconName()
	{
		return "ESP";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnInit( bool fromMenu )
	{
		ESPBox.espMenu = this;

		Widget mainSpacer = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 2, 1 );

		Widget upperSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 3, 1 );

		Widget lowerSpacerQuad = UIActionManager.CreateGridSpacer( upperSpacer, 2, 2 );
		
		m_UpdateShow = UIActionManager.CreateButton( lowerSpacerQuad, "Show ESP", this, "Click_UpdateESP" );
		UIActionManager.CreateCheckbox( lowerSpacerQuad, "Text Mode", this, "Click_ChangeESPMode", ESPBox.ShowJustName );

		UIActionManager.CreateButton( lowerSpacerQuad, "Enable Fullmap ESP", this, "Click_EnableFullMap" );
		UIActionManager.CreateCheckbox( lowerSpacerQuad, "Use Class Name", this, "Click_UseClassName", ESPBox.UseClassName );

		UIActionManager.CreateText( upperSpacer, "Info: Enabling full map ESP requires you to be in Free Cam." );
		UIActionManager.CreateText( upperSpacer, "Warning: Enabling full map ESP removes your player, relog to fix." );

		m_UpdateRate = UIActionManager.CreateSlider( upperSpacer, "Update Rate", 0, 10, this, "Change_UpdateRate" );
		m_UpdateRate.SetValue( ESPModule.Cast( module ).ESPUpdateTime );
		m_UpdateRate.SetAppend(" seconds");
		m_UpdateRate.SetStepValue( 0.5 );

		m_RangeSlider = UIActionManager.CreateSlider( upperSpacer, "Radius", 0, 1000, this, "Change_Range" );
		m_RangeSlider.SetValue( ESPModule.Cast( module ).ESPRadius );
		m_RangeSlider.SetAppend(" metres");
		m_RangeSlider.SetStepValue( 10 );

		Widget lowerSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		Widget leftSpacer = UIActionManager.CreateGridSpacer( lowerSpacer, 8, 1 );

		UIActionManager.CreateCheckbox( leftSpacer, "Player ESP", this, "Click_PlayerESP", ESPModule.Cast( module ).ViewPlayers );
		UIActionManager.CreateCheckbox( leftSpacer, "Base Building ESP", this, "Click_BaseBuildingESP", ESPModule.Cast( module ).ViewBaseBuilding );
		UIActionManager.CreateCheckbox( leftSpacer, "Vehicle ESP", this, "Click_VehicleESP", ESPModule.Cast( module ).ViewVehicles );
		UIActionManager.CreateCheckbox( leftSpacer, "Item ESP", this, "Click_ItemESP", ESPModule.Cast( module ).ViewItems );
		UIActionManager.CreateCheckbox( leftSpacer, "Infected ESP", this, "Click_InfectedESP", ESPModule.Cast( module ).ViewInfected );
		UIActionManager.CreateCheckbox( leftSpacer, "Creature ESP", this, "Click_CreatureESP", ESPModule.Cast( module ).ViewCreature );
		UIActionManager.CreateCheckbox( leftSpacer, "All ESP", this, "Click_AllESP", ESPModule.Cast( module ).ViewEverything );

		Widget rightSpacer = UIActionManager.CreateGridSpacer( lowerSpacer, 8, 1 );

		m_Position = UIActionManager.CreateEditableVector( rightSpacer, "Position: ", this, "Change_Position" );
		
		m_Pitch = UIActionManager.CreateSlider( rightSpacer, "Pitch", -180, 180, this, "Change_Pitch" );
		m_Pitch.SetValue( 0 );
		m_Pitch.SetAppend("°");
		m_Pitch.SetStepValue( 0.5 );

		m_Yaw = UIActionManager.CreateSlider( rightSpacer, "Yaw", -180, 180, this, "Change_Yaw" );
		m_Yaw.SetValue( 0 );
		m_Yaw.SetAppend("°");
		m_Yaw.SetStepValue( 0.5 );

		m_Roll = UIActionManager.CreateSlider( rightSpacer, "Roll", -180, 180, this, "Change_Roll" );
		m_Roll.SetValue( 0 );
		m_Roll.SetAppend("°");
		m_Roll.SetStepValue( 0.5 );

		m_Health = UIActionManager.CreateSlider( rightSpacer, "Health", 0, 100, this, "Change_Health" );
		m_Health.SetValue( 0 );
		m_Health.SetAppend(" HP");
		m_Health.SetStepValue( 1 );

		m_Set = UIActionManager.CreateButton( rightSpacer, "Set", this, "Click_Set" );
		m_Delete = UIActionManager.CreateButton( rightSpacer, "Delete", this, "Click_Delete" );
	}

	override void OnShow()
	{
		super.OnShow();

		OnSelect();

		m_Position.SetValue( ESPModule.Cast( module ).Position );
		m_Pitch.SetValue( ESPModule.Cast( module ).Rotation[1] );
		m_Yaw.SetValue( ESPModule.Cast( module ).Rotation[0] );
		m_Roll.SetValue( ESPModule.Cast( module ).Rotation[2] );
		m_Health.SetMax( ESPModule.Cast( module ).MaxHealth );
		m_Health.SetValue( ESPModule.Cast( module ).Health );
		m_RangeSlider.SetValue( ESPModule.Cast( module ).ESPRadius );
		m_UpdateRate.SetValue( ESPModule.Cast( module ).ESPUpdateTime );

		UpdateESPButtonName();
	}

	override void OnHide()
	{
		super.OnHide();
	}

	void OnSelect()
	{
		if ( ESPModule.Cast( module ).GetSelectedCount() != 1 )
		{
			m_Position.Disable();
			m_Pitch.Disable();
			m_Yaw.Disable();
			m_Roll.Disable();
			m_Set.Disable();
		} else
		{
			m_Position.Enable();
			m_Pitch.Enable();
			m_Yaw.Enable();
			m_Roll.Enable();
			m_Set.Enable();
		}

		m_Position.SetValue( ESPModule.Cast( module ).Position );
		m_Pitch.SetValue( ESPModule.Cast( module ).Rotation[1] );
		m_Yaw.SetValue( ESPModule.Cast( module ).Rotation[0] );
		m_Roll.SetValue( ESPModule.Cast( module ).Rotation[2] );
		m_Health.SetMax( ESPModule.Cast( module ).MaxHealth );
		m_Health.SetValue( ESPModule.Cast( module ).Health );
	}

	void UpdateESPButtonName()
	{
		bool isShowing = ESPModule.Cast( module ).IsShowing;

		if ( ESPModule.Cast( module ).ESPUpdateTime > 0 )
		{
			if ( isShowing )
			{
				m_UpdateShow.SetButton( "Stop Updating ESP" );
			} else
			{
				m_UpdateShow.SetButton( "Start Updating ESP" );
			}
		} else
		{
			if ( isShowing )
			{
				m_UpdateShow.SetButton( "Hide ESP" );
			} else
			{
				m_UpdateShow.SetButton( "Show ESP" );
			}
		}
	}

	void Click_UpdateESP( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).UpdateESP();

		UpdateESPButtonName();
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

	void Click_PlayerESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewPlayers = action.IsChecked();
	}

	void Click_BaseBuildingESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewBaseBuilding = action.IsChecked();
	}

	void Click_VehicleESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewVehicles = action.IsChecked();
	}

	void Click_ItemESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewItems = action.IsChecked();
	}

	void Click_InfectedESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewInfected = action.IsChecked();
	}

	void Click_CreatureESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewCreature = action.IsChecked();
	}

	void Click_AllESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).ViewEverything = action.IsChecked();
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

		UpdateESPButtonName();
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
	}
}