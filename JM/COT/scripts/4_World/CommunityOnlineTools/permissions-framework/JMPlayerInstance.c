class JMPlayerInstance: Managed
{
	ref JMPermission RootPermission;
	ref array< string > Roles;

	PlayerBase PlayerObject;
	PlayerIdentity IdentityPlayer;

	ref JMPlayerInformation Data;

	protected ref JMPlayerSerialize m_PlayerFile;

	protected bool m_HasPermissions;
	protected bool m_HasPlayerData;

	void JMPlayerInstance( PlayerIdentity identity )
	{
		PlayerObject = NULL;

		Data = new JMPlayerInformation;

		IdentityPlayer = identity;

		if ( IdentityPlayer && GetGame().IsServer() )
		{
			Data.SName = IdentityPlayer.GetName();
			Data.SGUID = IdentityPlayer.GetId();
			Data.SSteam64ID = IdentityPlayer.GetPlainId();
		}

		RootPermission = new JMPermission( Data.SSteam64ID );
		Roles = new array< string >;
	}

	void ~JMPlayerInstance()
	{
		delete RootPermission;
	}

	void SwapData( ref JMPlayerInformation newData )
	{
		Data = newData;
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

		if ( IdentityPlayer )
		{
			Data.IPingMin = IdentityPlayer.GetPingMin();
			Data.IPingMax = IdentityPlayer.GetPingMax();
			Data.IPingAvg = IdentityPlayer.GetPingAvg();
			
			Data.SSteam64ID = IdentityPlayer.GetPlainId();
			Data.SGUID = IdentityPlayer.GetId();
			Data.SName = IdentityPlayer.GetName();

			PlayerObject = GetPlayerObjectByIdentity( IdentityPlayer );

			if ( PlayerObject )
			{
				PlayerObject.SetAuthenticatedPlayer( this );
				Data.Load( PlayerObject );
			}
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

		RootPermission = new JMPermission( Data.SSteam64ID, NULL );

		m_HasPermissions = false;
	}

	void AddPermission( string permission, JMPermissionType type = JMPermissionType.INHERIT )
	{
		RootPermission.AddPermission( permission, type );

		m_HasPermissions = true;
	}

	bool HasPermission( string permission )
	{
		// RootPermission.DebugPrint( 0 );
		
		GetLogger().Log( "Checking " + GetSteam64ID() + " for permission " + permission, "JM_COT_PermissionFramework" );
		
		JMPermissionType permType;
		bool hasPermission = RootPermission.HasPermission( permission, permType );
		
		GetLogger().Log( "    " +  GetSteam64ID() + " -> " + hasPermission + " Type " + permType, "JM_COT_PermissionFramework" );

		if ( hasPermission )
		{
			GetLogger().Log( "Returned " + GetSteam64ID() + " true", "JM_COT_PermissionFramework" );
			return true;
		}

		if ( permType == JMPermissionType.DISALLOW )
		{
			GetLogger().Log( "Returned " + GetSteam64ID() + " false", "JM_COT_PermissionFramework" );
			return false;
		}

		for ( int j = 0; j < Roles.Count(); j++ )
		{
			hasPermission = GetPermissionsManager().HasRolePermission( Roles[j], permission, permType );

			GetLogger().Log( "    " +  Roles[j] + " -> " + hasPermission + " Type " + permType, "JM_COT_PermissionFramework" );

			if ( hasPermission )
			{
				GetLogger().Log( "Returned " + GetSteam64ID() + " true", "JM_COT_PermissionFramework" );
				return true;
			}
		}

		GetLogger().Log( "Returned " + GetSteam64ID() + " false", "JM_COT_PermissionFramework" );
		return false;
	}

	void AddRole( string role, bool updateHasData = true )
	{
		if ( !GetPermissionsManager().IsRole( role ) )
			return;

		m_HasPlayerData = true;

		Roles.Insert( role );

		if ( updateHasData )
		{
			m_HasPlayerData = true;
		}
	}

	void ClearRoles()
	{
		Roles.Clear();

		AddRole( "everyone" );
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

	string FileReadyStripName( string name )
	{
		name.Replace( "\\", "" );
		name.Replace( "/", "" );
		name.Replace( "=", "" );
		name.Replace( "+", "" );

		return name;
	}

	void Save()
	{
		if ( !GetGame().IsServer() )
			return;

		Serialize();

		if ( m_HasPlayerData )
		{   
			m_PlayerFile.Roles.Clear();
			m_PlayerFile.Roles.Copy( Data.ARoles );

			m_PlayerFile.Save();
		}

		if ( m_HasPermissions )
		{
			string filename = FileReadyStripName( Data.SSteam64ID );

			GetLogger().Log( "Saving permissions and player data for " + filename, "JM_COT_PermissionFramework" );
			FileHandle file = OpenFile( JMConstants.DIR_PERMISSIONS + filename + JMConstants.EXT_PLAYER, FileMode.WRITE );

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
		if ( !GetGame().IsServer() )
			return;

		m_HasPlayerData = JMPlayerSerialize.Load( Data, m_PlayerFile );

		for ( int j = 0; j < m_PlayerFile.Roles.Count(); j++ )
		{
			AddRole( m_PlayerFile.Roles[j], false );
		}

		GetLogger().Log( "Loading permissions for " + Data.SSteam64ID, "JM_COT_PermissionFramework" );

		m_HasPermissions = false;

		if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + Data.SSteam64ID + JMConstants.EXT_PERMISSION ) )
		{
			m_HasPermissions = true;
		} else if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + Data.SSteam64ID + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT ) )
		{
			m_HasPermissions = true;

			DeleteFile( JMConstants.DIR_PERMISSIONS + Data.SSteam64ID + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT );
			
			Save();
		} else if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + Data.SGUID + JMConstants.EXT_PERMISSION ) )
		{
			m_HasPermissions = true;
		} else if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + Data.SGUID + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT ) )
		{
			m_HasPermissions = true;

			DeleteFile( JMConstants.DIR_PERMISSIONS + Data.SGUID + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT );
			
			Save();
		}

		UpdatePlayerData();
	}

	void DebugPrint()
	{
		GetLogger().Log( "Printing permissions for " + Data.SSteam64ID, "JM_COT_PermissionFramework" );

		RootPermission.DebugPrint( 0 );
	}
}