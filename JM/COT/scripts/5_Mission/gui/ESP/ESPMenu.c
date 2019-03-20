class ESPMenu extends Form
{
	protected ref UIActionEditableVector m_Position;

	protected ref UIActionSlider m_Pitch;
	protected ref UIActionSlider m_Yaw;
	protected ref UIActionSlider m_Roll;

	protected ref UIActionButton m_Delete;

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
		Widget mainSpacer = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 2, 1 );

		Widget upperSpacer = UIActionManager.CreateGridSpacer( mainSpacer, 1, 2 );
		Widget leftSpacer = UIActionManager.CreateGridSpacer( upperSpacer, 7, 1 );
		Widget rightSpacer = UIActionManager.CreateGridSpacer( upperSpacer, 7, 1 );

		UIActionManager.CreateButton( leftSpacer, "Show/Update ESP", this, "Click_UpdateESP" );
		UIActionManager.CreateButton( leftSpacer, "Hide ESP", this, "Click_HideESP" );
		UIActionManager.CreateCheckbox( leftSpacer, "Player ESP", this, "Click_PlayerESP", ESPModule.Cast( module ).ViewPlayers );
		UIActionManager.CreateCheckbox( leftSpacer, "Base Building ESP", this, "Click_BaseBuildingESP", ESPModule.Cast( module ).ViewBaseBuilding );
		UIActionManager.CreateCheckbox( leftSpacer, "Vehicle ESP", this, "Click_VehicleESP", ESPModule.Cast( module ).ViewVehicles );
		UIActionManager.CreateCheckbox( leftSpacer, "Item ESP", this, "Click_ItemESP", ESPModule.Cast( module ).ViewItems );

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

		m_Delete = UIActionManager.CreateButton( rightSpacer, "Delete", this, "Click_Delete" );

		ref UIActionSlider rangeSlider = UIActionManager.CreateSlider( mainSpacer, "ESP Range", 0, 1000, this, "Change_Range" );
		rangeSlider.SetValue( ESPModule.Cast( module ).ESPRadius );
		rangeSlider.SetAppend(" metres");
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

	void Change_Position( UIEvent eid, ref UIActionEditableVector action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).SetPosition( action.GetVector() );
	}

	void Change_Pitch( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).SetPitch( action.GetValue() );
	}

	void Change_Yaw( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).SetYaw( action.GetValue() );
	}

	void Change_Roll( UIEvent eid, ref UIActionSlider action )
	{
		if ( eid != UIEvent.CHANGE ) return;
		
		ESPModule.Cast( module ).SetRoll( action.GetValue() );
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

	override void OnShow()
	{
		super.OnShow();
	}

	override void OnHide()
	{
		super.OnHide();
	}
}