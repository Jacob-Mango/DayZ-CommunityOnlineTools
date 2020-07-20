enum JMVehicleManagerModuleRPC
{
	INVALID = 10280,
	COUNT
};

class JMVehicleManagerModule: JMRenderableModuleBase
{
	protected ref array<ref ExpansionVehicleMetaData> m_Vehicles;

	void JMVehicleManagerModule()
	{
		GetPermissionsManager().RegisterPermission( "VehiclesManager.View" );
		GetPermissionsManager().RegisterPermission( "VehiclesManager.Delete" );
		GetPermissionsManager().RegisterPermission( "VehiclesManager.Delete.All" );
		GetPermissionsManager().RegisterPermission( "VehiclesManager.Delete.Destroyed" );
		GetPermissionsManager().RegisterPermission( "VehiclesManager.Delete.Unclaimed" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "VehiclesManager.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/vehiclesmanager_form.layout";
	}

	override string GetTitle()
	{
		return "#STR_COT_VEHICLE_MANAGER_MODULE_NAME";
	}
	
	override string GetIconName()
	{
		return "VM";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "Vehicle Manager Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();
	}

	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();
	}

	override int GetRPCMin()
	{
		return JMVehicleManagerModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMVehicleManagerModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		}
	}
	
	ref array<ref ExpansionVehicleMetaData> GetVehicles()
	{
		return m_Vehicles;
	}
}