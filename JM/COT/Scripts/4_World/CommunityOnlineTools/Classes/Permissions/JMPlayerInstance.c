enum JMPlayerVariables
{
	BLOODY_HANDS = 1,
	GODMODE = 2,
	FROZEN = 4,
	INVISIBILITY = 8,
	UNLIMITED_AMMO = 16,
	UNLIMITED_STAMINA = 32,
	BROKEN_LEGS = 64,
	RECEIVE_DMG_DEALT = 128,
	CANNOT_BE_TARGETED_BY_AI = 256,
	REMOVE_COLLISION = 512,
	ADMIN_NVG = 1024,
	HAS_CUSTOM_SCALE = 2048,
	INVISIBILITY_INTERACTIVE = 4096,
	UNCONSCIOUS = 8192,
	SICK = 16384,
	BLEEDING = 32768,
	DEAD = 65536,
	RAGDOLL = 131072
}

class JMPlayerInstance : Managed
{
#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
	protected ref JMPermission m_RootPermission;
#endif

	protected ref array< string > m_Roles;
	protected ref map<string, string> m_RoleNameRestrictions;
	protected ref map<string, bool> m_SyncedToClient;
	PlayerBase PlayerObject;

	//! protected, not private: DayZ-Expansion's `modded class JMPlayerInstance`
	//! (DayZExpansion_AI, DayZExpansion_Hardline) reads this in its Update()
	//! override to rate-limit faction/reputation netsync. A modded class cannot
	//! touch a private member of the class it mods.
	protected int m_DataLastUpdated;
	protected string m_Name;
	protected string m_GUID;
	protected string m_Steam64ID;
	protected int m_PingMax;
	protected int m_PingMin;
	protected int m_PingAvg;
	protected vector m_Position;
	protected vector m_Orientation;
	protected float m_Health;
	protected float m_Blood;
	protected float m_Shock;
	protected int m_BloodStatType;
	protected float m_Energy;
	protected float m_Water;
	protected float m_HeatComfort;
	protected float m_HeatBuffer;
	protected float m_Wet;
	protected float m_Tremor;
	protected float m_Stamina;
	protected int m_LifeSpanState;
	protected ref map<int, bool> m_PlayerVars;
	protected ref JMPlayerSerialize m_PlayerFile;

	void JMPlayerInstance( PlayerIdentity identity, string guid = JMConstants.OFFLINE_GUID )
	{
		PlayerObject = NULL;

		if ( identity && g_Game.IsServer() )
		{
			m_GUID = identity.GetId();
			m_Steam64ID = identity.GetPlainId();
			m_Name = identity.GetName();
		}
		else
		{
			m_GUID = guid;
			m_Steam64ID = JMConstants.OFFLINE_STEAM;
			m_Name = JMConstants.OFFLINE_NAME;
		}

		m_PlayerVars = new map<int, bool>;

	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		m_RootPermission = new JMPermission( JMConstants.PERM_ROOT );
		m_RootPermission.CopyPermissions(GetPermissionsManager().RootPermission);
	#endif

		m_Roles = new array< string >();
		m_RoleNameRestrictions = new map<string, string>();
		m_SyncedToClient = new map<string, bool>();
		m_PlayerFile = new JMPlayerSerialize();
	}

	int GetAvgPing()
	{
		return m_PingAvg;
	}

	float GetBlood()
	{
		return m_Blood;
	}

	int GetBloodStatType()
	{
		return m_BloodStatType;
	}

	bool GetCannotBeTargetedByAI()
	{
		return m_PlayerVars[JMPlayerVariables.CANNOT_BE_TARGETED_BY_AI];
	}

	int GetDataLastUpdatedTime()
	{
		return m_DataLastUpdated;
	}

	float GetEnergy()
	{
		return m_Energy;
	}

	string GetGUID()
	{
		return m_GUID;
	}

	float GetHealth()
	{
		return m_Health;
	}

