#ifndef CF_MODULE_PERMISSIONS
class JMRole : Managed
{
	ref JMPermission RootPermission;

	string Name;

	ref array< string > SerializedData;

	void JMRole( string name )
	{
		Name = name;

		RootPermission = new JMPermission( JMConstants.PERM_ROOT );
		RootPermission.CopyPermissions(GetPermissionsManager().RootPermission);
		
		SerializedData = new array< string >;
	}

	void CopyPermissions( JMPermission copy )
	{
		auto trace = CF_Trace_0(this, "CopyPermissions");

		RootPermission.CopyPermissions(copy);
	}

	void ClearPermissions()
	{
		RootPermission = new JMPermission( JMConstants.PERM_ROOT );
	}

	void AddPermission( string permission, JMPermissionType type = JMPermissionType.INHERIT )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "AddPermission").Add(permission).Add(typename.EnumToString(JMPermissionType, type));
		#endif

		RootPermission.AddPermission( permission, type );
	}

	bool HasPermission( string permission, out JMPermissionType permType )
	{
		return RootPermission.HasPermission( permission, permType );
	}

	array< string > Serialize()
	{
		auto trace = CF_Trace_2(this, "Serialize").Add(Name).Add(RootPermission.Children.Count());

		SerializedData.Clear();

		RootPermission.Serialize( SerializedData );

		return SerializedData;
	}

	void Deserialize()
	{
		auto trace = CF_Trace_2(this, "Deserialize").Add(Name).Add(SerializedData.Count());

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
		auto trace = CF_Trace_1(this, "Save").Add(Name);

		string filename = FileReadyStripName( Name );

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
			
			return true;
		} else
		{
			return false;
		}
	}

	bool Load()
	{
		auto trace = CF_Trace_1(this).Add(Name);

		string filename = FileReadyStripName( Name );
		FileHandle file = OpenFile( JMConstants.DIR_ROLES + filename + JMConstants.EXT_ROLE, FileMode.READ );

		if ( file != 0 )
		{
			string line;

			while ( FGets( file, line ) > 0 )
			{
				AddPermission( line );
			}

			CloseFile( file );
		} else
		{
			return false;
		}
		
		return true;
	}

	void DebugPrint()
	{
		RootPermission.DebugPrint( 0 );
	}
};
#endif