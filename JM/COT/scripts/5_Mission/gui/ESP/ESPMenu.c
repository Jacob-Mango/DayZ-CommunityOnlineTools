class ESPMenu extends Form
{
	protected Widget m_ActionsWrapper;

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
		m_ActionsWrapper = UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 4, 1 );

		UIActionManager.CreateButton( m_ActionsWrapper, "Update ESP", this, "Click_UpdateESP" );
		UIActionManager.CreateCheckbox( m_ActionsWrapper, "Player ESP", this, "Click_PlayerESP", false );
		UIActionManager.CreateCheckbox( m_ActionsWrapper, "Base Building ESP", this, "Click_BaseBuildingESP", false );
		UIActionManager.CreateCheckbox( m_ActionsWrapper, "Vehicle ESP", this, "Click_VehicleESP", ESPModule.Cast( module ).ViewVehicles );
	}

	void Click_UpdateESP( UIEvent eid, ref UIActionCheckbox action )
	{
		if ( eid != UIEvent.CLICK ) return;
		
		ESPModule.Cast( module ).UpdateESP();
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

	override void OnShow()
	{
		super.OnShow();
	}

	override void OnHide()
	{
		super.OnHide();
	}
}