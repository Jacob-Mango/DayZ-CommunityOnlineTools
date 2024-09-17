class JMPlayerBan
{
	string Message;
    int BanDuration;

	static JMPlayerBan Load(string guid, string steamid = string.Empty)
	{
		if (!FileExist(JMConstants.DIR_BANS))
			MakeDirectory(JMConstants.DIR_BANS);
		
		JMPlayerBan userFile = new JMPlayerBan();

        string filepath = JMConstants.DIR_BANS + FileReadyStripName( guid ) + JMConstants.EXT_BAN;
		if (FileExist(filepath))
        {
			JsonFileLoader<JMPlayerBan>.JsonLoadFile(filepath, userFile);
		    return userFile;
        }
		
        filepath = JMConstants.DIR_BANS + FileReadyStripName( steamid ) + JMConstants.EXT_BAN;
		if (FileExist(filepath))
        {
			JsonFileLoader<JMPlayerBan>.JsonLoadFile(filepath, userFile);
		    return userFile;
        }

        return NULL;
	}

	static void Save(JMPlayerBan data, string guid)
	{
		if (!FileExist(JMConstants.DIR_BANS))
			MakeDirectory(JMConstants.DIR_BANS);

        string filepath = JMConstants.DIR_BANS + FileReadyStripName( guid ) + JMConstants.EXT_BAN;
        JsonFileLoader<JMPlayerBan>.JsonSaveFile(filepath, data);
	}

	static bool DeleteBanFile(string guid, string steamid = string.Empty)
	{
        string filepath = JMConstants.DIR_BANS + FileReadyStripName( guid ) + JMConstants.EXT_BAN;
		if (DeleteFile(filepath))
		    return true;
		
        filepath = JMConstants.DIR_BANS + FileReadyStripName( steamid ) + JMConstants.EXT_PLAYER;
		if (DeleteFile(filepath))
		    return true;

		return false;
	}

	static string FileReadyStripName( string name )
	{
		name.Replace( "\\", "" );
		name.Replace( "/", "" );
		name.Replace( "=", "" );
		name.Replace( "+", "" );

		return name;
	}
};
