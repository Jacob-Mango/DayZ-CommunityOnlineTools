class JMLoadoutModule: JMRenderableModuleBase
{
	protected ref JMLoadoutMeta meta;

	void JMLoadoutModule()
	{
		GetPermissionsManager().RegisterPermission( "Loadouts.View" );
		GetPermissionsManager().RegisterPermission( "Loadouts.Create" );
		GetPermissionsManager().RegisterPermission( "Loadouts.Backup" );
		GetPermissionsManager().RegisterPermission( "Loadouts.Load" );
		GetPermissionsManager().RegisterPermission( "Loadouts.Delete" );

		GetPermissionsManager().RegisterPermission( "Loadouts.Spawn.Cursor" );
		GetPermissionsManager().RegisterPermission( "Loadouts.Spawn.Target" );
		GetPermissionsManager().RegisterPermission( "Loadouts.Spawn.SelectedPlayers" );
	}

	override void EnableUpdate()
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Loadouts.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleLoadout";
	}

	override string GetLayoutRoot()
	{
		return "JM\\COT\\GUI\\layouts\\loadout_form.layout";
	}

	override string GetTitle()
	{
		return "#STR_COT_LOADOUT_MODULE_NAME";
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
		return "Loadout Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Create" );
		types.Insert( "Delete" );
		types.Insert( "Backup" );
		types.Insert( "Spawn" );
	}

	override int GetRPCMin()
	{
		return JMLoadoutModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMLoadoutModuleRPC.COUNT;
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
		case JMLoadoutModuleRPC.Delete:
			RPC_Delete( ctx, sender, target );
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
		}
	}
	
	void Load()
	{
		if ( GetGame().IsClient() )
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

		if (GetGame().IsServer())
			Load();
	}

	override void OnSettingsUpdated()
	{
		super.OnSettingsUpdated();

		if ( IsMissionHost() )
			meta = JMLoadoutMeta.DeriveFromSettings( JMLoadoutSettings.GetFileNames() );
	}

	bool IsLoaded()
	{
		return meta != NULL;
	}

	array< string > GetLoadouts()
	{
		return meta.Loadouts;
	}

	private void Server_Load( PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Loadouts.Load", ident, instance ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( JMLoadoutMeta.DeriveFromSettings( JMLoadoutSettings.GetFileNames() ) );
		rpc.Send( NULL, JMLoadoutModuleRPC.Load, true, ident );
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

	void Delete( string Loadout )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( Loadout );
			rpc.Send( NULL, JMLoadoutModuleRPC.Delete, true, NULL );
		}
		else
		{
			Server_Delete( Loadout, NULL );
		}
	}

	private void Server_Delete( string Loadout, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Loadouts.Delete", ident, instance ) )
			return;

		JMLoadoutSettings.Delete(Loadout);
		
		GetCommunityOnlineToolsBase().Log( ident, "Deleted '" + Loadout + "'" );
		SendWebhook( "Delete", instance, "Deleted '" + Loadout + "'" );
	}

	private void RPC_Delete( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string Loadout;
			if ( !ctx.Read( Loadout ) )
			{
				Error("Failed");
				return;
			}

			Server_Delete( Loadout, senderRPC );
		}
	}

	void SpawnCursor( string Loadout, vector position )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( Loadout );
			rpc.Write( position );
			rpc.Send( NULL, JMLoadoutModuleRPC.SpawnCursor, true, NULL );
		} else
		{
			Server_SpawnCursor( Loadout, position, NULL );
		}
	}

	private void Server_SpawnCursor( string Loadout, vector position, PlayerIdentity ident )
	{
		JMLoadout file = JMLoadoutSettings.Load( Loadout );
		if ( !file )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Loadouts.Spawn.Cursor", ident, instance ) )
			return;

		SpawnLoadout( file, position );

		GetCommunityOnlineToolsBase().Log( ident, "Loadout set " + file + " spawned at " + position);
		SendWebhook( "Spawn", instance, "Spawned Loadout set \"" + file + "\" at " + position.ToString() );
	}

	private void RPC_SpawnCursor( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

			Server_SpawnCursor( Loadout, position, senderRPC );
		}
	}
	
	void SpawnPlayers( string Loadout, array< string > guids )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( Loadout );
			rpc.Write( guids );
			rpc.Send( NULL, JMLoadoutModuleRPC.SpawnPlayers, true, NULL );
		}
		else
		{
			Server_SpawnPlayers( Loadout, guids, NULL );
		}
	}

	private void Server_SpawnPlayers( string Loadout, array< string > guids, PlayerIdentity ident )
	{
		JMLoadout file = JMLoadoutSettings.Load( Loadout );
		if ( !file )
			return;

		int count = file.m_Items.Count();
		if ( count == 0 )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Loadouts.Spawn.SelectedPlayers", ident, instance ) )
			return;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );
		foreach(JMPlayerInstance pData: players)
		{
			pData.Update();			

			int id = Math.RandomInt(0, count);
			if (IsKindOfLoadout( file.m_Items[0].m_Classname, "SurvivorBase" ))
			{
				foreach(JMLoadoutSubItem itemSubData: file.m_Items[id].m_Attachments)
					SpawnInItem(itemSubData, pData.PlayerObject);
			}
			else
			{
				SpawnItem(file.m_Items[id], pData.PlayerObject);
			}

			GetCommunityOnlineToolsBase().Log( ident, "Loadout set " + file + " spawned on " + pData.GetGUID() );
			SendWebhook( "Spawn", instance, "Spawned Loadout set \"" + file + "\" on " + pData.FormatSteamWebhook() );
		}
	}

	private void RPC_SpawnPlayers( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

			Server_SpawnPlayers( Loadout, guids, senderRPC );
		}
	}
	
	void SpawnTarget( string Loadout, EntityAI ent )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( Loadout );
			rpc.Write( ent );
			rpc.Send( NULL, JMLoadoutModuleRPC.SpawnTarget, true, NULL );
		}
		else
		{
			Server_SpawnTarget( Loadout, ent, NULL );
		}
	}

	private void Server_SpawnTarget( string Loadout, EntityAI ent, PlayerIdentity ident )
	{
		JMLoadout file = JMLoadoutSettings.Load( Loadout );
		if ( !file )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Loadouts.Spawn.Target", ident, instance ) )
			return;

		SpawnLoadout( file, ent );
		
		GetCommunityOnlineToolsBase().Log( ident, "Loadout  " + Loadout + " spawned on " + ent.GetType() );
		SendWebhook( "Spawn", instance, "Spawned Loadout  \"" + Loadout + "\" on " + ent.GetType() );
	}

	private void RPC_SpawnTarget( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

			Server_SpawnTarget( Loadout, ent, senderRPC );
		}
	}
	
	private void SpawnLoadout(JMLoadout file, EntityAI ent)
	{
		int count = file.m_Items.Count();
		if ( count == 0 )
			return;
		
		if ( !ent )
			return;

		int id = Math.RandomInt(0, count);
		if (IsKindOfLoadout( file.m_Items[0].m_Classname, ent.GetType() ))
		{
			foreach(JMLoadoutSubItem itemSubData: file.m_Items[id].m_Attachments)
				SpawnInItem(itemSubData, ent);
		}
		else
		{
			SpawnItem(file.m_Items[id], ent);
		}
	}

	bool IsKindOfLoadout(string reciever, string giver)
	{
		if (GetGame().IsKindOf( reciever, giver ))
			return true;
		
		if (GetGame().IsKindOf( reciever, "SurvivorBase" ))
			if (GetGame().IsKindOf( giver, "SurvivorBase" ))
				return true;

		return false;
	}

	private void SpawnLoadout( JMLoadout file, vector position )
	{
		if ( file.m_Items.Count() == 0 )
			return;

		foreach(JMLoadoutItem itemData: file.m_Items)
			SpawnItem(itemData, position, file.m_IsLocalPosition);
	}

	private EntityAI SpawnItem( JMLoadoutItem itemData, vector centerPos, bool isLocalPos = true )
	{
		vector pos;
		if (isLocalPos)
			pos = centerPos - itemData.m_LocalPosition;
		else
			pos = itemData.m_LocalPosition;

		EntityAI ent;
		if ( !Class.CastTo( ent, GetGame().CreateObject( itemData.m_Classname, pos ) ) )
			return NULL;

		//! Orientation has to be set first, because it is used by PlaceOnSurfaceAtPosition
		ent.SetOrientation(itemData.m_LocalRotation);

		if (!ent.IsInherited(PlayerBase))
			CommunityOnlineToolsBase.PlaceOnSurfaceAtPosition(ent, pos);

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

			foreach(JMLoadoutSubItem subItemData: itemData.m_Attachments)
				SpawnInItem(subItemData, ent);

			if (!wasOpen)
				itembs.Close();
		}
		else
		{
			foreach(JMLoadoutSubItem subItemData2: itemData.m_Attachments)
				SpawnInItem(subItemData2, ent);
		}

		return ent;
	}

	private EntityAI SpawnItem( JMLoadoutItem itemData, EntityAI parent )
	{
		EntityAI ent = parent.GetInventory().CreateInInventory(itemData.m_Classname);
		if (!ent)
			return NULL;

		SetupItem(ent, itemData.m_Data);

		foreach(JMLoadoutSubItem subItemData: itemData.m_Attachments)
			SpawnInItem(subItemData, ent);

		return ent;
	}

	private EntityAI SpawnInItem( JMLoadoutSubItem itemData, EntityAI parent )
	{
		EntityAI ent = parent.GetInventory().CreateInInventory(itemData.m_Classname);
		if (!ent)
		{
			Weapon_Base oWpn = Weapon_Base.Cast(parent);
			if ( oWpn )
			{
				if (GetGame().IsKindOf(itemData.m_Classname, "Magazine_Base"))
					oWpn.SpawnAmmo(itemData.m_Classname);
			}
			return NULL;
		}
		else
		{
			SetupItem(ent, itemData.m_Data);

			foreach(JMLoadoutSubItem subItemData: itemData.m_Attachments)
				SpawnInItem(subItemData, ent);
		}

		return ent;
	}

	private void SetupItem(EntityAI ent, JMLoadoutItemData data)
	{
		ItemBase item;
		if ( Class.CastTo( item, ent ) )
		{
			if (data.m_Health != -1)
				item.SetHealth(data.m_Health);

			if (data.m_Quantity != -1)
				item.SetQuantity(data.m_Quantity);

			if (data.m_LiquidType != -1)
				item.SetLiquidType(data.m_LiquidType, true);

			if (data.m_Temperature != 0)
				item.SetTemperature(data.m_Temperature);
		}
	}
	
	private void SetupBaseBuilding(EntityAI ent, TStringArray builtParts)
	{
		if (!builtParts)
			return;

		BaseBuildingBase bb = BaseBuildingBase.Cast(ent);

		Man p;
		#ifdef SERVER
		array<Man> players = new array<Man>;
		GetGame().GetWorld().GetPlayerList(players);
		if (players.Count())
			p = players[0];
		#endif
		
		bb.GetConstruction().COT_BuildParts(builtParts, PlayerBase.Cast(p), false);
	}
	
	private void SetupVehicle(EntityAI entity)
	{
		CommunityOnlineToolsBase.Refuel(entity);
	}
	
	EntityAI GetObjectAtCursor()
	{ 
		vector rayStart = GetGame().GetCurrentCameraPosition();
		DayZPlayer player = GetGame().GetPlayer();
		DayZPlayerCamera3rdPerson camera3rdPerson;
		float distance = 10;

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

		Object resultObj;
		TIntArray types = {ObjIntersectFire, ObjIntersectView};
		foreach (int type: types)
		{
			rayInput.type = type;

			if (!DayZPhysics.RaycastRVProxy(rayInput, results))
				continue;

			foreach (RaycastRVResult result: results)
			{
				resultObj = result.obj;

				if ( resultObj == NULL )
					continue;

				EntityAI entity;
				if (!Class.CastTo(entity, resultObj))
					continue;

				resultObj = entity.GetHierarchyRoot();
				string name = resultObj.GetType();

				if ( name == "" )
					continue;

				if (resultObj.ConfigGetInt("scope") != 2)
					continue;

				return entity;
			}
		}

		return NULL;
	}

	void Create(string name)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		JM_GetSelected().SerializeObjects( rpc );
		rpc.Send( NULL, JMLoadoutModuleRPC.Create, true, NULL );
	}

	private void RPC_Create( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Loadouts.Create", senderRPC, instance ) )
			return;
		
		string name;
		if ( !ctx.Read( name ) )
			return;

		set< Object > objects = new set< Object >;
		if ( !JM_GetSelected().DeserializeObjects( ctx, objects ) )
			return;
		
		Exec_Create( name, objects, instance );
	}

	private void Exec_Create( string name, set< Object > objects, JMPlayerInstance instance )
	{
		array< ref JMLoadoutItem > loadouts = new array< ref JMLoadoutItem >;
		array< Object > props 				= new array< Object >;
		array< EntityAI > parents 			= new array< EntityAI >;
		vector avgPos;

		int count = objects.Count();
		for(int i=0; i < count; i++)
		{
			if (count != 0)
			{
				avgPos[0] = avgPos[0] + objects[i].GetPosition()[0];
				avgPos[2] = avgPos[2] + objects[i].GetPosition()[2];

				if (objects[i].GetPosition()[1] < avgPos[1] || avgPos[1] == 0)
					avgPos[1] = objects[i].GetPosition()[1];
			}

			EntityAI parent;
			if (Class.CastTo(parent, objects[i]))
				parents.Insert(parent);
			else
				props.Insert(objects[i]);
		}

		if (count > -1)
		{
			if (count != 0)
			{
				avgPos[0] = avgPos[0] / count;
				avgPos[2] = avgPos[2] / count;
			}

			foreach(EntityAI prnt: parents)
				loadouts.Insert(LoadoutProcessItem(prnt, avgPos - prnt.GetPosition(), prnt.GetOrientation()));
				
			foreach(Object obj: props)
				loadouts.Insert(LoadoutProcessObject(parent, avgPos - parent.GetPosition(), parent.GetOrientation()));

			JMLoadout loadout = new JMLoadout;
			loadout.m_Items = new array< ref JMLoadoutItem >;
			loadout.m_Items = loadouts;
			loadout.m_IsLocalPosition = true;			
			
			JMLoadoutSettings.Save(loadout, name);

			GetCommunityOnlineToolsBase().Log( instance, "Created Loadout '"+name+"'" );
			SendWebhook( "Create", instance, "Created Loadout '"+name+"'" );
		}
	}

	JMLoadoutItem LoadoutProcessObject(Object parent, vector pos = vector.Zero, vector rot = vector.Zero)
	{
		JMLoadoutItem item = new JMLoadoutItem;

		item.m_Classname 	 = parent.GetType();
		item.m_LocalPosition = pos;
		item.m_LocalRotation = rot;

		return item;
	}

	JMLoadoutItem LoadoutProcessItem(EntityAI parent, vector pos = vector.Zero, vector rot = vector.Zero)
	{
		JMLoadoutItem item = new JMLoadoutItem;
		JMLoadoutItemData dataItem = new JMLoadoutItemData;

		item.m_Classname 		= parent.GetType();
		item.m_LocalPosition = pos;
		item.m_LocalRotation = rot;

		item.m_Data = dataItem;
		item.m_Attachments = new array< ref JMLoadoutSubItem >;

		// We check the partname instead of the part ID for better readability for the users
		// if its causing too much perf issues we might switch to the int ID instead
		BaseBuildingBase bb;
		if (Class.CastTo(bb, parent))
		{
			item.m_ConstructionParts = new TStringArray;
			array<ConstructionPart> parts = bb.GetConstruction().GetConstructionParts().GetValueArray();
			foreach (ConstructionPart part: parts)
			{
				if (part.m_IsBuilt)
				{
					item.m_ConstructionParts.Insert(part.GetPartName());
				}
			}
		}

		if (!parent.IsInherited(Building) && !parent.IsInherited(AdvancedCommunication))
			dataItem.m_Health 		= parent.GetHealth();

		if (parent.HasQuantity())
		{
			dataItem.m_Quantity 	= parent.GetQuantity();
			dataItem.m_LiquidType 	= parent.GetLiquidType();
		}
		
		dataItem.m_Temperature 	= parent.GetTemperature();

		ItemBase child;
		for (int k=0; k < parent.GetInventory().AttachmentCount(); k++)
		{
			child = ItemBase.Cast(parent.GetInventory().GetAttachmentFromIndex( k ));
			item.m_Attachments.Insert(LoadoutProcessSubItem(child));
		}

		CargoBase cargo = parent.GetInventory().GetCargo();
		if(cargo)
		{
			for(int j=0; j < cargo.GetItemCount(); j++)
			{
				child = ItemBase.Cast(cargo.GetItem(j));
				item.m_Attachments.Insert(LoadoutProcessSubItem(child));
			}
		}

		return item;
	}

	JMLoadoutSubItem LoadoutProcessSubItem(EntityAI parent)
	{
		JMLoadoutSubItem item = new JMLoadoutSubItem;
		JMLoadoutItemData dataItem = new JMLoadoutItemData;

		item.m_Classname 		= parent.GetType();

		ItemBase itembs;
		if ( Class.CastTo( itembs, parent ) )
		{
			if (!itembs.IsInherited(Building) && !itembs.IsInherited(AdvancedCommunication))
				dataItem.m_Health 		= itembs.GetHealth();

			if (itembs.HasQuantity())
			{
				dataItem.m_Quantity 	= itembs.GetQuantity();
				dataItem.m_LiquidType 	= itembs.GetLiquidType();
			}
			
			dataItem.m_Temperature 	= itembs.GetTemperature();
		}

		item.m_Data = dataItem;
		
		item.m_Attachments = new array< ref JMLoadoutSubItem >;

		EntityAI child;
		for (int k=0; k < parent.GetInventory().AttachmentCount(); k++)
		{
			child = EntityAI.Cast(parent.GetInventory().GetAttachmentFromIndex( k ));
			item.m_Attachments.Insert(LoadoutProcessSubItem(child));
		}

		CargoBase cargo = parent.GetInventory().GetCargo();
		if(cargo)
		{
			for(int j=0; j < cargo.GetItemCount(); j++)
			{
				child = EntityAI.Cast(cargo.GetItem(j));
				item.m_Attachments.Insert(LoadoutProcessSubItem(child));
			}
		}

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

		if (count > -1)
		{
			foreach(EntityAI prnt: parents)
				loadouts.Insert(LoadoutProcessItem(prnt, prnt.GetPosition(), prnt.GetOrientation()));
				
			foreach(Object obj: props)
				loadouts.Insert(LoadoutProcessObject(parent, parent.GetPosition(), parent.GetOrientation()));

			JMLoadout loadout = new JMLoadout;
			loadout.m_Items = new array< ref JMLoadoutItem >;
			loadout.m_Items = loadouts;
			loadout.m_IsLocalPosition = false;
			
			string name = JMDate.Now( true ).ToString( "YYYY-MM-DD_hh-mm-ss" );
			JMLoadoutSettings.SaveDeletion(loadout, name);

			GetCommunityOnlineToolsBase().Log( instance, "Created Deletion Backup '"+name+"'" );
			SendWebhook( "Backup", instance, "Created Deletion Backup '"+name+"'" );
		}
	}
};
