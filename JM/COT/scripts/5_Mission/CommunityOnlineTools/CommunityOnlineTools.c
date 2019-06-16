class CommunityOnlineTools
{
	protected bool m_bLoaded;

	void CommunityOnlineTools()
	{
		Print("CommunityOnlineTools::CommunityOnlineTools");

		MakeDirectory( PERMISSION_FRAMEWORK_DIRECTORY );

		m_bLoaded = false;

		GetRPCManager().AddRPC( "COT", "UpdatePlayers", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT", "RemovePlayer", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT", "UpdatePlayer", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT", "UpdatePlayerData", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT", "UpdateRole", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT", "SetClientPlayer", this, SingeplayerExecutionType.Client );
		
		GetPermissionsManager().RegisterPermission( "Admin.Player.Read" );
		GetPermissionsManager().RegisterPermission( "Admin.Roles.Update" );
		
		NewModuleManager();
	}

	void RegisterModules()
	{
		GetModuleManager().RegisterModule( new DebugModule );
		GetModuleManager().RegisterModule( new ServerInformationModule );
		GetModuleManager().RegisterModule( new PlayerModule );
		GetModuleManager().RegisterModule( new ObjectModule );
		GetModuleManager().RegisterModule( new ESPModule );
		GetModuleManager().RegisterModule( new MapEditorModule );
		GetModuleManager().RegisterModule( new TeleportModule );
		GetModuleManager().RegisterModule( new CameraTool );
		GetModuleManager().RegisterModule( new ItemSetSpawnerModule );
		GetModuleManager().RegisterModule( new VehicleSpawnerModule );
		GetModuleManager().RegisterModule( new WeatherModule );
		GetModuleManager().RegisterModule( new COTMapModule );
	}

	void ~CommunityOnlineTools()
	{
		Print("CommunityOnlineTools::~CommunityOnlineTools");
	}
	
	void OnStart()
	{
		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			GetPermissionsManager().LoadRoles();
		}

		GetModuleManager().RegisterModules();

		RegisterModules();

		GetModuleManager().OnInit();
		
		GetModuleManager().ReloadSettings();

		GetModuleManager().OnMissionStart();
	}

	void OnFinish()
	{
		GetModuleManager().OnMissionFinish();

		NewModuleManager();
	}

	void OnLoaded()
	{
		if ( GetGame().IsClient() && GetGame().IsMultiplayer() )
		{
			GetRPCManager().SendRPC( "COT", "UpdatePlayers", new Param, true );
		}

		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.ReloadPlayerList, 1000, true );
		}

		GetModuleManager().OnMissionLoaded();
	}

	void OnUpdate( float timeslice )
	{
		if( !m_bLoaded && !GetDayZGame().IsLoading() )
		{
			m_bLoaded = true;
			OnLoaded();
		} else {
			GetModuleManager().OnUpdate( timeslice );
		}
	}

	void ReloadPlayerList()
	{
		array< ref AuthPlayer > toRemove = new array< ref AuthPlayer >;

		for ( int i = 0; i < GetPermissionsManager().AuthPlayers.Count(); i++ )
		{
			AuthPlayer ap = GetPermissionsManager().AuthPlayers[i];

			ap.UpdatePlayerData();

			if ( ap.IdentityPlayer == NULL )
			{
				toRemove.Insert( ap );
			}
		}

		for ( int j = 0; j < toRemove.Count(); j++ )
		{
			toRemove[j].Save();

			GetRPCManager().SendRPC( "COT", "RemovePlayer", new Param1< ref PlayerData >( SerializePlayer( toRemove[j] ) ), true );

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
					if ( sender && GetPermissionsManager().AuthPlayers[i].Data.SGUID == sender.GetPlainId() )
					{
						GetRPCManager().SendRPC( "COT", "SetClientPlayer", new Param1< ref PlayerData >( GetPermissionsManager().AuthPlayers[i].Data ), false, sender );
					} else 
					{
						GetRPCManager().SendRPC( "COT", "UpdatePlayerData", new Param1< ref PlayerData >( GetPermissionsManager().AuthPlayers[i].Data ), false, sender );
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
				ref Param1< ref PlayerData > data;
				if ( !ctx.Read( data ) )
					return;
				
				AuthPlayer player = GetPermissionsManager().GetPlayer( data.param1 );

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
				AuthPlayer player = GetPermissionsManager().GetPlayerByGUID( data.param1 );
				if ( !player )
					return;

				if ( data.param1 == sender.GetPlainId() )
				{
					GetRPCManager().SendRPC( "COT", "SetClientPlayer", new Param1< ref PlayerData >( player.Data ), false, sender );
				} else 
				{
					GetRPCManager().SendRPC( "COT", "UpdatePlayerData", new Param1< ref PlayerData >( player.Data ), false, sender );
				}
			}
		}

		if ( type == CallType.Client )
		{
			if ( GetGame().IsMultiplayer() )
			{
				ref Param1< ref PlayerData > cdata;
				if ( !ctx.Read( cdata ) )
					return;

				GetPermissionsManager().GetPlayer( cdata.param1 );
			}
		}
	}

	void SetClientPlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Client )
		{
			if ( GetGame().IsMultiplayer() )
			{
				ref Param1< ref PlayerData > data;
				if ( !ctx.Read( data ) )
					return;

				ClientAuthPlayer = GetPermissionsManager().GetPlayer( data.param1 );

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

		ref array< string > arr = new ref array< string >;
		arr.Copy( data.param2 );

		ref Role role = NULL;

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