	float GetHeatBuffer()
	{
		return m_HeatBuffer;
	}

	float GetHeatComfort()
	{
		return m_HeatComfort;
	}

	int GetLifeSpanState()
	{
		return m_LifeSpanState;
	}

	int GetMaxPing()
	{
		return m_PingMax;
	}

	int GetMinPing()
	{
		return m_PingMin;
	}

	string GetName()
	{
		return m_Name;
	}

	vector GetOrientation()
	{
		return m_Orientation;
	}

	JMPermission GetPermissions()
	{
	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		return m_RootPermission;
	#else
		Error("Individual permissions are not available! Use roles instead");
		return null;
	#endif
	}

	vector GetPosition()
	{
		return m_Position;
	}

	// doesn't check through roles.
	JMPermissionType GetRawPermissionType( string permission )
	{
	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		JMPermissionType permType;
		m_RootPermission.HasPermission( permission, permType );
		return permType;
	#else
		Error("Individual permissions are not available! Use roles instead");
		return JMPermissionType.DISALLOW;
	#endif
	}

	bool GetReceiveDmgDealt()
	{
		return m_PlayerVars[JMPlayerVariables.RECEIVE_DMG_DEALT];
	}

	bool GetRemoveCollision()
	{
		return m_PlayerVars[JMPlayerVariables.REMOVE_COLLISION];
	}

	string GetRoleNameRestriction( string role )
	{
		string val;
		m_RoleNameRestrictions.Find( role, val );
		return val;
	}

	array< string > GetRoles()
	{
		return m_Roles;
	}

	float GetShock()
	{
		return m_Shock;
	}

	float GetStamina()
	{
		return m_Stamina;
	}

	//! The stats block for this player, creating it on first use. Server-side
	//! only - the client copy of an instance has no player file.
	JMPlayerStats GetStats()
	{
		if ( !m_PlayerFile )
			return NULL;

		if ( !m_PlayerFile.Stats )
			m_PlayerFile.Stats = new JMPlayerStats();

		return m_PlayerFile.Stats;
	}

	string GetSteam64ID()
	{
		return m_Steam64ID;
	}

	float GetTremor()
	{
		return m_Tremor;
	}

	float GetWater()
	{
		return m_Water;
	}

	float GetWet()
	{
		return m_Wet;
	}

	bool HasAdminNVG()
	{
		return m_PlayerVars[JMPlayerVariables.ADMIN_NVG];
	}

	bool HasBloodyHands()
	{
		return m_PlayerVars[JMPlayerVariables.BLOODY_HANDS];
	}

	bool HasBrokenLegs()
	{
		return m_PlayerVars[JMPlayerVariables.BROKEN_LEGS];
	}

	bool HasGodMode()
	{
		return m_PlayerVars[JMPlayerVariables.GODMODE];
	}

	bool HasInvisibility()
	{
		return m_PlayerVars[JMPlayerVariables.INVISIBILITY];
	}

	//! Does this player hold `permission` in their OWN tree, with no role
	//! involved? HasPermission() below falls back to every role the player is
	//! in; this deliberately does not, so a caller can tell "granted to this
	//! person" apart from "comes with the role they are in".
	bool HasOwnPermission( string permission )
	{
	#ifdef JMPermissionType
		JMPermissionType ownPermType;
		return m_RootPermission.HasPermission( permission, ownPermType );
	#else
		Error("Individual permissions are not available! Use roles instead");
		return false;
	#endif
	}

