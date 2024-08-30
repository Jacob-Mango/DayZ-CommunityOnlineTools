class JMLoadout
{
	ref array< ref JMLoadoutItem > m_ItemData;
	
	static JMLoadout Load(string filename)
	{
		if (!FileExist(JMConstants.DIR_LOADOUTS))
			MakeDirectory(JMConstants.DIR_LOADOUTS);

        string filepath = JMConstants.DIR_LOADOUTS + FileReadyStripName( filename ) + JMConstants.EXT_LOADOUT;
		if (FileExist(filepath))
        {
			JMLoadout userFile = new JMLoadout();
			JsonFileLoader<JMLoadout>.JsonLoadFile(filepath, userFile);
		    return userFile;
        }

        return NULL;
	}

	static void Save(JMLoadout data, string filename)
	{
		if (!FileExist(JMConstants.DIR_LOADOUTS))
			MakeDirectory(JMConstants.DIR_LOADOUTS);

        string filepath = JMConstants.DIR_LOADOUTS + FileReadyStripName( filename ) + JMConstants.EXT_LOADOUT;
        JsonFileLoader<JMLoadout>.JsonSaveFile(filepath, data);
	}

	static bool Delete(string filename)
	{
		if (!FileExist(JMConstants.DIR_LOADOUTS))
			MakeDirectory(JMConstants.DIR_LOADOUTS);

        string filepath = JMConstants.DIR_LOADOUTS + FileReadyStripName( filename ) + JMConstants.EXT_LOADOUT;
		if (DeleteFile(filepath))
		    return true;

		return false;
	}

	static bool Rename(string oldFilename, string newFilename)
	{
        string filepath = JMConstants.DIR_LOADOUTS + FileReadyStripName( oldFilename ) + JMConstants.EXT_LOADOUT;
		if (FileExist(filepath))
		{
        	string newfilepath = JMConstants.DIR_LOADOUTS + FileReadyStripName( newFilename ) + JMConstants.EXT_LOADOUT;
			if (CopyFile(filepath, newfilepath))
			{
		    	return DeleteFile(filepath);
			}
		}

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
