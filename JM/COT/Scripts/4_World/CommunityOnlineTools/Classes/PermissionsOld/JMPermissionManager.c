#ifndef CF_MODULE_PERMISSIONS
class JMPermissionManager
{
	ref map< string, ref JMPlayerInstance > Players;
	ref map< string, ref JMRole > Roles;

	ref map< string, string > SteamToGUID;

	ref JMPermission RootPermission;

	private string m_ClientGUID;

	private bool m_MissionLoaded;

	void JMPermissionManager()
	{
		Players = new map< string, ref JMPlayerInstance >;
		Roles = new map< string, ref JMRole >;

		SteamToGUID = new map< string, string >;

		RootPermission = new JMPermission( JMConstants.PERM_ROOT );
		
		if ( IsMissionHost() )
		{
			MakeDirectory( JMConstants.DIR_PF );

			MakeDirectory( JMConstants.DIR_PERMISSIONS );
			MakeDirectory( JMConstants.DIR_PLAYERS );
			MakeDirectory( JMConstants.DIR_ROLES );
		}
		
		Assert_Null( Players );
		Assert_Null( Roles );
		Assert_Null( SteamToGUID );
		Assert_Null( RootPermission );
	}

	void GetRolesAsList( out array< JMRole > roles )
	{
		Assert_Null( Roles );

		if ( roles == NULL )
			roles = new array< JMRole >();

		roles.Copy( Roles.GetValueArray() );

		roles.Debug();
	}

	void GetPermissionsAsList( out array< JMPermission > permissions )
	{
		Assert_Null( RootPermission );

		if ( permissions == NULL )
			permissions = new array< JMPermission >();

		GetPermissionsAsList( RootPermission, "", permissions );
	}

	void GetPermissionsAsList( JMPermission permission, out array< JMPermission > permissions )
	{
		Assert_Null( RootPermission );

		if ( permissions == NULL )
			permissions = new array< JMPermission >();

		GetPermissionsAsList( permission, "", permissions );
	}

	private void GetPermissionsAsList( JMPermission permission, string indent, inout array< JMPermission > permissions, bool last = false )
	{
		Assert_Null( RootPermission );
		Assert_Null( permission );

		if (permission.Parent && permission.Parent.Name != "ROOT")
		{
			if (last)
			{
				permission.Indent = indent + " \\- ";
				indent += "    ";
			}
			else
			{
				permission.Indent = indent + " |- ";
				indent += " |  ";
			}
		}

		for ( int i = 0; i < permission.Children.Count(); ++i )
		{
			permissions.Insert( permission.Children[i] );

			GetPermissionsAsList( permission.Children[i], indent, permissions, i == permission.Children.Count() - 1 );
		}
	}

	string GetClientGUID()
	{
		return m_ClientGUID;
	}

	void SetClientGUID( string guid )
	{
		m_ClientGUID = guid;
	}

	JMPlayerInstance GetClientPlayer()
	{
		return Players.Get( m_ClientGUID );
	}

	void ResetMission()
	{
		Assert_Null( Players );
		Assert_Null( Roles );

		if ( !IsMissionClient() )
			return;

		Players.Clear();
		Roles.Clear();
	}

	void CreateFakePlayers()
	{
		for ( int i = 0; i < 200; i++ )
		{
			CreateFakePlayer( i );
		}
	}

	private void CreateFakePlayer( int i )
	{
		JMPlayerInstance instance = new JMPlayerInstance( NULL );
		instance.MakeFake( "GFake" + i, "SFake" + i, "NFake" + i );
		Players.Insert( instance.GetGUID(), instance );
	}

	/**
	 * This uses GUIDs now.
	 */
	array< JMPlayerInstance > GetPlayers( array< string > guidsGetPlayers = NULL )
	{
		Assert_Null( Players );
		
		if ( guidsGetPlayers == NULL || !GetGame().IsMultiplayer() )
		{
			return Players.GetValueArray();
		}

		array< JMPlayerInstance > players = new array< JMPlayerInstance >;

		for ( int i = 0; i < guidsGetPlayers.Count(); i++ )
		{
			JMPlayerInstance instance = Players.Get( guidsGetPlayers[i] );
			if ( instance )
				players.Insert( instance );
		}

		return players;
	}

