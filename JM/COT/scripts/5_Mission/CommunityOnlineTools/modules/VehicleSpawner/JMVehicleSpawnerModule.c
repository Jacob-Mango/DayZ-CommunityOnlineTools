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

		if ( IsMissionHost() && settings )
			settings.Save();
	}

	array< string > GetVehicles()
	{
		return meta.Vehicles;
	}

	override int GetRPCMin()
	{
		return JMVehicleSpawnerModuleRPC.INVALID;
	}

	override int GetRPCMax()
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

	void SpawnPosition( string vehicle, vector position, vector direction = "0 0 1" )
	{
		// position = position + "0 5 0";
		
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( vehicle );
			rpc.Write( position );
			rpc.Write( direction );
			rpc.Send( NULL, JMVehicleSpawnerModuleRPC.SpawnPosition, true, NULL );
		} else
		{
			Server_SpawnPosition( vehicle, position, direction, NULL );
		}
	}

	private void Server_SpawnPosition( string vehicle, vector position, vector direction, PlayerIdentity ident )
	{
		JMVehicleSpawnerSerialize file = settings.Vehicles.Get( vehicle );
		if ( !file )
		{
			return;
		}

		string perm = file.VehicleName;
		perm.Replace( " ", "." );
		if ( !GetPermissionsManager().HasPermission( "Vehicles." + perm, ident ) )
		{
			return;
		}

		EntityAI ent = SpawnVehicle( file, position, direction );
		if ( !ent )
		{
			return;
		}

		GetCommunityOnlineToolsBase().Log( ident, "Spawned vehicle " + ent.GetDisplayName() + " (" + vehicle + ") at " + position.ToString() );
	}

	private void RPC_SpawnPosition( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string vehicle;
			if ( !ctx.Read( vehicle ) )
				return;

			vector position;
			if ( !ctx.Read( position ) )
				return;

			vector direction;
			if ( !ctx.Read( direction ) )
				return;

			Server_SpawnPosition( vehicle, position, direction, senderRPC );
		}
	}

	private void FillCar( Car car, CarFluid fluid )
	{
		float cap = car.GetFluidCapacity( fluid );
		car.Fill( fluid, cap );
	}

	private EntityAI SpawnVehicle( ref JMVehicleSpawnerSerialize file, vector position, vector direction )
	{
		if ( file == NULL )
			return NULL;

		array< string > attachments = file.Parts;

		EntityAI vehicle = EntityAI.Cast( GetGame().CreateObject( file.VehicleName, position ) );
		// EntityAI vehicle = EntityAI.Cast( GetGame().CreateObject_WIP( file.VehicleName, position, ECE_CREATEPHYSICS ) );
        if ( vehicle )
        {
            for ( int j = 0; j < attachments.Count(); j++ )
            {
                vehicle.GetInventory().CreateInInventory( attachments[j] );
            }

            Car car;
            if ( Class.CastTo( car, vehicle ) )
            {
                FillCar( car, CarFluid.FUEL );
                FillCar( car, CarFluid.OIL );
                FillCar( car, CarFluid.BRAKE );
                FillCar( car, CarFluid.COOLANT );
            }

			vector boundingBox[2];
			float radius = vehicle.GetCollisionBox( boundingBox );

			float height = Math.AbsFloat( boundingBox[1][1] ) + Math.AbsFloat( boundingBox[0][1] );

			position[1] = position[1] + ( height * 2.0 );

			vehicle.SetPosition( position );
			vehicle.SetDirection( direction );
			
			//vehicle.SetPosition( position );
			//vehicle.SetOrientation( vehicle.GetOrientation() );
			//vehicle.SetPosition( position );
        }
		
		return vehicle;
	}
}