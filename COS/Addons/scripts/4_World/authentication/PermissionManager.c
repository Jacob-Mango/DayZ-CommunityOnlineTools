class PermissionManager
{
    protected ref array< ref AuthPlayer > AuthPlayers;

    void PermissionManager()
    {
        AuthPlayers = new ref array< ref AuthPlayer >;
    }

    ref array< ref AuthPlayer > GetPlayers()
    {
        return AuthPlayers;
    }

    bool SavePermissions()
    {
        return false;
    }

    bool HasPermission( PlayerIdentity player, string permission )
    {
        if ( player == NULL ) return false; // Player couldn't be found, can we trust this person here?

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

        auPlayer.Load();

        auPlayer.AddPermission( "Perms.Set", PermType.ALLOWED );
        auPlayer.AddPermission( "Perms.Load", PermType.ALLOWED );

        auPlayer.AddPermission( "COS.UseEditor", PermType.ALLOWED );

        auPlayer.AddPermission( "Teleport.SetPosition", PermType.ALLOWED );

        auPlayer.AddPermission( "Object.SpawnObjectPosition", PermType.ALLOWED );
        auPlayer.AddPermission( "Object.SpawnObjectInventory", PermType.ALLOWED );

        auPlayer.AddPermission( "Game.SpawnVehicle", PermType.ALLOWED );
        auPlayer.AddPermission( "Game.ChangeAimingMode", PermType.ALLOWED );
        auPlayer.AddPermission( "Game.EnableGodMode", PermType.ALLOWED );
        auPlayer.AddPermission( "Game.KillEntity", PermType.ALLOWED );

        auPlayer.AddPermission( "CameraTools.LeaveCamera", PermType.ALLOWED );
        auPlayer.AddPermission( "CameraTools.EnterCamera", PermType.ALLOWED );
        
        auPlayer.AddPermission( "Weather.SetStorm", PermType.ALLOWED );
        auPlayer.AddPermission( "Weather.SetFog", PermType.ALLOWED );
        auPlayer.AddPermission( "Weather.SetOvercast", PermType.ALLOWED );
        auPlayer.AddPermission( "Weather.SetWindFunctionParams", PermType.ALLOWED );
        auPlayer.AddPermission( "Weather.SetDate", PermType.ALLOWED );

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

    ref AuthPlayer GetPlayer( PlayerIdentity player )
    {
        if ( player == NULL ) return NULL;

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            ref AuthPlayer auPlayer = AuthPlayers[i];
            if ( auPlayer.GUID == player.GetId() )
            {
                return auPlayer;
            }
        }

        return NULL;
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