class JMObjectSpawnerModule: JMRenderableModuleBase
{
	void JMObjectSpawnerModule()
	{   
		GetRPCManager().AddRPC( "COT_Object", "SpawnObjectPosition", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_Object", "SpawnObjectInventory", this, SingeplayerExecutionType.Server );

		GetRPCManager().AddRPC( "COT_Object", "SpawnEntity", this, SingeplayerExecutionType.Server );

		GetRPCManager().AddRPC( "COT_Object", "DeleteObject", this, SingeplayerExecutionType.Server );

		GetPermissionsManager().RegisterPermission( "Object.Spawn.Position" );
		GetPermissionsManager().RegisterPermission( "Object.Spawn.Inventory" );
		GetPermissionsManager().RegisterPermission( "Object.Spawn.Entity" );
		GetPermissionsManager().RegisterPermission( "Object.View" );

		GetPermissionsManager().RegisterPermission( "Object.Delete" );
	}

	override void RegisterKeyMouseBindings() 
	{
		RegisterKeyMouseBinding( new JMModuleBinding( "SpawnRandomInfected",		"UAObjectModuleSpawnInfected",	true 	) );
		RegisterKeyMouseBinding( new JMModuleBinding( "SpawnRandomAnimal",			"UAObjectModuleSpawnAnimal",	true 	) );
		RegisterKeyMouseBinding( new JMModuleBinding( "SpawnRandomWolf",			"UAObjectModuleSpawnWolf",		true 	) );
		RegisterKeyMouseBinding( new JMModuleBinding( "DeleteOnCursor",				"UAObjectModuleDeleteOnCursor",	true 	) );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Object.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/objectspawner_form.layout";
	}
	
	void SpawnRandomInfected( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Object.Spawn.Entity" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
			return;
		}

		GetRPCManager().SendRPC( "COT_Object", "SpawnEntity", new Param2< string, vector >( WorkingZombieClasses().GetRandomElement(), GetPointerPos() ) );
	}

	void SpawnRandomAnimal( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Object.Spawn.Entity" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
			return;
		}

