class JMPlayerSerialize
{
	[NonSerialized()]
	string m_FileName;

	ref array< string > Roles;

	void JMPlayerSerialize()
	{
		Roles = new array< string >;
	}

	static bool Load( JMPlayerInformation data, out JMPlayerSerialize playerFile )
	{
		playerFile = new JMPlayerSerialize;

		playerFile.m_FileName = JMConstants.DIR_PLAYERS + data.SSteam64ID + JMConstants.EXT_PLAYER;
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );

			playerFile.Save();
			return true;
		}
		
		playerFile.m_FileName = JMConstants.DIR_PLAYERS + data.SGUID + JMConstants.EXT_PLAYER;
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );

			playerFile.Save();
			return true;
		}

		playerFile.m_FileName = JMConstants.DIR_PLAYERS + data.SSteam64ID + JMConstants.EXT_PLAYER + JMConstants.EXT_WINDOWS_DEFAULT;
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );

			DeleteFile( playerFile.m_FileName );
			playerFile.m_FileName = JMConstants.DIR_PLAYERS + data.SSteam64ID + JMConstants.EXT_PLAYER;

			playerFile.Save();
			return true;
		}
		
		playerFile.m_FileName = JMConstants.DIR_PLAYERS + data.SGUID + JMConstants.EXT_PLAYER + JMConstants.EXT_WINDOWS_DEFAULT;
		if ( FileExist( playerFile.m_FileName ) )
		{
			JsonFileLoader<JMPlayerSerialize>.JsonLoadFile( playerFile.m_FileName, playerFile );

			DeleteFile( playerFile.m_FileName );
			playerFile.m_FileName = JMConstants.DIR_PLAYERS + data.SGUID + JMConstants.EXT_PLAYER;

			playerFile.Save();
			return true;
		}

		playerFile.m_FileName = JMConstants.DIR_PLAYERS + data.SSteam64ID + JMConstants.EXT_PLAYER;
		playerFile.Roles.Insert( "everyone" );
		return false;
	}

	void Save()
	{
		JsonFileLoader<JMPlayerSerialize>.JsonSaveFile( m_FileName, this );
	}
}