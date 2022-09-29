enum JMObjectSpawnerModuleRPC
{
	INVALID = 10220,
	Position,
	Inventory,
	Delete,
	COUNT
};

class JMObjectSpawnerModule: JMRenderableModuleBase
{
	void JMObjectSpawnerModule()
	{
		GetPermissionsManager().RegisterPermission( "Entity.Spawn.Position" );
		GetPermissionsManager().RegisterPermission( "Entity.Spawn.Inventory" );
		GetPermissionsManager().RegisterPermission( "Entity.Delete" );
		GetPermissionsManager().RegisterPermission( "Entity.View" );
	}

	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();
		
		RegisterBinding( new JMModuleBinding( "SpawnRandomInfected",		"UAObjectModuleSpawnInfected",	true 	) );
		RegisterBinding( new JMModuleBinding( "SpawnRandomAnimal",			"UAObjectModuleSpawnAnimal",	true 	) );
		RegisterBinding( new JMModuleBinding( "SpawnRandomWolf",			"UAObjectModuleSpawnWolf",		true 	) );
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
		return "Object Spawner";
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

	private void RPC_DeleteEntity( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			if ( target == NULL )
				return;

			Server_DeleteEntity( target, senderRPC );
		}
	}

	void SpawnEntity_Position( string ent, vector position, float quantity = -1, float health = -1 )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( ent );
			rpc.Write( position );
			rpc.Write( quantity );
			rpc.Write( health );
			rpc.Send( NULL, JMObjectSpawnerModuleRPC.Position, true, NULL );
		} else
		{
			Server_SpawnEntity_Position( ent, position, quantity, health, NULL );
		}
	}

	private void Server_SpawnEntity_Position( string className, vector position, float quantity, float health, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Position", ident, instance ) )
			return;
		
		int flags = ECE_CREATEPHYSICS|ECE_PLACE_ON_SURFACE;
		if ( GetGame().IsKindOf( className, "DZ_LightAI" ) )
			flags |= 0x800;
		else if ( GetGame().IsKindOf( className, "CarScript" ) )
			flags = ECE_CREATEPHYSICS|ECE_UPDATEPATHGRAPH;

		EntityAI ent;
		if ( !Class.CastTo( ent, GetGame().CreateObjectEx( className, position, flags ) ) )
			return;

		vector tmItem[4];
		ent.GetTransform( tmItem );
		//ent.PlaceOnSurfaceRotated( tmItem, position, 0, 0, 0, true );

		SetupEntity( ent, quantity, health );

		GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + ent.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ") at " + position.ToString() );
		SendWebhook( "Vector", instance, "Spawned object \"" + className + "\" (" + ent.GetType() + ") at " + position );
	}

	private void RPC_SpawnEntity_Position( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

			Server_SpawnEntity_Position( ent, position, quantity, health, senderRPC );
		}
	}

	void SpawnEntity_Inventory( string ent, JMSelectedObjects selected, float quantity = -1, float health = -1 )
	{
		if ( IsMissionClient() && !IsMissionOffline() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( ent );
			rpc.Write( selected );
			rpc.Write( quantity );
			rpc.Write( health );
			rpc.Send( NULL, JMObjectSpawnerModuleRPC.Inventory, true, NULL );
		} else
		{
			Server_SpawnEntity_Inventory( ent, selected, quantity, health, NULL );
		}
	}

	private void Server_SpawnEntity_Inventory( string className, JMSelectedObjects selected, float quantity, float health, PlayerIdentity ident )
	{
		if ( GetGame().IsKindOf( className, "DZ_LightAI" ) )
			return;

		JMPlayerInstance callerInstance;
		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Inventory", ident, callerInstance ) )
			return;

		int flags = ECE_CREATEPHYSICS;

		array< string > players = selected.GetPlayers();

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
		
				if ( !Class.CastTo( ent, GetGame().CreateObjectEx( className, position, flags ) ) )
					continue;

				vector tmItem[4];
				ent.GetTransform( tmItem );
				//ent.PlaceOnSurfaceRotated( tmItem, position, 0, 0, 0, true );

				loggedSuffix = " at " + position.ToString();
			}

			SetupEntity( ent, quantity, health );

			GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + ent.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ") on " + instance.GetSteam64ID() + loggedSuffix );
			SendWebhook( "Player", callerInstance, "Spawned object \"" + ent.GetDisplayName() + "\" (" + ent.GetType() + ") on " + instance.FormatSteamWebhook() + loggedSuffix );
		}
	}

	private void RPC_SpawnEntity_Inventory( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string ent;
			if ( !ctx.Read( ent ) )
				return;

			JMSelectedObjects selected;
			if ( !ctx.Read( selected ) )
				return;
		
			float quantity;
			if ( !ctx.Read( quantity ) )
				return;

			float health;
			if ( !ctx.Read( health ) )
				return;

			Server_SpawnEntity_Inventory( ent, selected, quantity, health, senderRPC );
		}
	}

	private void SetupEntity( EntityAI obj, out float quantity, out float health )
	{
		ItemBase item;
		if ( Class.CastTo( item, obj ) )
		{
			if ( quantity == -1 )
			{
				if ( item.HasQuantity() )
					quantity = item.GetQuantityMax();
			}

			if ( quantity > 0 )
			{
				if ( quantity > item.GetQuantityMax() )
					quantity = item.GetQuantityMax();

				item.SetQuantity(quantity);
			}
		}

		if ( obj.IsKindOf("Weapon_Base") || obj.IsKindOf("CarScript") )
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
		if ( GetGame().IsKindOf( className, "DZ_LightAI" ) )
			flags |= 0x800;

		EntityAI ent;
		if ( !Class.CastTo( ent, GetGame().CreateObjectEx( className, position, flags ) ) )
			return;

		float quantity = -1;
		float health = -1;
		SetupEntity( ent, quantity, health );
		
		GetCommunityOnlineToolsBase().Log( sender, "Spawned Entity " + ent.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ") at " + position.ToString() );
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
}