class ESPMenu extends Form
{
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

		UIActionManager.CreateButton( leftSpacer, "Show/Update ESP", this, "Click_UpdateESP" );
		UIActionManager.CreateButton( leftSpacer, "Hide ESP", this, "Click_HideESP" );
		UIActionManager.CreateCheckbox( leftSpacer, "Player ESP", this, "Click_PlayerESP", ESPModule.Cast( module ).ViewPlayers );
		UIActionManager.CreateCheckbox( leftSpacer, "Base Building ESP", this, "Click_BaseBuildingESP", ESPModule.Cast( module ).ViewBaseBuilding );
		UIActionManager.CreateCheckbox( leftSpacer, "Vehicle ESP", this, "Click_VehicleESP", ESPModule.Cast( module ).ViewVehicles );
		UIActionManager.CreateCheckbox( leftSpacer, "Item ESP", this, "Click_ItemESP", ESPModule.Cast( module ).ViewItems );

		ref UIActionSlider rangeSlider = UIActionManager.CreateSlider( mainSpacer, "ESP Range", 0, 1000, this, "Click_UpdateRange" );
		rangeSlider.SetValue( ESPModule.Cast( module ).ESPRadius );
		rangeSlider.SetAppend(" metres");
	}

	void Click_UpdateESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).UpdateESP();
	}

	void Click_HideESP( UIEvent eid, ref UIActionCheckbox action )
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

	void Click_UpdateRange( UIEvent eid, ref UIActionSlider action )
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