	bool HasPermission( string permission )
	{
		JMPermissionType permType;
		bool hasPermission;

	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		hasPermission = m_RootPermission.HasPermission( permission, permType );
		
		// Print( "JMPlayerInstance::HasPermission - hasPermission=" + hasPermission );
		if ( hasPermission )
			return true;

		// Print( "JMPlayerInstance::HasPermission - permType=" + permType );
		if ( permType == JMPermissionType.DISALLOW )
			return false;
	#endif

		for ( int j = 0; j < m_Roles.Count(); j++ )
		{
			// Skip this role if a name restriction is set and the current name doesn't match
			string requiredName;
			if ( m_RoleNameRestrictions.Find( m_Roles[j], requiredName ) && requiredName != "" && m_Name != requiredName )
				continue;

			JMRole role = GetPermissionsManager().GetRole( m_Roles[j] );
			if ( !role )
				continue;

			hasPermission = role.HasPermission( permission, permType );

			// Print( "JMPlayerInstance::HasPermission - role[" + j + "]=" + Roles[j] );
			// Print( "JMPlayerInstance::HasPermission - permType=" + permType );
			// Print( "JMPlayerInstance::HasPermission - hasPermission=" + hasPermission );

			if ( hasPermission )
				return true;
		}

		return false;
	}

	bool HasRole( string role )
	{
		return m_Roles.Find( role ) >= 0;
	}

	//! True when this player has session history worth persisting.
	bool HasStats()
	{
		if ( !m_PlayerFile || !m_PlayerFile.Stats )
			return false;

		return !m_PlayerFile.Stats.IsEmpty();
	}

	bool HasUnlimitedAmmo()
	{
		return m_PlayerVars[JMPlayerVariables.UNLIMITED_AMMO];
	}

	bool HasUnlimitedStamina()
	{
		return m_PlayerVars[JMPlayerVariables.UNLIMITED_STAMINA];
	}

	bool IsBleeding()
	{
		return m_PlayerVars[JMPlayerVariables.BLEEDING];
	}

	bool IsDead()
	{
		return m_Health <= 0 || m_PlayerVars[JMPlayerVariables.DEAD];
	}

	// Returns true when the player has no customisation: only the "everyone" role and no
	// explicitly set permissions.  Files should not be written in this state.
	bool IsDefaultState()
	{
		if ( m_Roles.Count() != 1 || m_Roles[0] != "everyone" )
			return false;

	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		// m_Sync is set true as soon as any non-INHERIT permission is added to the tree.
		if ( m_RootPermission.m_Sync )
			return false;
	#endif

		// Session history counts as state worth keeping. Without this an
		// ordinary player - default roles, no explicit permissions - has their
		// file deleted on the very next save, taking their playtime with it.
		if ( HasStats() )
			return false;

		return true;
	}

	bool IsFrozen()
	{
		return m_PlayerVars[JMPlayerVariables.FROZEN];
	}

	bool IsRagdoll()
	{
		return m_PlayerVars[JMPlayerVariables.RAGDOLL];
	}

	bool IsSick()
	{
		return m_PlayerVars[JMPlayerVariables.SICK];
	}

	bool IsUnconscious()
	{
		return m_PlayerVars[JMPlayerVariables.UNCONSCIOUS];
	}

	void MakeFake( string gid, string sid, string nid )
	{
		m_GUID = gid;
		m_Steam64ID = sid;
		m_Name = nid;
	}

	bool CanSendData()
	{
		return PlayerObject != NULL; 
	}

	void Update()
	{
		if ( g_Game.IsServer() && ( g_Game.GetTime() - m_DataLastUpdated ) >= 100 )
		{
			if ( !g_Game.IsMultiplayer() && !PlayerObject )
				Class.CastTo( PlayerObject, g_Game.GetPlayer() );

			if ( PlayerObject )
			{
				m_DataLastUpdated = g_Game.GetTime();

				m_Position = PlayerObject.GetPosition();
				m_Orientation = PlayerObject.GetOrientation();
	
				m_Health = PlayerObject.GetHealth( "GlobalHealth","Health" );
				m_Blood = PlayerObject.GetHealth( "GlobalHealth", "Blood" );
				m_Shock = PlayerObject.GetHealth( "GlobalHealth", "Shock" );

				m_BloodStatType = PlayerObject.GetStatBloodType().Get();

				m_Energy = PlayerObject.GetStatEnergy().Get();
				m_Water = PlayerObject.GetStatWater().Get();
				m_HeatComfort = PlayerObject.GetStatHeatComfort().Get();
				m_HeatBuffer = PlayerObject.GetStatHeatBuffer().Get();
				m_Wet = PlayerObject.GetStatWet().Get();
				m_Tremor = PlayerObject.GetStatTremor().Get();
				m_Stamina = PlayerObject.GetStatStamina().Get();
				m_LifeSpanState = PlayerObject.GetLifeSpanState();

				PlayerObject.COT_UpdatePlayerVars(m_PlayerVars);
			}
		}
	}

