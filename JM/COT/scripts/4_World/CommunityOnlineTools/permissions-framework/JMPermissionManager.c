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
		
		if ( GetGame().IsServer() )
		{
			MakeDirectory( JMConstants.DIR_PF );

			MakeDirectory( JMConstants.DIR_PERMISSIONS );
			MakeDirectory( JMConstants.DIR_PLAYERS );
			MakeDirectory( JMConstants.DIR_ROLES );
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

	bool HasRolePermission( string name, string permission, out JMPermissionType permType = JMPermissionType.DISALLOW )
	{
		JMRole role;
		if ( Roles.Find( name, role ) )
		{
			return role.HasPermission( permission, permType )
		}
		return false;
	}

	bool IsRole( string role )
	{
		return Roles.Contains( role );
	}

	/**
	 * This uses GUIDs now.
	 */
	array< JMPlayerInstance > GetPlayers( ref array< string > guids = NULL )
	{
		if ( guids == NULL || !GetGame().IsMultiplayer() )
		{
			return Players.GetValueArray();
		}

		array< JMPlayerInstance > players = new array< JMPlayerInstance >;

		for ( int i = 0; i < guids.Count(); i++ )
		{
			JMPlayerInstance instance;
			if ( Players.Find( guids[i], instance ) )
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

	ref JMPermission GetRootPermission()
	{
		return RootPermission;
	}

	bool HasPermission( string permission, PlayerIdentity identity = NULL )
	{
		// PMPrint();

		if ( GetGame().IsServer() && !GetGame().IsMultiplayer() )
		{
			return true;
		}

		if ( GetGame().IsClient() ) 
		{
			if ( GetPermissionsManager().GetClientPlayer() == NULL )
			{
				Print( "Client Player is NULL!" );
				return false;
			}

			return GetPermissionsManager().GetClientPlayer().HasPermission( permission );
		}
		
		if ( identity == NULL )
		{
			return false;
		}

		JMPlayerInstance instance;
		if ( Players.Find( identity.GetId(), instance ) )
		{
			return instance.HasPermission( permission );
		}

		return false;
	}

	bool OnClientConnected( PlayerIdentity identity, out JMPlayerInstance instance )
	{
		string guid = "";
		
		if ( identity == NULL )
		{
			if ( GetGame().IsMultiplayer() )
			{
				return false;
			}

			guid = JMConstants.OFFLINE_GUID;
		} else
		{
			guid = identity.GetId();
		}

		if ( Players.Find( guid, instance ) )
		{
			return false;
		}

		instance = new JMPlayerInstance( identity );

		instance.CopyPermissions( RootPermission );
		instance.Load();

		instance.Serialize();

		Players.Insert( guid, instance );

		// PMPrint();

		return true;
	}

	bool OnClientDisconnected( string guid, out JMPlayerInstance instance )
	{
		if ( Players.Find( guid, instance ) )
		{
			instance.Save();

			Players.Remove( guid );
			return true;
		} else
		{
			return false;
		}
	}

	void PMPrint()
	{
		//Print( "Printing all authenticated players!" );

		for ( int i = 0; i < Players.Count(); i++ )
		{
			Players.GetElement( i ).DebugPrint();
		}
	}

	JMPlayerInstance GetPlayer( string guid )
	{
		if ( !GetGame().IsMultiplayer() )
		{
			return Players.GetElement( 0 );
		}

		return Players.Get( guid );
	}

	/**
	 * Sometimes PlayerIdentity::GetId doesn't work, for those situtations we use this
	 */
	JMPlayerInstance GetPlayerByIdentity( PlayerIdentity ident )
	{
		if ( !GetGame().IsMultiplayer() )
		{
			return Players.GetElement( 0 );
		}
		
		if ( ident == NULL )
		{
			return NULL;
		}

		JMPlayerInstance auPlayer = NULL;

		for ( int i = 0; i < Players.Count(); i++ )
		{
			if ( Players.GetElement( i ).IdentityPlayer == ident )
			{
				auPlayer = Players.GetElement( i );
				break;
			}
		}

		return auPlayer;
	}

	JMPlayerInstance UpdatePlayer( notnull JMPlayerInformation data, PlayerIdentity identity = NULL, PlayerBase player = NULL )
	{
		//Print( "UpdatePlayer (" + data.SGUID + ")" );
		JMPlayerInstance instance = GetPlayer( data.SGUID );

		if ( !instance )
		{
			//Print( " !instance" );
			if ( !IsMissionClient() )
			{
				//Print( " !IsMissionClient" );
				return NULL;
			}

			instance = new JMPlayerInstance( identity );
			Players.Insert( data.SGUID, instance );
		}

		instance.SwapData( data );
		instance.Deserialize();

		if ( identity != NULL )
		{
			//Print( " identity" );
			instance.IdentityPlayer = identity;
		}

		if ( IsMissionClient() )
		{
			if ( m_ClientGUID == data.SGUID )
			{
				GetModuleManager().OnClientPermissionsUpdated();
			}

			instance.PlayerObject = player;
		}

		return instance;
	}

	protected bool IsValidFolderForRoles( string name, FileAttr attributes )
	{
		string extenstion = ".txt";
		int strLength = name.Length();

		if ( name == extenstion )
			return false;

		if ( (attributes & FileAttr.DIRECTORY ) )
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

	ref JMRole LoadRole( string name, ref array< string > data = NULL )
	{
		ref JMRole role = new JMRole( name );

		if ( data == NULL )
		{
			if ( role.Load() )
			{
				Roles.Insert( name, role );
			}
		} else
		{
			role.SerializedData = data;
			role.Deserialize();

			if ( GetGame().IsMultiplayer() && GetGame().IsServer() )
			{
				role.Save();
			}

			Roles.Insert( name, role );
		}

		return role;
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
				LoadRole( sName.Substring(0, sName.Length() - 4) );
			}

			while (FindNextFile(oFileHandle, sName, oFileAttr))
			{
				if ( IsValidFolderForRoles( sName, oFileAttr ))
				{
					LoadRole( sName.Substring(0, sName.Length() - 4) );
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