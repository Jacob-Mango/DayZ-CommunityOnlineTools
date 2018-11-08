class PermissionManager
{
    protected ref array< ref AuthPlayer > AuthPlayers;

    //protected ref Permission RootPermission;

    void PermissionManager()
    {
        //RootPermission = new ref Permission( "ROOT", NULL );

        AuthPlayers = new ref array< ref AuthPlayer >;
    }

    ref array< ref AuthPlayer > GetPlayers()
    {
        return AuthPlayers;
    }

    bool SavePermissions()
    {
        /*
	    FileSerializer file = new FileSerializer();

        ref array< string > data = new ref array< string >;

        RootPermission.Serialize( data );

        if ( file.Open( "$profile:Permissions.txt", FileMode.WRITE ) )
        {
            file.Write( data );
            file.Close();

            return true;
        }
        */

        return false;
    }

    bool HasPermission( PlayerIdentity player, string permission )
    {
        if ( GetGame().IsClient() || !GetGame().IsMultiplayer() ) return true; // Assume there is always permission on the client.

        return true;
        /*
        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            if ( AuthPlayers[i].GUID == player.GetId() )
            {
                return AuthPlayers[i].HasPermission( permission );
            }
        }
        return false;
        */
    }

    void PlayerJoined( PlayerIdentity player )
    {
        bool AuthPlayerExists = false;

        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            if ( AuthPlayers[i].GUID == player.GetId() )
            {
                // AuthPlayers[i].Load();

                AuthPlayerExists = true;
            }
        }

        if ( !AuthPlayerExists )
        {
            ref AuthPlayer auPlayer = new ref AuthPlayer( player.GetId() );

            auPlayer.Load();

            auPlayer.AddPermission("Teleport.SetPosition");
            auPlayer.AddPermission("Object.SpawnObjectPosition");
            auPlayer.AddPermission("Object.SpawnObjectInventory");
            auPlayer.AddPermission("Game.SpawnVehicle");
            auPlayer.AddPermission("Game.ChangeAimingMode");
            auPlayer.AddPermission("Game.EnableGodMode");
            auPlayer.AddPermission("Game.KillEntity");
            auPlayer.AddPermission("CameraTools.LeaveCamera");
            auPlayer.AddPermission("CameraTools.EnterCamera");
            auPlayer.AddPermission("Weather.SetStorm");
            auPlayer.AddPermission("Weather.SetFog");
            auPlayer.AddPermission("Weather.SetOvercast");
            auPlayer.AddPermission("Weather.SetWindFunctionParams");
            auPlayer.AddPermission("Weather.SetDate");

            auPlayer.DebugPrint();

            AuthPlayers.Insert( auPlayer );
        }
    }

    void PlayerLeft( PlayerIdentity player )
    {
        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            ref AuthPlayer auPlayer = AuthPlayers[i];
            if ( auPlayer.GUID == player.GetId() )
            {
                auPlayer.Save();

                AuthPlayers.Remove( i );
            }
        }
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