class PermissionManager
{
    protected ref array< ref AuthPlayer > AuthPlayers;

    protected ref Permission RootPermission;

    void PermissionManager()
    {
        RootPermission = new ref Permission( "ROOT", NULL );

        AuthPlayers = new ref array< ref AuthPlayer >;
    }

    bool SavePermissions()
    {
	    FileSerializer file = new FileSerializer();

        ref array< string > data = new ref array< string >;

        RootPermission.Serialize( data );

        if ( file.Open( "$profile:Permissions.txt", FileMode.WRITE ) )
        {
            file.Write( data );
            file.Close();

            return true;
        }

        return false;
    }

    bool HasPermission( PlayerIdentity player, string permission )
    {
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
                AuthPlayers[i].Load();

                AuthPlayerExists = true;
            }
        }

        if ( !AuthPlayerExists )
        {
            AuthPlayer auPlayer = new AuthPlayer( player.GetId() );
            
            auPlayer.Load();

            AuthPlayers.Insert( auPlayer );
        }
    }

    void PlayerLeft( PlayerIdentity player )
    {
        for ( int i = 0; i < AuthPlayers.Count(); i++ )
        {
            if ( AuthPlayers[i].GUID == player.GetId() )
            {
                AuthPlayers[i].Save();
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