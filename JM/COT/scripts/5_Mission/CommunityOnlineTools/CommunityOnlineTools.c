class CommunityOnlineTools: CommunityOnlineToolsBase
{
	void CommunityOnlineTools()
	{
		Print("CommunityOnlineTools::CommunityOnlineTools");

		GetRPCManager().AddRPC( "COT", "UpdatePlayers", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT", "RemovePlayer", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT", "UpdatePlayer", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT", "UpdatePlayerData", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT", "UpdateRole", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT", "SetClientPlayer", this, SingeplayerExecutionType.Client );
		
		GetPermissionsManager().RegisterPermission( "Admin.Player.Read" );
		GetPermissionsManager().RegisterPermission( "Admin.Roles.Update" );
	}

	void ~CommunityOnlineTools()
	{
		Print("CommunityOnlineTools::~CommunityOnlineTools");
	}

	override void RegisterModules()
	{
		super.RegisterModules();

		GetModuleManager().RegisterModule( new CommunityOnlineToolsModule );
		GetModuleManager().RegisterModule( new JMDebugModule );
		// GetModuleManager().RegisterModule( new JMServerInfoModule );
		GetModuleManager().RegisterModule( new JMSelectedModule );
		GetModuleManager().RegisterModule( new JMPlayerModule );
		GetModuleManager().RegisterModule( new JMObjectSpawnerModule );
		GetModuleManager().RegisterModule( new JMESPModule );
		GetModuleManager().RegisterModule( new JMMapEditorModule );
		GetModuleManager().RegisterModule( new JMTeleportModule );
		GetModuleManager().RegisterModule( new JMCameraModule );
		GetModuleManager().RegisterModule( new JMItemSetSpawnerModule );
		GetModuleManager().RegisterModule( new JMVehicleSpawnerModule );
		GetModuleManager().RegisterModule( new JMWeatherModule );
		GetModuleManager().RegisterModule( new JMMapModule );
	}
	
	override void OnStart()
	{
		super.OnStart();
	}

	override void OnFinish()
	{
		super.OnFinish();
	}

	override void OnLoaded()
	{
		if ( GetGame().IsClient() && GetGame().IsMultiplayer() )
		{
			GetRPCManager().SendRPC( "COT", "UpdatePlayers", new Param, true );
		}

		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.ReloadPlayerList, 1000, true );
		}

		super.OnLoaded();
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );
	}

	void ReloadPlayerList()
	{
		array< ref JMPlayerInstance > toRemove = new array< ref JMPlayerInstance >;

		for ( int i = 0; i < GetPermissionsManager().AuthPlayers.Count(); i++ )
		{
			JMPlayerInstance ap = GetPermissionsManager().AuthPlayers[i];
			
			if ( ap.IdentityPlayer == NULL )
			{
				toRemove.Insert( ap );
				continue;
			}

			ap.UpdatePlayerData();
		}

		for ( int j = 0; j < toRemove.Count(); j++ )
		{
			toRemove[j].Save();

			GetRPCManager().SendRPC( "COT", "RemovePlayer", new Param1< ref JMPlayerInformation >( SerializePlayer( toRemove[j] ) ), true );

			GetPermissionsManager().AuthPlayers.RemoveItem( toRemove[j] );
		}
	}

	void UpdatePlayers( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Server )
		{
			if ( GetGame().IsMultiplayer() )
			{
				for ( int i = 0; i < GetPermissionsManager().AuthPlayers.Count(); i++ )
				{
					if ( sender && GetPermissionsManager().AuthPlayers[i].IdentityPlayer == sender )
					{
						GetRPCManager().SendRPC( "COT", "SetClientPlayer", new Param2< ref JMPlayerInformation, PlayerIdentity >( GetPermissionsManager().AuthPlayers[i].Data, GetPermissionsManager().AuthPlayers[i].IdentityPlayer ), false, sender );
					} else 
					{
						GetRPCManager().SendRPC( "COT", "UpdatePlayerData", new Param2< ref JMPlayerInformation, PlayerIdentity >( GetPermissionsManager().AuthPlayers[i].Data, GetPermissionsManager().AuthPlayers[i].IdentityPlayer ), false, sender );
					}
				}
			}
		}
	}

	void RemovePlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Client )
		{
			if ( GetGame().IsMultiplayer() )
			{
				ref Param1< ref JMPlayerInformation > data;
				if ( !ctx.Read( data ) )
					return;
				
				JMPlayerInstance player = GetPermissionsManager().GetPlayer( data.param1 );

				RemoveSelectedPlayer( player );
				GetPermissionsManager().AuthPlayers.RemoveItem( player );
			}
		}
	}

	void UpdatePlayerData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Server )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Read", sender ) )
				return;

			ref Param1< string > data;
			if ( !ctx.Read( data ) )
				return;

			if ( GetGame().IsMultiplayer() )
			{
				JMPlayerInstance player = GetPermissionsManager().GetPlayerByGUID( data.param1 );
				if ( !player )
					return;

				if ( player.IdentityPlayer == sender )
				{
					GetRPCManager().SendRPC( "COT", "SetClientPlayer", new Param2< ref JMPlayerInformation, PlayerIdentity >( player.Data, player.IdentityPlayer ), false, sender );
				} else 
				{
					GetRPCManager().SendRPC( "COT", "UpdatePlayerData", new Param2< ref JMPlayerInformation, PlayerIdentity >( player.Data, player.IdentityPlayer ), false, sender );
				}
			}
		}

		if ( type == CallType.Client )
		{
			if ( GetGame().IsMultiplayer() )
			{
				ref Param2< ref JMPlayerInformation, PlayerIdentity > cdata;
				if ( !ctx.Read( cdata ) )
					return;

				GetPermissionsManager().GetPlayer( cdata.param1 ).IdentityPlayer = cdata.param2;
			}
		}
	}

	void SetClientPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Client )
		{
			if ( GetGame().IsMultiplayer() )
			{
				ref Param2< ref JMPlayerInformation, PlayerIdentity > data;
				if ( !ctx.Read( data ) )
					return;

				ClientAuthPlayer = GetPermissionsManager().GetPlayer( data.param1 );

				ClientAuthPlayer.IdentityPlayer = data.param2;

				// ClientAuthPlayer.Data.DebugPrint();

				GetModuleManager().OnClientPermissionsUpdated();
			}
		}
	}

	void UpdateRole( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		ref Param2< string, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		ref array< string > arr = new array< string >;
		arr.Copy( data.param2 );

		ref JMRole role = NULL;

		if ( type == CallType.Server )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Roles.Update", sender ) )
				return;

			GetPermissionsManager().RolesMap.Find( data.param1, role );

			if ( role )
			{
				role.ClearPermissions();

				role.SerializedData = arr;

				role.Deserialize();
			} else 
			{
				role = GetPermissionsManager().LoadRole( data.param1, arr );
			}

			role.Serialize();
				
			GetRPCManager().SendRPC( "COT", "UpdateRole", new Param2< string, ref array< string > >( role.Name, role.SerializedData ), false );
		}

		if ( type == CallType.Client )
		{
			GetPermissionsManager().RolesMap.Find( data.param1, role );

			if ( role )
			{
				role.ClearPermissions();

				role.SerializedData = arr;

				role.Deserialize();
			} else 
			{
				GetPermissionsManager().LoadRole( data.param1, arr );
			}
		}
	}
}

CommunityOnlineTools GetCommunityOnlineTools()
{
    return CommunityOnlineTools.Cast( g_cotBase );
}