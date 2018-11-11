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
            if ( AuthPlayers[i].GUID == player.GetId() )
            {
                return AuthPlayers[i].HasPermission( permission );
            }
        }

        return false;
    }

    void PlayerJoined( PlayerIdentity player )
    {
        if ( player == NULL ) return;

        ref AuthPlayer auPlayer = NULL;

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            if ( AuthPlayers[i].GUID == player.GetId() )
            {
                auPlayer = AuthPlayers[i];
                break;
            }
        }

        if ( auPlayer == NULL )
        {
            auPlayer = new ref AuthPlayer( player.GetId() );

            AuthPlayers.Insert( auPlayer );
        }

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
    }

    void PlayerLeft( PlayerIdentity player )
    {
        if ( player == NULL ) return;

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            ref AuthPlayer auPlayer = AuthPlayers[i];
            if ( auPlayer.GUID == player.GetId() )
            {
                auPlayer.Save();

                AuthPlayers.Remove( i );

                break;
            }
        }
    }

    ref AuthPlayer GetPlayer( string guid )
    {
        ref AuthPlayer auPlayer = NULL;

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            if ( AuthPlayers[i].GUID == guid )
            {
                auPlayer = AuthPlayers[i];
                break;
            }
        }

        return auPlayer;
    }

    ref AuthPlayer GetPlayer( PlayerIdentity ident )
    {
        if ( ident == NULL ) return NULL;

        return GetPlayer( ident.GetId() );
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