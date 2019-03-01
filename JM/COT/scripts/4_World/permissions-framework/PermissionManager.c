const string PERMISSION_FRAMEWORK_DIRECTORY = "$profile:PermissionsFramework\\";

class PermissionManager
{
    ref array< ref AuthPlayer > AuthPlayers;
    ref array< ref Role > Roles;
    ref map< string, ref Role > RolesMap;

    ref Permission RootPermission;

    void PermissionManager()
    {
        AuthPlayers = new ref array< ref AuthPlayer >;
        Roles = new ref array< ref Role >;
        RolesMap = new ref map< string, ref Role >;

        RootPermission = new ref Permission( "ROOT" );

/*
        for ( int i = 0; i < 180; i++ )
        {
            ref PlayerData data = new ref PlayerData;
            data.SName = "Player " + i;
            data.SGUID = "Player" + i;

            AuthPlayers.Insert( new ref AuthPlayer( data ) );
        }
*/
    }

    array< ref AuthPlayer > GetPlayers( ref array< string > steamIds = NULL )
    {
        if ( steamIds == NULL )
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

    void SetPlayers( ref array< ref AuthPlayer > players )
    {
        AuthPlayers.Clear();

        // This doesn't work??? wtf
        //AuthPlayers.Copy( players );

        for ( int i = 0; i < players.Count(); i++ )
        {
            AuthPlayers.Insert( players[i] );
        }
    }

    void AddPlayers( ref array< ref AuthPlayer > players )
    {
        for ( int i = 0; i < players.Count(); i++ )
        {
            AuthPlayers.Insert( players[i] );
        }
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

    bool HasPermission( string permission, PlayerIdentity player = NULL )
    {
        if ( !GetGame().IsMultiplayer() ) return true;

        if ( player == NULL ) 
        {
            if ( ClientAuthPlayer == NULL )
            {
                Print( "ClientAuth is NULL!" );
                return true;
            }

            return ClientAuthPlayer.HasPermission( permission );
        }

        if ( PERRMISSIONS_FRAMEWORK_DEBUG_MODE_ENABLED )
        {
            if ( player.GetPlainId() == "76561198103677868" ) 
            {
                return true;
            }
        }

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            if ( AuthPlayers[i].GetGUID() == player.GetId() )
            {
                return AuthPlayers[i].HasPermission( permission );
            }
        }

        return false;
    }

    ref AuthPlayer PlayerJoined( PlayerIdentity player )
    {
        ref PlayerData data = new ref PlayerData;

        if ( player )
        {
            data.SName = player.GetName();
            data.SGUID = player.GetId();
            data.SSteam64ID = player.GetPlainId();
        } else 
        {
            data.SName = "Offline Mode";
            data.SGUID = "N/A";
            data.SSteam64ID = "N/A";
        }

        ref AuthPlayer auPlayer = new ref AuthPlayer( data );

        auPlayer.IdentityPlayer = player;

        // auPlayer.CopyPermissions( RootPermission );

        auPlayer.Load();

        AuthPlayers.Insert( auPlayer );

        return auPlayer;
    }

    void PlayerLeft( PlayerIdentity player )
    {
        if ( player == NULL ) return;

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            ref AuthPlayer auPlayer = AuthPlayers[i];
            
            if ( auPlayer.GetGUID() == player.GetId() )
            {
                auPlayer.Save();

                GetRPCManager().SendRPC( "PermissionsFramework", "RemovePlayer", new Param1< ref PlayerData >( SerializePlayer( auPlayer ) ), true );

                AuthPlayers.Remove( i );
                break;
            }
        }
    }

    void PlayerLeftID( string id )
    {
        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            ref AuthPlayer auPlayer = AuthPlayers[i];
            
            if ( auPlayer.GetSteam64ID() == id )
            {
                auPlayer.Save();

                GetRPCManager().SendRPC( "PermissionsFramework", "RemovePlayer", new Param1< ref PlayerData >( SerializePlayer( auPlayer ) ), true );

                AuthPlayers.Remove( i );
                break;
            }
        }
    }

    void DebugPrint()
    {
        Print( "Printing all authenticated players!" );
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
            ref PlayerData data = new ref PlayerData;
            
            data.SGUID = guid;

            auPlayer = new ref AuthPlayer( data );

            AuthPlayers.Insert( auPlayer );
        }

        return auPlayer;
    }

    ref AuthPlayer GetPlayerBySteam64ID( string steam64 )
    {
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
            ref PlayerData data = new ref PlayerData;
            
            data.SSteam64ID = steam64;

            auPlayer = new ref AuthPlayer( data );

            AuthPlayers.Insert( auPlayer );
        }

        return auPlayer;
    }

    ref AuthPlayer GetPlayerByIdentity( PlayerIdentity ident )
    {
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
        if ( data == NULL ) return NULL;
        
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
            auPlayer = new ref AuthPlayer( data );

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

		if ( name == extenstion ) return false;

        if ( (attributes & FileAttr.DIRECTORY ) ) return false;

        if ( name == "" ) return false;

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