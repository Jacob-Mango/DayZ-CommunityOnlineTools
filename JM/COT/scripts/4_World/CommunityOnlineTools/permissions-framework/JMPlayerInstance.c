class JMPlayerInstance
{
	autoptr JMPermission RootPermission;
	autoptr array< string > Roles;

	PlayerBase PlayerObject;

	autoptr JMPlayerInformation Data;

	private autoptr JMPlayerSerialize m_PlayerFile;

	void JMPlayerInstance( PlayerIdentity identity )
	{
		PlayerObject = NULL;

		Data = new JMPlayerInformation;

		if ( identity && GetGame().IsServer() )
		{
			Data.SGUID = identity.GetId();
			Data.SSteam64ID = identity.GetPlainId();
			Data.SName = identity.GetName();
		} else if ( IsMissionOffline() )
		{
			Data.SGUID = JMConstants.OFFLINE_GUID;
			Data.SSteam64ID = JMConstants.OFFLINE_STEAM;
			Data.SName = JMConstants.OFFLINE_NAME;
		}

		RootPermission = new JMPermission( JMConstants.PERM_ROOT );
		Roles = new array< string >;
	}

	void ~JMPlayerInstance()
	{
		delete RootPermission;
	}

	void MakeFake( string gid, string sid, string nid )
	{
		Data.SGUID = gid;
		Data.SSteam64ID = sid;
		Data.SName = nid;
	}

	void SwapData( JMPlayerInformation newData )
	{
		Data.Copy( newData );
	}

	bool CanSendData()
	{
		return PlayerObject != NULL; 
	}

	string GetGUID()
	{
		return Data.SGUID;
	}

	string GetSteam64ID()
	{
		return Data.SSteam64ID;
	}

	string GetName()
	{
		return Data.SName;
	}

	void UpdatePlayerData()
	{
		if ( !GetGame().IsServer() )
			return;

		if ( IsMissionOffline() )
		{
			PlayerObject = PlayerBase.Cast( GetGame().GetPlayer() );
		}

		if ( PlayerObject )
		{
			Data.Load( PlayerObject );
		}
	}

	void CopyPermissions( ref JMPermission copy )
	{
		ref array< string > data = new array< string >;
		copy.Serialize( data );

		for ( int i = 0; i < data.Count(); i++ )
		{
			RootPermission.AddPermission( data[i], JMPermissionType.INHERIT );
		}
	}

	void ClearPermissions()
	{
		delete RootPermission;

		RootPermission = new JMPermission( JMConstants.PERM_ROOT );
	}

	void LoadPermissions( array< string > permissions )
	{
		// Print( "LoadPermissions" );
		// Print( permissions );
		ClearPermissions();

		for ( int i = 0; i < permissions.Count(); i++ )
		{
			AddPermission( permissions[i] );
		}

		Save();
	}

	void AddPermission( string permission, JMPermissionType type = JMPermissionType.INHERIT )
	{
		// Print( "AddPermission" );
		// Print( permission );
		// Print( type );
		RootPermission.AddPermission( permission, type );
	}

	void LoadRoles( array< string > roles )
	{
		ClearRoles();

		for ( int i = 0; i < roles.Count(); i++ )
		{
			AddRole( roles[i] );
		}

		Save();
	}

	void AddRole( string role )
	{
		if ( !GetPermissionsManager().IsRole( role ) )
			return;

		if ( Roles.Find( role ) < 0 )
		{
			Roles.Insert( role );
		}
	}

	bool HasRole( string role )
	{
		return Roles.Find( role ) >= 0;
	}

	// doesn't check through roles.
	JMPermissionType GetRawPermissionType( string permission )
	{
		JMPermissionType permType;
		RootPermission.HasPermission( permission, permType );
		return permType;
	}

	void ClearRoles()
	{
		Roles.Clear();

		AddRole( "everyone" );
	}

	bool HasPermission( string permission )
	{
		// RootPermission.DebugPrint( 0 );
		
		JMPermissionType permType;
		bool hasPermission = RootPermission.HasPermission( permission, permType );
		
		// Print( "JMPlayerInstance::HasPermission - hasPermission=" + hasPermission );
		if ( hasPermission )
		{
			return true;
		}

		// Print( "JMPlayerInstance::HasPermission - permType=" + permType );
		if ( permType == JMPermissionType.DISALLOW )
		{
			return false;
		}

		for ( int j = 0; j < Roles.Count(); j++ )
		{
			hasPermission = GetPermissionsManager().HasRolePermission( Roles[j], permission, permType );

			// Print( "JMPlayerInstance::HasPermission - role[" + j + "]=" + Roles[j] );
			// Print( "JMPlayerInstance::HasPermission - permType=" + permType );
			// Print( "JMPlayerInstance::HasPermission - hasPermission=" + hasPermission );

			if ( hasPermission )
			{
				return true;
			}
		}

		return false;
	}

	void Serialize()
	{
		Data.APermissions.Clear();
		Data.ARoles.Clear();

		RootPermission.Serialize( Data.APermissions );
		Data.ARoles.Copy( Roles );
	}

	void Deserialize()
	{
		ClearRoles();
		ClearPermissions();
		
		for ( int i = 0; i < Data.APermissions.Count(); i++ )
		{
			AddPermission( Data.APermissions[i] );
		}

		for ( int j = 0; j < Data.ARoles.Count(); j++ )
		{
			AddRole( Data.ARoles[j] );
		}
	}

	void Save()
	{
		if ( !GetGame().IsServer() )
			return;

		Serialize();

		m_PlayerFile.Roles.Clear();
		m_PlayerFile.Roles.Copy( Data.ARoles );
		m_PlayerFile.Save();

		FileHandle file = OpenFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( Data.SGUID ) + JMConstants.EXT_PERMISSION, FileMode.WRITE );

		if ( file != 0 )
		{
			string line;
			for ( int i = 0; i < Data.APermissions.Count(); i++ )
			{
				FPrintln( file, Data.APermissions[i] );
			}
			
			CloseFile(file);
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

	protected bool ReadPermissions( string filename )
	{
		if ( !FileExist( filename ) )
			return false;

		FileHandle file = OpenFile( filename, FileMode.READ );

		if ( file < 0 )
			return false;

		array< string > data = new array< string >;

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

		return true;
	}

	void Load()
	{
		if ( !IsMissionHost() )
			return;

		UpdatePlayerData();

		JMPlayerSerialize.Load( Data, m_PlayerFile );

		for ( int j = 0; j < m_PlayerFile.Roles.Count(); j++ )
		{
			AddRole( m_PlayerFile.Roles[j] );
		}

		if ( !ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( Data.SGUID ) + JMConstants.EXT_PERMISSION ) )
		{
			if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( Data.SGUID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT ) )
			{
				DeleteFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( Data.SGUID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT );
			} else if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( Data.SSteam64ID ) + JMConstants.EXT_PERMISSION ) )
			{
				DeleteFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( Data.SSteam64ID ) + JMConstants.EXT_PERMISSION );
			} else if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( Data.SSteam64ID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT ) )
			{
				DeleteFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( Data.SSteam64ID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT );
			}
		}

		Save();
	}

	void DebugPrint()
	{
		// Print( "  SGUID: " + Data.SGUID );
		// Print( "  SSteam64ID: " + Data.SSteam64ID );
		// Print( "  SName: " + Data.SName );

		RootPermission.DebugPrint( 2 );
	}
}