class JMLoadoutModule: JMRenderableModuleBase
{
	protected ref JMLoadoutMeta meta;

	void JMLoadoutModule()
	{
		GetPermissionsManager().RegisterPermission( "Loadouts.View" );
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
		types.Insert( "Vector" );
		types.Insert( "Player" );
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

		//GetCommunityOnlineToolsBase().Log( ident, "Item set " + file.Name + " spawned on " + position );
		//SendWebhook( "Vector", instance, "Spawned item set \"" + file.Name + "\" at " + position.ToString() );
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

		//GetCommunityOnlineToolsBase().Log( ident, "Item set " + file.Name + " spawned on " + position );
		//SendWebhook( "Vector", instance, "Spawned item set \"" + file.Name + "\" at " + position.ToString() );
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

			int id = Math.RandomIntInclusive(0, count);
			if (IsKindOfLoadout( file.m_Items[0].m_Classname, "SurvivorBase" ))
			{
				foreach(JMLoadoutSubItem itemSubData: file.m_Items[id].m_Attachments)
					SpawnInItem(itemSubData, pData.PlayerObject);
			}
			else
			{
				SpawnItem(file.m_Items[id], pData.PlayerObject);
			}

			//GetCommunityOnlineToolsBase().Log( ident, "Item set " + file.Name + " spawned on " + players[i].GetGUID() );
			//SendWebhook( "Player", instance, "Spawned item set \"" + file.Name + "\" on " + players[i].FormatSteamWebhook() );
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
	
	void SpawnTarget( string Loadout, vector pos )
	{
		EntityAI ent;
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
		
		//GetCommunityOnlineToolsBase().Log( ident, "Item set " + file.Name + " spawned on " + Target[i].GetGUID() );
		//SendWebhook( "Player", instance, "Spawned item set \"" + file.Name + "\" on " + Target[i].FormatSteamWebhook() );
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

		int id = Math.RandomIntInclusive(0, count);
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
			SpawnItem(itemData, position);
	}

	private EntityAI SpawnItem( JMLoadoutItem itemData, vector centerPos )
	{
		vector pos = centerPos - itemData.m_LocalPosition;

		bool isTrans;
		bool isBaseBuilding;

		if (GetGame().IsKindOf( itemData.m_Classname, "CarScript" ))
			isTrans = true;
		else if (GetGame().IsKindOf( itemData.m_Classname, "BoatScript" ))
			isTrans = true;
		else if (GetGame().IsKindOf( itemData.m_Classname, "BaseBuildingBase" ))
			isBaseBuilding = true;

		int flags = ECE_CREATEPHYSICS;
		if (isTrans)
		{
			if ( !COT_SurfaceIsWater( pos ) )
					flags |= ECE_PLACE_ON_SURFACE;
		}
		else
		{
			if ( GetGame().IsKindOf( itemData.m_Classname, "DZ_LightAI" ) )
				flags |= 0x800;
		}

		EntityAI ent;
		if ( !Class.CastTo( ent, GetGame().CreateObjectEx( itemData.m_Classname, pos, flags ) ) )
			return NULL;

		ent.SetOrientation(itemData.m_LocalRotation);

		if (isTrans)
			SetupVehicle(ent);
		else if (isBaseBuilding)
			SetupBaseBuilding(ent, itemData.m_ConstructionParts);

		SetupItem(ent, itemData.m_Data);

		foreach(JMLoadoutSubItem subItemData: itemData.m_Attachments)
			SpawnInItem(subItemData, ent);

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
			return NULL;

		SetupItem(ent, itemData.m_Data);

		foreach(JMLoadoutSubItem subItemData: itemData.m_Attachments)
			SpawnInItem(subItemData, ent);

		return ent;
	}

	private void SetupItem(EntityAI item, JMLoadoutItemData data)
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
	
	private void SetupBaseBuilding(EntityAI entity, TStringArray builtParts)
	{
		if (!builtParts)
			return;

		BaseBuildingBase bb;
		if (!Class.CastTo(bb, entity))
			return;

		Man p;
		#ifdef SERVER
		array<Man> players = new array<Man>;
		GetGame().GetWorld().GetPlayerList(players);
		if (players.Count())
			p = players[0];
		#endif
		
		array<ConstructionPart> parts = bb.GetConstruction().GetConstructionParts().GetValueArray();
		foreach (ConstructionPart part: parts)
		{
			string partName = part.GetPartName();
			foreach (string builtName: builtParts)
			{
				if (partName.Contains(builtName))
				{
					bb.OnPartBuiltServer(p, partName, AT_BUILD_PART);
				}
			}
		}
		
		bb.GetConstruction().UpdateVisuals();
	}
	
	private void SetupVehicle(EntityAI entity)
	{
		Car car;
		if (Class.CastTo(car, entity))
		{
			FillCar(car, CarFluid.FUEL);
			FillCar(car, CarFluid.OIL);
			FillCar(car, CarFluid.BRAKE);
			FillCar(car, CarFluid.COOLANT);
		}
	}

	private void FillCar(Car car, CarFluid fluid)
	{
		float cap = car.GetFluidCapacity( fluid );
		car.Fill( fluid, cap );
	}
};
