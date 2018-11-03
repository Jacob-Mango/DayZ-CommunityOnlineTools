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
}