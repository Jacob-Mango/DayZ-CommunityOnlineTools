class AuthPlayer
{
    ref Permission RootPermission;

    string GUID;

    void AuthPlayer( string guid )
    {
        GUID = guid;

        RootPermission = new ref Permission( GUID, NULL );
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
	    FileSerializer file = new FileSerializer();

        ref array< string > data = new ref array< string >;

        RootPermission.Serialize( data );

        if ( file.Open( "$profile:Players/" + GUID + ".perm", FileMode.WRITE ) )
        {
            file.Write( data );
            file.Close();

            Print("Wrote to the players");
            return true;
        } else
        {
            Print("Failed to open the file for the player");
            return false;
        }
    }

    bool Load()
    {
        FileSerializer file = new FileSerializer();
            
        ref array< string > data = new ref array< string >;

        if ( file.Open( "$profile:Players/" + GUID + ".perm", FileMode.READ ) )
        {
            file.Read(data);
            file.Close();
            
            Print("Read the players");
        } else {
            file.Close();

            Print("Failed to open the file for the player");
            return false;
        }
        
        for ( int i = 0; i < data.Count(); i++ )
        {
            RootPermission.AddPermission( data[i] );
        }

        return true;
    }
}