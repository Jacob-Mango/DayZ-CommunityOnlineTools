class JMRole
{
	ref JMPermission RootPermission;

	string Name;

	ref array< string > SerializedData;

	void JMRole( string name )
	{
		Name = name;

		RootPermission = new JMPermission( Name );
		
		SerializedData = new array< string >;
	}

	void ~JMRole()
	{
		delete RootPermission;
	}

	void CopyPermissions( ref JMPermission copy )
	{
		ref array< string > data = new array< string >;
		copy.Serialize( data );

		for ( int i = 0; i < data.Count(); i++ )
		{
			AddPermission( data[i] );
		}
	}

	void ClearPermissions()
	{
		delete RootPermission;

		RootPermission = new JMPermission( Name, NULL );
	}

	void AddPermission( string permission, JMPermissionType type = JMPermissionType.INHERIT )
	{
		RootPermission.AddPermission( permission, type );
	}

	bool HasPermission( string permission, out JMPermissionType permType )
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
		FileHandle file = OpenFile( JMConstants.DIR_ROLES + filename + JMConstants.EXT_ROLE, FileMode.WRITE );
			
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
		FileHandle file = OpenFile( JMConstants.DIR_ROLES + filename + JMConstants.EXT_ROLE, FileMode.READ );
			
		ref array< string > data = new array< string >;

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