enum JMVehicleSpawnerModuleRPC
{
    INVALID = 10240,
    Load,
	SpawnPosition,
    COUNT
};

class JMVehicleSpawnerModule: JMRenderableModuleBase
{
	protected ref JMVehicleSpawnerSettings settings;
	protected ref JMVehicleSpawnerMeta meta;

	void JMVehicleSpawnerModule()
	{
		GetPermissionsManager().RegisterPermission( "Vehicles.View" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Vehicles.View" );
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		Load();
		
		meta = JMVehicleSpawnerMeta.DeriveFromSettings( settings );
	}

	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();

		settings = JMVehicleSpawnerSettings.Load();

		for ( int i = 0; i < settings.Vehicles.Count(); i++ )
		{
			string vehicle = settings.Vehicles.GetKey( i );
			vehicle.Replace( " ", "." );
			GetPermissionsManager().RegisterPermission( "Vehicles." + vehicle );
		}

		meta = JMVehicleSpawnerMeta.DeriveFromSettings( settings );
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		if ( GetGame().IsServer() )
			settings.Save();
	}

	int GetRPCMin()
	{
		return JMVehicleSpawnerModuleRPC.INVALID;
	}

	int GetRPCMax()
	{
		return JMVehicleSpawnerModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMVehicleSpawnerModuleRPC.Load:
			RPC_Load( ctx, sender, target );
			break;
		case JMVehicleSpawnerModuleRPC.SpawnPosition:
			RPC_SpawnPosition( ctx, sender, target );
			break;
		}
    }

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/vehiclespawner_form.layout";
	}

	ref array< string > GetVehicles()
	{
		return meta.Vehicles;
	}
	
	void Load()
	{
		if ( IsMissionClient() && !IsMissionOffline() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMVehicleSpawnerModuleRPC.Load, true, NULL );
		}
	}

	private void Client_Load( string json )
	{
		JsonFileLoader< JMVehicleSpawnerMeta >.JsonLoadData( json, meta );

		OnSettingsUpdated();
	}

	private void Server_Load( PlayerIdentity ident )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( JsonFileLoader< JMVehicleSpawnerMeta >.JsonMakeData( JMVehicleSpawnerMeta.DeriveFromSettings( settings ) ) );
		rpc.Send( NULL, JMVehicleSpawnerModuleRPC.Load, true, ident );
	}

	private void RPC_Load( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			Server_Load( senderRPC );
		}

		if ( IsMissionClient() )
		{
			string json;
			if ( !ctx.Read( json ) )
			{
				return;
			} 

			Client_Load( json );
		}
	}

	void SpawnPosition( string vehicle, vector position )
	{
		if ( IsMissionClient() )
		{
			Client_SpawnPosition( vehicle, position );
		} else
		{
			Server_SpawnPosition( vehicle, position, NULL );
		}
	}

	private void Client_SpawnPosition( string vehicle, vector position )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( vehicle );
		rpc.Write( position );
		rpc.Send( NULL, JMVehicleSpawnerModuleRPC.SpawnPosition, true, NULL );
	}

	private void Server_SpawnPosition( string vehicle, vector position, PlayerIdentity ident )
	{
		JMVehicleSpawnerSerialize file;
		if ( !settings.Vehicles.Find( vehicle, file ) )
		{
			return;
		}

		string perm = file.VehicleName;
		perm.Replace( " ", "." );
		if ( !GetPermissionsManager().HasPermission( "Vehicles." + perm, ident ) )
		{
			return;
		}

		Car car = SpawnVehicle( file, position );
		if ( !car )
		{
			return;
		}

		GetCommunityOnlineToolsBase().Log( ident, "Spawned vehicle " + car.GetDisplayName() + " (" + vehicle + ") at " + position.ToString() );
	}

	private void RPC_SpawnPosition( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string vehicle;
			if ( !ctx.Read( vehicle ) )
			{
				return;
			} 

			vector position;
			if ( !ctx.Read( position ) )
			{
				return;
			} 

			Server_SpawnPosition( vehicle, position, senderRPC );
		}
	}

	private void FillCar( Car car, CarFluid fluid )
	{
		float cap = car.GetFluidCapacity( fluid );
		car.Fill( fluid, cap );
	}

	private Car SpawnVehicle( JMVehicleSpawnerSerialize file, vector position )
	{
		array< string > attachments = file.Parts;

		if ( attachments.Count() == 0 )
			return NULL;

		Car oCar = Car.Cast( GetGame().CreateObject( file.VehicleName, position ) );

		for (int j = 0; j < attachments.Count(); j++)
		{
			oCar.GetInventory().CreateInInventory( attachments[j] );
		}

		FillCar( oCar, CarFluid.FUEL );
		FillCar( oCar, CarFluid.OIL );
		FillCar( oCar, CarFluid.BRAKE );
		FillCar( oCar, CarFluid.COOLANT );
		
		return oCar;
	}
}