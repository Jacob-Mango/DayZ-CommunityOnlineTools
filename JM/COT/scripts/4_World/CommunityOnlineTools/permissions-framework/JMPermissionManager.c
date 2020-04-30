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
	}

	void GetRolesAsList( out array< JMRole > roles )
	{
		if ( roles == NULL )
			roles = new array< JMRole >();

		Roles.GetValueArray().Debug();

		// Print("copying");

		roles.Copy( Roles.GetValueArray() );

		roles.Debug();
	}

	void GetPermissionsAsList( out array< JMPermission > permissions )
	{
		if ( permissions == NULL )
			permissions = new array< JMPermission >();

		GetPermissionsAsList( RootPermission, -1, permissions );
	}

	private void GetPermissionsAsList( ref JMPermission permission, int depth, out array< JMPermission > permissions )
	{
		permission.Depth = depth;

		for ( int i = 0; i < permission.Children.Count(); ++i )
		{
			permissions.Insert( permission.Children[i] );

			GetPermissionsAsList( permission.Children[i], depth + 1, permissions );
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
		JMRole role = Roles.Get( name );
		if ( role )
			return role.HasPermission( permission, permTypeHasRolePermission );

		return false;
	}

	bool IsRole( string role )
	{
		return Roles.Contains( role );
	}

	void ResetMission()
	{
		if ( !IsMissionClient() )
			return;

		Players.Clear();
		Roles.Clear();
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
		instance.MakeFake( "GFake" + i, "SFake" + i, "NFake" + i );
		Players.Insert( instance.GetGUID(), instance );
	}

	/**
	 * This uses GUIDs now.
	 */
	ref array< JMPlayerInstance > GetPlayers( ref array< string > guidsGetPlayers = NULL )
	{
		if ( guidsGetPlayers == NULL || !GetGame().IsMultiplayer() )
		{
			return Players.GetValueArray();
		}

		array< JMPlayerInstance > players = new array< JMPlayerInstance >;

		for ( int i = 0; i < guidsGetPlayers.Count(); i++ )
		{
			JMPlayerInstance instance = Players.Get( guidsGetPlayers[i] );
			if ( instance )
				players.Insert( instance );
		}

		return players;
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

	ref JMPermission GetRootPermission()
	{
		return RootPermission;
	}

	bool HasPermission( string permission )
	{
		if ( IsMissionClient() ) 
		{
			// Print( "JMPermissionManager::HasPermission - IsMissionClient" );

			if ( IsMissionHost() )
			{
				// Print( "JMPermissionManager::HasPermission - IsMissionHost" );
				return true;
			}
			
			JMPlayerInstance instance = GetClientPlayer();

			// Print( "JMPermissionManager::HasPermission - instance=" + instance );
			if ( instance == NULL )
			{
				// Print( "Client Player is NULL!" );
				return false;
			}

			return instance.HasPermission( permission );
		}

		return false;
	}

	bool HasPermission( string permission, PlayerIdentity ihp )
	{
		if ( IsMissionOffline() )
		{
			return true;
		}

		JMPlayerInstance instance;
		return HasPermission( permission, ihp, instance );
	}

	bool HasPermission( string permission, notnull PlayerIdentity identity, out JMPlayerInstance instance )
	{
		// Print( "JMPermissionManager::HasPermission - Start" );
		
		if ( IsMissionClient() ) 
		{
			// Print( "JMPermissionManager::HasPermission - IsMissionClient" );

			instance = GetClientPlayer();

			if ( IsMissionHost() )
			{
				// Print( "JMPermissionManager::HasPermission - IsMissionHost" );
				return true;
			}
			
			// Print( "JMPermissionManager::HasPermission - instance=" + instance );
			if ( instance == NULL )
			{
				// Print( "Client Player is NULL!" );
				return false;
			}

			return instance.HasPermission( permission );
		}
		
		// Print( "JMPermissionManager::HasPermission - identity=" + identity );
		if ( identity == NULL )
		{
			return false;
		}

		// Print( "JMPermissionManager::HasPermission - identity::GetId=" + identity.GetId() );

		instance = Players.Get( identity.GetId() );
		if ( instance )
		{
			// Print( "JMPermissionManager::HasPermission - instance=" + instance );
			return instance.HasPermission( permission );
		}

		return false;
	}

	bool OnClientConnected( PlayerIdentity ident, out JMPlayerInstance inst )
	{
		string guid = "";
		
		if ( ident == NULL )
		{
			if ( GetGame().IsMultiplayer() )
			{
				return false;
			}

			guid = JMConstants.OFFLINE_GUID;
		} else
		{
			guid = ident.GetId();
		}

		inst = Players.Get( guid );
		if ( inst )
		{
			return false;
		}

		inst = new JMPlayerInstance( ident );

		inst.CopyPermissions( RootPermission );
		inst.Load();

		inst.Serialize();

		Players.Insert( guid, inst );

		// PMPrint();

		return true;
	}

	bool OnClientDisconnected( string guid, out JMPlayerInstance inst )
	{
		inst = Players.Get( guid );
		if ( inst )
		{
			inst.Save();

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
			// Print( "Printing all authenticated players!" );

			for ( int i = 0; i < Players.Count(); i++ )
			{
				Players.GetElement( i ).DebugPrint();
			}
		}
	}

	ref JMPlayerInstance GetPlayer( string guid )
	{
		return Players.Get( guid );
	}

	ref JMPlayerInstance UpdatePlayer( notnull JMPlayerInformation data, PlayerBase playerUpdatePlayer = NULL )
	{
		JMPlayerInstance instance = GetPlayer( data.SGUID );

		if ( !instance )
		{
			if ( !IsMissionClient() )
			{
				return NULL;
			}

			instance = new JMPlayerInstance( NULL );
			Players.Insert( data.SGUID, instance );
		}

		instance.SwapData( data );
		instance.Deserialize();

		if ( IsMissionClient() )
		{
			if ( m_ClientGUID == data.SGUID )
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
		role = Roles.Get( name );

		if ( !role )
		{
			role = new JMRole( name );
			Roles.Insert( name, role );

			return false;
		}

		return true;
	}

	void LoadRoleFromFile( string name )
	{
		// Print( "Loading role " + name );

		ref JMRole role = new JMRole( name );
		// Print( "Role " + role );
		if ( role.Load() )
		{
			// Print( "Loaded role " + role );

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

		// Print( "Roles count: " + Roles.Count().ToString() );
		for ( int i = 0; i < Roles.Count(); i++ )
		{
			// Print( "["+Roles.GetKey( i )+"] => " + Roles.GetElement( i ) );
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