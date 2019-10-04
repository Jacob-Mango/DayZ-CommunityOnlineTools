class JMPermissionManager
{
	autoptr map< string, ref JMPlayerInstance > Players;
	autoptr map< string, ref JMRole > Roles;

	ref JMPermission RootPermission;

	private string m_ClientGUID;

	void JMPermissionManager()
	{
		Players = new map< string, ref JMPlayerInstance >;
		Roles = new map< string, ref JMRole >;

		RootPermission = new JMPermission( JMConstants.PERM_ROOT );
		
		if ( IsMissionHost() )
		{
			MakeDirectory( JMConstants.DIR_PF );

			MakeDirectory( JMConstants.DIR_PERMISSIONS );
			MakeDirectory( JMConstants.DIR_PLAYERS );
			MakeDirectory( JMConstants.DIR_ROLES );
		}

		if ( IsMissionOffline() )
		{
			CreateFakePlayers();
		}
	}

	string GetClientGUID()
	{
		return m_ClientGUID;
	}

	void SetClientGUID( string guid )
	{
		m_ClientGUID = guid;
	}

	JMPlayerInstance GetClientPlayer()
	{
		return Players.Get( m_ClientGUID );
	}

	bool HasRolePermission( string name, string permission, out JMPermissionType permTypeHasRolePermission = JMPermissionType.DISALLOW )
	{
		JMRole role;
		if ( Roles.Find( name, role ) )
		{
			return role.HasPermission( permission, permTypeHasRolePermission )
		}
		return false;
	}

	bool IsRole( string role )
	{
		return Roles.Contains( role );
	}

	void CreateFakePlayers()
	{
		for ( int i = 0; i < 200; i++ )
		{
			CreateFakePlayer( i );
		}
	}

	private void CreateFakePlayer( int i )
	{
		JMPlayerInstance instance = new JMPlayerInstance( NULL );
		instance.MakeFake( "GFake" + i, "SFake" + i, "NFake" + i )
		Players.Insert( instance.GetGUID(), instance );
	}

	/**
	 * This uses GUIDs now.
	 */
	array< JMPlayerInstance > GetPlayers( ref array< string > guidsGetPlayers = NULL )
	{
		if ( guidsGetPlayers == NULL || !GetGame().IsMultiplayer() )
		{
			return Players.GetValueArray();
		}

		array< JMPlayerInstance > players = new array< JMPlayerInstance >;

		for ( int i = 0; i < guidsGetPlayers.Count(); i++ )
		{
			JMPlayerInstance instance;
			if ( Players.Find( guidsGetPlayers[i], instance ) )
			{
				players.Insert( instance );
			}
		}

		return players;
	}

	void SortPlayersArray()
	{
		// for ( int k = 0; k < Players.Count(); k++ )
		// {
		// 	for ( int j = 0; j < Players.Count(); j++ )
		// 	{
		// 		if ( Players[k].GetName() < Players[j].GetName() )
		// 			Players.SwapItems( k, j );
		// 		else if ( Players[k].GetName() > Players[j].GetName() )
		// 			Players.SwapItems( j, k );
		// 	}
		// }
	}

	void RegisterPermission( string permission )
	{
		RootPermission.AddPermission( permission, JMPermissionType.INHERIT );
	}

	ref array< string > Serialize()
	{
		ref array< string > data = new array< string >;
		RootPermission.Serialize( data );
		return data;
	}

	JMPermission GetRootPermission()
	{
		return RootPermission;
	}

	bool HasPermission( string permission, PlayerIdentity identityHasPermission = NULL )
	{
		PMPrint();

		if ( IsMissionClient() ) 
		{
			if ( IsMissionHost() )
			{
				return true;
			}
			
			if ( GetPermissionsManager().GetClientPlayer() == NULL )
			{
				Print( "Client Player is NULL!" );
				return false;
			}

			return GetPermissionsManager().GetClientPlayer().HasPermission( permission );
		}
		
		if ( identityHasPermission == NULL )
		{
			return false;
		}

		JMPlayerInstance instance;
		if ( Players.Find( identityHasPermission.GetId(), instance ) )
		{
			return instance.HasPermission( permission );
		}

		return false;
	}

	bool OnClientConnected( PlayerIdentity identityOnClientConnected, out JMPlayerInstance instanceOnClientConnected )
	{
		string guid = "";
		
		if ( identityOnClientConnected == NULL )
		{
			if ( GetGame().IsMultiplayer() )
			{
				return false;
			}

			guid = JMConstants.OFFLINE_GUID;
		} else
		{
			guid = identityOnClientConnected.GetId();
		}

		if ( Players.Find( guid, instanceOnClientConnected ) )
		{
			return false;
		}

		instanceOnClientConnected = new JMPlayerInstance( identityOnClientConnected );

		instanceOnClientConnected.CopyPermissions( RootPermission );
		instanceOnClientConnected.Load();

		instanceOnClientConnected.Serialize();

		Players.Insert( guid, instanceOnClientConnected );

		// PMPrint();

		return true;
	}

	bool OnClientDisconnected( string guid, out JMPlayerInstance instanceOnClientDisconnected )
	{
		if ( Players.Find( guid, instanceOnClientDisconnected ) )
		{
			instanceOnClientDisconnected.Save();

			Players.Remove( guid );
			return true;
		} else
		{
			return false;
		}
	}

	void PMPrint()
	{
		if ( IsMissionClient() && false )
		{
			Print( "Printing all authenticated players!" );

			for ( int i = 0; i < Players.Count(); i++ )
			{
				Players.GetElement( i ).DebugPrint();
			}
		}
	}

	JMPlayerInstance GetPlayer( string guid )
	{
		// if ( !GetGame().IsMultiplayer() )
		// {
		// 	return Players.GetElement( 0 );
		// }

		return Players.Get( guid );
	}

	JMPlayerInstance UpdatePlayer( notnull JMPlayerInformation dataUpdatePlayer, PlayerIdentity identityUpdatePlayer = NULL, PlayerBase playerUpdatePlayer = NULL )
	{
		JMPlayerInstance instance = GetPlayer( dataUpdatePlayer.SGUID );

		if ( !instance )
		{
			if ( !IsMissionClient() )
			{
				return NULL;
			}

			instance = new JMPlayerInstance( identityUpdatePlayer );
			Players.Insert( dataUpdatePlayer.SGUID, instance );
		}

		instance.SwapData( dataUpdatePlayer );
		instance.Deserialize();

		if ( identityUpdatePlayer != NULL )
		{
			instance.IdentityPlayer = identityUpdatePlayer;
		}

		if ( IsMissionClient() )
		{
			if ( m_ClientGUID == dataUpdatePlayer.SGUID )
			{
				GetModuleManager().OnClientPermissionsUpdated();
			}

			instance.PlayerObject = playerUpdatePlayer;
		}

		return instance;
	}

	protected bool IsValidFolderForRoles( string name, FileAttr attributes )
	{
		string extenstion = ".txt";
		int strLength = name.Length();

		if ( name == extenstion )
			return false;

		if ( attributes & FileAttr.DIRECTORY )
			return false;

		if ( name == "" )
			return false;

		return true;
	}
	
	ref JMRole CreateRole( string name, ref array< string > data )
	{
		ref JMRole role = new JMRole( name );

		role.SerializedData.Copy( data );
		role.Deserialize();

		role.Save();

		Roles.Insert( name, role );

		return role;
	}

	bool LoadRole( string name, out JMRole role )
	{
		if ( !Roles.Find( name, role ) )
		{
			role = new JMRole( name );
			Roles.Insert( name, role );

			return false;
		}

		return true;
	}

	void LoadRoleFromFile( string name )
	{
		JMRole role = new JMRole( name );
		if ( role.Load() )
		{
			Roles.Insert( name, role );
		}
	}

	void LoadRoles()
	{
		string sName = "";
		FileAttr oFileAttr = FileAttr.INVALID;
		FindFileHandle oFileHandle = FindFile( JMConstants.DIR_ROLES + "*" + JMConstants.EXT_ROLE, sName, oFileAttr, FindFileFlags.ALL );

		if ( sName != "" )
		{
			if ( IsValidFolderForRoles( sName, oFileAttr ) )
			{
				LoadRoleFromFile( sName.Substring(0, sName.Length() - 4) );
			}

			while (FindNextFile(oFileHandle, sName, oFileAttr))
			{
				if ( IsValidFolderForRoles( sName, oFileAttr ))
				{
					LoadRoleFromFile( sName.Substring(0, sName.Length() - 4) );
				}
			}
		}
	}

	bool RoleExists( string role )
	{
		return Roles.Contains( role );
	}
}

ref JMPermissionManager g_com_PermissionsManager;

ref JMPermissionManager GetPermissionsManager()
{
	if( !g_com_PermissionsManager )
	{
		g_com_PermissionsManager = new JMPermissionManager();
	}

	return g_com_PermissionsManager;
}