enum JMVehicleSpawnerModuleRPC
{
    INVALID = 10260,
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

	override string GetInputToggle()
	{
		return "UACOTToggleVehicle";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/vehiclespawner_form.layout";
	}

	override string GetTitle()
	{
		return "Vehicle Spawner";
	}
	
	override string GetIconName()
	{
		return "VS";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		Load();
	}

	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();

		if ( IsMissionHost() )
		{
			if ( settings )
			{
				for ( int i = 0; i < settings.Vehicles.Count(); i++ )
				{
					string vehicle = settings.Vehicles.GetKey( i );
					vehicle.Replace( " ", "." );
					GetPermissionsManager().RegisterPermission( "Vehicles." + vehicle );
				}

				meta = JMVehicleSpawnerMeta.DeriveFromSettings( settings );
			}
		} else if ( IsMissionClient() )
		{
			if ( meta )
			{
				for ( i = 0; i < meta.Vehicles.Count(); i++ )
				{
					vehicle = meta.Vehicles.Get( i );
					vehicle.Replace( " ", "." );
					GetPermissionsManager().RegisterPermission( "Vehicles." + vehicle );
				}
			}
		}
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		if ( GetGame().IsServer() )
			settings.Save();
	}

	array< string > GetVehicles()
	{
		return meta.Vehicles;
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
	
	void Load()
	{
		if ( GetGame().IsClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMVehicleSpawnerModuleRPC.Load, true, NULL );
		} else
		{
			settings = JMVehicleSpawnerSettings.Load();

			OnSettingsUpdated();
		}
	}

	private void Server_Load( PlayerIdentity ident )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta );
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
			if ( ctx.Read( meta ) )
			{
				OnSettingsUpdated();
			}
		}
	}

	void SpawnPosition( string vehicle, vector position )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( vehicle );
			rpc.Write( position );
			rpc.Send( NULL, JMVehicleSpawnerModuleRPC.SpawnPosition, true, NULL );
		} else
		{
			Server_SpawnPosition( vehicle, position, NULL );
		}
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
		if ( file == NULL )
			return NULL;

		array< string > attachments = file.Parts;

		Car oCar = Car.Cast( GetGame().CreateObject( file.VehicleName, position ) );

		for ( int j = 0; j < attachments.Count(); j++ )
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