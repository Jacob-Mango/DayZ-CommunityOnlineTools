class JMPermissionManager
{
	ref array< ref JMPlayerInstance > AuthPlayers;
	ref array< ref JMRole > Roles;
	ref map< string, ref JMRole > RolesMap;

	ref JMPermission RootPermission;

	void JMPermissionManager()
	{
		AuthPlayers = new array< ref JMPlayerInstance >;
		Roles = new array< ref JMRole >;
		RolesMap = new map< string, ref JMRole >;

		RootPermission = new JMPermission( "ROOT" );
		
		if ( GetGame().IsServer() )
		{
			MakeDirectory( JMConstants.DIR_PF );

			MakeDirectory( JMConstants.DIR_PERMISSIONS );
			MakeDirectory( JMConstants.DIR_PLAYERS );
			MakeDirectory( JMConstants.DIR_ROLES );
		}
	}

	array< ref JMPlayerInstance > GetPlayersFromArray( ref array< string > steamIds = NULL )
	{
		Print( steamIds );

		if ( steamIds == NULL || !GetGame().IsMultiplayer() )
		{
			return AuthPlayers;
		}

		array< ref JMPlayerInstance > tempArray = new array< ref JMPlayerInstance >;

		for ( int i = 0; i < steamIds.Count(); i++ )
		{
			for ( int k = 0; k < AuthPlayers.Count(); k++ )
			{
				if ( steamIds[i] == AuthPlayers[k].GetSteam64ID() )
				{
					tempArray.Insert( AuthPlayers[k] );
				}
			}
		}

		return tempArray;
	}

	void SortPlayersArray()
	{
		for ( int k = 0; k < AuthPlayers.Count(); k++ )
		{
			for ( int j = 0; j < AuthPlayers.Count(); j++ )
			{
				if ( AuthPlayers[k].Data.SName < AuthPlayers[j].Data.SName )
					AuthPlayers.SwapItems( k, j );
				else if ( AuthPlayers[k].Data.SName > AuthPlayers[j].Data.SName )
					AuthPlayers.SwapItems( j, k );
			}
		}
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
		if ( GetGame().IsServer() && !GetGame().IsMultiplayer() )
		{
			return true;
		}

		if ( GetGame().IsClient() ) 
		{
			if ( ClientAuthPlayer == NULL )
			{
				GetLogger().Log( "ClientAuthPlayer: " + ClientAuthPlayer, "JM_COT_PermissionFramework" );
				return false;
			}

			return ClientAuthPlayer.HasPermission( permission );
		}
		
		if ( identity == NULL )
		{
			return false;
		}

		PlayerBase player = GetPlayerObjectByIdentity( identity );

		if ( player != NULL )
		{
			return player.GetAuthenticatedPlayer().HasPermission( permission );
		}

		for ( int i = 0; i < AuthPlayers.Count(); i++ )
		{
			if ( AuthPlayers[i].GetGUID() == identity.GetId() )
			{
				return AuthPlayers[i].HasPermission( permission );
			}
		}

		return false;
	}

	ref JMPlayerInstance PlayerJoined( PlayerIdentity player )
	{
		JMPlayerInstance auPlayer = new JMPlayerInstance( player );

		auPlayer.CopyPermissions( RootPermission );
		auPlayer.Load();

		AuthPlayers.Insert( auPlayer );

		return auPlayer;
	}

	void DebugPrint()
	{
		GetLogger().Log( "Printing all authenticated players!", "JM_COT_PermissionFramework" );
		for ( int i = 0; i < AuthPlayers.Count(); i++ )
		{
			AuthPlayers[i].DebugPrint();
		}
	}

	ref JMPlayerInstance GetPlayerByGUID( string guid )
	{
		ref JMPlayerInstance auPlayer = NULL;

		for ( int i = 0; i < AuthPlayers.Count(); i++ )
		{
			if ( AuthPlayers[i].GetGUID() == guid )
			{
				auPlayer = AuthPlayers[i];
				break;
			}
		}

		if ( auPlayer == NULL )
		{
			auPlayer = new JMPlayerInstance( NULL );
			auPlayer.Data.SGUID = guid;

			AuthPlayers.Insert( auPlayer );
		}

		return auPlayer;
	}

	ref JMPlayerInstance GetPlayerBySteam64ID( string steam64 )
	{
		Error( "Deprecated method." );

		if ( !GetGame().IsMultiplayer() )
		{
			return AuthPlayers[0];
		}
		
		ref JMPlayerInstance auPlayer = NULL;

		for ( int i = 0; i < AuthPlayers.Count(); i++ )
		{
			if ( AuthPlayers[i].GetSteam64ID() == steam64 )
			{
				auPlayer = AuthPlayers[i];
				break;
			}
		}

		if ( auPlayer == NULL )
		{
			auPlayer = new JMPlayerInstance( NULL );
			auPlayer.Data.SSteam64ID = steam64;

			AuthPlayers.Insert( auPlayer );
		}

		return auPlayer;
	}

	ref JMPlayerInstance GetPlayerByIdentity( PlayerIdentity ident )
	{
		if ( !GetGame().IsMultiplayer() )
		{
			return AuthPlayers[0];
		}
		
		if ( ident == NULL )
		{
			return NULL;
		}

		ref JMPlayerInstance auPlayer = NULL;

		for ( int i = 0; i < AuthPlayers.Count(); i++ )
		{
			if ( AuthPlayers[i].IdentityPlayer == ident )
			{
				auPlayer = AuthPlayers[i];
				break;
			}
		}

		if ( auPlayer == NULL )
		{
			auPlayer = PlayerJoined( ident );
		}

		return auPlayer;
	}

	ref JMPlayerInstance GetPlayer( ref JMPlayerInformation data )
	{
		if ( !GetGame().IsMultiplayer() )
			return AuthPlayers[0];
		
		if ( data == NULL )
			return NULL;
		
		ref JMPlayerInstance auPlayer = NULL;

		for ( int i = 0; i < AuthPlayers.Count(); i++ )
		{
			if ( AuthPlayers[i].GetGUID() == data.SGUID )
			{
				auPlayer = AuthPlayers[i];
				break;
			}
		}

		if ( auPlayer == NULL )
		{
			auPlayer = new JMPlayerInstance( NULL );
			auPlayer.Data.Copy( data, GetGame().IsClient() );

			AuthPlayers.Insert( auPlayer );
		}

		auPlayer.SwapData( data );

		auPlayer.Deserialize();

		return auPlayer;
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

		Roles.Insert( role );
		RolesMap.Insert( name, role );

		return role;
	}

	ref JMRole LoadRole( string name, ref array< string > data = NULL )
	{
		ref JMRole role = new JMRole( name );

		if ( data == NULL )
		{
			if ( role.Load() )
			{
				Roles.Insert( role );
				RolesMap.Insert( name, role );
			}
		} else
		{
			role.SerializedData = data;
			role.Deserialize();

			if ( GetGame().IsMultiplayer() && GetGame().IsServer() )
			{
				role.Save();
			}

			Roles.Insert( role );
			RolesMap.Insert( name, role );
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
		return RolesMap.Contains( role );
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