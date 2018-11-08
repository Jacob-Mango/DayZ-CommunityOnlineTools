class AuthPlayer
{
    const string AUTH_DIRECTORY = "$profile:Players\\";
    const string FILE_TYPE = "";

    ref Permission RootPermission;

    string GUID;

    void AuthPlayer( string guid )
    {
        GUID = guid;

        RootPermission = new ref Permission( GUID, NULL );

        MakeDirectory( AUTH_DIRECTORY );
    }

    void AddPermission( string permission )
    {
        RootPermission.AddPermission( permission );
    }

    bool HasPermission( string permission )
    {
        return RootPermission.HasPermission( permission );
    }

    bool Save()
    {
        FileHandle file = OpenFile( AUTH_DIRECTORY + GUID + FILE_TYPE, FileMode.WRITE );
            
        ref array< string > data = new ref array< string >;
        RootPermission.Serialize( data );

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
        FileHandle file = OpenFile( AUTH_DIRECTORY + GUID + FILE_TYPE, FileMode.READ );
            
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

            file = OpenFile( AUTH_DIRECTORY + GUID + FILE_TYPE, FileMode.WRITE );
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
            RootPermission.AddPermission( data[i] );
        }

        return true;
    }

    void DebugPrint()
    {
        Print( "Printing permissions for " + GUID );

        RootPermission.DebugPrint( 0 );
    }
}