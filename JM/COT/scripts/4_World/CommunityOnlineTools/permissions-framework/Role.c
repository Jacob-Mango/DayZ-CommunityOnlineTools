class Role
{
	const string AUTH_DIRECTORY = PERMISSION_FRAMEWORK_DIRECTORY + "Roles\\";
	const string FILE_TYPE = ".txt";

	ref Permission RootPermission;

	string Name;

	ref array< string > SerializedData;

	void Role( string name )
	{
		Name = name;

		RootPermission = new ref Permission( Name );
		
		SerializedData = new ref array< string >;

		if ( GetGame().IsServer() )
		{
			MakeDirectory( AUTH_DIRECTORY );
		}
	}

	void ~Role()
	{
		delete RootPermission;
	}

	void CopyPermissions( ref Permission copy )
	{
		ref array< string > data = new ref array< string >;
		copy.Serialize( data );

		for ( int i = 0; i < data.Count(); i++ )
		{
			AddPermission( data[i] );
		}
	}

	void ClearPermissions()
	{
		delete RootPermission;

		RootPermission = new ref Permission( Name, NULL );
	}

	void AddPermission( string permission, PermissionType type = PermissionType.INHERIT )
	{
		RootPermission.AddPermission( permission, type );
	}

	bool HasPermission( string permission, out PermissionType permType )
	{
		return RootPermission.HasPermission( permission, permType );
	}

	ref array< string > Serialize()
	{
		SerializedData.Clear();

		RootPermission.Serialize( SerializedData );

		return SerializedData;
	}

	void Deserialize()
	{
		for ( int i = 0; i < SerializedData.Count(); i++ )
		{
			AddPermission( SerializedData[i] );
		}
	}

	string FileReadyStripName( string name )
	{
		name.Replace( "\\", "" );
		name.Replace( "/", "" );
		name.Replace( "=", "" );
		name.Replace( "+", "" );

		return name;
	}

	bool Save()
	{
		string filename = FileReadyStripName( Name );

		GetLogger().Log( "Saving role for " + filename, "JM_COT_PermissionFramework" );
		FileHandle file = OpenFile( AUTH_DIRECTORY + filename + FILE_TYPE, FileMode.WRITE );
			
		Serialize();

		if ( file != 0 )
		{
			string line;

			for ( int i = 0; i < SerializedData.Count(); i++ )
			{
				FPrintln( file, SerializedData[i] );
			}
			
			CloseFile(file);
			
			GetLogger().Log( "Wrote to the roles", "JM_COT_PermissionFramework" );
			return true;
		} else
		{
			GetLogger().Log( "Failed to open the file for the role for writing.", "JM_COT_PermissionFramework" );
			return false;
		}
	}

	bool Load()
	{
		string filename = FileReadyStripName( Name );
		GetLogger().Log( "Loading role " + filename, "JM_COT_PermissionFramework" );
		FileHandle file = OpenFile( AUTH_DIRECTORY + filename + FILE_TYPE, FileMode.READ );
			
		ref array< string > data = new ref array< string >;

		if ( file != 0 )
		{
			string line;

			while ( FGets( file, line ) > 0 )
			{
				data.Insert( line );
			}

			CloseFile( file );

			for ( int i = 0; i < data.Count(); i++ )
			{
				AddPermission( data[i] );
			}

			Serialize();
		} else
		{
			return false;
		}
		
		return true;
	}

	void DebugPrint()
	{
		GetLogger().Log( "Printing permissions for role " + Name, "JM_COT_PermissionFramework" );

		RootPermission.DebugPrint( 0 );
	}
}