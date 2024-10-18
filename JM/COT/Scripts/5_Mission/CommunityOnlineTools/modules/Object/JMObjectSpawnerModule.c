class JMObjectSpawnerModule: JMRenderableModuleBase
{
	int m_ObjSetupMode = COT_ObjectSetupMode.DEBUGSPAWN;
	bool m_AutoShow;
	string m_CurrentType;
	string m_SearchText;

	//! Items that are unfinished may not work or show properly or may even cause the game to segfault
	private ref array< string > m_UnfinishedItems =
	{
		"quickiebow",
		"recurvebow",
		"gp25base",
		"gp25",
		"gp25_standalone",
		"m203base",
		"m203",
		"m203_standalone",
		"red9",
		"pvcbow",
		"crossbow",
		"m249",
		"undersluggrenadem4",
		"groza",
		"pm73rak",
		"trumpet",
		"lawbase",
		"law",
		"rpg7base",
		"rpg7",
		"dartgun",
		"shockpistol",
		"shockpistol_black",
		"fnx45_arrow",
		"makarovpb",
		"mp133shotgun_pistolgrip",

		"largetentbackpack",
		"leatherbelt_natural",
		"leatherbelt_beige",
		"leatherbelt_brown",
		"leatherbelt_black",
		"leatherknifesheath"
	};

	private ref array< string > m_RestrictedClassNames =
	{
		"placing",
		"debug",
		"bldr_",
		"land_",
		"staticobj_"
	};

	bool m_AllowRestrictedClassNames;

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
		Bind( new JMModuleBinding( "DeleteCursor",			"UAObjectModuleDeleteOnCursor",	true 	) );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Entity.View" );
	}

	override void Hide()
	{
		m_AutoShow = false;

		super.Hide();
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
		return "JM\\COT\\GUI\\textures\\modules\\Object.paa";
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
		return "Object Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Delete" );
		types.Insert( "Vector" );
		types.Insert( "Player" );
	}

	void DeleteCursor( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( GetGame().GetUIManager().GetMenu() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Entity.Delete" ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
			return;
		}

		Object obj = GetObjectAtCursor();

		if (!obj)
			return;

		m_AutoShow = !IsVisible();
		if ( m_AutoShow )
			Show();

		JMObjectSpawnerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.DeleteCursor(obj);
	}
	
	//! Default distance is chosen such that if you can see the item hint on HUD, raycast should also hit
	Object GetObjectAtCursor(bool ignorePlayer = true, float distance = 3.0)
	{ 
		vector rayStart = GetGame().GetCurrentCameraPosition();

		DayZPlayer player = GetGame().GetPlayer();
		DayZPlayerCamera3rdPerson camera3rdPerson;
		if (player && !CurrentActiveCamera && Class.CastTo(camera3rdPerson, player.GetCurrentCamera()))
		{
			vector headPos = player.GetBonePositionWS(player.GetBoneIndexByName("Head"));
			distance += vector.Distance(rayStart, headPos);
		}

		vector rayEnd = rayStart + (GetGame().GetCurrentCameraDirection() * distance);

		RaycastRVParams rayInput = new RaycastRVParams( rayStart, rayEnd, GetGame().GetPlayer() );
		rayInput.flags = CollisionFlags.ALLOBJECTS;
		rayInput.radius = 0.1;
		array< ref RaycastRVResult > results = new array< ref RaycastRVResult >;

		Object obj;
		Object resultObj;
		TIntArray types = {ObjIntersectFire, ObjIntersectView};

		// Because way too many modders are too lazy...
		if (m_AllowRestrictedClassNames)
			types.Insert(ObjIntersectGeom);

		foreach (int type: types)
		{
			rayInput.type = type;

			if (!DayZPhysics.RaycastRVProxy(rayInput, results))
				continue;

			foreach ( RaycastRVResult result: results )
			{
				resultObj = result.obj;

				if ( resultObj == NULL )
				{
					continue;
				}

				EntityAI entity;
				if (Class.CastTo(entity, resultObj))
					resultObj = entity.GetHierarchyRoot();

				if ( PlayerBase.Cast( resultObj ) && ignorePlayer )
				{
					continue;
				}

				string name = resultObj.GetType();

				if ( name == "" )
				{
					continue;
				}

				name.ToLower();

				if ( name == "#particlesourceenf" )
				{
					continue;
				}

				if ( !m_AllowRestrictedClassNames )
				{
					if (resultObj.ConfigGetInt("scope") != 2)
						continue;

					bool blacklisted = false;

					foreach (string blacklistedName: m_RestrictedClassNames)
					{
						if ( name.Contains(blacklistedName) )
						{
							blacklisted = true;
							break;
						}
					}

					if (blacklisted)
						continue;
				}

				obj = resultObj;
				break;
			}

			if (obj)
				break;
		}

		return obj;
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

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
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
		if ( IsMissionClient() && !IsMissionOffline() )
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

	void SpawnEntity_Position( string className, vector position, float quantity = -1, float health = -1, float temp = -1, int itemState = -1, bool targetInventory = false )
	{
		EntityAI targetEnt;

		if ( IsMissionClient() )
		{
			if (targetInventory && !GetGame().IsKindOf(className, "DZ_LightAI"))
			{
				if (Class.CastTo(targetEnt, GetObjectAtCursor(false, 1000.0)) && !targetEnt.GetInventory())
					targetEnt = null;
			}
		}

		if ( IsMissionClient() && !IsMissionOffline() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( className );
			rpc.Write( position );
			rpc.Write( quantity );
			rpc.Write( health );
			rpc.Write( temp );
			rpc.Write( itemState );
			rpc.Write( m_ObjSetupMode );
			rpc.Send( targetEnt, JMObjectSpawnerModuleRPC.Position, true, NULL );
		}
		else if (!targetEnt)
		{
			Server_SpawnEntity_Position( className, position, quantity, health, temp, itemState, NULL );
		}
		else
		{
			Server_SpawnEntity_TargetInventory( className, targetEnt, position, quantity, health, temp, itemState, NULL );
		}
	}

	private void Server_SpawnEntity_Position( string className, vector position, float quantity, float health, float temp, int itemState, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Entity.Spawn.Position", ident, instance ) )
			return;
		
		int flags = ECE_CREATEPHYSICS;
		if ( GetGame().IsKindOf( className, "CarScript" ) && !COT_SurfaceIsWater( position ) )
			flags |= ECE_PLACE_ON_SURFACE;
		else if ( GetGame().IsKindOf( className, "BoatScript" ) && !COT_SurfaceIsWater( position ) )
			flags |= ECE_PLACE_ON_SURFACE; //! TODO: Check if its even needed
		
		if ( GetGame().IsKindOf( className, "DZ_LightAI" ) )
			flags |= 0x800;

		if (m_ObjSetupMode == COT_ObjectSetupMode.CE)
			flags |= ECE_EQUIP;

		EntityAI ent;
		if ( !Class.CastTo( ent, GetGame().CreateObjectEx( className, position, flags ) ) )
			return;

		SetupEntity( ent, quantity, health, temp, itemState, instance.PlayerObject, m_ObjSetupMode );

		GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + ent.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ", " + temp + ", "+itemState +") at " + position.ToString() );
		SendWebhook( "Vector", instance, "Spawned object \"" + className + "\" (" + ent.GetType() + ") at " + position );
	}

	private void RPC_SpawnEntity_Position( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string className;
			if ( !ctx.Read( className ) )
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

			float temp;
			if ( !ctx.Read( temp ) )
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

			if ( !ctx.Read( m_ObjSetupMode ) )
				return;

			EntityAI targetEnt;
			if (Class.CastTo(targetEnt, target))
				Server_SpawnEntity_TargetInventory( className, targetEnt, position, quantity, health, temp, itemState, senderRPC );
			else
				Server_SpawnEntity_Position( className, position, quantity, health, temp, itemState, senderRPC );
		}
	}

	void SpawnEntity_Inventory( string className, array< string > players, float quantity = -1, float health = -1, float temp = -1, int itemState = -1 )
	{
		if ( IsMissionClient() && !IsMissionOffline() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( className );
			rpc.Write( players );
			rpc.Write( quantity );
			rpc.Write( health );
			rpc.Write( temp );
			rpc.Write( itemState );
			rpc.Write( m_ObjSetupMode );
			rpc.Send( NULL, JMObjectSpawnerModuleRPC.Inventory, true, NULL );
		}
		else
		{
			Server_SpawnEntity_Inventory( className, players, quantity, health, temp, itemState, NULL );
		}
	}

	private void Server_SpawnEntity_Inventory( string className, array< string > players, float quantity, float health, float temp, int itemState, PlayerIdentity ident )
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
				else if ( GetGame().IsKindOf( className, "BoatScript" ) && !COT_SurfaceIsWater( position ) )
					flags |= ECE_PLACE_ON_SURFACE; //! TODO: Check if its even needed

				if (m_ObjSetupMode == COT_ObjectSetupMode.CE)
					flags |= ECE_EQUIP;
		
				if ( !Class.CastTo( ent, GetGame().CreateObjectEx( className, position, flags ) ) )
					continue;

				loggedSuffix = " at " + position.ToString();
			}

			SetupEntity( ent, quantity, health, temp, itemState, instance.PlayerObject, m_ObjSetupMode );

			GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + ent.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ", " + temp + ", "+ itemState+") on " + instance.GetSteam64ID() + loggedSuffix );
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

			float temp;
			if ( !ctx.Read( temp ) )
				return;

			int itemState;
			if ( !ctx.Read( itemState ) )
				return;

			if ( !ctx.Read( m_ObjSetupMode ) )
				return;

			Server_SpawnEntity_Inventory( ent, players, quantity, health, temp, itemState, senderRPC );
		}
	}
	
	private void Server_SpawnEntity_TargetInventory( string className, EntityAI targetEnt, vector position, float quantity, float health, float temp, int itemState, PlayerIdentity ident )
	{
		JMPlayerInstance callerInstance;

		EntityAI ent;

		if ( GetPermissionsManager().HasPermission( "Entity.Spawn.Inventory", ident, callerInstance ) && !GetGame().IsKindOf( className, "DZ_LightAI" ) && targetEnt.GetInventory() && Class.CastTo( ent, targetEnt.GetInventory().CreateInInventory( className ) ) )
		{
			string loggedSuffix = " at " + position.ToString();

			SetupEntity( ent, quantity, health, temp, itemState, callerInstance.PlayerObject, m_ObjSetupMode );

			GetCommunityOnlineToolsBase().Log( ident, "Spawned Entity " + ent.GetDisplayName() + " (" + ent + ", " + quantity + ", " + health + ", " + temp + ", "+ itemState +") on " + targetEnt.ToString() + loggedSuffix );
			SendWebhook( "Player", callerInstance, "Spawned object \"" + ent.GetDisplayName() + "\" (" + ent.GetType() + ") on " + targetEnt.ToString() + loggedSuffix );
		}
		else
		{
			Server_SpawnEntity_Position(className, position, quantity, health, temp, itemState, ident);
		}
	}

	private void SetupEntity( EntityAI entity, float quantity, float health, float temp, int itemState, PlayerBase player, COT_ObjectSetupMode mode = COT_ObjectSetupMode.NONE )
	{
		switch (mode)
		{
			case COT_ObjectSetupMode.DEBUGSPAWN:
				int depth;
				if (!entity.IsMan())
					depth = 3;
				OnDebugSpawn(entity, player, depth);
				break;

			case COT_ObjectSetupMode.CE:
				entity.EEOnCECreate();
				break;
		}

		ItemBase item;
		if ( Class.CastTo( item, entity ) )
		{
			if (quantity != -1)
			{
				Magazine mag;
				if (Class.CastTo(mag, item))
				{
					mag.ServerSetAmmoCount(quantity);
				}
				else if (item.HasQuantity())
				{
					item.SetQuantity(quantity);

					if (item.GetCompEM())
						item.GetCompEM().SetEnergy0To1(quantity / item.GetQuantityMax());
				}
			}

			if ( itemState != 0 )
			{
				if ( item.IsLiquidContainer() )
				{
					int liquidType = itemState;
					if ( item.IsBloodContainer() )
						liquidType = LIQUID_BLOOD_0_P * Math.Pow(2, itemState - 1);
					
					if (Liquid.GetNutritionalProfileByType(liquidType))
						item.SetLiquidType(liquidType);
					else
						Error("Invalid liquid type " + liquidType);
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

		float maxHealth = MiscGameplayFunctions.GetTypeMaxGlobalHealth(entity.GetType());
		if (maxHealth > 0)
		{
			if ( health == -1 )
				health = maxHealth;

			if ( health >= 0 )
			{
				entity.SetHealth( "", "", health );

				float health01 = health / maxHealth;

				TStringArray dmgZones = {};
				entity.GetDamageZones(dmgZones);
				foreach (string dmgZone: dmgZones)
				{
					entity.SetHealth01(dmgZone, "Health", health01);
				}

				if (entity.GetInventory())
				{
					for (int i = 0; i < entity.GetInventory().AttachmentCount(); i++)
					{
						EntityAI attachment = entity.GetInventory().GetAttachmentFromIndex(i);
						float attachmentHealth = MiscGameplayFunctions.GetTypeMaxGlobalHealth(attachment.GetType()) * health01;
						SetupEntity(attachment, -1, attachmentHealth, temp, 0, player, COT_ObjectSetupMode.NONE);
					}
				}
			}
		}

		if ( temp != -1 )
			entity.SetTemperatureEx(new TemperatureData(temp));
	}

	bool IsExcludedClassName( string className )
	{
		if ( m_UnfinishedItems.Find( className ) > -1 )
		{
			return true;
		}

		if ( !m_AllowRestrictedClassNames )
		{
			foreach ( string restrictedClassName: m_RestrictedClassNames )
			{
				if ( className.Contains( restrictedClassName ) )
				{
					return true;
				}
			}
		}

		return false;
	}

	void OnDebugSpawn(EntityAI entity, PlayerBase player, int depth = 3) 
	{
		ItemBase item;
		CarScript car;
		BoatScript boat;
		BuildingBase building;

		if (Class.CastTo(item, entity))
			item.COT_OnDebugSpawn(player);
		else if (Class.CastTo(car, entity))
			car.COT_OnDebugSpawn(player);
		else if (Class.CastTo(boat, entity))
			boat.COT_OnDebugSpawn(player);
		else if (Class.CastTo(building, entity))
			building.COT_OnDebugSpawn(player);

		if (!entity.GetInventory())
			return;

		if (!entity.GetInventory().GetAttachmentSlotsCount())
			return;

		if (entity.GetInventory().AttachmentCount())
			return;

		if (entity.IsInherited(DayZCreature)  || entity.IsInherited(TentBase) || entity.IsInherited(Weapon_Base))
			return;

		//! If no atts were spawned, do it ourself (except for creatures, tents & weapons)
		SpawnCompatibleAttachments(entity, player, depth);
	}

	//! @note this does what vanilla EntityAI::OnDebugSpawn *should* be doing (get inventorySlot as array, use slot IDs instead of case-sensitive match of slot names, filter bad items)
	void SpawnCompatibleAttachments(EntityAI entity, PlayerBase player, int depth = 3) 
	{
		TIntArray slot_ids = {};
		int slot_id;

		TStringArray atts = {};
		entity.ConfigGetTextArray("attachments", atts);

		foreach (string att: atts)
		{
			slot_id = InventorySlots.GetSlotIdFromString(att);
			if (slot_id != InventorySlots.INVALID && InventorySlots.GetShowForSlotId(slot_id))
			{
				CF_Log.Info("Entity %1 has visible attachment slot %2 (ID %3)", entity.GetType(), att, slot_id.ToString());
				slot_ids.Insert(slot_id);
			}
		}

		TStringArray all_paths = new TStringArray;

		all_paths.Insert(CFG_VEHICLESPATH);
		all_paths.Insert(CFG_WEAPONSPATH);

		string child_name;
		int scope;
		string path;
		string model;
		int idx;
		EntityAI child;

		foreach (string config_path: all_paths)
		{
			int children_count = GetGame().ConfigGetChildrenCount(config_path);

			for (int i = 0; i < children_count; i++)
			{
				GetGame().ConfigGetChildName(config_path, i, child_name);
				path = config_path + " " + child_name;
				scope = GetGame().ConfigGetInt(path + " scope");

				if (scope == 2)
				{
					if (!GetGame().ConfigGetText(path + " model", model) || model == string.Empty || model == "bmp")
						continue;

					TStringArray inv_slots = {};
					GetGame().ConfigGetTextArray(path + " inventorySlot", inv_slots);

					foreach (string inv_slot: inv_slots)
					{
						slot_id = InventorySlots.GetSlotIdFromString(inv_slot);
						if (slot_id != InventorySlots.INVALID)
						{
							idx = slot_ids.Find(slot_id);
							if (idx > -1)
							{
								//if (!IsInventoryBase(path))
									//break;

								child_name.ToLower();
								if (IsExcludedClassName(child_name))
									break;

								//! Limit character attachments to clothing
								if (entity.IsMan() && !GetGame().IsKindOf(child_name, "Clothing_Base"))
									break;

								child = entity.GetInventory().CreateAttachmentEx(child_name, slot_id);
								if (child)
								{
									CF_Log.Info("Successfully spawned %1 in slot %2 on %3", child_name, inv_slot, entity.GetType());
									slot_ids.Remove(idx);
									if (depth > 0)
										OnDebugSpawn(child, player, depth - 1);
									if (slot_ids.Count() == 0)
										return;
								}
							}
						}
					}
				}
			}
		}
	}

	bool IsInventoryBase( string path )
	{
		TStringArray full_path = new TStringArray;
		
		GetGame().ConfigGetFullPath(path, full_path);
		
		string cfg_parent_name = "inventory_base";
		foreach (string tmp: full_path)
		{
			tmp.ToLower();
			if (tmp == cfg_parent_name)
			{
				return true;
			}
		}
	
		return false;
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
		else if ( GetGame().IsKindOf( className, "BoatScript" ) && !COT_SurfaceIsWater( position ) )
			flags |= ECE_PLACE_ON_SURFACE; //! TODO: Check if its even needed
		
		if ( GetGame().IsKindOf( className, "DZ_LightAI" ) )
			flags |= 0x800;

		if (m_ObjSetupMode == COT_ObjectSetupMode.CE)
			flags |= ECE_EQUIP;

		EntityAI ent;
		if ( !Class.CastTo( ent, GetGame().CreateObjectEx( className, position, flags ) ) )
			return;

		float quantity = -1;
		float health = -1;
		float temp = -1;
		int itemState = -1;
		SetupEntity( ent, quantity, health, temp, itemState, instance.PlayerObject, m_ObjSetupMode );
		
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