		GetRPCManager().SendRPC( "COT_Object", "SpawnEntity", new Param2< string, vector >( GetRandomChildFromBaseClass( "cfgVehicles", "AnimalBase", 2, "Animal_CanisLupus" ), GetPointerPos() ) );
	}

	void SpawnRandomWolf( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Object.Spawn.Entity" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
			return;
		}

		GetRPCManager().SendRPC( "COT_Object", "SpawnEntity", new Param2< string, vector >( GetRandomChildFromBaseClass( "cfgVehicles", "Animal_CanisLupus" ), GetPointerPos() ) );
	}

	void DeleteOnCursor( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Object.Delete" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
			return;
		}

		GetRPCManager().SendRPC( "COT_Object", "DeleteObject", new Param, false, NULL, GetCursorObject( 10.0, GetGame().GetPlayer(), 0.01 ) );
	}

	void SpawnEntity( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Object.Spawn.Entity", sender ) )
			return;

		Param2< string, vector > data;
		if ( !ctx.Read( data ) ) return;
		
		if ( type == CallType.Server )
		{
			EntityAI entity = EntityAI.Cast( GetGame().CreateObject( data.param1, data.param2, false, GetGame().IsKindOf( data.param1, "DZ_LightAI" ) ) );

			if ( entity == NULL ) return;

			GetCommunityOnlineToolsBase().Log( sender, "Spawned Entity " + entity.GetDisplayName() + " (" + data.param1 + ") at " + data.param2.ToString() );

			//SendAdminNotification( sender, NULL, "You have spawned a " + entity.GetDisplayName() + " at " + VectorToString( data.param2, 1 ) );
		}
	}
	
	void SpawnObjectPosition( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Object.Spawn.Position", sender ) )
			return;

		Param3< string, vector, string > data;
		if ( !ctx.Read( data ) ) return;
		
		if ( type == CallType.Server )
		{
			bool ai = false;

			if ( GetGame().IsKindOf( data.param1, "DZ_LightAI" ) ) 
			{
				ai = true;
			}

			EntityAI entity = EntityAI.Cast( GetGame().CreateObject( data.param1, data.param2, false, ai ) );

			if ( entity == NULL ) return;

			entity.SetHealth( entity.GetMaxHealth() );

			int quantity = 0;
			
			if ( entity.IsInherited( ItemBase ) )
			{
				ItemBase oItem = ItemBase.Cast( entity );
				SetupSpawnedItem( oItem, oItem.GetMaxHealth(), 1 );


				string text = data.param3;

				text.ToUpper();

				if (text == "MAX")
				{
					quantity = oItem.GetQuantityMax();
				} else
				{
					quantity = text.ToInt();
				}

				oItem.SetQuantity(quantity);

				// oItem.CreateDynamicPhysics( PhxInteractionLayers.DYNAMICITEM );
				// oItem.EnableDynamicCCD( true );
			}

			entity.PlaceOnSurface();

			GetCommunityOnlineToolsBase().Log( sender, "Spawned object " + entity.GetDisplayName() + " (" + data.param1 + ") at " + data.param2.ToString() + " with amount " + quantity );

			//SendAdminNotification( sender, NULL, "You have spawned " + entity.GetDisplayName() + " at " + VectorToString( data.param2, 1 ) + ", quantity " + quantity );
		}
	}
	
	protected void SpawnItemOnPlayer( ref PlayerIdentity sender, PlayerBase player, string item, string quantText )
	{
		if ( !player )
			return;

		EntityAI entity = player.GetInventory().CreateInInventory( item );

		entity.SetHealth( entity.GetMaxHealth() );

		int quantity = 0;
				
		if ( entity.IsInherited( ItemBase ) )
		{
			ItemBase oItem = ItemBase.Cast( entity );
			SetupSpawnedItem( oItem, oItem.GetMaxHealth(), 1 );

			quantText.ToUpper();

			if ( quantText == "MAX")
			{
				quantity = oItem.GetQuantityMax();
			} else
			{
				quantity = quantText.ToInt();
			}

			oItem.SetQuantity(quantity);
		}
		
		GetCommunityOnlineToolsBase().Log( sender, "Spawned object " + entity.GetDisplayName() + " (" + item + ") on " + player.GetAuthenticatedPlayer().Data.SSteam64ID + " with amount " + quantity );

		SendAdminNotification( sender, player.GetIdentity(), entity.GetDisplayName() + " has been added to your inventory, quantity " + quantity );

		//if ( sender.GetPlainId() != player.GetIdentity().GetPlainId() )
		//	SendAdminNotification( player.GetIdentity(), sender, entity.GetDisplayName() + " has been added to their inventory, quantity " + quantity );
	}

	void SpawnObjectInventory( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Object.Spawn.Inventory", sender ) )
			return;

		ref Param3< string, string, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;
		
		array< string > guids = new array< string >;
		guids.Copy( data.param3 );

		if ( type == CallType.Server )
		{
			if ( !GetGame().IsMultiplayer() )
			{
				SpawnItemOnPlayer( sender, GetGame().GetPlayer(), data.param1, data.param2 );
			} else
			{
				array< ref JMPlayerInstance > players = GetPermissionsManager().GetPlayersFromArray( guids );
	
				for ( int i = 0; i < players.Count(); i++ )
				{
					SpawnItemOnPlayer( sender, players[i].PlayerObject, data.param1, data.param2 );
				}
			}
		}
	}

	void DeleteObject( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Object.Delete", sender ) )
			return;
		
		if ( type == CallType.Server )
		{
			if ( target == NULL ) return;

			string obtype;
			GetGame().ObjectGetType( target, obtype );

			GetCommunityOnlineToolsBase().Log( sender, "Deleted object " + target.GetDisplayName() + " (" + obtype + ") at " + target.GetPosition() );
			SendAdminNotification( sender, NULL, target.GetDisplayName() + " has been deleted from the world." );

			GetGame().ObjectDelete( target );
		}
	}
}