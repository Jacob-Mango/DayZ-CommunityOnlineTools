class PlayerFile
{
	ref array< string > Names;
	string GUID;
	string Steam64ID;

	ref array< string > Roles;

	void PlayerFile()
	{
		Names = new ref array< string >;
		Roles = new ref array< string >;
	}

	bool Load( PlayerIdentity identity )
	{
		if ( identity == NULL ) return false;
		
		ref PlayerFile playerFile = new ref PlayerFile;
		string file = PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + identity.GetPlainId() + ".json";
		if ( FileExist( file ) )
		{
			JsonFileLoader<PlayerFile>.JsonLoadFile( file, playerFile );

			if ( playerFile == NULL )
			{
				Roles.Insert( "everyone" );
				return false;
			}

			Names = playerFile.Names;
			GUID = playerFile.GUID;
			Steam64ID = playerFile.Steam64ID;
			Roles = playerFile.Roles;

			bool requiresSaving = false;

			if ( Steam64ID != identity.GetPlainId() )
			{
				Steam64ID = identity.GetPlainId();
				requiresSaving = true;
			}

			if ( GUID != identity.GetId() )
			{
				GUID = identity.GetId();
				requiresSaving = true;
			}

			if ( Names.Find( identity.GetName() ) < 0 )
			{   
				Names.Insert( identity.GetName() );
				requiresSaving = true;
			}

			if ( requiresSaving )
			{
				Save();
			}
			
			return true;
		} else 
		{
			Roles.Insert( "everyone" );
			return false;
		}
	}

	void Save()
	{
		JsonFileLoader<PlayerFile>.JsonSaveFile( PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + Steam64ID + ".json", this );
	}
}