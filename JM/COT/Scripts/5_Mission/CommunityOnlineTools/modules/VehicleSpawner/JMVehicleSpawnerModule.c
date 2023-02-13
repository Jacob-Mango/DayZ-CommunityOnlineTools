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

	override void EnableUpdate()
	{
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
		return "#STR_COT_VEHICLE_MODULE_NAME";
	}
	
	override string GetIconName()
	{
		return "VS";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "Vehicle Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
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
			return;

		string perm = file.VehicleName;
		perm.Replace( " ", "." );
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Vehicles." + perm, ident, instance ) )
			return;

		EntityAI ent = SpawnVehicle( file, position, direction );
		if ( !ent )
			return;

		GetCommunityOnlineToolsBase().Log( ident, "Spawned vehicle " + ent.GetDisplayName() + " (" + vehicle + ") at " + position.ToString() );
		SendWebhook( "", instance, "Spawned vehicle \"" + ent.GetDisplayName() + "\" (" + ent.GetType() + ") at " + position.ToString() );
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

		int flags = ECE_CREATEPHYSICS;
		if ( !COT_SurfaceIsWater( position ) )
			flags |= ECE_PLACE_ON_SURFACE;

		EntityAI ent;
		if ( !Class.CastTo( ent, GetGame().CreateObjectEx( file.VehicleName, position, flags ) ) )
			return NULL;

		for ( int j = 0; j < attachments.Count(); j++ )
		{
			ent.GetInventory().CreateInInventory( attachments[j] );
		}
		
		OnSpawnVehicle(ent);

		return ent;
	}
	
	void OnSpawnVehicle(EntityAI entity)
	{
		Car car;
		if (Class.CastTo(car, entity))
		{
			FillCar(car, CarFluid.FUEL);
			FillCar(car, CarFluid.OIL);
			FillCar(car, CarFluid.BRAKE);
			FillCar(car, CarFluid.COOLANT);
		}
	}
};