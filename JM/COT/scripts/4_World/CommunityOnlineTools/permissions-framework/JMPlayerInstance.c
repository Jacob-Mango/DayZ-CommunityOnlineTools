class JMPlayerInstance
{
	autoptr JMPermission RootPermission;
	autoptr array< string > Roles;

	PlayerBase PlayerObject;
	PlayerIdentity IdentityPlayer;

	autoptr JMPlayerInformation Data;

	private autoptr JMPlayerSerialize m_PlayerFile;

	void JMPlayerInstance( PlayerIdentity identity )
	{
		PlayerObject = NULL;

		Data = new JMPlayerInformation;

		IdentityPlayer = identity;

		if ( IdentityPlayer && GetGame().IsServer() )
		{
			Data.SGUID = IdentityPlayer.GetId();
			Data.SSteam64ID = IdentityPlayer.GetPlainId();
			Data.SName = IdentityPlayer.GetName();
		}

		if ( !GetGame().IsMultiplayer() )
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

	void SwapData( JMPlayerInformation newData )
	{
		Data.Copy( newData );
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

		if ( !GetGame().IsMultiplayer() )
		{
			PlayerObject = GetGame().GetPlayer();
			
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

		RootPermission = new JMPermission( JMConstants.PERM_ROOT );
	}

	void AddPermission( string permission, JMPermissionType type = JMPermissionType.INHERIT )
	{
		RootPermission.AddPermission( permission, type );
	}

	bool HasPermission( string permission )
	{
		// RootPermission.DebugPrint( 0 );
		
		GetCommunityOnlineToolsBase().LogServer( "Checking " + GetSteam64ID() + " for permission " + permission );
		
		JMPermissionType permType;
		bool hasPermission = RootPermission.HasPermission( permission, permType );
		
		GetCommunityOnlineToolsBase().LogServer( "    " +  GetSteam64ID() + " -> " + hasPermission + " Type " + permType );

		if ( hasPermission )
		{
			GetCommunityOnlineToolsBase().LogServer( "Returned " + GetSteam64ID() + " true" );
			return true;
		}

		if ( permType == JMPermissionType.DISALLOW )
		{
			GetCommunityOnlineToolsBase().LogServer( "Returned " + GetSteam64ID() + " false" );
			return false;
		}

		for ( int j = 0; j < Roles.Count(); j++ )
		{
			hasPermission = GetPermissionsManager().HasRolePermission( Roles[j], permission, permType );

			GetCommunityOnlineToolsBase().LogServer( "    " +  Roles[j] + " -> " + hasPermission + " Type " + permType );

			if ( hasPermission )
			{
				GetCommunityOnlineToolsBase().LogServer( "Returned " + GetSteam64ID() + " true" );
				return true;
			}
		}

		GetCommunityOnlineToolsBase().LogServer( "Returned " + GetSteam64ID() + " false" );
		return false;
	}

	void AddRole( string role )
	{
		if ( !GetPermissionsManager().IsRole( role ) )
			return;

		Roles.Insert( role );
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
		Print( "Printing permissions for " + Data.SGUID );

		RootPermission.DebugPrint( 0 );
	}
}