	void CopyPermissions( JMPermission copy )
	{
	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		auto trace = CF_Trace_0(this, "CopyPermissions");

		m_RootPermission.CopyPermissions( copy );

		m_SyncedToClient.Clear();
	#else
		Error("Individual permissions are not available! Use roles instead");
	#endif
	}

	void ClearPermissions()
	{
	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		m_RootPermission.Clear();

		m_SyncedToClient.Clear();
	#else
		Error("Individual permissions are not available! Use roles instead");
	#endif
	}

	void RemoveSyncedToClient(string guid)
	{
		m_SyncedToClient.Remove(guid);
	}

	void LoadPermissions( array< string > permissions )
	{
	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		auto trace = CF_Trace_1(this).Add(permissions.Count());

		m_RootPermission.Deserialize( permissions );
		m_SyncedToClient.Clear();
		Save();
	#else
		Error("Individual permissions are not available! Use roles instead");
	#endif
	}

	void AddPermission( string permission, JMPermissionType type = JMPermissionType.INHERIT )
	{
	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		m_RootPermission.AddPermission( permission, type );

		m_SyncedToClient.Clear();
	#else
		Error("Individual permissions are not available! Use roles instead");
	#endif
	}

	void LoadRoles( notnull array< string > roles, map< string, string > nameRestrictions = NULL )
	{
		ClearRoles();

		for ( int i = 0; i < roles.Count(); i++ )
		{
			string nameRestriction = "";
			if ( nameRestrictions && nameRestrictions.Contains( roles[i] ) )
				nameRestriction = nameRestrictions.Get( roles[i] );

			AddRole( roles[i], nameRestriction );
		}

		EnsureDefaultRole();

		Save();
	}

	void AddRole( string role, string nameRestriction = "" )
	{
	#ifdef DIAG_DEVELOPER
	#ifdef DZ_Expansion_Core
		EXError.Info(this, "Adding role " + role + " (nameRestriction=" + nameRestriction + ") to player " + m_Name + " (GUID=" + m_GUID + ")");
	#endif
	#endif

		if ( !GetPermissionsManager().IsRole( role ) )
			return;

		if ( m_Roles.Find( role ) < 0 )
			m_Roles.Insert( role );

		if ( nameRestriction != "" )
			m_RoleNameRestrictions.Insert( role, nameRestriction );
		else
			m_RoleNameRestrictions.Remove( role );

		m_SyncedToClient.Clear();
	}

	void AddRoleByIndex( int index, string nameRestriction = "" )
	{
		string role = GetPermissionsManager().GetRoleNameByIndex( index );
		AddRole( role, nameRestriction );
	}

	void ClearRoles()
	{
	#ifdef DIAG_DEVELOPER
	#ifdef DZ_Expansion_Core
		EXError.Info(this, "Clearing roles for player " + m_Name + " (GUID=" + m_GUID + ")");
	#endif
	#endif

		m_Roles.Clear();
		m_RoleNameRestrictions.Clear();
	}

	void EnsureDefaultRole()
	{
		if ( m_Roles.Count() == 0 )
			AddRole( "everyone" );
	}

