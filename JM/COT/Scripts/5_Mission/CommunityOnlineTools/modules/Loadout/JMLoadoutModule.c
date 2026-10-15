class JMLoadoutModule: JMRenderableModuleBase
{
	protected ref JMLoadoutMeta meta;

	array< string > GetLoadouts()
	{
		return meta.Loadouts;
	}

	bool IsKindOfLoadout(string reciever, string giver)
	{
		if (g_Game.IsKindOf( reciever, giver ))
			return true;
		
		if (g_Game.IsKindOf( reciever, "SurvivorBase" ))
			if (g_Game.IsKindOf( giver, "SurvivorBase" ))
				return true;

		return false;
	}

	bool IsLoaded()
	{
		return meta != NULL;
	}

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "#STR_COT_LOADOUT_MODULE_NAME";
		info.Icon = "backpack";
		info.Layout = "JM/COT/GUI/layouts/loadout_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_ITEMS;
		info.ViewPermission = JMConstants.PERM_LOADOUTS_VIEW;
		info.InputToggle = "UACOTToggleLoadout";
		info.WebhookTitle = "Loadout Module";
		info.SetRPCRange( JMLoadoutModuleRPC.INVALID, JMLoadoutModuleRPC.COUNT );

		info.AddPermission( JMConstants.PERM_LOADOUTS_CREATE );
		info.AddPermission( JMConstants.PERM_LOADOUTS_BACKUP );
		info.AddPermission( JMConstants.PERM_LOADOUTS_LOAD );
		info.AddPermission( JMConstants.PERM_LOADOUTS_DELETE );
		info.AddPermission( JMConstants.PERM_LOADOUTS_SPAWN_CURSOR );
		info.AddPermission( JMConstants.PERM_LOADOUTS_SPAWN_TARGET );
		info.AddPermission( JMConstants.PERM_LOADOUTS_SPAWN_SELECTEDPLAYERS );

		info.AddWebhookType( "Create" );
		info.AddWebhookType( "Delete" );
		info.AddWebhookType( "Backup" );
		info.AddWebhookType( "Spawn" );
	}

	override void EnableUpdate()
	{
	}

	//! The delete permission and its webhook type come from the action.
	override void RegisterActions()
	{
		super.RegisterActions();

		DefineAction( JMLoadoutDelete );
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMLoadoutModuleRPC.Load:
			RPC_Load( ctx, sender, target );
			break;
		case JMLoadoutModuleRPC.Create:
			RPC_Create( ctx, sender, target );
			break;
		case JMLoadoutModuleRPC.SpawnCursor:
			RPC_SpawnCursor( ctx, sender, target );
			break;
		case JMLoadoutModuleRPC.SpawnTarget:
			RPC_SpawnTarget( ctx, sender, target );
			break;
		case JMLoadoutModuleRPC.SpawnPlayers:
			RPC_SpawnPlayers( ctx, sender, target );
			break;
		default:
			RunAction( sender, rpc_type, ctx );
			break;
		}
	}

	void Load()
	{
		if ( g_Game.IsClient() )
		{
			meta = JMLoadoutMeta.Create();

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMLoadoutModuleRPC.Load, true, NULL );
		}
		else
		{
			meta = JMLoadoutMeta.DeriveFromSettings( JMLoadoutSettings.GetFileNames() );

			OnSettingsUpdated();
		}
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if (g_Game.IsServer())
			Load();
	}

	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();

		if ( IsMissionHost() )
			meta = JMLoadoutMeta.DeriveFromSettings( JMLoadoutSettings.GetFileNames() );
	}

	protected void Exec_Load( PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.Has( JMConstants.PERM_LOADOUTS_LOAD, ident, instance ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( JMLoadoutMeta.DeriveFromSettings( JMLoadoutSettings.GetFileNames() ) );
		rpc.Send( NULL, JMLoadoutModuleRPC.Load, true, ident );
	}

	protected void RPC_Load( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( g_Game.IsDedicatedServer() )
		{
			if ( !senderRPC )
				return;
			Exec_Load( senderRPC );
		}
		else
		{
			if ( ctx.Read( meta ) )
			{
				OnSettingsUpdated();
			}
		}
	}

	void Delete( string Loadout )
	{
		JMLoadoutDelete action = new JMLoadoutDelete();
		action.Loadout = Loadout;
		SubmitAction( action );
	}

	void SpawnCursor( string Loadout, vector position )
	{
		if ( g_Game.IsClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( Loadout );
			rpc.Write( position );
			rpc.Send( NULL, JMLoadoutModuleRPC.SpawnCursor, true, NULL );
		} else
		{
			Exec_SpawnCursor( Loadout, position, NULL );
		}
	}

	protected void Exec_SpawnCursor( string Loadout, vector position, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_LOADOUTS_SPAWN_CURSOR, ident, instance ) )
			return;

	#ifdef DZ_Expansion_Core
		// Try to load as Expansion format first
		ExpansionPrefab expLoadout;
		if ( JMExpansionLoadoutFile.Read( Loadout, expLoadout ) )
		{
			SpawnExpansionLoadout( expLoadout, position );
			GetCommunityOnlineToolsBase().Log( ident, "Expansion loadout set " + Loadout + " spawned at " + position );
			SendWebhookColored( "Spawn", instance, "Spawned Expansion loadout set \"" + Loadout + "\" at " + position.ToString(), JMConstants.WEBHOOK_COLOR_SPAWN );
			return;
		}
	#endif

		// Fallback to legacy format
		JMLoadout file = JMLoadoutSettings.Load( Loadout );
		if ( !file )
			return;

		SpawnLoadout( file, position );

		GetCommunityOnlineToolsBase().Log( ident, "Loadout set " + file + " spawned at " + position);
		SendWebhookColored( "Spawn", instance, "Spawned Loadout set \"" + file + "\" at " + position.ToString(), JMConstants.WEBHOOK_COLOR_SPAWN );
	}

	protected void RPC_SpawnCursor( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string Loadout;
			if ( !ctx.Read( Loadout ) )
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

			if ( !COT.IsValidWorldPosition( position ) )
				return;

			Exec_SpawnCursor( Loadout, position, senderRPC );
		}
	}

	void SpawnPlayers( string Loadout, array< string > guids )
	{
		if ( g_Game.IsClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( Loadout );
			rpc.Write( guids );
			rpc.Send( NULL, JMLoadoutModuleRPC.SpawnPlayers, true, NULL );
		}
		else
		{
			Exec_SpawnPlayers( Loadout, guids, NULL );
		}
	}

	protected void Exec_SpawnPlayers( string Loadout, array< string > guids, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_LOADOUTS_SPAWN_SELECTEDPLAYERS, ident, instance ) )
			return;

	#ifdef DZ_Expansion_Core
		// Try to load as Expansion format first
		ExpansionPrefab expLoadout;
		if ( JMExpansionLoadoutFile.Read( Loadout, expLoadout ) )
		{
			array< JMPlayerInstance > expplayers = GetPermissionsManager().GetPlayers( guids );
			foreach ( JMPlayerInstance exppData: expplayers )
			{
				exppData.Update();

				//! Tell the anti-cheat this armful of items is ours, or
				//! the item-add-rate rule reads an admin loadout as a
				//! spawn cheat. Announced BEFORE the spawn: the
				//! inventory poll can land mid-loadout.
				JMAntiCheatSanction.Grant( exppData.GetGUID(), JMAntiCheatSanction.INVENTORY, JMAntiCheatSanction.SPAWN_GRACE_SECONDS );

				SpawnExpansionLoadout( expLoadout, exppData.PlayerObject );

				GetCommunityOnlineToolsBase().Log( ident, "Expansion loadout set " + Loadout + " spawned on " + exppData.GetGUID() );
				SendWebhookColored( "Spawn", instance, "Spawned Expansion loadout set \"" + Loadout + "\" on " + exppData.FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_SPAWN );
			}
			return;
		}
	#endif

		// Fallback to legacy format
		JMLoadout file = JMLoadoutSettings.Load( Loadout );
		if ( !file )
			return;

		int count = file.m_Items.Count();
		if ( count == 0 )
			return;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
		foreach ( JMPlayerInstance pData: players )
		{
			pData.Update();

			//! See above - a sanctioned spawn, declared before it happens.
			JMAntiCheatSanction.Grant( pData.GetGUID(), JMAntiCheatSanction.INVENTORY, JMAntiCheatSanction.SPAWN_GRACE_SECONDS );

			int id = Math.RandomInt( 0, count );
			if ( IsKindOfLoadout( file.m_Items[0].m_Classname, "SurvivorBase" ) )
			{
				SpawnAttachments(file.m_Items[id], pData.PlayerObject);
			}
			else
			{
				SpawnItem( file.m_Items[id], pData.PlayerObject );
			}

			GetCommunityOnlineToolsBase().Log( ident, "Loadout set " + file + " spawned on " + pData.GetGUID() );
			SendWebhookColored( "Spawn", instance, "Spawned Loadout set \"" + file + "\" on " + pData.FormatSteamWebhook(), JMConstants.WEBHOOK_COLOR_SPAWN );
		}
	}

	protected void RPC_SpawnPlayers( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string Loadout;
			if ( !ctx.Read( Loadout ) )
			{
				Error("Failed");
				return;
			} 

			array< string > guids;
			if ( !ctx.Read( guids ) )
			{
				Error("Failed");
				return;
			}

			if ( guids.Count() > JMConstants.RPC_MAX_GUIDS )
				return;

			Exec_SpawnPlayers( Loadout, guids, senderRPC );
		}
	}

	void SpawnTarget( string Loadout, EntityAI ent )
	{
		if ( g_Game.IsClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( Loadout );
			rpc.Write( ent );
			rpc.Send( NULL, JMLoadoutModuleRPC.SpawnTarget, true, NULL );
		}
		else
		{
			Exec_SpawnTarget( Loadout, ent, NULL );
		}
	}

	protected void Exec_SpawnTarget( string Loadout, EntityAI ent, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !JMPermissions.HasRPC( JMConstants.PERM_LOADOUTS_SPAWN_TARGET, ident, instance ) )
			return;

	#ifdef DZ_Expansion_Core
		// Try to load as Expansion format first
		ExpansionPrefab expLoadout;
		if ( JMExpansionLoadoutFile.Read( Loadout, expLoadout ) )
		{
			SpawnExpansionLoadout( expLoadout, ent );
			GetCommunityOnlineToolsBase().Log( ident, "Expansion loadout " + Loadout + " spawned on " + ent.GetType() );
			SendWebhookColored( "Spawn", instance, "Spawned Expansion loadout \"" + Loadout + "\" on " + ent.GetType(), JMConstants.WEBHOOK_COLOR_SPAWN );
			return;
		}
	#endif

		// Fallback to legacy format
		JMLoadout file = JMLoadoutSettings.Load( Loadout );
		if ( !file )
			return;

		SpawnLoadout( file, ent );

		GetCommunityOnlineToolsBase().Log( ident, "Loadout  " + Loadout + " spawned on " + ent.GetType() );
		SendWebhookColored( "Spawn", instance, "Spawned Loadout  \"" + Loadout + "\" on " + ent.GetType(), JMConstants.WEBHOOK_COLOR_SPAWN );
	}

	protected void RPC_SpawnTarget( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string Loadout;
			if ( !ctx.Read( Loadout ) )
			{
				Error("Failed");
				return;
			} 

			EntityAI ent;
			if ( !ctx.Read( ent ) )
			{
				Error("Failed");
				return;
			}

			Exec_SpawnTarget( Loadout, ent, senderRPC );
		}
	}

	protected void SpawnLoadout(JMLoadout file, EntityAI ent)
	{
		int count = file.m_Items.Count();
		if ( count == 0 )
			return;
		
		if ( !ent )
			return;

		int id = Math.RandomInt(0, count);
		if (IsKindOfLoadout( file.m_Items[0].m_Classname, ent.GetType() ))
		{
			SpawnAttachments(file.m_Items[id], ent);
		}
		else
		{
			SpawnItem(file.m_Items[id], ent);
		}
	}

	protected void SpawnLoadout( JMLoadout file, vector position )
	{
		if ( file.m_Items.Count() == 0 )
			return;

		foreach(JMLoadoutItem itemData: file.m_Items)
			SpawnItem(itemData, position, file.m_IsLocalPosition);
	}

	EntityAI SpawnItem( JMLoadoutItem itemData, vector centerPos, bool isLocalPos = true )
	{
		vector pos;
		if (isLocalPos)
			pos = centerPos - itemData.m_LocalPosition;
		else
			pos = itemData.m_LocalPosition;

		int flags = ECE_PLACE_ON_SURFACE;

		if ( g_Game.IsKindOf( itemData.m_Classname, "DZ_LightAI" ) )
			flags |= ECE_INITAI;

		Object obj = g_Game.CreateObjectEx( itemData.m_Classname, pos, flags );

		EntityAI ent;
		if ( !Class.CastTo( ent, obj ) )
		{
			if ( obj )
				g_Game.ObjectDelete( obj );

			return NULL;
		}

		//! Orientation has to be set first, because it is used by PlaceOnSurfaceAtPosition
		ent.SetOrientation(itemData.m_LocalRotation);

		if (!ent.IsInherited(PlayerBase))
			COT.PlaceOnSurfaceAtPosition(ent, pos);

		if ((ent.IsInherited(Transport)))
			SetupVehicle(ent);
		else if (ent.IsInherited(BaseBuildingBase))
			SetupBaseBuilding(ent, itemData.m_ConstructionParts);
		
		SetupItem(ent, itemData.m_Data);

		ItemBase itembs;
		if (Class.CastTo(itembs, ent))
		{
			// I hate it, but at least it fix most spawning 
			// issues from modded containers -LT
			bool wasOpen = itembs.IsOpen();
			itembs.Open();

			SpawnAttachments(itemData, ent);

			if (!wasOpen)
				itembs.Close();
		}
		else
		{
			SpawnAttachments(itemData, ent);
		}

		return ent;
	}

	EntityAI SpawnItem( JMLoadoutItem itemData, EntityAI parent )
	{
		EntityAI ent = parent.GetInventory().CreateInInventory(itemData.m_Classname);
		if (!ent)
			return NULL;

		SetupItem(ent, itemData.m_Data);

		SpawnAttachments(itemData, ent);

		return ent;
	}

	EntityAI SpawnInItem( JMLoadoutSubItem itemData, EntityAI parent )
	{
		EntityAI ent;

		Weapon_Base weapon;
		if ( Class.CastTo( weapon, parent ) && g_Game.IsKindOf(itemData.m_Classname, "Magazine_Base") )
		{
			//! Treat quantity as bullets to chamber in mag
			ent = weapon.COTSpawnAttachedMagazine(itemData.m_Classname, WeaponWithAmmoFlags.CHAMBER, (int)itemData.m_Data.m_Quantity);
		}
		else
		{
			ent = parent.GetInventory().CreateInInventory(itemData.m_Classname);
		}

		if (ent)
		{
			SetupItem(ent, itemData.m_Data);

			SpawnAttachments(itemData, ent);
		}

		return ent;
	}

	protected void SpawnAttachments(JMLoadoutItem_Base itemData, EntityAI ent)
	{
		foreach(JMLoadoutSubItem subItemData: itemData.m_Attachments)
			SpawnInItem(subItemData, ent);
	}

	protected void SetupItem(EntityAI ent, JMLoadoutItemData data)
	{
		if (data.m_Health != -1)
			ent.SetHealth(data.m_Health);

		Weapon_Base weapon;
		Magazine mag;
		ItemBase item;
		if ( Class.CastTo( weapon, ent ) )
		{
			if (weapon.HasInternalMagazine(-1))
			{
				//! Treat quantity as bullets to chamber in internal mag
				string ammoType = weapon.COTGetFirstChamberableAmmoTypeName(0);
				weapon.COTFillInnerMagazine(ammoType, WeaponWithAmmoFlags.CHAMBER, (int)data.m_Quantity);
			}
		}
		else if ( Class.CastTo( mag, ent ) )
		{
			if (data.m_Quantity >= 0)
				mag.ServerSetAmmoCount((int)Math.Min(data.m_Quantity, mag.GetAmmoMax()));
		}
		else if ( Class.CastTo( item, ent ) )
		{
			if (data.m_Quantity != -1)
				item.SetQuantity(data.m_Quantity);

			if (data.m_LiquidType != -1)
				item.SetLiquidType(data.m_LiquidType, true);
		}

		if (data.m_Temperature != 0)
			ent.SetTemperature(data.m_Temperature);
	}

	protected void SetupBaseBuilding(EntityAI ent, TStringArray builtParts)
	{
		if (!builtParts)
			return;

		BaseBuildingBase bb = BaseBuildingBase.Cast(ent);

		Man p;
		#ifdef SERVER
		array<Man> players = new array<Man>;
		g_Game.GetWorld().GetPlayerList(players);
		if (players.Count())
			p = players[0];
		#endif
		
		bb.GetConstruction().COT_BuildParts(builtParts, PlayerBase.Cast(p), false);
	}

	protected void SetupVehicle(EntityAI entity)
	{
		COT.Refuel(entity);
	}

	void Create(string name)
	{
		if ( !g_Game.IsMultiplayer() )
		{
			set< Object > objects = new set< Object >;
			set< ref JMSelectedObject > selectedObjs = JM_GetSelected().GetObjects();

			foreach ( JMSelectedObject selectedObj: selectedObjs )
			{
				if ( selectedObj.obj )
					objects.Insert( selectedObj.obj );
			}
			
			Exec_Create( name, objects, GetPermissionsManager().GetClientPlayer() );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		JM_GetSelected().SerializeObjects( rpc );
		rpc.Send( NULL, JMLoadoutModuleRPC.Create, true, NULL );
	}

	protected void RPC_Create( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !senderRPC )
			return;

		if ( !JMPermissions.HasRPC( JMConstants.PERM_LOADOUTS_CREATE, senderRPC, instance ) )
			return;
		
		string name;
		if ( !ctx.Read( name ) )
			return;

		set< Object > objects = new set< Object >;
		if ( !JM_GetSelected().DeserializeObjects( ctx, objects ) )
			return;
		
		Exec_Create( name, objects, instance );
	}

	protected void Exec_Create( string name, set< Object > objects, JMPlayerInstance instance )
	{
		array< ref JMLoadoutItem > loadouts = new array< ref JMLoadoutItem >;
		array< Object > props 				= new array< Object >;
		array< EntityAI > parents 			= new array< EntityAI >;
		vector avgPos;

		int count = objects.Count();
		for(int i=0; i < count; i++)
		{
			avgPos[0] = avgPos[0] + objects[i].GetPosition()[0];
			avgPos[2] = avgPos[2] + objects[i].GetPosition()[2];

			if (objects[i].GetPosition()[1] < avgPos[1] || avgPos[1] == 0)
				avgPos[1] = objects[i].GetPosition()[1];

			EntityAI parent;
			if (Class.CastTo(parent, objects[i]))
				parents.Insert(parent);
			else
				props.Insert(objects[i]);
		}

		if (count > 0)
		{
			avgPos[0] = avgPos[0] / count;
			avgPos[2] = avgPos[2] / count;

			foreach(EntityAI prnt: parents)
				loadouts.Insert(LoadoutProcessItem(prnt, avgPos - prnt.GetPosition(), prnt.GetOrientation()));
				
			foreach(Object obj: props)
				loadouts.Insert(LoadoutProcessObject(obj, avgPos - obj.GetPosition(), obj.GetOrientation()));

			JMLoadout loadout = new JMLoadout;
			loadout.m_Items = loadouts;
			loadout.m_IsLocalPosition = true;			
			
			JMLoadoutSettings.Save(loadout, name);

			GetCommunityOnlineToolsBase().Log( instance, "Created Loadout '"+name+"'" );
			SendWebhookColored( "Create", instance, "Created Loadout '"+name+"'", JMConstants.WEBHOOK_COLOR_SUCCESS );
		}
	}

	JMLoadoutItem LoadoutProcessObject(Object parent, vector pos = vector.Zero, vector rot = vector.Zero)
	{
		JMLoadoutItem item = new JMLoadoutItem(parent);
		item.m_LocalPosition = pos;
		item.m_LocalRotation = rot;

		return item;
	}

	JMLoadoutItem LoadoutProcessItem(EntityAI parent, vector pos = vector.Zero, vector rot = vector.Zero)
	{
		return LoadoutProcessObject(parent, pos, rot);
	}

	[Obsolete("Use new JMLoadoutSubItem(parent) instead")]
	JMLoadoutSubItem LoadoutProcessSubItem(EntityAI parent)
	{
		JMLoadoutSubItem item = new JMLoadoutSubItem(parent);

		return item;
	}

	void Exec_CreateDeletionBackup( set< Object > objects, JMPlayerInstance instance)
	{
		array< ref JMLoadoutItem > loadouts = new array< ref JMLoadoutItem >;
		array< Object > props = new array< Object >;
		array< EntityAI > parents = new array< EntityAI >;

		int count = objects.Count();
		for(int i=0; i < count; i++)
		{
			EntityAI parent;
			if (Class.CastTo(parent, objects[i]))
				parents.Insert(parent);
			else
				props.Insert(objects[i]);
		}

		if (count > 0)
		{
			foreach(EntityAI prnt: parents)
				loadouts.Insert(LoadoutProcessItem(prnt, prnt.GetPosition(), prnt.GetOrientation()));
				
			foreach(Object obj: props)
				loadouts.Insert(LoadoutProcessObject(obj, obj.GetPosition(), obj.GetOrientation()));

			JMLoadout loadout = new JMLoadout;
			loadout.m_Items = new array< ref JMLoadoutItem >;
			loadout.m_Items = loadouts;
			loadout.m_IsLocalPosition = false;
			
			string name = JMDate.Now().ToString( "YYYY-MM-DD_hh-mm-ss" );
			JMLoadoutSettings.SaveDeletion(loadout, name);

			GetCommunityOnlineToolsBase().Log( instance, "Created Deletion Backup '"+name+"'" );
			SendWebhookColored( "Backup", instance, "Created Deletion Backup '"+name+"'", JMConstants.WEBHOOK_COLOR_INFO );
		}
	}

	void Exec_CreateLoadoutSet( string name, set< Object > objects, JMPlayerInstance instance )
	{
		array< ref JMLoadoutItem > loadouts = new array< ref JMLoadoutItem >;
		array< Object > props = new array< Object >;
		array< EntityAI > parents = new array< EntityAI >;

		int count = objects.Count();
		for ( int i = 0; i < count; i++ )
		{
			EntityAI parent;
			if ( Class.CastTo( parent, objects[i] ) )
				parents.Insert( parent );
			else
				props.Insert( objects[i] );
		}

		if ( count > 0 )
		{
			foreach ( EntityAI prnt: parents )
				loadouts.Insert( LoadoutProcessItem( prnt, prnt.GetPosition(), prnt.GetOrientation() ) );

			foreach ( Object obj: props )
				loadouts.Insert( LoadoutProcessObject( obj, obj.GetPosition(), obj.GetOrientation() ) );

			JMLoadout loadout = new JMLoadout;
			loadout.m_Items = new array< ref JMLoadoutItem >;
			loadout.m_Items = loadouts;
			loadout.m_IsLocalPosition = false;

			JMLoadoutSettings.Save( loadout, name );

			GetCommunityOnlineToolsBase().Log( instance, "Created loadout set '" + name + "'" );
			SendWebhookColored( "Create", instance, "Created loadout set '" + name + "'", JMConstants.WEBHOOK_COLOR_INFO );

			Load();
		}
	}

#ifdef DZ_Expansion_Core
	protected void SpawnExpansionLoadout( ExpansionPrefab prefab, vector position )
	{
		if ( !prefab )
			return;

		prefab.Spawn( position );
	}

	protected void SpawnExpansionLoadout( ExpansionPrefab prefab, EntityAI target )
	{
		if ( !prefab || !target )
			return;

		prefab.SpawnOn( target );
	}
#endif
}
