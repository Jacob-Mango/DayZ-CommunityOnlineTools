class VehicleSpawnerModule: EditorModule
{
	protected ref VehicleSpawnerSettings settings;
	protected ref VehicleSpawnerMeta meta;

	void VehicleSpawnerModule()
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
		
		meta = VehicleSpawnerMeta.DeriveFromSettings( settings );
	}

	override void ReloadSettings()
	{
		super.ReloadSettings();

		settings = VehicleSpawnerSettings.Load();

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
		return "JM/COT/GUI/layouts/VehicleSpawner/VehicleSpawner.layout";
	}

	ref array< string > GetVehicles()
	{
		return meta.Vehicles;
	}
	
	void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		//if( type == CallType.Server )
		//{
		//	GetRPCManager().SendRPC( "COT_VehicleSpawner", "LoadData", new Param1< string >( JsonFileLoader< VehicleSpawnerMeta >.JsonMakeData( VehicleSpawnerMeta.DeriveFromSettings( settings ) ) ), true );
		//}

		if( type == CallType.Client )
		{
			Param1< string > data;
			if ( !ctx.Read( data ) ) return;

			JsonFileLoader< VehicleSpawnerMeta >.JsonLoadData( data.param1, meta );

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

	void SpawnCursor( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		ref Param2< string, vector > data;
		if ( !ctx.Read( data ) ) return;

		Print("debug : " + data.param1 + " - " + data.param2);
		ref VehicleSpawnerFile file = settings.Vehicles.Get( data.param1 );

		if ( file == NULL ) return;

		ref array< string > attachments = file.Parts;

		if ( attachments.Count() == 0 )
		{
			return;
		}

		if ( !GetPermissionsManager().HasPermission( "VehicleSpawner.Vehicle." + data.param1, sender ) )
		{
			return;
		}
		
		if( type == CallType.Server )
		{
			Car oCar = Car.Cast( GetGame().CreateObject( data.param1, data.param2 ) );

			for (int j = 0; j < attachments.Count(); j++)
			{
				oCar.GetInventory().CreateInInventory( attachments[j] );
			}

			FillCar( oCar, CarFluid.FUEL );
			FillCar( oCar, CarFluid.OIL );
			FillCar( oCar, CarFluid.BRAKE );
			FillCar( oCar, CarFluid.COOLANT );

			COTLog( sender, "Spawned vehicle " + oCar.GetDisplayName() + " (" + data.param1 + ") at " + data.param2.ToString() );
		}
	}
}