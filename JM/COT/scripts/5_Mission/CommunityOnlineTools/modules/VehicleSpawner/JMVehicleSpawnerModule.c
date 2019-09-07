class JMVehicleSpawnerModule: JMRenderableModuleBase
{
	protected ref JMVehicleSpawnerSettings settings;
	protected ref JMVehicleSpawnerMeta meta;

	void JMVehicleSpawnerModule()
	{
		GetRPCManager().AddRPC( "COT_VehicleSpawner", "LoadData", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT_VehicleSpawner", "SpawnCursor", this, SingeplayerExecutionType.Server );

		GetPermissionsManager().RegisterPermission( "VehicleSpawner.View" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "VehicleSpawner.View" );
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if ( GetGame().IsClient() )
			GetRPCManager().SendRPC( "COT_VehicleSpawner", "LoadData", new Param, true );
		
		meta = JMVehicleSpawnerMeta.DeriveFromSettings( settings );
	}

	override void ReloadSettings()
	{
		super.ReloadSettings();

		settings = JMVehicleSpawnerSettings.Load();

		for ( int i = 0; i < settings.Vehicles.Count(); i++ )
		{
			string vehicle = settings.Vehicles.GetKey( i );
			vehicle.Replace( " ", "." );
			GetPermissionsManager().RegisterPermission( "VehicleSpawner." + vehicle );
		}
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		if ( GetGame().IsServer() )
			settings.Save();
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/vehiclespawner_form.layout";
	}

	ref array< string > GetVehicles()
	{
		return meta.Vehicles;
	}
	
	void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Server )
		{
			GetRPCManager().SendRPC( "COT_VehicleSpawner", "LoadData", new Param1< string >( JsonFileLoader< JMVehicleSpawnerMeta >.JsonMakeData( JMVehicleSpawnerMeta.DeriveFromSettings( settings ) ) ) );
		}

		if ( type == CallType.Client )
		{
			Param1< string > data;
			if ( !ctx.Read( data ) ) return;

			JsonFileLoader< JMVehicleSpawnerMeta >.JsonLoadData( data.param1, meta );

			for ( int i = 0; i < meta.Vehicles.Count(); i++ )
			{
				string vehicle = meta.Vehicles[i];
				vehicle.Replace( " ", "." );
				GetPermissionsManager().RegisterPermission( "VehicleSpawner.Vehicle." + vehicle );
			}
		}
	}

	private void FillCar( Car car, CarFluid fluid )
	{
		float cap = car.GetFluidCapacity( fluid );
		car.Fill( fluid, cap );
	}

	Car SpawnVehicle( string type, vector position, PlayerIdentity sender = NULL )
	{
		ref JMVehicleSpawnerSerialize file = settings.Vehicles.Get( type );

		if ( file == NULL ) 
			return NULL;

		ref array< string > attachments = file.Parts;

		if ( attachments.Count() == 0 )
			return NULL;

		Car oCar = Car.Cast( GetGame().CreateObject( type, position ) );

		for (int j = 0; j < attachments.Count(); j++)
		{
			oCar.GetInventory().CreateInInventory( attachments[j] );
		}

		FillCar( oCar, CarFluid.FUEL );
		FillCar( oCar, CarFluid.OIL );
		FillCar( oCar, CarFluid.BRAKE );
		FillCar( oCar, CarFluid.COOLANT );

		GetCommunityOnlineToolsBase().Log( sender, "Spawned vehicle " + oCar.GetDisplayName() + " (" + type + ") at " + position.ToString() );

		// SendAdminNotification( sender, NULL, "You have spawned a " + oCar.GetDisplayName() + " at " + VectorToString( position, 1 ) );

		return oCar;
	}

	void SpawnCursor( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		ref Param2< string, vector > data;
		if ( !ctx.Read( data ) )
			return;

		if ( !GetPermissionsManager().HasPermission( "VehicleSpawner.Vehicle." + data.param1, sender ) )
			return;
		
		if ( type == CallType.Server )
		{
			SpawnVehicle( data.param1, data.param2, sender );
		}
	}
}

static Car SpawnVehicleAtPosition( string type, vector position, PlayerIdentity sender = NULL )
{
	JMVehicleSpawnerModule module = JMVehicleSpawnerModule.Cast( GetModuleManager().GetModule( JMVehicleSpawnerModule ) );
	if ( module )
	{
		return module.SpawnVehicle( type, position, sender );
	}
	return NULL;
}