class AuthPlayer
{
    const string AUTH_DIRECTORY = ROOT_COT_DIR + "Players\\";
    const string FILE_TYPE = "";

    ref Permission RootPermission;

    PlayerBase PlayerObject;
    PlayerIdentity IdentityPlayer;

    ref PlayerData Data;

    void AuthPlayer( ref PlayerData data )
    {
        PlayerObject = NULL;

        Data = data;

        if ( Data == NULL )
        {
            Data = new ref PlayerData;
        }

        RootPermission = new ref Permission( Data.SGUID, NULL );

        if ( GetGame().IsServer() )
        {
            MakeDirectory( AUTH_DIRECTORY );
        }
    }

    void ~AuthPlayer()
    {
        delete RootPermission;
    }

    void SetIdentity( PlayerIdentity identity )
    {
        IdentityPlayer = identity;

        Data.SSteam64ID = IdentityPlayer.GetPlainId();
    }

    void SwapData( ref PlayerData newData )
    {
        Data = newData;
    }

    string GetGUID()
    {
        return Data.SGUID;
    }

    string GetName()
    {
        return Data.SName;
    }

    void SetPlayerObjects( ref PlayerBase obj, ref PlayerIdentity identity )
    {
        IdentityPlayer = identity;
        PlayerObject = obj;
    }

    void UpdatePlayerData()
    {
        if ( IdentityPlayer == NULL ) return;

        Data.IPingMin = IdentityPlayer.GetPingMin();
        Data.IPingMax = IdentityPlayer.GetPingMax();
        Data.IPingAvg = IdentityPlayer.GetPingAvg();
        
        Data.SSteam64ID = IdentityPlayer.GetPlainId();
        Data.SGUID = IdentityPlayer.GetId();
        Data.SName = IdentityPlayer.GetName();

        if ( PlayerObject == NULL ) return;

        PlayerData.Load( Data, PlayerObject );
    }

    void CopyPermissions( ref Permission copy )
    {
        ref array< string > data = new ref array< string >;
        copy.Serialize( data );

        for ( int i = 0; i < data.Count(); i++ )
        {
            AddPermission( data[i] );
        }
    }

    void ClearPermissions()
    {
        delete RootPermission;

        RootPermission = new ref Permission( Data.SGUID, NULL );
    }

    void AddPermission( string permission, PermissionType type = PermissionType.INHERIT )
    {
        RootPermission.AddPermission( permission, type );
    }

    bool HasPermission( string permission )
    {
        return RootPermission.HasPermission( permission );
    }

    ref array< string > Serialize()
    {        
        Data.APermissions.Clear();

        RootPermission.Serialize( Data.APermissions );

        return Data.APermissions;
    }

    void Deserialize()
    {
        for ( int i = 0; i < Data.APermissions.Count(); i++ )
        {
            AddPermission( Data.APermissions[i] );
        }
    }

    string FileReadyStripName( string name )
    {
        name.Replace( "\\", "" );
        name.Replace( "/", "" );
        name.Replace( "=", "" );
        name.Replace( "+", "" );

        return name;
    }

    bool Save()
    {
        string filename = FileReadyStripName( Data.SGUID );

        Print( "Saving permissions for " + filename );
        FileHandle file = OpenFile( AUTH_DIRECTORY + filename + FILE_TYPE, FileMode.WRITE );
            
        ref array< string > data = Serialize();

        if ( file != 0 )
        {
            string line;

            for ( int i = 0; i < data.Count(); i++ )
            {
                FPrintln( file, data[i] );
            }
            
            CloseFile(file);
            
            Print("Wrote to the players");
            return true;
        } else
        {
            Print("Failed to open the file for the player for writing.");
            return false;
        }
    }

    bool Load()
    {
        string filename = FileReadyStripName( Data.SGUID );
        Print( "Loading permissions for " + filename );
        FileHandle file = OpenFile( AUTH_DIRECTORY + filename + FILE_TYPE, FileMode.READ );
            
        ref array< string > data = new ref array< string >;

        if ( file != 0 )
        {
            string line;

            while ( FGets( file, line ) > 0 )
            {
                data.Insert( line );
            }

            CloseFile( file );

            for ( int i = 0; i < data.Count(); i++ )
            {
                AddPermission( data[i] );
            }
        } else
        {
            Print( "Failed to open the file for the player to read. Attemping to create." );

            Save();
            return false;
        }
        
        return true;
    }

    void DebugPrint()
    {
        Print( "Printing permissions for " + Data.SGUID );

        RootPermission.DebugPrint( 0 );
    }

    // TODO: Figure out how to make it work properly?
    void Kick()
    {
        ForceDisconnect();
    }

    // TODO: Maybe actually ban the player?
    void Ban()
    {
        ForceDisconnect();
    }

    private void ForceDisconnect()
    {
        if ( IdentityPlayer )
        {
            if ( PlayerObject.CanBeDeleted() )
            {
                PlayerObject.Delete();	
            }
            else
            {
                PlayerObject.SetHealth( "", "", 0.0 );
            }

            GetGame().DisconnectPlayer( IdentityPlayer, IdentityPlayer.GetId() );
        }
    }
}