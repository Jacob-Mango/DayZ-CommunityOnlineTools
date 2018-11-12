class PermissionManager
{
    protected ref array< ref AuthPlayer > AuthPlayers;

    protected ref Permission RootPermission;

    void PermissionManager()
    {
        AuthPlayers = new ref array< ref AuthPlayer >;

        RootPermission = new ref Permission( "ROOT", NULL );
    }

    ref array< ref AuthPlayer > GetPlayers()
    {
        return AuthPlayers;
    }

    void SetPlayers( ref array< ref AuthPlayer > players )
    {
        AuthPlayers.Clear();

        // This doesn't work???
        //AuthPlayers.Copy( players );

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

    bool HasPermission( PlayerIdentity player, string permission )
    {
        if ( GetGame().IsClient() || !GetGame().IsMultiplayer() ) return true; // Assume there is always permission on the client. 

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
        if ( player == NULL ) return NULL;

        ref AuthPlayer auPlayer = GetPlayer( player );

        auPlayer.SetIdentity( player );

        RootPermission.DebugPrint();

        auPlayer.CopyPermissions( RootPermission );

        auPlayer.Load();

        auPlayer.AddPermission( "Admin", PermissionType.ALLOW );
        auPlayer.AddPermission( "Teleport", PermissionType.ALLOW );
        auPlayer.AddPermission( "Object", PermissionType.ALLOW );
        auPlayer.AddPermission( "Game", PermissionType.ALLOW );
        auPlayer.AddPermission( "CameraTools", PermissionType.ALLOW );
        auPlayer.AddPermission( "Weather", PermissionType.ALLOW );

        auPlayer.DebugPrint();

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

                AuthPlayers.Remove( i );

                break;
            }
        }
    }

    ref AuthPlayer GetPlayer( string guid, string name = "" )
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
            
            data.SName = name;
            data.SGUID = guid;

            auPlayer = new ref AuthPlayer( data );

            AuthPlayers.Insert( auPlayer );
        }

        return auPlayer;
    }

    ref AuthPlayer GetPlayer( PlayerIdentity ident )
    {
        if ( ident == NULL ) return NULL;

        return GetPlayer( ident.GetId(), ident.GetName() );
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

        return auPlayer;
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