class PermissionsFramework
{
	protected ref array< Man > m_ServerPlayers;
	protected ref array< PlayerIdentity > m_ServerIdentities;

	protected bool m_bLoaded;

	void PermissionsFramework()
	{
		MakeDirectory( PERMISSION_FRAMEWORK_DIRECTORY );

		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			m_ServerPlayers = new ref array< Man >;
			m_ServerIdentities = new ref array< PlayerIdentity >;
		}

		m_bLoaded = false;

		GetRPCManager().AddRPC( "PermissionsFramework", "UpdatePlayers", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "PermissionsFramework", "RemovePlayer", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "PermissionsFramework", "UpdatePlayer", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "PermissionsFramework", "UpdatePlayerData", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "PermissionsFramework", "UpdateRole", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "PermissionsFramework", "SetClientPlayer", this, SingeplayerExecutionType.Client );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Read" );
		GetPermissionsManager().RegisterPermission( "Admin.Roles.Update" );
	}

	void ~PermissionsFramework()
	{
		Print("PermissionsFramework::~PermissionsFramework");

		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( this.ReloadPlayerList );

			delete m_ServerPlayers;
		}
	}
	
	void OnStart()
	{
		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			GetPermissionsManager().LoadRoles();
		}
	}

	void OnFinish()
	{
	}

	void OnLoaded()
	{
		if ( GetGame().IsClient() && GetGame().IsMultiplayer() )
		{
			GetRPCManager().SendRPC( "PermissionsFramework", "UpdatePlayers", new Param, true );
		}

		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.ReloadPlayerList, 1000, true );
		}
	}

	void Update( float timeslice )
	{
		if( !m_bLoaded && !GetDayZGame().IsLoading() )
		{
			m_bLoaded = true;
			OnLoaded();
		} else {
			OnUpdate( timeslice );
		}
	}

	void OnUpdate( float timeslice )
	{

	}

	private bool CheckIfExists( ref AuthPlayer auPlayer )
	{
		for ( int i = 0; i < m_ServerIdentities.Count(); i++ )
		{
			if ( auPlayer.GetGUID() == m_ServerIdentities[i].GetId() )
			{
				return true;
			}
		}

		return false;
	}

	void ReloadPlayerList()
	{
		GetGame().GetPlayers( m_ServerPlayers );

		GetGame().GetPlayerIndentities( m_ServerIdentities );

		for ( int i = 0; i < GetPermissionsManager().AuthPlayers.Count(); i++ )
		{
			ref AuthPlayer ap = GetPermissionsManager().AuthPlayers[i];

			if ( !CheckIfExists( ap ) )
			{
				ap.Save();

				GetRPCManager().SendRPC( "PermissionsFramework", "RemovePlayer", new Param1< ref PlayerData >( SerializePlayer( ap ) ), true );

				GetPermissionsManager().AuthPlayers.Remove( i );

				i = i - 1;
			}
			
		}

		for ( int j = 0; j < m_ServerPlayers.Count(); j++ )
		{
			Man man = m_ServerPlayers[j];
			PlayerBase player = PlayerBase.Cast( man );

			ref AuthPlayer auPlayer = GetPermissionsManager().GetPlayerByIdentity( man.GetIdentity() );

			if ( player )
			{
				player.SetAuthenticatedPlayer( auPlayer );
			}

			if ( auPlayer )
			{
				auPlayer.PlayerObject = player;
				auPlayer.IdentityPlayer = man.GetIdentity();

				auPlayer.UpdatePlayerData();
			}
		}

		m_ServerPlayers.Clear();
		m_ServerIdentities.Clear();
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
						GetRPCManager().SendRPC( "PermissionsFramework", "SetClientPlayer", new Param1< ref PlayerData >( GetPermissionsManager().AuthPlayers[i].Data ), false, sender );
					} else 
					{
						GetRPCManager().SendRPC( "PermissionsFramework", "UpdatePlayerData", new Param1< ref PlayerData >( GetPermissionsManager().AuthPlayers[i].Data ), false, sender );
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

				if ( sender && data.param1 == sender.GetPlainId() )
				{
					GetRPCManager().SendRPC( "PermissionsFramework", "SetClientPlayer", new Param1< ref PlayerData >( player.Data ), false, sender );
				} else 
				{
					GetRPCManager().SendRPC( "PermissionsFramework", "UpdatePlayerData", new Param1< ref PlayerData >( player.Data ), false, sender );
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
				
			GetRPCManager().SendRPC( "PermissionsFramework", "UpdateRole", new Param2< string, ref array< string > >( role.Name, role.SerializedData ), false );
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