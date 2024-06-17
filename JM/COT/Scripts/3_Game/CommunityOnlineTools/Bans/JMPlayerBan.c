class JMPlayerBan
{
	string Message;
    int BanDuration;

	static JMPlayerBan Load(string userID)
	{
		if (!FileExist(JMConstants.DIR_BANS))
			MakeDirectory(JMConstants.DIR_BANS);
		
        string filepath = JMConstants.DIR_BANS + FileReadyStripName( userID ) + JMConstants.EXT_PLAYER;
		if (FileExist(filepath))
        {
		    JMPlayerBan userFile = new JMPlayerBan();
			JsonFileLoader<JMPlayerBan>.JsonLoadFile(filepath, userFile);
		    return userFile;
        }

        return NULL;
	}

	static void Save(JMPlayerBan data, string userID)
	{
		if (!FileExist(JMConstants.DIR_BANS))
			MakeDirectory(JMConstants.DIR_BANS);

        string filepath = JMConstants.DIR_BANS + FileReadyStripName( userID ) + JMConstants.EXT_PLAYER;
        JsonFileLoader<JMPlayerBan>.JsonSaveFile(filepath, data);
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
