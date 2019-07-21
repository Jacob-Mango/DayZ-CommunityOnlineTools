class JMPlayerSerialize
{
	[NonSerialized()]
	string m_FileName;

	ref array< string > Roles;

	void JMPlayerSerialize()
	{
		Roles = new ref array< string >;
	}

	static bool Load( JMPlayerInformation data, out JMPlayerSerialize playerFile )
	{
		playerFile = new ref JMPlayerSerialize;

		playerFile.m_FileName = PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + data.SSteam64ID + ".json";
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );

			playerFile.Save();
			return true;
		}
		
		playerFile.m_FileName = PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + data.SGUID + ".json";
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );

			playerFile.Save();
			return true;
		}

		playerFile.m_FileName = PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + data.SSteam64ID + ".json.txt";
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );

			DeleteFile( playerFile.m_FileName );
			playerFile.m_FileName = PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + data.SSteam64ID + ".json";

			playerFile.Save();
			return true;
		}
		
		playerFile.m_FileName = PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + data.SGUID + ".json.txt";
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );

			DeleteFile( playerFile.m_FileName );
			playerFile.m_FileName = PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + data.SGUID + ".json";

			playerFile.Save();
			return true;
		}

		playerFile.m_FileName = PERMISSION_FRAMEWORK_DIRECTORY + "Players\\" + data.SSteam64ID + ".json";
		playerFile.Roles.Insert( "everyone" );
		return false;
	}

	void Save()
	{
		JsonFileLoader<JMPlayerSerialize>.JsonSaveFile( m_FileName, this );
	}
}