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

        if ( file.Open( "$profile:Players/" + GUID + ".txt", FileMode.WRITE ) )
        {
            file.Write( data );
            file.Close();

            return true;
        }

        return false;
    }

    bool Load()
    {

    }
}