	void OnSend( ParamsWriteContext ctx, string sendToGUID = JMConstants.OFFLINE_GUID )
	{
		OnSendPermissions( ctx, sendToGUID );
		OnSendPosition( ctx );
		OnSendOrientation( ctx );
		OnSendStatus( ctx );
	}

	//! Old methods with typo and no return value
	[Obsolete("Use OnReceive")]
	void OnRecieve( ParamsReadContext ctx );
	[Obsolete("Use OnReceivePermissions")]
	void OnRecievePermissions( ParamsReadContext ctx );
	[Obsolete("Use OnReceivePosition")]
	void OnRecievePosition( ParamsReadContext ctx );
	[Obsolete("Use OnReceiveOrientation")]
	void OnRecieveOrientation( ParamsReadContext ctx );
	[Obsolete("Use OnReceiveStatus")]
	void OnRecieveHealth( ParamsReadContext ctx );

	bool OnReceive( ParamsReadContext ctx )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnReceive");
		#endif

		if (!OnReceivePermissions( ctx ))
			return false;
		if (!OnReceivePosition( ctx ))
			return false;
		if (!OnReceiveOrientation( ctx ))
			return false;
		if (!OnReceiveStatus( ctx ))
			return false;

		m_DataLastUpdated = g_Game.GetTime();