	void RegisterPermission( string permission )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RegisterPermission").Add(permission);
		#endif

		Assert_Null( RootPermission );

		if (m_MissionLoaded)
			Error("Cannot register new permissions once mission is loaded!");
		else
			RootPermission.AddPermission( permission, JMPermissionType.INHERIT, false );
	}

	array< string > Serialize()
	{
		auto trace = CF_Trace_1(this, "Serialize").Add(RootPermission.Children.Count());

		Assert_Null( RootPermission );

		array< string > data = new array< string >;
		RootPermission.Serialize( data );
		return data;
	}

	JMPermission GetRootPermission()
	{
		Assert_Null( RootPermission );

		return RootPermission;
	}

	bool HasPermission( string permission )
	{
		if ( IsMissionClient() ) 
		{
			if ( IsMissionHost() )
				return true;
			
			JMPlayerInstance instance = GetClientPlayer();
			if ( !instance /*Assert_Null( instance )*/ )
				return false;

			return instance.HasPermission( permission );
		}

		Error( "JMPermissionManager::HasPermission( permission = " + permission + " ) bool; was called on server!" );

		return false;
	}

	bool HasPermission( string permission, PlayerIdentity ihp )
	{
		if ( IsMissionOffline() )
		{
			return true;
		}

		JMPlayerInstance instance;
		return HasPermission( permission, ihp, instance );
	}

	bool HasPermission( string permission, PlayerIdentity identity, out JMPlayerInstance instance )
	{		
		if ( IsMissionClient() ) 
		{
			instance = GetClientPlayer();

			if ( IsMissionHost() )
			{
				return true;
			}
			
			if ( Assert_Null( instance ) )
			{
				return false;
			}

			return instance.HasPermission( permission );
		}

		instance = Players.Get( identity.GetId() );
		if ( Assert_Null( instance ) )
		{
			Print( "JMPlayerInstance does not exist for " + identity.GetId() );
			return false;
		}

		return instance.HasPermission( permission );
	}

	bool HasPermissions( TStringArray permissions, PlayerIdentity identity, out JMPlayerInstance instance, bool requireAll = true )
	{
		foreach (string permission: permissions)
		{
			if (HasPermission(permission, identity, instance))
			{
				if (!requireAll)
					return true;
			}
			else if (requireAll)
			{
				return false;
			}
		}

		return true;
	}

	bool HasQuickActionAccess(PlayerBase player)
	{
	#ifdef SERVER
		PlayerIdentity identity = player.GetIdentity();
		if (identity && GetCommunityOnlineToolsBase().IsActive(identity) && HasPermission("Actions.QuickActions", identity))
	#else
		if (GetCommunityOnlineToolsBase().IsActive() && HasPermission("Actions.QuickActions"))
	#endif
			return true;

		return false;
	}

	bool OnClientConnected( PlayerIdentity ident, out JMPlayerInstance inst )
	{
		Assert_Null( RootPermission );
		Assert_Null( Players );

		string guid = "";
		
		if ( ident == NULL )
		{
			if ( GetGame().IsMultiplayer() )
			{
				return false;
			}

			guid = JMConstants.OFFLINE_GUID;
		} else
		{
			guid = ident.GetId();
		
			SteamToGUID.Insert( ident.GetPlainId(), guid );
		}

		RemoveSyncedToClient(guid);

		inst = Players.Get( guid );
		if ( inst )
		{
			return true;
		}

		inst = new JMPlayerInstance( ident );

		//inst.CopyPermissions( RootPermission );
		inst.Load();

		Players.Insert( guid, inst );

		// PMPrint();

		return true;
	}

	bool OnClientDisconnected( string guid, out JMPlayerInstance inst )
	{
		Assert_Null( Players );

		inst = Players.Get( guid );
		if ( inst )
		{
			inst.Save();

			Players.Remove( guid );

			RemoveSyncedToClient(guid);

			return true;
		} else
		{
			return false;
		}
	}

	private void RemoveSyncedToClient(string guid)
	{
		foreach (JMPlayerInstance player: Players)
		{
			player.RemoveSyncedToClient(guid);
		}
	}

	void PMPrint()
	{
		if ( IsMissionClient() && false )
		{
			// Print( "Printing all authenticated players!" );

			for ( int i = 0; i < Players.Count(); i++ )
			{
				Players.GetElement( i ).DebugPrint();
			}
		}
	}

	string GetGUIDForSteam( string uid )
	{
		return SteamToGUID.Get( uid );
	}

	JMPlayerInstance GetPlayer( string guid )
	{
		Assert_Null( Players );

		return Players.Get( guid );
	}

	JMPlayerInstance UpdatePlayer( string guid, ParamsReadContext ctx, PlayerBase playerUpdatePlayer = NULL )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "UpdatePlayer").Add(guid).Add(playerUpdatePlayer.ToString());
		#endif

		JMPlayerInstance instance = GetPlayer( guid );

		if ( !instance )
		{
			if ( !IsMissionClient() )
			{
				return NULL;
			}

			instance = new JMPlayerInstance( NULL, guid );
			Players.Insert( guid, instance );
		}

		instance.OnRecieve( ctx );

		if ( IsMissionClient() )
		{
			if ( m_ClientGUID == guid )
			{
				GetModuleManager().OnClientPermissionsUpdated();
			}

			instance.PlayerObject = playerUpdatePlayer;
		}

		return instance;
	}

	protected bool IsValidFolderForRoles( string name, FileAttr attributes )
	{
		string extenstion = ".txt";
		int strLength = name.Length();

		if ( name == extenstion )
			return false;

		if ( attributes & FileAttr.DIRECTORY )
			return false;

		if ( name == "" )
			return false;

		return true;
	}
	
	JMRole CreateRole( string name, array< string > data )
	{
		Assert_Null( Roles );

		JMRole role = GetRole( name );

		if ( !role )
		{
			role = new JMRole( name );

			Roles.Insert( name, role );
		}

		role.SerializedData.Copy( data );
		role.Deserialize();

		role.Save();

		return role;
	}

	bool LoadRole( string name, out JMRole role )
	{
		Assert_Null( Roles );

		role = GetRole( name );

		if ( !role )
		{
			role = new JMRole( name );
			Roles.Insert( name, role );

			return false;
		}

		return true;
	}

	void LoadRoleFromFile( string name )
	{
		Assert_Null( Roles );

		JMRole role = new JMRole( name );
		
		if ( role.Load() )
		{
			Roles.Insert( name, role );
		}
	}

	void LoadRoles()
	{
		string sName = "";
		FileAttr oFileAttr = FileAttr.INVALID;
		FindFileHandle oFileHandle = FindFile( JMConstants.DIR_ROLES + "*" + JMConstants.EXT_ROLE, sName, oFileAttr, FindFileFlags.ALL );

		if ( sName != "" )
		{
			if ( IsValidFolderForRoles( sName, oFileAttr ) )
			{
				LoadRoleFromFile( sName.Substring( 0, sName.Length() - 4 ) );
			}

			while ( FindNextFile( oFileHandle, sName, oFileAttr) )
			{
				if ( IsValidFolderForRoles( sName, oFileAttr ) )
				{
					LoadRoleFromFile( sName.Substring( 0, sName.Length() - 4 ) );
				}
			}
		}
	}

	bool RoleExists( string role )
	{
		return Roles.Contains( role );
	}

	bool IsRole( string role )
	{
		return Roles.Contains( role );
	}

	JMRole GetRole( string name )
	{
		Assert_Null( Roles );

		return Roles.Get( name );
	}

	void SetMissionLoaded()
	{
		m_MissionLoaded = true;
	}
};


ref JMPermissionManager g_cot_PermissionsManager;

JMPermissionManager GetPermissionsManager()
{
	if ( !g_cot_PermissionsManager )
	{
		g_cot_PermissionsManager = new JMPermissionManager();
	}

	Assert_Null( g_cot_PermissionsManager );

	return g_cot_PermissionsManager;
}
#endif