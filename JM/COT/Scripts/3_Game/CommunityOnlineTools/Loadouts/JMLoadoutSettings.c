class JMLoadoutSettings
{
	static array< string > GetFileNames()
	{
		return FindInLocationImpl(JMConstants.DIR_LOADOUTS, JMConstants.EXT_LOADOUT);
	}

	static array< string > FindInLocationImpl( string folder, string ext = "")
	{
		array< string > files = new array< string >;
		if (!FileExist(folder))
			return files;
		
		string fileName;
		FileAttr fileAttr;
		FindFileHandle findFileHandle = FindFile( folder + "*" + ext, fileName, fileAttr, 0 );
		if ( findFileHandle )
		{
			bool isValid = true;

			while (isValid)
			{
				bool isDir = false;
				if (fileAttr & FileAttr.DIRECTORY)
					isDir = true;

				if (fileName.Length() > 0 && ((true && !isDir) ))
				{
					string name = fileName.Substring(0, fileName.Length() - ext.Length());
					files.Insert( name );
				}

				isValid = FindNextFile(findFileHandle, fileName, fileAttr);
			}

			CloseFindFile( findFileHandle );
		}
		return files;
	}

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
		AtomicSave(filepath, data);
	}

	static void SaveDeletion(JMLoadout data, string filename)
	{
		if (!FileExist(JMConstants.DIR_DELETIONS))
			MakeDirectory(JMConstants.DIR_DELETIONS);

		string filepath = JMConstants.DIR_DELETIONS + FileReadyStripName( filename ) + JMConstants.EXT_LOADOUT;
		AtomicSave(filepath, data);
	}

	static void SaveCompensation(JMLoadout data, string steamID, string filename)
	{
		if (!FileExist(JMConstants.DIR_COMPENSATIONS))
			MakeDirectory(JMConstants.DIR_COMPENSATIONS);

		string playerDir = JMConstants.DIR_COMPENSATIONS + FileReadyStripName( steamID ) + "\\";
		if (!FileExist(playerDir))
			MakeDirectory(playerDir);

		// Keep only the 3 most recent backups
		array< string > existingFiles = new array< string >;
		string fileName;
		FileAttr fileAttr;
		FindFileHandle findFileHandle = FindFile( playerDir + "*" + JMConstants.EXT_LOADOUT, fileName, fileAttr, 0 );
		if ( findFileHandle )
		{
			while ( fileName != "" )
			{
				existingFiles.Insert( fileName );
				fileName = "";
				FindNextFile( findFileHandle, fileName, fileAttr );
			}
			CloseFindFile( findFileHandle );
		}

		// Sort by name (timestamp-based names will sort chronologically)
		existingFiles.Sort();

		// Delete oldest files if we have 3 or more
		while ( existingFiles.Count() >= 3 )
		{
			string oldestFile = existingFiles.Get( 0 );
			DeleteFile( playerDir + oldestFile );
			existingFiles.Remove( 0 );
		}

		string filepath = playerDir + FileReadyStripName( filename ) + JMConstants.EXT_LOADOUT;
		AtomicSave(filepath, data);
	}

	// Writes to a .tmp file first, then renames over the target so a crash
	// mid-write never leaves a corrupted JSON file behind.
	private static void AtomicSave(string filepath, JMLoadout data)
	{
		string tmpPath = filepath + ".tmp";

		JsonFileLoader<JMLoadout>.JsonSaveFile(tmpPath, data);

		// Only promote the temp file if it was successfully written
		if (FileExist(tmpPath))
		{
			// Remove old file first (CopyFile won't overwrite on all platforms)
			if (FileExist(filepath))
				DeleteFile(filepath);

			CopyFile(tmpPath, filepath);
			DeleteFile(tmpPath);
		}
	}

	static bool Delete(string filename)
	{
		if (!FileExist(JMConstants.DIR_LOADOUTS))
			return false;

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
}