		return true;
	}

	void OnSendPermissions( ParamsWriteContext ctx, string sendToGUID )
	{
		#ifdef JM_COT_DIAG_LOGGING
		Print("OnSendPermissions - GUID " + m_GUID + ", already synced to " + sendToGUID + " " + m_SyncedToClient[sendToGUID]);
		#endif

		ctx.Write( !m_SyncedToClient[sendToGUID] );

		if ( m_SyncedToClient[sendToGUID] )
			return;

		ctx.Write( m_Steam64ID );
		ctx.Write( m_Name );

	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		#ifdef DIAG_DEVELOPER
		#ifdef DZ_Expansion_Core
		EXError.Info(this, string.Format("Sending permissions for player %1 (GUID=%2)", m_Name, m_GUID));
		#endif
		#endif
		m_RootPermission.OnSend( ctx );
	#endif

		ctx.Write( m_Roles.Count() );
		foreach ( string role: m_Roles )
			ctx.Write( GetPermissionsManager().GetRoleIndexByName( role ) );

		m_SyncedToClient[sendToGUID] = true;
	}

	bool OnReceivePermissions( ParamsReadContext ctx )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnReceivePermissions");
		#endif

		bool permissionsUpdate;
		ctx.Read( permissionsUpdate );
		#ifdef JM_COT_DIAG_LOGGING
		Print("OnReceivePermissions - GUID " + m_GUID + " update " + permissionsUpdate);
		#endif

		if ( !permissionsUpdate )
			return true;

		ctx.Read( m_Steam64ID );
		ctx.Read( m_Name );

	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		#ifdef DIAG_DEVELOPER
		#ifdef DZ_Expansion_Core
		EXError.Info(this, string.Format("Receiving permissions for player %1 (GUID=%2)", m_Name, m_GUID));
		#endif
		#endif

		if (!m_RootPermission.OnReceive( ctx ))
		{
			CF.FormatError("Couldn't receive permissions for player %1 (GUID=%2)", m_Name, m_GUID);
			return false;
		}
	#endif

		#ifdef DIAG_DEVELOPER
		#ifdef DZ_Expansion_Core
		EXError.Info(this, string.Format("Receiving roles for player %1 (GUID=%2)", m_Name, m_GUID));
		#endif
		#endif

		int count;

		if (!ctx.Read( count ))
		{
			CF.FormatError("Couldn't read role count for player %1 (GUID=%2)", m_Name, m_GUID);
			return false;
		}

		if (count < 0 || count > 256)
		{
			CF.FormatError("Invalid role count %1 received for player %2 (GUID=%2)", count.ToString(), m_Name, m_GUID);
			return false;
		}

		ClearRoles();

		while (count--)
		{
			int index;
			if (!ctx.Read( index ))
			{
				CF.FormatError("Couldn't read role index for player %1 (GUID=%2)", m_Name, m_GUID);
				return false;
			}

			AddRoleByIndex( index );
		}

		EnsureDefaultRole();

		return true;
	}

	void OnSendPosition( ParamsWriteContext ctx )
	{
		for (int i = 0; i < 3; ++i)
			ctx.Write( m_Position[i] );
	}

	bool OnReceivePosition( ParamsReadContext ctx )
	{
		for (int i = 0; i < 3; ++i)
		{
			float value;
			if (!ctx.Read( value ))
				return false;

			m_Position[i] = value;
		}

		return true;
	}

	void OnSendOrientation( ParamsWriteContext ctx )
	{
		for (int i = 0; i < 3; ++i)
			ctx.Write( m_Orientation[i] );
	}

	bool OnReceiveOrientation( ParamsReadContext ctx )
	{
		for (int i = 0; i < 3; ++i)
		{
			float value;
			if (!ctx.Read( value ))
				return false;

			m_Orientation[i] = value;
		}

		return true;
	}

	[Obsolete("Use OnSendStatus")]
	void OnSendHealth( ParamsWriteContext ctx );

	void OnSendStatus( ParamsWriteContext ctx )
	{
		ctx.Write( m_Health );
		ctx.Write( m_Blood );
		ctx.Write( m_Shock );
		ctx.Write( m_BloodStatType );
		ctx.Write( m_Energy );
		ctx.Write( m_Water );
		ctx.Write( m_HeatComfort );
		ctx.Write( m_HeatBuffer );		
		ctx.Write( m_Wet );
		ctx.Write( m_Tremor );
		ctx.Write( m_Stamina );
		ctx.Write( m_LifeSpanState );

		int bitmask;
		foreach (int value, bool enabled: m_PlayerVars)
		{
			if (enabled)
				bitmask |= value;
		}

		ctx.Write( bitmask );
	}

	bool OnReceiveStatus( ParamsReadContext ctx )
	{
		if (!ctx.Read( m_Health ))
			return false;
		if (!ctx.Read( m_Blood ))
			return false;
		if (!ctx.Read( m_Shock ))
			return false;
		if (!ctx.Read( m_BloodStatType ))
			return false;
		if (!ctx.Read( m_Energy ))
			return false;
		if (!ctx.Read( m_Water ))
			return false;
		if (!ctx.Read( m_HeatComfort ))
			return false;
		if (!ctx.Read( m_HeatBuffer ))
			return false;		
		if (!ctx.Read( m_Wet ))
			return false;
		if (!ctx.Read( m_Tremor ))
			return false;
		if (!ctx.Read( m_Stamina ))
			return false;
		if (!ctx.Read( m_LifeSpanState ))
			return false;
		
		int bitmask;
		if (!ctx.Read( bitmask ))
			return false;

		for (int i = 0; i < EnumTools.GetEnumSize(JMPlayerVariables); i++)
		{
			int value = EnumTools.GetEnumValue(JMPlayerVariables, i);
			m_PlayerVars[value] = (bitmask & value) == value;
		}

		return true;
	}

	void Save()
	{
		auto trace = CF_Trace_1(this, "Save").Add(m_GUID);

		if ( !g_Game.IsServer() )
			return;

		string playerFilePath    = m_PlayerFile.m_FileName;
		string permissionsPath   = JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_GUID ) + JMConstants.EXT_PERMISSION;

		// If the player is back to default, remove any files we previously wrote.
		if ( IsDefaultState() )
		{
			if ( FileExist( playerFilePath ) )
				DeleteFile( playerFilePath );

			if ( FileExist( permissionsPath ) )
				DeleteFile( permissionsPath );

			return;
		}

		// Write the player JSON when roles differ from default OR there is
		// session history to keep. Deleting on default roles alone would throw
		// away the playtime of every ordinary player on the server.
		bool rolesAreDefault = ( m_Roles.Count() == 1 && m_Roles[0] == "everyone" );
		if ( !rolesAreDefault || HasStats() )
		{
			m_PlayerFile.Roles.Clear();
			m_PlayerFile.Roles.Copy( m_Roles );

			m_PlayerFile.NameRestrictions.Clear();
			for ( int nr = 0; nr < m_RoleNameRestrictions.Count(); nr++ )
				m_PlayerFile.NameRestrictions.Insert( m_RoleNameRestrictions.GetKey( nr ), m_RoleNameRestrictions.GetElement( nr ) );

			m_PlayerFile.Save();
		}
		else if ( FileExist( playerFilePath ) )
		{
			DeleteFile( playerFilePath );
		}

	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		FileHandle file = OpenFile( permissionsPath, FileMode.WRITE );
		if ( file != 0 )
		{
			array< string > permissions = new array< string >;
			m_RootPermission.Serialize( permissions );

			string line;
			for ( int i = 0; i < permissions.Count(); i++ )
			{
				FPrintln( file, permissions[i] );
			}
			
			CloseFile( file );
		}
	#else
		if ( FileExist( permissionsPath ) )
		{
			DeleteFile( permissionsPath );
		}
	#endif
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
	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		auto trace = CF_Trace_1(this).Add(filename);

		if ( !FileExist( filename ) )
			return false;

		FileHandle file = OpenFile( filename, FileMode.READ );

		if ( file < 0 )
			return false;

		string line;

		while ( FGets( file, line ) > 0 )
		{
			AddPermission( line );
		}

		CloseFile( file );

		return true;
	#else
		Error("Individual permissions are not available! Use roles instead");
		return false;
	#endif
	}

	void Load()
	{
		auto trace = CF_Trace_1(this, "Load").Add(m_GUID);

		if ( !IsMissionHost() )
			return;

		Update();

		bool hadFile = JMPlayerSerialize.Load( this, m_PlayerFile );

		for ( int j = 0; j < m_PlayerFile.Roles.Count(); j++ )
		{
			string loadedNameRestriction = "";
			if ( m_PlayerFile.NameRestrictions && m_PlayerFile.NameRestrictions.Contains( m_PlayerFile.Roles[j] ) )
				loadedNameRestriction = m_PlayerFile.NameRestrictions.Get( m_PlayerFile.Roles[j] );

			AddRole( m_PlayerFile.Roles[j], loadedNameRestriction );
		}

		EnsureDefaultRole();

	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		// Track whether any legacy permission file was migrated so we know whether to re-save.
		bool migratedPermissions = false;

		if ( !ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_GUID ) + JMConstants.EXT_PERMISSION ) )
		{
			if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_GUID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT ) )
			{
				DeleteFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_GUID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT );
				migratedPermissions = true;
			}
			else if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_Steam64ID ) + JMConstants.EXT_PERMISSION ) )
			{
				DeleteFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_Steam64ID ) + JMConstants.EXT_PERMISSION );
				migratedPermissions = true;
			}
			else if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_Steam64ID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT ) )
			{
				DeleteFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_Steam64ID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT );
				migratedPermissions = true;
			}
		}

		// Only persist on load when we migrated legacy files.
		// Normal connects (new player or existing GUID file) are saved on demand by admin actions.
		if ( migratedPermissions )
			Save();
	#endif
	}

	void DebugPrint()
	{
		// Print( "  SGUID: " + m_GUID );
		// Print( "  SSteam64ID: " + m_Steam64ID );
		// Print( "  SName: " + m_Name );

	#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
		m_RootPermission.DebugPrint( 2 );
	#endif
	}

	string FormatSteamWebhook()
	{
		return "[" + m_Name + "](https://steamcommunity.com/profiles/" + m_Steam64ID + ")";
	}
}
