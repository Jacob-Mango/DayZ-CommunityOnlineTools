class PlayerFile
{
	[NonSerialized()]
	string m_FileName;

	ref array< string > Roles;

	void PlayerFile()
	{
		Roles = new ref array< string >;
	}

	static PlayerFile Load( PlayerData data, out bool exists )
	{
		ref PlayerFile playerFile = new ref PlayerFile;

		playerFile.m_FileName = PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + data.SSteam64ID + ".json";
		
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<PlayerFile>.JsonLoadFile( playerFile.m_FileName, playerFile );
			exists = true;
			return playerFile;
		}
		
		playerFile.m_FileName = PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + data.SGUID + ".json";

		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<PlayerFile>.JsonLoadFile( playerFile.m_FileName, playerFile );
			exists = true;
			return playerFile;
		}

		playerFile.Roles.Insert( "everyone" );
		exists = false;
		return playerFile;
	}

	void Save()
	{
		JsonFileLoader<PlayerFile>.JsonSaveFile( PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + m_FileName + ".json", this );
	}
}