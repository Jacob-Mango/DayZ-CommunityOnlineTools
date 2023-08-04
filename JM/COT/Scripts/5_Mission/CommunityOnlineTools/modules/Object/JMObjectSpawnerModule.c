class JMObjectSpawnerModule: JMRenderableModuleBase
{
	bool m_OnDebugSpawn = true;
	string m_CurrentType;
	string m_SearchText;

	void JMObjectSpawnerModule()
	{
		GetPermissionsManager().RegisterPermission( "Entity.Spawn.Position" );
		GetPermissionsManager().RegisterPermission( "Entity.Spawn.Inventory" );
		GetPermissionsManager().RegisterPermission( "Entity.Delete" );
		GetPermissionsManager().RegisterPermission( "Entity.View" );
	}

	override void EnableUpdate()
	{
	}

	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();
		
		Bind( new JMModuleBinding( "SpawnRandomInfected",		"UAObjectModuleSpawnInfected",	true 	) );
		Bind( new JMModuleBinding( "SpawnRandomAnimal",			"UAObjectModuleSpawnAnimal",	true 	) );
		Bind( new JMModuleBinding( "SpawnRandomWolf",			"UAObjectModuleSpawnWolf",		true 	) );
		Bind( new JMModuleBinding( "DeleteAtCursor",			"UAObjectModuleDeleteOnCursor",	true 	) );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Entity.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleEntity";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/objectspawner_form.layout";
	}

	override string GetTitle()
	{
		return "#STR_COT_OBJECT_MODULE_NAME";
	}
	
	override string GetIconName()
	{
		return "OB";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "Object Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Delete" );
		types.Insert( "Vector" );
		types.Insert( "Player" );
	}

	void DeleteAtCursor( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Entity.Delete" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
			return;
		}

		float distance = 10.0;
		vector rayStart = GetGame().GetCurrentCameraPosition();
		vector rayEnd = rayStart + ( GetGame().GetCurrentCameraDirection() * distance );

		RaycastRVParams rayInput = new RaycastRVParams( rayStart, rayEnd, GetGame().GetPlayer() );
		rayInput.flags = CollisionFlags.NEARESTCONTACT;
		rayInput.radius = 1.0;
		array< ref RaycastRVResult > results = new array< ref RaycastRVResult >;

		Object obj;
		if ( DayZPhysics.RaycastRVProxy( rayInput, results ) )
		{
			for ( int i = 0; i < results.Count(); ++i )
			{
				if ( results[i].obj == NULL || PlayerBase.Cast( results[i].obj ) )
					continue;

				if ( results[i].obj.GetType() == "" )
					continue;

				if ( results[i].obj.GetType() == "#particlesourceenf" )
					continue;

				obj = results[i].obj;
				break;
			}
		}

		if ( obj == NULL )
			return;

		DeleteEntity( obj );
	}
	
	void SpawnRandomInfected( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Position" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
			return;
		}

		string className = WorkingZombieClasses().GetRandomElement();
		vector position = GetPointerPos();

		SpawnEntity_Position( className, position );
	}

	void SpawnRandomAnimal( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Position" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
			return;
		}

		string className = GetRandomChildFromBaseClass( "cfgVehicles", "AnimalBase", 2, "Animal_CanisLupus" );
		vector position = GetPointerPos();

		SpawnEntity_Position( className, position );
	}

	void SpawnRandomWolf( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Position" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
			return;
		}

		string className = GetRandomChildFromBaseClass( "cfgVehicles", "Animal_CanisLupus" );
		vector position = GetPointerPos();

		SpawnEntity_Position( className, position );
	}
	
	override int GetRPCMin()
	{
		return JMObjectSpawnerModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMObjectSpawnerModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMObjectSpawnerModuleRPC.Position:
			RPC_SpawnEntity_Position( ctx, sender, target );
			break;
		case JMObjectSpawnerModuleRPC.Inventory:
			RPC_SpawnEntity_Inventory( ctx, sender, target );
			break;
		case JMObjectSpawnerModuleRPC.Delete:
			RPC_DeleteEntity( ctx, sender, target );
			break;
		}
	}

	void DeleteEntity( Object obj )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( obj, JMObjectSpawnerModuleRPC.Delete, true, NULL );
		} else
		{
			Server_DeleteEntity( obj, NULL );
		}
	}

	private void Server_DeleteEntity( notnull Object obj, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Entity.Delete", ident, instance ) )
			return;

		PlayerBase player;
		if ( Class.CastTo( player, obj ) && ( player.IsAlive() || player.GetIdentity() != null ) )
			return;

		string obtype = Object.GetDebugName( obj );

		vector transform[4];
		obj.GetTransform( transform );

		GetGame().ObjectDelete( obj );
		
		GetCommunityOnlineToolsBase().Log( ident, "Deleted Entity " + obtype + " at " + transform[3].ToString() );
		SendWebhook( "Delete", instance, "Deleted object " + obtype + " at " + transform[3].ToString() );
	}

	private void RPC_DeleteEntity( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			if ( target == NULL )
				return;

			Server_DeleteEntity( target, senderRPC );
		}
	}

	void SpawnEntity_Position( string ent, vector position, float quantity = -1, float health = -1, int itemState = -1 )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( ent );
			rpc.Write( position );
			rpc.Write( quantity );
			rpc.Write( health );
			rpc.Write( itemState );
			rpc.Write( m_OnDebugSpawn );
			rpc.Send( NULL, JMObjectSpawnerModuleRPC.Position, true, NULL );
		} else
		{
			Server_SpawnEntity_Position( ent, position, quantity, health, itemState, NULL );
		}
	}

	private void Server_SpawnEntity_Position( string className, vector position, float quantity, float health, int itemState, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Position", ident, instance ) )
			return;
		
		int flags = ECE_CREATEPHYSICS;
		if ( GetGame().IsKindOf( className, "CarScript" ) && !COT_SurfaceIsWater( position ) )
			flags |= ECE_PLACE_ON_SURFACE;
		
		if ( GetGame().IsKindOf( className, "DZ_LightAI" ) )
			flags |= 0x800;

		EntityAI ent;
		if ( !Class.CastTo( ent, GetGame().CreateObjectEx( className, position, flags ) ) )
			return;

		SetupEntity( ent, quantity, health, itemState );

		GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + ent.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ", "+itemState +") at " + position.ToString() );
		SendWebhook( "Vector", instance, "Spawned object \"" + className + "\" (" + ent.GetType() + ") at " + position );
	}

	private void RPC_SpawnEntity_Position( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string ent;
			if ( !ctx.Read( ent ) )
			{
				Error("Failed");
				return;
			}

			vector position;
			if ( !ctx.Read( position ) )
			{
				Error("Failed");
				return;
			}
		
			float quantity;
			if ( !ctx.Read( quantity ) )
			{
				Error("Failed");
				return;
			}

			float health;
			if ( !ctx.Read( health ) )
			{
				Error("Failed");
				return;
			}

			int itemState;
			if ( !ctx.Read( itemState ) )
			{
				Error("Failed");
				return;
			}

			if ( !ctx.Read( m_OnDebugSpawn ) )
				return;

			Server_SpawnEntity_Position( ent, position, quantity, health, itemState, senderRPC );
		}
	}

	void SpawnEntity_Inventory( string ent, array< string > players, float quantity = -1, float health = -1, int itemState = -1 )
	{
		if ( IsMissionClient() && !IsMissionOffline() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( ent );
			rpc.Write( players );
			rpc.Write( quantity );
			rpc.Write( health );
			rpc.Write( itemState );
			rpc.Write( m_OnDebugSpawn );
			rpc.Send( NULL, JMObjectSpawnerModuleRPC.Inventory, true, NULL );
		}
		else
		{
			Server_SpawnEntity_Inventory( ent, players, quantity, health, itemState, NULL );
		}
	}

	private void Server_SpawnEntity_Inventory( string className, array< string > players, float quantity, float health, int itemState, PlayerIdentity ident )
	{
		if ( GetGame().IsKindOf( className, "DZ_LightAI" ) )
			return;

		JMPlayerInstance callerInstance;
		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Inventory", ident, callerInstance ) )
			return;

		for ( int i = 0; i < players.Count(); i++ )
		{
			JMPlayerInstance instance = GetPermissionsManager().GetPlayer( players[i] );
			if ( !instance || !instance.PlayerObject )
				continue;

			instance.Update();

			string loggedSuffix = "";

			EntityAI ent;
			if ( !Class.CastTo( ent, instance.PlayerObject.GetInventory().CreateInInventory( className ) ) )
			{
				vector position = instance.PlayerObject.GetPosition();

				int flags = ECE_CREATEPHYSICS;
				if ( GetGame().IsKindOf( className, "CarScript" ) && !COT_SurfaceIsWater( position ) )
					flags |= ECE_PLACE_ON_SURFACE;
		
				if ( !Class.CastTo( ent, GetGame().CreateObjectEx( className, position, flags ) ) )
					continue;

				loggedSuffix = " at " + position.ToString();
			}

			SetupEntity( ent, quantity, health, itemState );

			GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + ent.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ", "+ itemState+") on " + instance.GetSteam64ID() + loggedSuffix );
			SendWebhook( "Player", callerInstance, "Spawned object \"" + ent.GetDisplayName() + "\" (" + ent.GetType() + ") on " + instance.FormatSteamWebhook() + loggedSuffix );
		}
	}

	private void RPC_SpawnEntity_Inventory( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string ent;
			if ( !ctx.Read( ent ) )
				return;

			array< string > players;
			if ( !ctx.Read( players ) )
				return;
		
			float quantity;
			if ( !ctx.Read( quantity ) )
				return;

			float health;
			if ( !ctx.Read( health ) )
				return;

			float itemState;
			if ( !ctx.Read( itemState ) )
				return;

			if ( !ctx.Read( m_OnDebugSpawn ) )
				return;

			Server_SpawnEntity_Inventory( ent, players, quantity, health, itemState, senderRPC );
		}
	}

	private void SetupEntity( EntityAI obj, out float quantity, out float health, out int itemState )
	{
		ItemBase item;
		if ( Class.CastTo( item, obj ) )
		{
			if ( item.HasQuantity() )
				item.SetQuantity(quantity);

			if ( itemState != 0 )
			{
				if ( item.IsLiquidContainer() )
				{
					if ( item.IsBloodContainer() )
					{
						item.SetLiquidType(Math.Pow(LIQUID_BLOOD_0_P, itemState));
					}
					else
					{
						item.SetLiquidType(Math.Pow(LIQUID_WATER, itemState));
					}
				}
				else if ( item.HasFoodStage() && item.CanBeCooked() )
				{
					if (item.IsInherited(Edible_Base)) 
					{
						Edible_Base foodItem = Edible_Base.Cast(item);
						foodItem.GetFoodStage().ChangeFoodStage( itemState );
					}
				}
			}
		}

		if ( m_OnDebugSpawn && ( obj.IsKindOf("Weapon_Base") || obj.IsKindOf("CarScript") ) )
			obj.OnDebugSpawn();

		if ( health == -1 )
			health = obj.GetMaxHealth();

		if ( health >= 0 )
			obj.SetHealth( "", "", health );
	}

	void Command_Spawn(JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance)
	{
		string className;
		if (!params.Next(className)) return;

		vector position;
		if (params.HasNext())
		{
			float x, y, z;

			if (!params.Next(x)) return;
			if (!params.Next(y)) return;
			if (!params.Next(z)) return;

			position = Vector(x, y, z);
		}
		else
		{
			PlayerBase player = GetPlayerObjectByIdentity(sender);
			if (player) position = player.GetPosition();
		}
		
		int flags = ECE_CREATEPHYSICS;
		if ( GetGame().IsKindOf( className, "CarScript" ) && !COT_SurfaceIsWater( position ) )
			flags |= ECE_PLACE_ON_SURFACE;
		
		if ( GetGame().IsKindOf( className, "DZ_LightAI" ) )
			flags |= 0x800;

		EntityAI ent;
		if ( !Class.CastTo( ent, GetGame().CreateObjectEx( className, position, flags ) ) )
			return;

		float quantity = -1;
		float health = -1;
		int itemState = -1;
		SetupEntity( ent, quantity, health, itemState );
		
		GetCommunityOnlineToolsBase().Log( sender, "Spawned Entity " + ent.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ", "+ itemState+") at " + position.ToString() );
		SendWebhook( "Vector", instance, "Spawned object \"" + className + "\" (" + ent.GetType() + ") at " + position );
	}

	override void GetSubCommands(inout array<ref JMCommand> commands)
	{
		AddSubCommand(commands, "spawn", "Command_Spawn", "Entity.Spawn.Position");
	}

	override array<string> GetCommandNames()
	{
		auto names = new array<string>();
		names.Insert("object");
		return names;
	}
};
