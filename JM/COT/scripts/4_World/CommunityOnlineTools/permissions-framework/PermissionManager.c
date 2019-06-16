class PermissionManager
{
	ref array< ref AuthPlayer > AuthPlayers;
	ref array< ref Role > Roles;
	ref map< string, ref Role > RolesMap;

	ref Permission RootPermission;

	void PermissionManager()
	{
		AuthPlayers = new array< ref AuthPlayer >;
		Roles = new array< ref Role >;
		RolesMap = new map< string, ref Role >;

		RootPermission = new ref Permission( "ROOT" );
	}

	array< ref AuthPlayer > GetPlayersFromArray( ref array< string > steamIds = NULL )
	{
		Print( steamIds );

		if ( steamIds == NULL || !GetGame().IsMultiplayer() )
		{
			return AuthPlayers;
		}

		array< ref AuthPlayer > tempArray = new array< ref AuthPlayer >;

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

	void RegisterPermission( string permission )
	{
		RootPermission.AddPermission( permission, PermissionType.INHERIT );
	}

	ref array< string > Serialize()
	{
		ref array< string > data = new ref array< string >;
		RootPermission.Serialize( data );
		return data;
	}

	ref Permission GetRootPermission()
	{
		return RootPermission;
	}

	bool HasPermission( string permission, PlayerIdentity identity = NULL )
	{
		if ( !GetGame().IsMultiplayer() ) return true;

		if ( GetGame().IsClient() ) 
		{
			if ( ClientAuthPlayer == NULL )
			{
				GetLogger().Log( "ClientAuth is NULL!", "JM_COT_PermissionFramework" );
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

	ref AuthPlayer PlayerJoined( PlayerIdentity player )
	{
		AuthPlayer auPlayer = new AuthPlayer( player );

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

	ref AuthPlayer GetPlayerByGUID( string guid )
	{
		ref AuthPlayer auPlayer = NULL;

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
			auPlayer = new AuthPlayer( NULL );
			auPlayer.Data.SGUID = guid;

			AuthPlayers.Insert( auPlayer );
		}

		return auPlayer;
	}

	ref AuthPlayer GetPlayerBySteam64ID( string steam64 )
	{
		Error( "Deprecated method." );

		if ( !GetGame().IsMultiplayer() )
		{
			return AuthPlayers[0];
		}
		
		ref AuthPlayer auPlayer = NULL;

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
			auPlayer = new AuthPlayer( NULL );
			auPlayer.Data.SSteam64ID = steam64;

			AuthPlayers.Insert( auPlayer );
		}

		return auPlayer;
	}

	ref AuthPlayer GetPlayerByIdentity( PlayerIdentity ident )
	{
		if ( !GetGame().IsMultiplayer() )
		{
			return AuthPlayers[0];
		}
		
		if ( ident == NULL ) return NULL;

		ref AuthPlayer auPlayer = NULL;

		for ( int i = 0; i < AuthPlayers.Count(); i++ )
		{
			if ( AuthPlayers[i].GetGUID() == ident.GetId() )
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

	ref AuthPlayer GetPlayer( ref PlayerData data )
	{
		if ( !GetGame().IsMultiplayer() )
			return AuthPlayers[0];
		
		if ( data == NULL )
			return NULL;
		
		ref AuthPlayer auPlayer = NULL;

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
			auPlayer = new AuthPlayer( NULL );
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
	
	ref Role CreateRole( string name, ref array< string > data )
	{
		ref Role role = new ref Role( name );

		role.SerializedData.Copy( data );
		role.Deserialize();

		role.Save();

		Roles.Insert( role );
		RolesMap.Insert( name, role );

		return role;
	}

	ref Role LoadRole( string name, ref array< string > data = NULL )
	{
		ref Role role = new ref Role( name );

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
		FindFileHandle oFileHandle = FindFile(PERMISSION_FRAMEWORK_DIRECTORY + "Roles\\*.txt", sName, oFileAttr, FindFileFlags.ALL);

		if (sName != "")
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

ref PermissionManager g_com_PermissionsManager;

ref PermissionManager GetPermissionsManager()
{
	if( !g_com_PermissionsManager )
	{
		g_com_PermissionsManager = new ref PermissionManager();
	}

	return g_com_PermissionsManager;
}