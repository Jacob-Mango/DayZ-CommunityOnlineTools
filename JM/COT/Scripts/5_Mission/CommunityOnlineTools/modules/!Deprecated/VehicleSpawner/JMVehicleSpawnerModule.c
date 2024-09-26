class JMVehicleSpawnerModule: JMRenderableModuleBase
{
	protected ref JMVehicleSpawnerSettings settings;
	protected ref JMVehicleSpawnerMeta meta;

	void JMVehicleSpawnerModule()
	{
		GetPermissionsManager().RegisterPermission( "Vehicles.View" );
		GetPermissionsManager().RegisterPermission( "Vehicles.Spawn" );
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
		return "#STR_COT_VEHICLE_MODULE_NAME (Legacy)";
	}

	override string GetIconName()
	{
		return "JM\\COT\\GUI\\textures\\modules\\SetSpawner.paa";
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
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

		if (GetGame().IsServer())
			Load();
	}

	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();

		if ( IsMissionHost() )
		{
			if ( settings )
			{
				meta = JMVehicleSpawnerMeta.DeriveFromSettings( settings );
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

	array< string > GetVehiclesName()
	{
		return meta.VehiclesName;
	}

	override int GetRPCMin()
	{
		return JMVehicleSpawnerModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMVehicleSpawnerModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
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
	
	//! TODO: doesnt update between server restarts for clients
	void Load()
	{
		if ( GetGame().IsClient() )
		{
			if (meta)
				return;

			meta = JMVehicleSpawnerMeta.Create();

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMVehicleSpawnerModuleRPC.Load, true, NULL );
		} else
		{
			settings = JMVehicleSpawnerSettings.Load();

			OnSettingsUpdated();
		}
	}

	bool IsLoaded()
	{
		return meta != null;
	}

	private void Server_Load( PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Vehicles.View", ident, instance ) )
			return;

		CF_Log.Debug("Has permission");

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( meta );
		rpc.Send( NULL, JMVehicleSpawnerModuleRPC.Load, true, ident );
	}

	private void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( GetGame().IsDedicatedServer() )
		{
			Server_Load( senderRPC );
		}
		else
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

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Vehicles.Spawn", ident, instance ) )
			return;

		EntityAI ent = SpawnVehicle( file, position, direction );
		if ( !ent )
			return;

		GetCommunityOnlineToolsBase().Log( ident, "Spawned vehicle " + ent.GetDisplayName() + " (" + vehicle + ") at " + position.ToString() );
		SendWebhook( "", instance, "Spawned vehicle \"" + ent.GetDisplayName() + "\" (" + ent.GetType() + ") at " + position.ToString() );
	}

	private void RPC_SpawnPosition( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

	private EntityAI SpawnVehicle( JMVehicleSpawnerSerialize file, vector position, vector direction )
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
		CommunityOnlineToolsBase.Refuel(entity);
	}
};