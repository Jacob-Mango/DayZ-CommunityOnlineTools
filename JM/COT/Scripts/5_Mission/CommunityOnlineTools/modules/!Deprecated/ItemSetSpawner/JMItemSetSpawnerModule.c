class JMItemSetSpawnerModule: JMRenderableModuleBase
{
	protected ref JMItemSetSettings settings;
	protected ref JMItemSetMeta meta;

	void JMItemSetSpawnerModule()
	{
		GetPermissionsManager().RegisterPermission( "Items.View" );
		GetPermissionsManager().RegisterPermission( "Items.CreateSet" );
		GetPermissionsManager().RegisterPermission( "Items.Spawn" );
	}

	override void EnableUpdate()
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Items.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleItem";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/itemsetspawner_form.layout";
	}

	override string GetTitle()
	{
		return "#STR_COT_ITEMSET_MODULE_NAME (Legacy)";
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
		return "Item Set Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Vector" );
		types.Insert( "Player" );
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

		if ( settings )
		{
			meta = JMItemSetMeta.DeriveFromSettings( settings );
		}
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		//! TODO: Only save if changed
		if ( !GetGame().IsClient() && settings )
			settings.Save();
	}

	override int GetRPCMin()
	{
		return JMItemSetSpawnerModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMItemSetSpawnerModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMItemSetSpawnerModuleRPC.Load:
			RPC_Load( ctx, sender, target );
			break;
		case JMItemSetSpawnerModuleRPC.SpawnPosition:
			RPC_SpawnPosition( ctx, sender, target );
			break;
		case JMItemSetSpawnerModuleRPC.SpawnPlayers:
			RPC_SpawnPlayers( ctx, sender, target );
			break;
		}
	}

	array< string > GetItemSets()
	{
		return meta.ItemSets;
	}
	
	void Load()
	{
		if ( GetGame().IsClient() )
		{
			if (meta)
				return;

			meta = JMItemSetMeta.Create();

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMItemSetSpawnerModuleRPC.Load, true, NULL );
		} else
		{
			settings = JMItemSetSettings.Load();

			meta = JMItemSetMeta.DeriveFromSettings( settings );

			OnSettingsUpdated();
		}
	}

	bool IsLoaded()
	{
		return meta != null;
	}

	private void Server_Load( PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Items.Spawn", ident, instance ) )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( JMItemSetMeta.DeriveFromSettings( settings ) );
		rpc.Send( NULL, JMItemSetSpawnerModuleRPC.Load, true, ident );
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

	void SpawnPosition( string itemSet, vector position )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( itemSet );
			rpc.Write( position );
			rpc.Send( NULL, JMItemSetSpawnerModuleRPC.SpawnPosition, true, NULL );
		} else
		{
			Server_SpawnPosition( itemSet, position, NULL );
		}
	}

	private void Server_SpawnPosition( string itemSet, vector position, PlayerIdentity ident )
	{
		JMItemSetSerialize file = settings.ItemSets.Get( itemSet );
		if ( !file )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Items.Spawn", ident, instance ) )
			return;

		SpawnItemSet( file, position );

		GetCommunityOnlineToolsBase().Log( ident, "Item set " + file.Name + " spawned on " + position );
		SendWebhook( "Vector", instance, "Spawned item set \"" + file.Name + "\" at " + position.ToString() );
	}

	private void RPC_SpawnPosition( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string itemSet;
			if ( !ctx.Read( itemSet ) )
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

			Server_SpawnPosition( itemSet, position, senderRPC );
		}
	}
	
	void SpawnPlayers( string itemSet, array< string > guids )
	{
		if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( itemSet );
			rpc.Write( guids );
			rpc.Send( NULL, JMItemSetSpawnerModuleRPC.SpawnPlayers, true, NULL );
		} else
		{
			Server_SpawnPlayers( itemSet, guids, NULL );
		}
	}

	private void Server_SpawnPlayers( string itemSet, array< string > guids, PlayerIdentity ident )
	{
		JMItemSetSerialize file = settings.ItemSets.Get( itemSet );
		if ( !file )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Items.Spawn", ident, instance ) )
			return;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers( guids );

		for ( int i = 0; i < players.Count(); i++ )
		{
			players[i].Update();
			
			SpawnItemSet( file, players[i].GetPosition() );
			
			GetCommunityOnlineToolsBase().Log( ident, "Item set " + file.Name + " spawned on " + players[i].GetGUID() );
			SendWebhook( "Player", instance, "Spawned item set \"" + file.Name + "\" on " + players[i].FormatSteamWebhook() );
		}
	}

	private void RPC_SpawnPlayers( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			string itemSet;
			if ( !ctx.Read( itemSet ) )
			{
				return;
			} 

			array< string > guids;
			if ( !ctx.Read( guids ) )
			{
				return;
			} 

			Server_SpawnPlayers( itemSet, guids, senderRPC );
		}
	}

	//TODO: Rotate item to try fit
	private bool DetermineWillFit( EntityAI fittingCargo, string ClassName )
	{
		TIntArray values = new TIntArray;
		GetGame().ConfigGetIntArray( "CfgVehicles " + ClassName + " itemSize", values );

		if ( values.Count() != 2 ) return false;

		int iwidth = values[0];
		int iheight = values[1];

		CargoBase cargo = fittingCargo.GetInventory().GetCargo();

		int cwidth = cargo.GetWidth();
		int cheight = cargo.GetHeight();

		if ( iwidth > cwidth )
			return false;
		if ( iheight > cheight )
			return false;

		return true;
	}

	private EntityAI SpawnItem( vector pos, string className )
	{
		/*
		vector position = Vector( pos[0], pos[1], pos[2] );

		position[0] = position[0] + ( Math.RandomFloatInclusive( -0.5, 0.5 ) * 10.0 );
		position[2] = position[2] + ( Math.RandomFloatInclusive( -0.5, 0.5 ) * 10.0 );

		position[1] = GetGame().SurfaceY( position[0], position[2] );
		*/

		int flags = ECE_CREATEPHYSICS;
		if ( GetGame().IsKindOf( className, "CarScript" ) && !COT_SurfaceIsWater( pos ) )
			flags |= ECE_PLACE_ON_SURFACE;
		else if ( GetGame().IsKindOf( className, "BoatScript" ) && !COT_SurfaceIsWater( pos ) )
			flags |= ECE_PLACE_ON_SURFACE; //! TODO: Check if its even needed
		
		if ( GetGame().IsKindOf( className, "DZ_LightAI" ) )
			flags |= 0x800;

		EntityAI ent;
		if ( !Class.CastTo( ent, GetGame().CreateObjectEx( className, pos, flags ) ) )
			return NULL;

		return ent;
	}

	private EntityAI SpawnItemInContainer( string container, vector position, EntityAI chest, string ClassName, float numStacks, float stackSize )
	{
		EntityAI item;

		for ( float i = 0; i < numStacks; i++ )
		{
			item = EntityAI.Cast( chest.GetInventory().CreateInInventory( ClassName ) );

			if ( !item )
			{
				if ( DetermineWillFit( chest, ClassName ) && container != "" && chest )
				{
					chest = SpawnItem( position, container );

					item = EntityAI.Cast( chest.GetInventory().CreateInInventory( ClassName ) );
				} else
				{
					item = SpawnItem( position, ClassName );
				}
			}
			
			ItemBase itemBs = ItemBase.Cast( item );

			if ( itemBs )
			{
				itemBs.SetQuantity( stackSize );
			}
		}

		return chest;
	}

	private void SpawnItemSet( JMItemSetSerialize file, vector position )
	{
		if ( file.Items.Count() == 0 )
		{
			return;
		}

		EntityAI chest;

		if ( file.ContainerClassName != "" )
		{
			chest = SpawnItem( position, file.ContainerClassName );
		}
	
		for ( int j = 0; j < file.Items.Count(); j++ )
		{
			chest = SpawnItemInContainer( file.ContainerClassName, position, chest, file.Items[j].ItemName, file.Items[j].NumberOfStacks, file.Items[j].StackSize );
		}
	}
};
