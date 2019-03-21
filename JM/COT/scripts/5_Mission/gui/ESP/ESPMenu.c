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
		UIActionManager.CreateCheckbox( quadSpacer, "Text Mode", this, "Click_ChangeESPMode", ESPBox.ShowJustName );

		UIActionManager.CreateButton( quadSpacer, "Enable Fullmap ESP", this, "Click_EnableFullMap" );
		UIActionManager.CreateCheckbox( quadSpacer, "Use Class Name", this, "Click_UseClassName", ESPBox.UseClassName );

		UIActionManager.CreateText( mainSpacer, "Information: ", "Enabling full map ESP requires you to be in Free Cam." );
		UIActionManager.CreateText( mainSpacer, "Warning: ", "Enabling full map ESP removes your player, relog to fix." );

		m_UpdateRate = UIActionManager.CreateSlider( mainSpacer, "Update Rate", 0, 10, this, "Change_UpdateRate" );
		m_UpdateRate.SetValue( ESPModule.Cast( module ).ESPUpdateTime );
		m_UpdateRate.SetAppend(" seconds");
		m_UpdateRate.SetStepValue( 0.5 );

		m_RangeSlider = UIActionManager.CreateSlider( mainSpacer, "Radius", 0, 1000, this, "Change_Range" );
		m_RangeSlider.SetValue( ESPModule.Cast( module ).ESPRadius );
		m_RangeSlider.SetAppend(" metres");
		m_RangeSlider.SetStepValue( 10 );
	}

	void ESPFilters( Widget mainSpacer )
	{
		UIActionManager.CreateCheckbox( mainSpacer, "Player ESP", this, "Click_PlayerESP", ESPModule.Cast( module ).ViewPlayers );
		UIActionManager.CreateCheckbox( mainSpacer, "Base Building ESP", this, "Click_BaseBuildingESP", ESPModule.Cast( module ).ViewBaseBuilding );
		UIActionManager.CreateCheckbox( mainSpacer, "Vehicle ESP", this, "Click_VehicleESP", ESPModule.Cast( module ).ViewVehicles );
		UIActionManager.CreateCheckbox( mainSpacer, "Item ESP", this, "Click_ItemESP", ESPModule.Cast( module ).ViewItems );
		UIActionManager.CreateCheckbox( mainSpacer, "Infected ESP", this, "Click_InfectedESP", ESPModule.Cast( module ).ViewInfected );
		UIActionManager.CreateCheckbox( mainSpacer, "Creature ESP", this, "Click_CreatureESP", ESPModule.Cast( module ).ViewCreature );
		UIActionManager.CreateCheckbox( mainSpacer, "All ESP", this, "Click_AllESP", ESPModule.Cast( module ).ViewEverything );

		UIActionManager.CreateEditableText( mainSpacer, "Type Filter: ", this, "Change_Filter", ESPModule.Cast( module ).Filter );
	}

	void ESPObjectControl( Widget mainSpacer )
	{
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

		Widget mainSpacer = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 2, 1 );

		ESPControls( UIActionManager.CreateGridSpacer( mainSpacer, 3, 1 ) );

		Widget splitSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		ESPFilters( UIActionManager.CreateGridSpacer( splitSpacer, 8, 1 ) );

		ESPObjectControl( UIActionManager.CreateGridSpacer( splitSpacer, 8, 1 ) );
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

		ESPModule.Cast( module ).Filter

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

		if ( ESPModule.Cast( module ).IsShowing )
		{
			ESPModule.Cast( module ).HideESP();
		} else
		{
			ESPModule.Cast( module ).UpdateESP();
		}

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
	
	void Change_Filter( UIEvent eid, ref UIActionEditableText action )
	{
		if ( eid != UIEvent.CHANGE ) return;

		ESPModule.Cast( module ).Filter = action.GetText();
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