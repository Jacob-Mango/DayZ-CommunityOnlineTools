class AuthPlayer
{
    const string AUTH_DIRECTORY = PERMISSION_FRAMEWORK_DIRECTORY + "Permissions\\";
    const string FILE_TYPE = ".txt";

    ref Permission RootPermission;
    ref array< Role > Roles;

    PlayerBase PlayerObject;
    PlayerIdentity IdentityPlayer;

    ref PlayerData Data;

    protected ref PlayerFile m_PlayerFile;

    protected bool m_HasPermissions;
    protected bool m_HasPlayerData;

    void AuthPlayer( ref PlayerData data )
    {
        PlayerObject = NULL;

        Data = data;

        if ( Data == NULL )
        {
            Data = new ref PlayerData;
        }

        RootPermission = new ref Permission( Data.SSteam64ID );
        Roles = new ref array< Role >;

        m_PlayerFile = new ref PlayerFile;

        if ( GetGame().IsServer() )
        {
            MakeDirectory( AUTH_DIRECTORY );
            MakeDirectory( PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" );
        }
    }

    void ~AuthPlayer()
    {
        delete RootPermission;
    }

    void SwapData( ref PlayerData newData )
    {
        Data = newData;
    }

    string GetGUID()
    {
        return Data.SGUID;
    }

    string GetSteam64ID()
    {
        return Data.SSteam64ID;
    }

    string GetName()
    {
        return Data.SName;
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

        RootPermission = new ref Permission( Data.SSteam64ID, NULL );

        m_HasPermissions = false;
    }

    void AddPermission( string permission, PermissionType type = PermissionType.INHERIT )
    {
        RootPermission.AddPermission( permission, type );

        m_HasPermissions = true;
    }

    bool HasPermission( string permission )
    {
        PermissionType permType;
        PermissionType rolePermType;

        bool has = RootPermission.HasPermission( permission, permType );

        Print( "Player " +  GetName() + " is " + has + " with perm type " + permType );

        if ( has )
            return true;

        for ( int j = 0; j < Roles.Count(); j++ )
        {
            bool roleHas = Roles[j].HasPermission( permission, rolePermType );

            Print( "Role " +  Roles[j].Name + " is " + roleHas + " with perm type " + rolePermType );

            if ( roleHas )
            {
                return true;
            }
        }

        return false;
    }

    void AddStringRole( string role )
    {
        ref Role r = GetPermissionsManager().RolesMap.Get( role );

        Print( "Adding role " + role + ": " + r );

        if ( Roles.Find( r ) < 0 ) 
        {
            AddRole( r );
        }
    }

    void AddRole( Role role )
    {
        Print( "Adding role " + role );

        m_HasPlayerData = true;

        Roles.Insert( role );
    }

    void Serialize()
    {
        Data.APermissions.Clear();
        Data.ARoles.Clear();

        RootPermission.Serialize( Data.APermissions );

        for ( int j = 0; j < Roles.Count(); j++ )
        {
            Data.ARoles.Insert( Roles[j].Name );
        }
    }

    void Deserialize()
    {
        for ( int i = 0; i < Data.APermissions.Count(); i++ )
        {
            AddPermission( Data.APermissions[i] );
        }

        for ( int j = 0; j < Data.ARoles.Count(); j++ )
        {
            AddStringRole( Data.ARoles[j] );
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

    void Save()
    {
        if ( m_HasPlayerData )
        {   
            if ( m_PlayerFile.Steam64ID != GetSteam64ID() )
            {
                m_PlayerFile.Steam64ID = GetSteam64ID();
            }

            if ( m_PlayerFile.GUID != GetGUID() )
            {
                m_PlayerFile.GUID = GetGUID();
            }

            if ( m_PlayerFile.Names.Find( GetName() ) < 0 )
            {   
                m_PlayerFile.Names.Insert( GetName() );
            }

            m_PlayerFile.Roles.Clear();

            for ( int j = 0; j < Roles.Count(); j++ )
            {
                m_PlayerFile.Roles.Insert( Roles[j].Name );
            }

            m_PlayerFile.Save();
        }

        if ( m_HasPermissions )
        {
            string filename = FileReadyStripName( Data.SSteam64ID );

            Serialize();

            Print( "Saving permissions and player data for " + filename );
            FileHandle file = OpenFile( AUTH_DIRECTORY + filename + FILE_TYPE, FileMode.WRITE );

            if ( file != 0 )
            {
                string line;

                for ( int i = 0; i < Data.APermissions.Count(); i++ )
                {
                    FPrintln( file, Data.APermissions[i] );
                }
                
                CloseFile(file);
            }
        }
    }

    bool Load()
    {
        string filename = FileReadyStripName( Data.SSteam64ID );
        Print( "Loading permissions for " + filename );
        FileHandle file = OpenFile( AUTH_DIRECTORY + filename + FILE_TYPE, FileMode.READ );
            
        ref array< string > data = new ref array< string >;

        m_HasPlayerData = m_PlayerFile.Load( IdentityPlayer );

        Print( m_HasPlayerData );
        Print( m_PlayerFile.Roles );

        for ( int j = 0; j < m_PlayerFile.Roles.Count(); j++ )
        {
            AddStringRole( m_PlayerFile.Roles[j] );
        }

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

            m_HasPermissions = true;
        } else
        {
            m_HasPermissions = false;

            return false;
        }
        
        return true;
    }

    void DebugPrint()
    {
        Print( "Printing permissions for " + Data.SSteam64ID );

        RootPermission.DebugPrint( 0 );
    }

    // TODO: Figure out how to make it work properly?
    void Kick()
    {
    }

    // TODO: Maybe actually ban the player?
    void Ban()
    {
    }
}