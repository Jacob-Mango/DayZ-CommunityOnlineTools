enum JMObjectSpawnerModuleRPC
{
    INVALID = 10220,
    Position,
	Inventory,
    COUNT
};

class JMObjectSpawnerModule: JMRenderableModuleBase
{
	void JMObjectSpawnerModule()
	{
		GetPermissionsManager().RegisterPermission( "Entity.Position" );
		GetPermissionsManager().RegisterPermission( "Entity.Inventory" );
		GetPermissionsManager().RegisterPermission( "Entity.View" );
	}

	override void RegisterKeyMouseBindings() 
	{
		RegisterBinding( new JMModuleBinding( "SpawnRandomInfected",		"UAObjectModuleSpawnInfected",	true 	) );
		RegisterBinding( new JMModuleBinding( "SpawnRandomAnimal",			"UAObjectModuleSpawnAnimal",	true 	) );
		RegisterBinding( new JMModuleBinding( "SpawnRandomWolf",			"UAObjectModuleSpawnWolf",		true 	) );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Entity.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/objectspawner_form.layout";
	}
	
	void SpawnRandomInfected( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Entity.Position" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
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

		if ( !GetPermissionsManager().HasPermission( "Entity.Position" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
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

		if ( !GetPermissionsManager().HasPermission( "Entity.Position" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
			return;
		}

		string className = GetRandomChildFromBaseClass( "cfgVehicles", "Animal_CanisLupus" );
		vector position = GetPointerPos();

		SpawnEntity_Position( className, position );
	}
	
	int GetRPCMin()
	{
		return JMObjectSpawnerModuleRPC.INVALID;
	}

	int GetRPCMax()
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
		}
    }

	void SpawnEntity_Position( string ent, vector position, float quantity = -1, float health = -1 )
	{
		if ( IsMissionClient() )
		{
			Client_SpawnEntity_Position( ent, position, quantity, health );
		} else
		{
			Server_SpawnEntity_Position( ent, position, quantity, health, NULL );
		}
	}

	private void Client_SpawnEntity_Position( string ent, vector position, float quantity, float health )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( new Param1< string >( ent ) );
		rpc.Write( new Param1< vector >( position ) );
		rpc.Write( new Param1< float >( quantity ) );
		rpc.Write( new Param1< float >( health ) );
		rpc.Send( NULL, JMObjectSpawnerModuleRPC.Position, true, NULL );
	}

	private void Server_SpawnEntity_Position( string ent, vector position, float quantity, float health, PlayerIdentity ident )
	{
		if ( !GetPermissionsManager().HasPermission( "Entity.Position", ident ) )
		{
			return;
		}

		Object obj = GetGame().CreateObject( ent, position, false, GetGame().IsKindOf( ent, "DZ_LightAI" ) );

		if ( !obj )
		{
			return;
		}

		SetupEntity( obj, quantity, health );

		obj.PlaceOnSurface();

		GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + obj.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ") at " + position.ToString() );
	}

	private void RPC_SpawnEntity_Position( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			Param1< string > ent;
			if ( !ctx.Read( ent ) )
			{
				Error("Failed");
				return;
			}

			Param1< vector > position;
			if ( !ctx.Read( position ) )
			{
				Error("Failed");
				return;
			}
		
			Param1< float > quantity;
			if ( !ctx.Read( quantity ) )
			{
				Error("Failed");
				return;
			}

			Param1< float > health;
			if ( !ctx.Read( health ) )
			{
				Error("Failed");
				return;
			}

			Server_SpawnEntity_Position( ent.param1, position.param1, quantity.param1, health.param1, senderRPC );
		}
	}

	void SpawnEntity_Inventory( string ent, array< EntityAI > objects, float quantity = -1, float health = -1 )
	{
		if ( IsMissionClient() )
		{
			Client_SpawnEntity_Inventory( ent, objects, quantity, health );
		} else
		{
			Server_SpawnEntity_Inventory( ent, objects, quantity, health, NULL );
		}
	}

	private void Client_SpawnEntity_Inventory( string ent, array< EntityAI > objects, float quantity, float health )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( new Param1< string >( ent ) );
		rpc.Write( new Param1< array< EntityAI > >( objects ) );
		rpc.Write( new Param1< float >( quantity ) );
		rpc.Write( new Param1< float >( health ) );
		rpc.Send( NULL, JMObjectSpawnerModuleRPC.Inventory, true, NULL );
	}

	private void Server_SpawnEntity_Inventory( string ent, array< EntityAI > objects, float quantity, float health, PlayerIdentity ident )
	{
		if ( GetGame().IsKindOf( ent, "DZ_LightAI" ) )
		{
			return;
		}

		if ( !GetPermissionsManager().HasPermission( "Entity.Inventory", ident ) )
		{
			return;
		}

		Object obj;

		for ( int i = 0; i < objects.Count(); i++ )
		{
			obj = NULL;

			obj = objects[i].GetInventory().CreateInInventory( ent );
			if ( !obj )
			{
				obj = GetGame().CreateObject( ent, objects[i].GetPosition() );

				if ( !obj )
				{
					continue;
				}

				obj.PlaceOnSurface();
			}

			SetupEntity( obj, quantity, health );

			GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + obj.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ") on " + objects[i] );
		}
	}

	private void RPC_SpawnEntity_Inventory( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			Param1< string > ent;
			if ( !ctx.Read( ent ) )
			{
				return;
			}

			Param1< array< EntityAI > > objects;
			if ( !ctx.Read( objects ) )
			{
				return;
			}
		
			Param1< float > quantity;
			if ( !ctx.Read( quantity ) )
			{
				return;
			}

			Param1< float > health;
			if ( !ctx.Read( health ) )
			{
				return;
			}

			Server_SpawnEntity_Inventory( ent.param1, objects.param1, quantity.param1, health.param1, senderRPC );
		}
	}

	private void SetupEntity( Object obj, out float quantity, out float health )
	{
		ItemBase item;
		if ( Class.CastTo( item, obj ) )
		{
			if ( quantity == -1 )
			{
				if ( item.HasQuantity() )
				{
					quantity = item.GetQuantityInit();
				}
			}

			if ( quantity > 0 )
			{
				if ( quantity > item.GetQuantityMax() )
				{
					quantity = item.GetQuantityMax();
				}

				item.SetQuantity(quantity);
			}
		}

		if ( health == -1 )
		{
			health = obj.GetMaxHealth();
		}

		if ( health >= 0 )
		{
			obj.SetHealth( "", "", health );
		}
	}
}