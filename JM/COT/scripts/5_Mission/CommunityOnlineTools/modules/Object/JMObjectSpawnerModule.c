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
	
	void SpawnRandomInfected( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Position" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLED_OFF" ) );
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
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLED_OFF" ) );
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
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLED_OFF" ) );
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
		if ( !GetPermissionsManager().HasPermission( "Entity.Delete", ident ) )
		{
			return;
		}

		if ( PlayerBase.Cast( obj ) )
			return;

		string obtype;
		GetGame().ObjectGetType( obj, obtype );

		vector transform[4];
		obj.GetTransform( transform );

		GetGame().ObjectDelete( obj );
		
		GetCommunityOnlineToolsBase().Log( ident, "Deleted Entity " + obtype + " at " + transform[3].ToString() );
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

	private void Server_SpawnEntity_Position( string ent, vector position, float quantity, float health, PlayerIdentity ident )
	{
		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Position", ident ) )
		{
			return;
		}

		//int flags = ECE_CREATEPHYSICS;
		//if ( GetGame().IsKindOf( ent, "DZ_LightAI" ) )
		//	flags |= 0x800;
		//Object obj = GetGame().CreateObjectEx( ent, position, flags );

		Object obj = GetGame().CreateObject( ent, position, false, GetGame().IsKindOf( ent, "DZ_LightAI" ) );
		if ( !obj )
		{
			return;
		}

		SetupEntity( obj, quantity, health );

		vector boundingBox[2];
		float radius = obj.GetCollisionBox( boundingBox );

		float height = Math.AbsFloat( boundingBox[1][1] ) + Math.AbsFloat( boundingBox[0][1] );

		position[1] = position[1] + ( height * 2.0 );

		obj.SetPosition( position );

		GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + obj.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ") at " + position.ToString() );
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

	void SpawnEntity_Inventory( string ent, array< EntityAI > objects, float quantity = -1, float health = -1 )
	{
		if ( IsMissionClient() && !IsMissionOffline() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( ent );
			rpc.Write( objects );
			rpc.Write( quantity );
			rpc.Write( health );
			rpc.Send( NULL, JMObjectSpawnerModuleRPC.Inventory, true, NULL );
		} else
		{
			Server_SpawnEntity_Inventory( ent, objects, quantity, health, NULL );
		}
	}

	private void Server_SpawnEntity_Inventory( string ent, array< EntityAI > objects, float quantity, float health, PlayerIdentity ident )
	{
		if ( GetGame().IsKindOf( ent, "DZ_LightAI" ) )
		{
			return;
		}

		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Inventory", ident ) )
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
				vector position = objects[i].GetPosition();
				obj = GetGame().CreateObject( ent, position );

				if ( !obj )
				{
					continue;
				}
				
				vector boundingBox[2];
				obj.GetCollisionBox( boundingBox );

				float height = Math.AbsFloat( boundingBox[1][1] ) + Math.AbsFloat( boundingBox[0][1] );

				position[1] = position[1] + ( height * 2.0 );

				obj.SetPosition( position );
			}

			SetupEntity( obj, quantity, health );

			GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + obj.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ") on " + objects[i] );
		}
	}

	private void RPC_SpawnEntity_Inventory( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string ent;
			if ( !ctx.Read( ent ) )
			{
				return;
			}

			array< EntityAI > objects;
			if ( !ctx.Read( objects ) )
			{
				return;
			}
		
			float quantity;
			if ( !ctx.Read( quantity ) )
			{
				return;
			}

			float health;
			if ( !ctx.Read( health ) )
			{
				return;
			}

			Server_SpawnEntity_Inventory( ent, objects, quantity, health, senderRPC );
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