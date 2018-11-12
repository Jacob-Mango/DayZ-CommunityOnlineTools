class AuthPlayer
{
    const string AUTH_DIRECTORY = "$profile:Players\\";
    const string FILE_TYPE = "";

    ref Permission RootPermission;

    protected Man m_Man;
    protected PlayerIdentity m_Identity;

    protected ref PlayerData m_Data;

    void AuthPlayer( PlayerData data )
    {
        m_Man = NULL;

        m_Data = data;

        RootPermission = new ref Permission( m_Data.SGUID, NULL );

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
        m_Identity = identity;

        m_Data.SSteam64ID = m_Identity.GetPlainId();
    }

    PlayerIdentity GetIdentity()
    {
        return m_Identity;
    }

    ref PlayerData GetData()
    {
        return m_Data;
    }  

    string GetGUID()
    {
        return m_Data.SGUID;
    }

    string GetName()
    {
        return m_Data.SName;
    }

    ref Man GetPlayerObject()
    {
        return m_Man;
    }

    void UpdatePlayerObject( ref Man obj )
    {
        m_Man = obj;

        PlayerData.Load( m_Data, m_Man );
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

        RootPermission = new ref Permission( m_Data.SGUID, NULL );
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
        m_Data.APermissions.Clear();

        RootPermission.Serialize( m_Data.APermissions );

        return m_Data.APermissions;
    }

    void Deserialize()
    {
        for ( int i = 0; i < m_Data.APermissions.Count(); i++ )
        {
            AddPermission( m_Data.APermissions[i] );
        }
    }

    bool Save()
    {
        FileHandle file = OpenFile( AUTH_DIRECTORY + m_Data.SGUID + FILE_TYPE, FileMode.WRITE );
            
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
        FileHandle file = OpenFile( AUTH_DIRECTORY + m_Data.SGUID + FILE_TYPE, FileMode.READ );
            
        ref array< string > data = new ref array< string >;

        if ( file != 0 )
        {
            string line;

            while ( FGets( file, line ) > 0 )
            {
                data.Insert( line );
            }

            CloseFile( file );
        } else
        {
            Print( "Failed to open the file for the player to read. Attemping to create." );

            file = OpenFile( AUTH_DIRECTORY + m_Data.SGUID + FILE_TYPE, FileMode.WRITE );

            if ( file != 0 )
            {
                CloseFile( file );
                Print( "Created the file." );
            } else
            {
                Print( "Failed to create the file." );
            }
            return false;
        }
        
        for ( int i = 0; i < data.Count(); i++ )
        {
            AddPermission( data[i] );
        }

        return true;
    }

    void DebugPrint()
    {
        Print( "Printing permissions for " + m_Data.SGUID );

        RootPermission.DebugPrint( 0 );
    }

    // TODO: Figure out how to make it work properly?
    void Kick()
    {
        if ( m_Identity )
        {
            GetGame().DisconnectPlayer( m_Identity, m_Identity.GetId() );
        }
    }

    // TODO: Maybe actually ban the player?
    void Ban()
    {
        if ( m_Identity )
        {
            GetGame().DisconnectPlayer( m_Identity, m_Identity.GetId() );
        }
    }
}