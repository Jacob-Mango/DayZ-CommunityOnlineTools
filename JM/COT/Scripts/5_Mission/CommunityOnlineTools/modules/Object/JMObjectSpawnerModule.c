class JMObjectSpawnerModule: JMRenderableModuleBase
{
	int m_ObjSetupMode = COT_ObjectSetupMode.DEBUGSPAWN;

	//! Which ammo a spawned magazine is filled with, as the classname of an
	//! entry from its own ammoItems list. "" leaves the magazine on whatever it
	//! is configured to hold.
	//!
	//! It rides the spawn RPC as module state rather than as another parameter
	//! on nine signatures, exactly as m_ObjSetupMode already does.
	string m_SpawnAmmoType;

	//! Upper bounds for a randomised spawn. -1 means "not a range": the value
	//! passed as quantity / health is used exactly.
	//!
	//! The roll happens per ENTITY, on the server, so spawning into five
	//! inventories at once gives five different values rather than one value
	//! five times - which is the whole point of asking for a range.
	float m_SpawnQuantityMax = -1;
	float m_SpawnHealthMax   = -1;
	bool m_AutoShow;
	string m_CurrentType;
	string m_SearchText;
	int m_OverrideDebugSpawnDepth;

	// Loaded from SpawnerConfig.json - no longer hardcoded
	protected ref array< string > m_UnfinishedItems     = new array< string >;
	protected ref array< string > m_RestrictedClassNames = new array< string >;
	protected ref array< string > m_RestrictedStartClassNames = new array< string >;
	bool m_AllowRestrictedClassNames;
	bool m_FilterWithDisplayName;

	//! classname (lowercased, color token stripped) -> { token -> real classname }.
	//! Built once from CfgVehicles + CfgWeapons and kept for the module's
	//! whole lifetime - those trees are static game data, so re-scanning tens
	//! of thousands of config entries on every "Change Colors" click (once
	//! per candidate color, as FindColorVariant used to) is pure waste and
	//! was the whole of the "big lag opening the menu" complaint.
	protected static ref map<string, ref map<string, string>> s_ColorVariantsByBase;

	//! Which colors "Change Colors" can actually offer for `entity` - not the
	//! full token list, only the ones a real sibling classname exists for,
	//! either on the entity itself or on one of its already-attached
	//! children. Most vanilla bodies (a car chassis, for instance) have no
	//! color-suffixed sibling at all - only mods/attachments that were built
	//! with one do - so an unfiltered list would mostly be dead rows that do
	//! nothing when clicked.
	array<string> GetAvailableColorVariants(EntityAI entity, int depth = 3)
	{
		array<string> result = new array<string>;
		if (!entity)
			return result;

		array<string> tokens = GetColorTokens();
		for (int i = 0; i < tokens.Count(); ++i)
		{
			if (HasColorVariant(entity, tokens[i], depth))
				result.Insert(tokens[i]);
		}

		return result;
	}

	static string GetColorTokenAt(int index)
	{
		array<string> tokens = GetColorTokens();
		if (index < 0 || index >= tokens.Count())
			return "";

		return tokens[index];
	}

	static int GetColorTokenIndex(string token)
	{
		return GetColorTokens().Find(token);
	}

	//! The color tokens this mod recognizes in a classname, e.g. the "blue" in
	//! "CivilianSedan_Blue". Shared by DetectColorToken and the "Change Colors"
	//! context-menu action so both sides of that wire agree on what index N
	//! means.
	//! "darkblue" before "blue": Contains() matches the first hit, and every
	//! "darkblue" classname also contains "blue" as a substring - checking
	//! the longer token first is what keeps a darkblue part from being
	//! mis-tokenized as a blue one with "dark" left dangling in its base name.
	static ref array<string> GetColorTokens()
	{
		return { "darkblue", "white", "black", "blue", "red", "green", "tan", "camo", "orange", "grey", "gray", "yellow", "wine", "beige", "rust" };
	}

	//! Default distance is chosen such that if you can see the item hint on HUD, raycast should also hit
	Object GetObjectAtCursor(bool ignorePlayers = true, float distance = 3.0)
	{ 
		vector rayStart = g_Game.GetCurrentCameraPosition();

		DayZPlayer player = g_Game.GetPlayer();
		DayZPlayerCamera3rdPerson camera3rdPerson;
		if (player && !CurrentActiveCamera && Class.CastTo(camera3rdPerson, player.GetCurrentCamera()))
		{
			vector headPos = player.GetBonePositionWS(player.GetBoneIndexByName("Head"));
			distance += vector.Distance(rayStart, headPos);
		}

		vector rayEnd = rayStart + (g_Game.GetCurrentCameraDirection() * distance);

		RaycastRVParams rayInput = new RaycastRVParams(rayStart, rayEnd);

		//! Only ignore client player if not in freecam/spectator cam
		if (!CurrentActiveCamera || !CurrentActiveCamera.IsActive())
			rayInput.ignore = player;

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
					continue;

				if ((resultObj.IsBush() || resultObj.IsTree()) && type != ObjIntersectView)
					continue;

				EntityAI entity;
				if (Class.CastTo(entity, resultObj))
					resultObj = entity.GetHierarchyRoot();

				if ( PlayerBase.Cast( resultObj ) && ignorePlayers )
					continue;

				string name = resultObj.GetType();

				if ( name == "" )
					continue;

				name.ToLower();

				if ( name == "#particlesourceenf" )
					continue;

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

					foreach (string blacklistedStartName: m_RestrictedStartClassNames)
					{
						if ( name.IndexOf(blacklistedStartName) == 0 )
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

	//! Read-only walk of `entity` and its attachments (mirrors the walk
	//! RecolorEntityAndAttachments does to actually perform the swap) -
	//! true the moment any part has a real sibling classname for `newColor`.
	protected bool HasColorVariant(EntityAI entity, string newColor, int depth)
	{
		if (!entity)
			return false;

		if (FindColorVariant(entity.GetType(), newColor) != "")
			return true;

		if (depth <= 0)
			return false;

		GameInventory inventory = entity.GetInventory();
		if (!inventory)
			return false;

		int count = inventory.GetAttachmentSlotsCount();
		for (int i = 0; i < count; ++i)
		{
			int slot_id = inventory.GetAttachmentSlotId(i);
			if (slot_id == InventorySlots.INVALID)
				continue;

			EntityAI child = inventory.FindAttachment(slot_id);
			if (child && HasColorVariant(child, newColor, depth - 1))
				return true;
		}

		return false;
	}

	protected static bool HasDelimitedToken(string lower, string token)
	{
		return FindDelimitedTokenIndex(lower, token) != -1;
	}

	bool IsExcludedClassName( string className )
	{
		if ( m_UnfinishedItems.Find( className ) > -1 )
			return true;

		if ( !m_AllowRestrictedClassNames )
		{
			foreach ( string restrictedClassName: m_RestrictedClassNames )
			{
				if ( className.Contains( restrictedClassName ) )
					return true;
			}
			foreach (string blacklistedStartName: m_RestrictedStartClassNames)
			{
				if ( className.IndexOf(blacklistedStartName) == 0 )
					return true;
			}
		}

		return false;
	}

	bool IsInventoryBase( string path )
	{
		TStringArray full_path = new TStringArray;
		
		g_Game.ConfigGetFullPath(path, full_path);
		
		string cfg_parent_name = "inventory_base";
		foreach (string tmp: full_path)
		{
			tmp.ToLower();
			if (tmp == cfg_parent_name)
				return true;
		}
	
		return false;
	}

	bool IsInventoryType(string type)
	{
		if (g_Game.IsKindOf(type, "Inventory_Base"))
			return true;

		if (g_Game.ConfigIsExisting(CFG_WEAPONSPATH + " " + type))
			return true;

		if (g_Game.ConfigIsExisting(CFG_MAGAZINESPATH + " " + type))
			return true;

		return false;
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "#STR_COT_OBJECT_MODULE_NAME";
		info.WebhookTitle = "Object Module";
		info.Icon = "package-plus";
		info.Layout = "JM/COT/GUI/layouts/objectspawner_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_ITEMS;
		info.ViewPermission = JMConstants.PERM_ENTITY_VIEW;
		info.InputToggle = "UACOTToggleEntity";
		info.SetRPCRange( JMObjectSpawnerModuleRPC.INVALID, JMObjectSpawnerModuleRPC.COUNT );

		//! Called on both client and server as the module registers, before the mission loads.
		info.AddPermission( JMConstants.PERM_ENTITY_SPAWN_POSITION );
		info.AddPermission( JMConstants.PERM_ENTITY_SPAWN_INVENTORY );
		info.AddPermission( JMConstants.PERM_ENTITY_DELETE );

		info.AddWebhookType( "Delete" );
		info.AddWebhookType( "Vector" );
		info.AddWebhookType( "Player" );
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if ( IsMissionHost() )
			LoadSpawnerConfig();
	}

	protected void LoadSpawnerConfig()
	{
		JMSpawnerConfig cfg = JMSpawnerConfig.Load();

		m_UnfinishedItems.Clear();
		foreach ( string item: cfg.UnfinishedItems )
			m_UnfinishedItems.Insert( item );

		m_RestrictedClassNames.Clear();
		foreach ( string pattern: cfg.RestrictedPatterns )
			m_RestrictedClassNames.Insert( pattern );

		m_RestrictedStartClassNames.Clear();
		foreach ( string startPattern: cfg.RestrictedStartPatterns )
			m_RestrictedStartClassNames.Insert( startPattern );
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

	override void Hide()
	{
		m_AutoShow = false;

		super.Hide();
	}

	void DeleteCursor( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( g_Game.GetUIManager().GetMenu() )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_ENTITY_DELETE ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_INPUT_DELETE_CROSSHAIR");
			return;
		}

		Object obj = GetObjectAtCursor();

		if (!obj)
			return;

		//! Only stamp m_AutoShow on the press that actually opens the panel.
		//! The confirm flow now spans two presses (arm, then confirm) with the
		//! panel already visible for the second one - recomputing this from
		//! IsVisible() on every press would flip it back to false right as the
		//! confirm press lands, and the panel we auto-opened would never
		//! auto-close after the delete goes through.
		if ( !IsVisible() )
		{
			m_AutoShow = true;
			Show();
		}

		JMObjectSpawnerForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.DeleteCursor(obj);
	}

	void SpawnRandomInfected( UAInput input )
	{
		if ( !input.LocalPress() )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_ENTITY_SPAWN_POSITION ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_INPUT_SPAWN_INFECTED");
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

		if ( !JMPermissions.Has( JMConstants.PERM_ENTITY_SPAWN_POSITION ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_INPUT_SPAWN_ANIMAL");
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

		if ( !JMPermissions.Has( JMConstants.PERM_ENTITY_SPAWN_POSITION ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			ShowInactiveNotification("STR_COT_INPUT_SPAWN_WOLF");
			return;
		}

		string className = GetRandomChildFromBaseClass( "cfgVehicles", "Animal_CanisLupus" );
		vector position = GetPointerPos();

		SpawnEntity_Position( className, position );
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
		}
		else
			Exec_DeleteEntity( obj, NULL );

		CF_Modules<JMESPModule>.Get().m_RemoveDeleted = true;
	}

	protected void Exec_DeleteEntity( notnull Object obj, PlayerIdentity ident )
	{
		Server_DeleteEntity( obj, ident );
	}

	//! DEPRECATED - override Exec_DeleteEntity( obj, ident ) instead.
	//!
	//! This is the old name of Exec_DeleteEntity and it still holds the body, so a
	//! third-party `override Server_DeleteEntity` (ZenCOT) keeps running. An
	//! override point is not a call, so there is no runtime warning.
	protected void Server_DeleteEntity( notnull Object obj, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_ENTITY_DELETE, ident, instance ) )
			return;

		PlayerBase player;
		if ( Class.CastTo( player, obj ) && ( player.IsAlive() || player.GetIdentity() != null ) )
			return;

		string obtype = Object.GetDebugName( obj );

		vector transform[4];
		obj.GetTransform( transform );

		g_Game.ObjectDelete( obj );
		
		GetCommunityOnlineToolsBase().Log( ident, "Deleted Entity " + obtype + " at " + transform[3].ToString() );
		SendWebhookColored( "Delete", instance, "Deleted object " + obtype + " at " + transform[3].ToString(), JMConstants.WEBHOOK_COLOR_DANGER );
	}

	protected void RPC_DeleteEntity( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			if ( target == NULL )
				return;

			Exec_DeleteEntity( target, senderRPC );
		}
	}

	protected void SpawnEntity_WriteTo(ParamsWriteContext ctx, string className, float quantity, float health, float temp, int itemState)
	{
		ctx.Write( className );
		ctx.Write( quantity );
		ctx.Write( health );
		ctx.Write( temp );
		ctx.Write( itemState );
		ctx.Write( m_ObjSetupMode );
		ctx.Write( m_SpawnAmmoType );
		ctx.Write( m_SpawnQuantityMax );
		ctx.Write( m_SpawnHealthMax );
	}

	protected bool SpawnEntity_ReadFrom(ParamsReadContext ctx, out string className, out float quantity, out float health, out float temp, out int itemState)
	{
		if ( !ctx.Read( className ) )
			return false;
	
		if ( !ctx.Read( quantity ) )
			return false;

		if ( !ctx.Read( health ) )
			return false;

		if ( !ctx.Read( temp ) )
			return false;

		if ( !ctx.Read( itemState ) )
			return false;

		if ( !ctx.Read( m_ObjSetupMode ) )
			return false;

		if ( !ctx.Read( m_SpawnAmmoType ) )
			return false;

		if ( !ctx.Read( m_SpawnQuantityMax ) )
			return false;

		if ( !ctx.Read( m_SpawnHealthMax ) )
			return false;

		return true;
	}

	void SpawnEntity_Position( string className, vector position, float quantity = -1, float health = -1, float temp = -1, int itemState = -1, bool targetInventory = false )
	{
		EntityAI targetEnt;

		if ( IsMissionClient() )
		{
			if (targetInventory && !g_Game.IsKindOf(className, "DZ_LightAI"))
			{
				if (Class.CastTo(targetEnt, GetObjectAtCursor(false, 1000.0)) && !targetEnt.GetInventory())
					targetEnt = null;
			}
		}

		if ( IsMissionClient() && !IsMissionOffline() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( position );
			SpawnEntity_WriteTo(rpc, className, quantity, health, temp, itemState);
			rpc.Send( targetEnt, JMObjectSpawnerModuleRPC.Position, true, NULL );
		}
		else if (!targetEnt)
			Exec_SpawnEntity_Position( className, position, quantity, health, temp, itemState, NULL );
		else
			Exec_SpawnEntity_TargetInventory( className, targetEnt, position, quantity, health, temp, itemState, NULL );
	}

	protected void Exec_SpawnEntity_Position( string className, vector position, float quantity, float health, float temp, int itemState, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_ENTITY_SPAWN_POSITION, ident, instance ) )
			return;

		SpawnEntity(className, null, position, quantity, health, temp, itemState, instance);
	}

	protected void RPC_SpawnEntity_Position( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !senderRPC )
			return;

		if ( IsMissionHost() )
		{
			vector position;

			if ( !ctx.Read( position ) )
			{
				Error("Failed");
				return;
			}

			if ( !COT.IsValidWorldPosition( position ) )
				return;

			string className;
			float quantity;
			float health;
			float temp;
			int itemState;

			if ( !SpawnEntity_ReadFrom(ctx, className, quantity, health, temp, itemState) )
			{
				Error("Failed");
				return;
			}

			EntityAI targetEnt;
			if (Class.CastTo(targetEnt, target))
				Exec_SpawnEntity_TargetInventory( className, targetEnt, position, quantity, health, temp, itemState, senderRPC );
			else
				Exec_SpawnEntity_Position( className, position, quantity, health, temp, itemState, senderRPC );
		}
	}

	void SpawnEntity_Inventory( string className, array< string > players, float quantity = -1, float health = -1, float temp = -1, int itemState = -1 )
	{
		if ( IsMissionClient() && !IsMissionOffline() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( players );
			SpawnEntity_WriteTo(rpc, className, quantity, health, temp, itemState);
			rpc.Send( NULL, JMObjectSpawnerModuleRPC.Inventory, true, NULL );
		}
		else
			Exec_SpawnEntity_Inventory( className, players, quantity, health, temp, itemState, NULL );
	}

	protected void Exec_SpawnEntity_Inventory( string className, array< string > players, float quantity, float health, float temp, int itemState, PlayerIdentity ident )
	{
		if ( g_Game.IsKindOf( className, "DZ_LightAI" ) )
			return;

		JMPlayerInstance callerInstance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_ENTITY_SPAWN_INVENTORY, ident, callerInstance ) )
			return;

		for ( int i = 0; i < players.Count(); i++ )
		{
			JMPlayerInstance instance = GetPermissionsManager().GetPlayer( players[i] );
			if ( !instance || !instance.PlayerObject )
				continue;

			instance.Update();

			EntityAI parent = instance.PlayerObject;
			vector position = instance.PlayerObject.GetPosition();
			SpawnEntity(className, parent, position, quantity, health, temp, itemState, callerInstance);
		}
	}

	protected void RPC_SpawnEntity_Inventory( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			array< string > players;
			if ( !ctx.Read( players ) )
			{
				Error("Failed");
				return;
			}

			string className;
			float quantity;
			float health;
			float temp;
			int itemState;

			if ( !SpawnEntity_ReadFrom(ctx, className, quantity, health, temp, itemState) )
			{
				Error("Failed");
				return;
			}

			Exec_SpawnEntity_Inventory( className, players, quantity, health, temp, itemState, senderRPC );
		}
	}

	void SpawnEntity_Inventory(string className, set<ref JMSelectedObject> targetObjects, float quantity = -1, float health = -1, float temp = -1, int itemState = -1)
	{
		if (!JMPermissions.Has(JMConstants.PERM_ENTITY_SPAWN_INVENTORY))
			return;

		foreach (JMSelectedObject targetObject: targetObjects)
		{
			EntityAI targetEnt;
			if (Class.CastTo(targetEnt, targetObject.obj) && targetEnt.GetInventory())
			{
				vector position = targetEnt.GetPosition();

				if (g_Game.IsClient())
				{
					ScriptRPC rpc = new ScriptRPC();
					rpc.Write( position );
					SpawnEntity_WriteTo(rpc, className, quantity, health, temp, itemState);
					rpc.Send( targetEnt, JMObjectSpawnerModuleRPC.Position, true, NULL );
				}
				else
				{
					Exec_SpawnEntity_TargetInventory(className, targetEnt, position, quantity, health, temp, itemState, null);
				}
			}
		}
	}

	protected void Exec_SpawnEntity_TargetInventory( string className, EntityAI targetEnt, vector position, float quantity, float health, float temp, int itemState, PlayerIdentity ident )
	{
		JMPlayerInstance callerInstance;
		if (!JMPermissions.HasRPC(JMConstants.PERM_ENTITY_SPAWN_INVENTORY, ident, callerInstance))
			return;

		SpawnEntity(className, targetEnt, position, quantity, health, temp, itemState, callerInstance);
	}

	//! @note LocationCreateEntity ignores ECE_EQUIP so we always use ObjectCreateEx and move entity to parent (if given) afterwards
	EntityAI SpawnEntity(string type, EntityAI parent, vector position, float quantity, float health, float temp, int itemState, JMPlayerInstance callerInstance)
	{
		int flags;

		bool inInventory;

		if (parent && IsInventoryType(type))
		{
			flags = ECE_LOCAL | ECE_IN_INVENTORY;
			inInventory = true;
		}
		else
		{
			if (COT_SurfaceIsWater(position))
				flags = ECE_OBJECT_SWAP;  //! Keep height, no surface align
			else
				flags = ECE_PLACE_ON_SURFACE;

			if (g_Game.IsKindOf(type, "DZ_LightAI"))
				flags |= ECE_INITAI;
		}

		if (m_ObjSetupMode == COT_ObjectSetupMode.CE)
			flags |= ECE_EQUIP;

		Object obj = g_Game.CreateObjectEx(type, position, flags);

		if (!obj)
		{
			CF_Log.Error("JMObjectSpawnerModule::SpawnEntity failed - CreateObjectEx returned null for class '%1' (flags=%2)", type, flags.ToString());
			if ( callerInstance && callerInstance.PlayerObject )
				GetCommunityOnlineToolsBase().Log( callerInstance.PlayerObject.GetIdentity(), "Spawn FAILED for class '" + type + "' - class not found or not spawnable" );
			return null;
		}

		EntityAI ent;

		if (!Class.CastTo(ent, obj))
		{
			g_Game.ObjectDelete(obj);
			CF_Log.Error("JMObjectSpawnerModule::SpawnEntity failed - '%1' is not an EntityAI", type);
			return null;
		}

		if (inInventory)
		{
			//! Move to parent inventory
			InventoryLocation src = new InventoryLocation();
			InventoryLocation dst = new InventoryLocation();

			int locationType = FindInventoryLocationType.CARGO | FindInventoryLocationType.ATTACHMENT | FindInventoryLocationType.HANDS;

			bool srcValid = ent.GetInventory().GetCurrentInventoryLocation(src);
			bool dstValid = parent.GetInventory().FindFreeLocationFor(ent, locationType, dst);

			if (srcValid && dstValid && !GameInventory.LocationSyncMoveEntity(src, dst))
				CF_Log.Warn("Couldn't move %1 to %2", ent.ToString(), parent.ToString());
		}

		parent = ent.GetHierarchyParent();

		if (!parent)
		{
			//! If no parent or move to parent inventory failed, update pathgraph (navmesh) if neccessary
			if (obj.CanAffectPathgraph() && (flags & ECE_UPDATEPATHGRAPH) == 0)
				g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(g_Game.UpdatePathgraphRegionByObject, 100, false, obj);
		}

		SetupEntity(ent, quantity, health, temp, itemState, callerInstance.PlayerObject, m_ObjSetupMode);

		//! Ground spawns only: an item that went into an inventory has no world
		//! position to restore it to, so undo would put it somewhere it never was.
		if (!parent)
			JMActionHistory.Push( new JMSpawnHistoryEntry( ent ), JMActionHistory.OwnerOfInstance( callerInstance ) );

		if ((flags & ECE_LOCAL) == ECE_LOCAL)
		{
			g_Game.RemoteObjectTreeDelete(obj);  //! Needed since SetupEntity might have spawned atts, and those wouldn't be created by RemoteObjectTreeCreate if they weren't spawned only locally...
			g_Game.RemoteObjectTreeCreate(obj);
		}

		string loggedSuffix;

		if (parent)
		{
			PlayerBase owner;
			if (Class.CastTo(owner, parent))
				loggedSuffix = " on " + owner.FormatSteamWebhook();
			else
				loggedSuffix = " on " + parent.ToString();
		}

		loggedSuffix += " at " + position.ToString();

		string tmp = "Spawned Entity \"%1\" (%2, q=%3, h=%4, t=%5, s=%6)%7";
		string msg = string.Format(tmp, ent.GetDisplayName(), ent.GetDebugName(), quantity, health, temp, itemState, loggedSuffix);

		GetCommunityOnlineToolsBase().Log(callerInstance.PlayerObject.GetIdentity(), msg);
		if (parent)
			SendWebhookColored( "Player", callerInstance, msg, JMConstants.WEBHOOK_COLOR_SPAWN );
		else
			SendWebhookColored( "Vector", callerInstance, msg, JMConstants.WEBHOOK_COLOR_SPAWN );

		return ent;
	}

	//! Apply the attachment half of a setup mode - and nothing else. Kept apart
	//! from SetupEntity so the spawner form can run the exact same pass over the
	//! local entity behind its item preview: the preview then shows the
	//! attachments and magazine the real spawn would produce, instead of a bare
	//! model that never matches what lands in the world.
	//!
	//! Quantity / health / temperature deliberately stay in SetupEntity - the
	//! preview drives those from its own sliders.
	void SetupEntityForMode( EntityAI entity, PlayerBase player, COT_ObjectSetupMode mode )
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
				Weapon_Base weapon;
				if (Class.CastTo(weapon, entity))
				{
					int muzzleIndex = weapon.GetCurrentMuzzle();
					if (!weapon.IsChamberFull(muzzleIndex))
					{
						string ammoTypeName;
						float ammoDamage;
						if (weapon.HasInternalMagazine(muzzleIndex) && weapon.GetInternalMagazineCartridgeCount(muzzleIndex) > 0)
						{
							if (weapon.GetInternalMagazineCartridgeInfo(muzzleIndex, 0, ammoDamage, ammoTypeName))
								weapon.FillChamber(ammoTypeName);
						}
						else
						{
							Magazine attachedMag = weapon.GetMagazine(muzzleIndex);
							if (attachedMag && attachedMag.GetAmmoCount() > 0)
							{
								if (attachedMag.GetCartridgeAtIndex(0, ammoDamage, ammoTypeName))
									weapon.FillChamber(ammoTypeName);
							}
						}
					}
				}
				break;
		}
	}

	//! Pick a value out of a spawn range.
	//!
	//! @param high -1, or anything not above `low`, means the range is really a
	//! single value and `low` is it.
	protected float RollSpawnValue( float low, float high )
	{
		if ( low == -1 || high == -1 || high <= low )
			return low;

		return Math.RandomFloatInclusive( low, high );
	}

	protected void SetupEntity( EntityAI entity, float quantity, float health, float temp, int itemState, PlayerBase player, COT_ObjectSetupMode mode = COT_ObjectSetupMode.NONE )
	{
		//! Rolled here rather than at the call site: this runs once per spawned
		//! entity, so every one of them gets its own value.
		quantity = RollSpawnValue( quantity, m_SpawnQuantityMax );
		health   = RollSpawnValue( health, m_SpawnHealthMax );

		SetupEntityForMode( entity, player, mode );

		ItemBase item;
		if ( Class.CastTo( item, entity ) )
		{
			Magazine mag;
			Class.CastTo(mag, item);

			if (quantity != -1)
			{
				if (mag)
					mag.ServerSetAmmoCount(quantity);
				else if (item.HasQuantity())
				{
					item.SetQuantity(quantity);

					if (item.GetCompEM())
						item.GetCompEM().SetEnergy0To1(quantity / item.GetQuantityMax());
				}
			}

			if (mag && m_SpawnAmmoType != "")
				FillMagazine(mag, m_SpawnAmmoType, quantity);

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
				else if ( item.CanBeDisinfected() )
				{
					// itemState is already "whatever state this kind of item
					// has": a liquid type for a container, a cooking stage for
					// food. For a rag, a bandage or a sewing kit it is the
					// cleanness flag, and the three are mutually exclusive -
					// nothing that can be disinfected is also food or a liquid
					// container.
					//
					// allow_client stays false: this runs on the server, and the
					// value reaches the client through the item's own sync.
					item.SetCleanness( itemState );
				}
			}
		}

		SetupEntityHealth( entity, health, temp );
	}

	//! Refill a magazine with one specific kind of cartridge.
	//!
	//! A magazine's ammoItems config lists AMMO PILE classes (Ammo_762x39 and
	//! friends); what a cartridge inside the magazine is identified by is the
	//! CfgAmmo class those piles name in their own "ammo" entry. There is no
	//! script call that takes the pile, so the pile is resolved to its cartridge
	//! here and the rounds are stored one at a time - which is the only way to
	//! choose what a magazine is loaded with.
	//!
	//! @param count -1 means "fill it".
	protected void FillMagazine(Magazine mag, string ammoPile, float count)
	{
		string cartridge;

		if (!g_Game.ConfigGetText(CFG_MAGAZINESPATH + " " + ammoPile + " ammo", cartridge) || cartridge == "")
			return;

		int wanted = mag.GetAmmoMax();

		if (count != -1)
			wanted = count;

		if (wanted > mag.GetAmmoMax())
			wanted = mag.GetAmmoMax();

		mag.ServerSetAmmoCount(0);

		for (int i = 0; i < wanted; i++)
			mag.ServerStoreCartridge(0, cartridge);

		mag.SetSynchDirty();
	}

	//! Push a global health value onto an entity, its damage zones, and
	//! everything hanging off it - attachments take the same PERCENTAGE of their
	//! own max health rather than the raw number, so a 50% rifle carries a 50%
	//! optic and not an optic pinned to the rifle's HP value.
	//!
	//! Recurses, so an attachment's own attachments follow too.
	//!
	//! Split out of SetupEntity so the spawner form can run the identical pass
	//! over its local preview entity: the preview spawns the same attachments
	//! the real spawn would, and they have to answer to the health slider the
	//! same way.
	//!
	//! @param health -1 means "full". @param temp -1 means "leave alone".
	void SetupEntityHealth( EntityAI entity, float health, float temp )
	{
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
						SetupEntityHealth(attachment, attachmentHealth, temp);
					}
				}
			}
		}

		if ( temp != -1 )
			entity.SetTemperatureEx(new TemperatureData(temp));
	}

	void OnDebugSpawn(EntityAI entity, PlayerBase player, int depth = 3) 
	{
		ItemBase item;
		CarScript car;
		BoatScript boat;
#ifndef DAYZ_1_29
		MotorbikeScript bike;
#endif
		BuildingBase building;
		DayZPlayerImplement npc;

		if (depth > 0)
			m_OverrideDebugSpawnDepth = depth;

		if (Class.CastTo(item, entity))
			item.COT_OnDebugSpawn(player);
		else if (Class.CastTo(car, entity))
			car.COT_OnDebugSpawn(player);
		else if (Class.CastTo(boat, entity))
			boat.COT_OnDebugSpawn(player);
#ifndef DAYZ_1_29
		//! CarScript/BoatScript both get COT_OnDebugSpawn (refuel, etc) via
		//! their own modded-class override - Motorbike never got the same
		//! branch here despite MotorbikeScript.c already defining its own
		//! COT_OnDebugSpawn (refuel via COT_Refuel), so a spawned bike fell
		//! through to nothing and came out with an empty tank.
		else if (Class.CastTo(bike, entity))
			bike.COT_OnDebugSpawn(player);
#endif
		else if (Class.CastTo(building, entity))
			building.COT_OnDebugSpawn(player);
		else if (Class.CastTo(npc, entity))
			npc.COT_OnDebugSpawn(player);

		if (m_OverrideDebugSpawnDepth > 0)
			m_OverrideDebugSpawnDepth = 0;

		if (!entity.GetInventory())
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
		if (m_OverrideDebugSpawnDepth > 0)
		{
			depth = m_OverrideDebugSpawnDepth;
			m_OverrideDebugSpawnDepth = 0;
		}

		CF_Log.Debug("JMObjectSpawnerModule::SpawnCompatibleAttachments %1 %2 %3", entity.ToString(), player.ToString(), depth.ToString());

		GameInventory inventory = entity.GetInventory();
		int count = inventory.GetAttachmentSlotsCount();

		if (count == 0)
			return;

		int i;

		TIntArray slot_ids = {};
		int slot_id;

		for (i = 0; i < count; ++i)
		{
			slot_id = inventory.GetAttachmentSlotId(i);
			if (slot_id != InventorySlots.INVALID && InventorySlots.GetShowForSlotId(slot_id))
			{
				string att = InventorySlots.GetSlotName(slot_id);
				CF_Log.Info("Entity %1 has visible attachment slot %2 (ID %3)", entity.GetType(), att, slot_id.ToString());
				slot_ids.Insert(slot_id);
			}
		}

		if (slot_ids.Count() == 0)
			return; 

		TStringArray all_paths = new TStringArray;

		all_paths.Insert(CFG_VEHICLESPATH);
		all_paths.Insert(CFG_WEAPONSPATH);

		string child_name;
		int scope;
		string path;
		string model;
		int idx;
		EntityAI child;
		TStringSet seen = new TStringSet;

		foreach (string config_path: all_paths)
		{
			int children_count = g_Game.ConfigGetChildrenCount(config_path);

			for (i = 0; i < children_count; i++)
			{
				g_Game.ConfigGetChildName(config_path, i, child_name);
				path = config_path + " " + child_name;
				scope = g_Game.ConfigGetInt(path + " scope");

				if (scope == 2)
				{
					if (!g_Game.ConfigGetText(path + " model", model) || model == string.Empty || model == "bmp")
						continue;

					//! Don't spawn food items as attachments, that's silly, even more so since it would likely be candycanes
					//! We don't check for Edible_Base because we still want liquid containers like bottles/canteens
					//! (they inherit from Bottle_Base which inherits from Edible_Base).
					//! Checking nutrition is faster than traversing parent hierarchy anyway and only food/drink items will have it
					if (g_Game.ConfigIsExisting(path + " Nutrition"))
						continue;

					TStringArray inv_slots = {};
					g_Game.ConfigGetTextArray(path + " inventorySlot", inv_slots);

					foreach (string inv_slot: inv_slots)
					{
						slot_id = InventorySlots.GetSlotIdFromString(inv_slot);
						if (slot_id != InventorySlots.INVALID)
						{
							idx = slot_ids.Find(slot_id);
							if (idx > -1)
							{
								child_name.ToLower();
								if (IsExcludedClassName(child_name))
									break;

								if (seen.Find(child_name) > -1)
									break;

								//! Limit character attachments to clothing
								if (entity.IsMan() && !g_Game.IsKindOf(child_name, "Clothing_Base"))
									break;

								child = entity.GetInventory().CreateAttachmentEx(child_name, slot_id);
								if (child)
								{
									CF_Log.Info("Successfully spawned %1 in slot %2 on %3", child_name, inv_slot, entity.GetType());
									slot_ids.Remove(idx);

									if (depth > 0)
										OnDebugSpawn(child, player, depth - 1);

									if (!entity.IsTransport())
										seen.Insert(child_name);

									if (child_name == "compass")
										seen.Insert("orienteeringcompass");

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

	//! Plain Contains() lets a color token match as a coincidental substring
	//! of an unrelated word (e.g. "tan" inside some classname that has
	//! nothing to do with color) - that false match then poisons the whole
	//! base-name bucket in the variant index, so real DayZ vehicles ended up
	//! with "available" colors that don't actually exist for them. DayZ's
	//! real color-variant naming is always an underscore-delimited suffix
	//! (e.g. "Hatchback_02_White"), so require the token to be bordered by
	//! "_" (or the string's start/end) on both sides.
	protected static int FindDelimitedTokenIndex(string lower, string token)
	{
		int tokenLen = token.Length();
		int lowerLen = lower.Length();
		int searchFrom = 0;

		while (searchFrom <= lowerLen - tokenLen)
		{
			int idx = lower.IndexOfFrom(searchFrom, token);
			if (idx == -1)
				return -1;

			bool leftOk = (idx == 0) || (lower.Substring(idx - 1, 1) == "_");
			int afterIdx = idx + tokenLen;
			bool rightOk = (afterIdx == lowerLen) || (lower.Substring(afterIdx, 1) == "_");

			if (leftOk && rightOk)
				return idx;

			searchFrom = idx + 1;
		}

		return -1;
	}

	//! Strip a delimited token match plus exactly one bordering "_" (the
	//! trailing one if there is one, else the leading one) so that e.g.
	//! "hatchback_02_white" and "hatchback_02_blue" normalize to the same
	//! "hatchback_02" base regardless of which color they started as.
	protected static string StripDelimitedToken(string lower, int idx, int tokenLen)
	{
		int lowerLen = lower.Length();
		int removeStart = idx;
		int removeEnd = idx + tokenLen;

		if (removeEnd < lowerLen && lower.Substring(removeEnd, 1) == "_")
			removeEnd = removeEnd + 1;
		else if (removeStart > 0 && lower.Substring(removeStart - 1, 1) == "_")
			removeStart = removeStart - 1;

		string before = lower.Substring(0, removeStart);
		string after = lower.Substring(removeEnd, lowerLen - removeEnd);
		return before + after;
	}

	string DetectColorToken(EntityAI entity)
	{
		if (!entity)
			return "";

		ref array<string> colorTokens = GetColorTokens();

		string typeStr = entity.GetType();
		typeStr.ToLower();

		int i;
		for (i = 0; i < colorTokens.Count(); ++i)
		{
			if (HasDelimitedToken(typeStr, colorTokens[i]))
				return colorTokens[i];
		}

		if (entity.GetInventory())
		{
			int attCount = entity.GetInventory().AttachmentCount();
			for (int j = 0; j < attCount; ++j)
			{
				EntityAI att = entity.GetInventory().GetAttachmentFromIndex(j);
				if (att)
				{
					string attType = att.GetType();
					attType.ToLower();
					for (i = 0; i < colorTokens.Count(); ++i)
					{
						if (HasDelimitedToken(attType, colorTokens[i]))
							return colorTokens[i];
					}
				}
			}
		}

		return "";
	}

	void SpawnCompatibleAttachmentsWithColor(EntityAI entity, PlayerBase player, int depth = 3, string preferredColor = "")
	{
		if (!entity)
			return;

		if (preferredColor == "")
			preferredColor = DetectColorToken(entity);

		GameInventory inventory = entity.GetInventory();
		if (!inventory)
			return;

		int count = inventory.GetAttachmentSlotsCount();
		if (count == 0)
			return;

		ref array<int> slot_ids = new array<int>;
		int slot_id;
		int i;

		for (i = 0; i < count; ++i)
		{
			slot_id = inventory.GetAttachmentSlotId(i);
			if (slot_id != InventorySlots.INVALID && InventorySlots.GetShowForSlotId(slot_id))
			{
				if (!inventory.FindAttachment(slot_id))
				{
					slot_ids.Insert(slot_id);
				}
			}
		}

		if (slot_ids.Count() == 0)
			return;

		TStringArray all_paths = new TStringArray;
		all_paths.Insert(CFG_VEHICLESPATH);
		all_paths.Insert(CFG_WEAPONSPATH);

		map<int, ref TStringArray> slotCandidates = new map<int, ref TStringArray>;
		for (i = 0; i < slot_ids.Count(); ++i)
		{
			slotCandidates.Insert(slot_ids[i], new TStringArray);
		}

		string child_name;
		int scope;
		string path;
		string model;

		foreach (string config_path: all_paths)
		{
			int children_count = g_Game.ConfigGetChildrenCount(config_path);

			for (i = 0; i < children_count; i++)
			{
				g_Game.ConfigGetChildName(config_path, i, child_name);
				path = config_path + " " + child_name;
				scope = g_Game.ConfigGetInt(path + " scope");

				if (scope == 2)
				{
					if (!g_Game.ConfigGetText(path + " model", model) || model == string.Empty || model == "bmp")
						continue;

					if (g_Game.ConfigIsExisting(path + " Nutrition"))
						continue;

					ref array<string> inv_slots = new array<string>;
					g_Game.ConfigGetTextArray(path + " inventorySlot", inv_slots);

					foreach (string inv_slot: inv_slots)
					{
						slot_id = InventorySlots.GetSlotIdFromString(inv_slot);
						if (slot_id != InventorySlots.INVALID && slotCandidates.Contains(slot_id))
						{
							string lowerChild = child_name;
							lowerChild.ToLower();
							if (IsExcludedClassName(lowerChild))
								continue;

							if (entity.IsMan() && !g_Game.IsKindOf(lowerChild, "Clothing_Base"))
								continue;

							TStringArray candList = slotCandidates.Get(slot_id);
							if (candList.Find(child_name) == -1)
								candList.Insert(child_name);
						}
					}
				}
			}
		}

		for (i = 0; i < slot_ids.Count(); ++i)
		{
			int targetSlot = slot_ids[i];
			TStringArray candidates = slotCandidates.Get(targetSlot);
			if (!candidates || candidates.Count() == 0)
				continue;

			string bestMatch = "";
			if (preferredColor != "")
			{
				for (int c = 0; c < candidates.Count(); ++c)
				{
					string candLower = candidates[c];
					candLower.ToLower();
					if (HasDelimitedToken(candLower, preferredColor))
					{
						bestMatch = candidates[c];
						break;
					}
				}
			}

			if (bestMatch == "")
				bestMatch = candidates[0];

			EntityAI child = inventory.CreateAttachmentEx(bestMatch, targetSlot);
			if (child && depth > 0)
			{
				SpawnCompatibleAttachmentsWithColor(child, player, depth - 1, preferredColor);
			}
		}
	}

	protected static void EnsureColorVariantIndex()
	{
		if (s_ColorVariantsByBase)
			return;

		s_ColorVariantsByBase = new map<string, ref map<string, string>>;

		array<string> tokens = GetColorTokens();

		TStringArray all_paths = new TStringArray;
		all_paths.Insert(CFG_VEHICLESPATH);
		all_paths.Insert(CFG_WEAPONSPATH);

		string child_name;
		foreach (string config_path: all_paths)
		{
			int children_count = g_Game.ConfigGetChildrenCount(config_path);

			for (int i = 0; i < children_count; i++)
			{
				g_Game.ConfigGetChildName(config_path, i, child_name);

				string lower = child_name;
				lower.ToLower();

				for (int t = 0; t < tokens.Count(); ++t)
				{
					int tokenIdx = FindDelimitedTokenIndex(lower, tokens[t]);
					if (tokenIdx == -1)
						continue;

					string base = StripDelimitedToken(lower, tokenIdx, tokens[t].Length());

					map<string, string> variants = s_ColorVariantsByBase.Get(base);
					if (!variants)
					{
						variants = new map<string, string>;
						s_ColorVariantsByBase.Insert(base, variants);
					}

					//! First classname found for this (base, token) wins - good
					//! enough, since a mod defining two siblings with the same
					//! color token is not a case worth picking between.
					if (!variants.Contains(tokens[t]))
						variants.Insert(tokens[t], child_name);

					break;
				}
			}
		}
	}

	//! Find the classname that is `sourceClass` with its color token swapped
	//! for `newColor` - e.g. "CivilianSedan_Blue" -> "CivilianSedan_White".
	//! Empty string if there's no color token to swap or no such classname is
	//! configured.
	string FindColorVariant(string sourceClass, string newColor)
	{
		EnsureColorVariantIndex();

		string lowerSource = sourceClass;
		lowerSource.ToLower();

		array<string> tokens = GetColorTokens();
		string oldToken = "";
		int oldTokenIdx = -1;
		for (int t = 0; t < tokens.Count(); ++t)
		{
			oldTokenIdx = FindDelimitedTokenIndex(lowerSource, tokens[t]);
			if (oldTokenIdx != -1)
			{
				oldToken = tokens[t];
				break;
			}
		}

		if (oldToken == "" || oldToken == newColor)
			return "";

		string base = StripDelimitedToken(lowerSource, oldTokenIdx, oldToken.Length());

		map<string, string> variants = s_ColorVariantsByBase.Get(base);
		if (!variants || !variants.Contains(newColor))
			return "";

		return variants.Get(newColor);
	}

	//! "Change Colors": snapshot `entity` (classname, health/quantity/liquid/
	//! temperature, and every attachment/cargo item recursively - the same
	//! capture JMLoadoutModule uses for its own deletion backups), swap the
	//! classname of the snapshot's root and every part in it for the
	//! `newColor` sibling wherever one exists, then respawn the whole tree
	//! from that snapshot. There is no in-place "retype" API in this engine,
	//! so this is delete-old/spawn-new - but going through the loadout
	//! snapshot instead of a bare CreateObjectEx is what keeps everything
	//! that was inside or attached to it intact across the swap.
	//!
	//! Returns the newly-spawned entity: the one passed in does not survive.
	EntityAI RecolorEntityAndAttachments(EntityAI entity, PlayerBase player, string newColor, int depth = 3)
	{
		if (!entity || newColor == "")
			return entity;

		JMLoadoutModule loadoutModule;
		if (!CF_Modules<JMLoadoutModule>.Get(loadoutModule))
			return entity;

		vector pos = entity.GetPosition();
		vector ori = entity.GetOrientation();

		JMLoadoutItem snapshot = loadoutModule.LoadoutProcessItem(entity, pos, ori);
		if (!snapshot)
			return entity;

		RecolorLoadoutItem(snapshot, newColor, depth);

		g_Game.ObjectDelete(entity);

		EntityAI recreated = loadoutModule.SpawnItem(snapshot, pos, false);

		//! Anything still empty after the recolored respawn gets filled the
		//! same way Repair & Fill Slots already does - e.g. a slot the
		//! original had nothing in, which a fresh spawn of this classname
		//! would otherwise fill in whatever color the config defaults to.
		if (recreated)
			SpawnCompatibleAttachmentsWithColor(recreated, player, depth, newColor);

		return recreated;
	}

	//! Mutate a captured snapshot's classname, and recursively every
	//! attachment/cargo entry's, to the `newColor` sibling wherever one
	//! exists - left as-is (contents intact) where there is none.
	protected void RecolorLoadoutItem(JMLoadoutItem item, string newColor, int depth)
	{
		if (!item)
			return;

		string variant = FindColorVariant(item.m_Classname, newColor);
		if (variant != "")
			item.m_Classname = variant;

		if (depth <= 0 || !item.m_Attachments)
			return;

		foreach (JMLoadoutSubItem sub: item.m_Attachments)
			RecolorLoadoutSubItem(sub, newColor, depth - 1);
	}

	protected void RecolorLoadoutSubItem(JMLoadoutSubItem item, string newColor, int depth)
	{
		if (!item)
			return;

		string variant = FindColorVariant(item.m_Classname, newColor);
		if (variant != "")
			item.m_Classname = variant;

		if (depth <= 0 || !item.m_Attachments)
			return;

		foreach (JMLoadoutSubItem sub: item.m_Attachments)
			RecolorLoadoutSubItem(sub, newColor, depth - 1);
	}

	void Command_Spawn(JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance)
	{
		string className;
		if (!params.Next(className))
			return;

		vector position;
		if (params.HasNext())
		{
			float x, y, z;

			if (!params.Next(x))
				return;

			if (!params.Next(y))
				return;

			if (!params.Next(z))
				return;

			position = Vector(x, y, z);
		}
		else
		{
			PlayerBase player = GetPlayerObjectByIdentity(sender);
			if (player) position = player.GetPosition();
		}

		float quantity = -1;
		float health = -1;
		float temp = -1;
		int itemState = -1;
		SpawnEntity(className, null, position, quantity, health, temp, itemState, instance);
	}

	override void GetSubCommands(inout array<ref JMCommand> commands)
	{
		AddSubCommand(commands, "spawn", "Command_Spawn", JMConstants.PERM_ENTITY_SPAWN_POSITION);
	}

	override array<string> GetCommandNames()
	{
		auto names = new array<string>();
		names.Insert("object");
		return names;
	}
}
