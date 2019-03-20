class ESPMenu extends Form
{
	protected ref UIActionEditableVector m_Position;

	protected ref UIActionSlider m_Pitch;
	protected ref UIActionSlider m_Yaw;
	protected ref UIActionSlider m_Roll;

	protected ref UIActionButton m_Set;
	protected ref UIActionButton m_Delete;

	protected ref UIActionSlider m_RangeSlider;

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

		Widget upperSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );

		m_RangeSlider = UIActionManager.CreateSlider( mainSpacer, "ESP Range", 0, 1000, this, "Change_Range" );
		m_RangeSlider.SetValue( ESPModule.Cast( module ).ESPRadius );
		m_RangeSlider.SetAppend(" metres");

		Widget leftSpacer = UIActionManager.CreateGridSpacer( upperSpacer, 2, 1 );
		Widget leftUpperSpacer = UIActionManager.CreateGridSpacer( leftSpacer, 9, 1 );
		Widget leftLowerSpacer = UIActionManager.CreateGridSpacer( leftSpacer, 5, 1 );

		Widget rightSpacer = UIActionManager.CreateGridSpacer( upperSpacer, 7, 1 );

		UIActionManager.CreateButton( leftUpperSpacer, "Show/Update ESP", this, "Click_UpdateESP" );
		UIActionManager.CreateButton( leftUpperSpacer, "Hide ESP", this, "Click_HideESP" );
		UIActionManager.CreateCheckbox( leftUpperSpacer, "Minified ESP Mode", this, "Click_ChangeESPMode", ESPBox.ShowJustName );
		UIActionManager.CreateCheckbox( leftUpperSpacer, "Player ESP", this, "Click_PlayerESP", ESPModule.Cast( module ).ViewPlayers );
		UIActionManager.CreateCheckbox( leftUpperSpacer, "Base Building ESP", this, "Click_BaseBuildingESP", ESPModule.Cast( module ).ViewBaseBuilding );
		UIActionManager.CreateCheckbox( leftUpperSpacer, "Vehicle ESP", this, "Click_VehicleESP", ESPModule.Cast( module ).ViewVehicles );
		UIActionManager.CreateCheckbox( leftUpperSpacer, "Item ESP", this, "Click_ItemESP", ESPModule.Cast( module ).ViewItems );
		UIActionManager.CreateCheckbox( leftUpperSpacer, "Infected ESP", this, "Click_InfectedESP", ESPModule.Cast( module ).ViewInfected );
		UIActionManager.CreateCheckbox( leftUpperSpacer, "Creature ESP", this, "Click_CreatureESP", ESPModule.Cast( module ).ViewCreature );
		UIActionManager.CreateCheckbox( leftLowerSpacer, "All ESP", this, "Click_AllESP", ESPModule.Cast( module ).ViewEverything );

		m_Position = UIActionManager.CreateEditableVector( rightSpacer, "Position: ", this, "Change_Position" );
		
		m_Pitch = UIActionManager.CreateSlider( rightSpacer, "Pitch", -180, 180, this, "Change_Pitch" );
		m_Pitch.SetValue( 0 );
		m_Pitch.SetAppend("°");

		m_Yaw = UIActionManager.CreateSlider( rightSpacer, "Yaw", -180, 180, this, "Change_Yaw" );
		m_Yaw.SetValue( 0 );
		m_Yaw.SetAppend("°");

		m_Roll = UIActionManager.CreateSlider( rightSpacer, "Roll", -180, 180, this, "Change_Roll" );
		m_Roll.SetValue( 0 );
		m_Roll.SetAppend("°");

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
		m_RangeSlider.SetValue( ESPModule.Cast( module ).ESPRadius );
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
	}

	void Click_UpdateESP( UIEvent eid, ref UIActionButton action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).UpdateESP();
	}

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

	void Change_Range( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).ESPRadius = action.GetValue();
	}
}