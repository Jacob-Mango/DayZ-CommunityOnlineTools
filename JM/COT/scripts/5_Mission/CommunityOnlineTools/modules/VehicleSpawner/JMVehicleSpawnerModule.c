enum JMVehicleSpawnerModuleRPC
{
    INVALID = 10600,
    Load,
	SpawnCursor,
    MAX
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

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		super.OnRPC( sender, target, rpc_type, ctx );

		if ( rpc_type > JMVehicleSpawnerModuleRPC.INVALID && rpc_type < JMVehicleSpawnerModuleRPC.MAX )
		{
			switch ( rpc_type )
			{
			case JMVehicleSpawnerModuleRPC.Load:
				RPC_Load( ctx, sender, target );
				break;
			case JMVehicleSpawnerModuleRPC.SpawnCursor:
				RPC_SpawnCursor( ctx, sender, target );
				break;
			}
			return;
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

	private void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

	void SpawnCursor( string vehicle, vector position )
	{
		if ( IsMissionClient() )
		{
			Client_SpawnCursor( vehicle, position );
		} else
		{
			Server_SpawnCursor( vehicle, position, NULL );
		}
	}

	private void Client_SpawnCursor( string vehicle, vector position )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( vehicle );
		rpc.Write( position );
		rpc.Send( NULL, JMVehicleSpawnerModuleRPC.SpawnCursor, true, NULL );
	}

	private void Server_SpawnCursor( string vehicle, vector position, PlayerIdentity ident )
	{
		SpawnVehicle( vehicle, position, ident );
	}

	private void RPC_SpawnCursor( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

			Server_SpawnCursor( vehicle, position, senderRPC );
		}
	}

	private void FillCar( Car car, CarFluid fluid )
	{
		float cap = car.GetFluidCapacity( fluid );
		car.Fill( fluid, cap );
	}

	Car SpawnVehicle( string type, vector position, PlayerIdentity senderRPC = NULL )
	{
		JMVehicleSpawnerSerialize file = settings.Vehicles.Get( type );

		if ( file == NULL ) 
			return NULL;

		array< string > attachments = file.Parts;

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

		GetCommunityOnlineToolsBase().Log( senderRPC, "Spawned vehicle " + oCar.GetDisplayName() + " (" + type + ") at " + position.ToString() );

		return oCar;
	}
}

static Car SpawnVehicleAtPosition( string type, vector position, PlayerIdentity senderRPC = NULL )
{
	JMVehicleSpawnerModule module;
	if ( Class.CastTo( module, GetModuleManager().GetModule( JMVehicleSpawnerModule ) ) )
	{
		return module.SpawnVehicle( type, position, senderRPC );
	}
	return NULL;
}