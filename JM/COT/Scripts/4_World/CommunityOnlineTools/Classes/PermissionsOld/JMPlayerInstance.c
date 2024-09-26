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
	INVISIBILITY_INTERACTIVE = 4096
}

#ifndef CF_MODULE_PERMISSIONS
class JMPlayerInstance : Managed
{
	private ref JMPermission m_RootPermission;
	private ref array< string > m_Roles;
	private ref map<string, bool> m_SyncedToClient;

	PlayerBase PlayerObject;

	private int m_DataLastUpdated;

	private string m_Name;
	private string m_GUID;
	private string m_Steam64ID;

	private int m_PingMax;
	private int m_PingMin;
	private int m_PingAvg;

	private vector m_Position;
	private vector m_Orientation;

	private float m_Health;
	private float m_Blood;
	private float m_Shock;

	private int m_BloodStatType;

	private float m_Energy;
	private float m_Water;

	private float m_HeatComfort;
	private float m_HeatBuffer;

	private float m_Wet;
	private float m_Tremor;
	private float m_Stamina;

	private int m_LifeSpanState;

	private ref map<int, bool> m_PlayerVars;

	private ref JMPlayerSerialize m_PlayerFile;

	void JMPlayerInstance( PlayerIdentity identity, string guid = JMConstants.OFFLINE_GUID )
	{
		PlayerObject = NULL;

		if ( identity && GetGame().IsServer() )
		{
			m_GUID = identity.GetId();
			m_Steam64ID = identity.GetPlainId();
			m_Name = identity.GetName();
		} else
		{
			m_GUID = guid;
			m_Steam64ID = JMConstants.OFFLINE_STEAM;
			m_Name = JMConstants.OFFLINE_NAME;
		}

		m_PlayerVars = new map<int, bool>;

		m_RootPermission = new JMPermission( JMConstants.PERM_ROOT );
		m_RootPermission.CopyPermissions(GetPermissionsManager().RootPermission);
		m_Roles = new array< string >();
		m_SyncedToClient = new map<string, bool>();
		m_PlayerFile = new JMPlayerSerialize();
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
		if ( GetGame().IsServer() && ( GetGame().GetTime() - m_DataLastUpdated ) >= 100 )
		{
			if ( !GetGame().IsMultiplayer() )
			{
				Class.CastTo( PlayerObject, GetGame().GetPlayer() );
			}

			if ( PlayerObject )
			{
				m_DataLastUpdated = GetGame().GetTime();

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
		auto trace = CF_Trace_0(this, "CopyPermissions");

		Assert_Null( m_RootPermission );
		Assert_Null( copy );

		m_RootPermission.CopyPermissions( copy );

		m_SyncedToClient.Clear();
	}

	void ClearPermissions()
	{
		Assert_Null( m_RootPermission );

		m_RootPermission.Clear();

		m_SyncedToClient.Clear();
	}

	void RemoveSyncedToClient(string guid)
	{
		m_SyncedToClient.Remove(guid);
	}

	void LoadPermissions( array< string > permissions )
	{
		auto trace = CF_Trace_1(this).Add(permissions.Count());

		Assert_Null( m_RootPermission );

		m_RootPermission.Deserialize( permissions );
		m_SyncedToClient.Clear();
		Save();
	}

	void AddPermission( string permission, JMPermissionType type = JMPermissionType.INHERIT )
	{
		Assert_Null( m_RootPermission );

		m_RootPermission.AddPermission( permission, type );

		m_SyncedToClient.Clear();
	}

	void LoadRoles( notnull array< string > roles )
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
		Assert_Null( GetPermissionsManager() );
		Assert_Null( m_Roles );

		if ( !GetPermissionsManager().IsRole( role ) )
			return;

		if ( m_Roles.Find( role ) < 0 )
		{
			m_Roles.Insert( role );
		}

		m_SyncedToClient.Clear();
	}

	bool HasRole( string role )
	{
		return m_Roles.Find( role ) >= 0;
	}

	array< string > GetRoles()
	{
		return m_Roles;
	}

	// doesn't check through roles.
	JMPermissionType GetRawPermissionType( string permission )
	{
		Assert_Null( m_RootPermission );

		JMPermissionType permType;
		m_RootPermission.HasPermission( permission, permType );
		return permType;
	}

	void ClearRoles()
	{
		m_Roles.Clear();

		AddRole( "everyone" );
	}

	bool HasPermission( string permission )
	{
		Assert_Null( m_RootPermission );
		
		JMPermissionType permType;
		bool hasPermission = m_RootPermission.HasPermission( permission, permType );
		
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

		for ( int j = 0; j < m_Roles.Count(); j++ )
		{
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

	void OnSend( ParamsWriteContext ctx, string sendToGUID = JMConstants.OFFLINE_GUID )
	{
		OnSendPermissions( ctx, sendToGUID );
		OnSendPosition( ctx );
		OnSendOrientation( ctx );
		OnSendHealth( ctx );
	}

	void OnRecieve( ParamsReadContext ctx )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnRecieve");
		#endif

		OnRecievePermissions( ctx );
		OnRecievePosition( ctx );
		OnRecieveOrientation( ctx );
		OnRecieveHealth( ctx );

		m_DataLastUpdated = GetGame().GetTime();
	}

	void OnSendPermissions( ParamsWriteContext ctx, string sendToGUID )
	{
		#ifdef JM_COT_DIAG_LOGGING
		Print("OnSendPermissions - GUID " + m_GUID + ", already synced to " + sendToGUID + " " + m_SyncedToClient[sendToGUID]);
		#endif

		Assert_Null( m_RootPermission );

		ctx.Write( !m_SyncedToClient[sendToGUID] );

		if ( m_SyncedToClient[sendToGUID] )
			return;

		ctx.Write( m_Steam64ID );
		ctx.Write( m_Name );

		m_RootPermission.OnSend( ctx );
		ctx.Write( m_Roles );

		m_SyncedToClient[sendToGUID] = true;
	}

	void OnRecievePermissions( ParamsReadContext ctx )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnRecievePermissions");
		#endif

		Assert_Null( m_RootPermission );
		
		bool permissionsUpdate;
		ctx.Read( permissionsUpdate );
		#ifdef JM_COT_DIAG_LOGGING
		Print("OnRecievePermissions - GUID " + m_GUID + " update " + permissionsUpdate);
		#endif
		if ( !permissionsUpdate )
			return;

		ctx.Read( m_Steam64ID );
		ctx.Read( m_Name );

		array< string > roles = new array< string >;

		m_RootPermission.OnReceive( ctx );
		ctx.Read( roles );

		ClearRoles();
		for ( int j = 0; j < roles.Count(); j++ )
			AddRole( roles[j] );
	}
	
	void OnSendPosition( ParamsWriteContext ctx )
	{
		ctx.Write( m_Position );
	}

	void OnRecievePosition( ParamsReadContext ctx )
	{
		ctx.Read( m_Position );
	}
	
	void OnSendOrientation( ParamsWriteContext ctx )
	{
		ctx.Write( m_Orientation );
	}

	void OnRecieveOrientation( ParamsReadContext ctx )
	{
		ctx.Read( m_Orientation );
	}
	
	void OnSendHealth( ParamsWriteContext ctx )
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

	void OnRecieveHealth( ParamsReadContext ctx )
	{
		ctx.Read( m_Health );
		ctx.Read( m_Blood );
		ctx.Read( m_Shock );
		ctx.Read( m_BloodStatType );
		ctx.Read( m_Energy );
		ctx.Read( m_Water );
		ctx.Read( m_HeatComfort );
		ctx.Read( m_HeatBuffer );		
		ctx.Read( m_Wet );
		ctx.Read( m_Tremor );
		ctx.Read( m_Stamina );
		ctx.Read( m_LifeSpanState );
		
		int bitmask;
		ctx.Read( bitmask );

		for (int i = 0; i < EnumTools.GetEnumSize(JMPlayerVariables); i++)
		{
			int value = EnumTools.GetEnumValue(JMPlayerVariables, i);
			m_PlayerVars[value] = (bitmask & value) == value;
		}
	}

	void Save()
	{
		auto trace = CF_Trace_1(this, "Save").Add(m_GUID);

		Assert_Null( m_RootPermission );
		if ( !Assert_Null( m_PlayerFile ) )
			Assert_Null( m_PlayerFile.Roles );

		if ( !GetGame().IsServer() )
			return;

		array< string > permissions = new array< string >;
		m_RootPermission.Serialize( permissions );

		m_PlayerFile.Roles.Clear();
		m_PlayerFile.Roles.Copy( m_Roles );
		m_PlayerFile.Save();

		FileHandle file = OpenFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_GUID ) + JMConstants.EXT_PERMISSION, FileMode.WRITE );
		if ( file != 0 )
		{
			string line;
			for ( int i = 0; i < permissions.Count(); i++ )
			{
				FPrintln( file, permissions[i] );
			}
			
			CloseFile( file );
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
		auto trace = CF_Trace_1(this).Add(filename);

		Assert_Null( m_RootPermission );

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
	}

	void Load()
	{
		auto trace = CF_Trace_1(this, "Load").Add(m_GUID);

		Assert_Null( m_PlayerFile );

		if ( !IsMissionHost() )
			return;

		Update();

		JMPlayerSerialize.Load( this, m_PlayerFile );

		for ( int j = 0; j < m_PlayerFile.Roles.Count(); j++ )
		{
			AddRole( m_PlayerFile.Roles[j] );
		}

		if ( !ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_GUID ) + JMConstants.EXT_PERMISSION ) )
		{
			if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_GUID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT ) )
			{
				DeleteFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_GUID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT );
			} else if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_Steam64ID ) + JMConstants.EXT_PERMISSION ) )
			{
				DeleteFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_Steam64ID ) + JMConstants.EXT_PERMISSION );
			} else if ( ReadPermissions( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_Steam64ID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT ) )
			{
				DeleteFile( JMConstants.DIR_PERMISSIONS + FileReadyStripName( m_Steam64ID ) + JMConstants.EXT_PERMISSION + JMConstants.EXT_WINDOWS_DEFAULT );
			}
		}

		Save();
	}

	void DebugPrint()
	{
		// Print( "  SGUID: " + m_GUID );
		// Print( "  SSteam64ID: " + m_Steam64ID );
		// Print( "  SName: " + m_Name );

		m_RootPermission.DebugPrint( 2 );
	}

	int GetDataLastUpdatedTime()
	{
		return m_DataLastUpdated;
	}

	string GetGUID()
	{
		return m_GUID;
	}

	string GetSteam64ID()
	{
		return m_Steam64ID;
	}

	string GetName()
	{
		return m_Name;
	}

	string FormatSteamWebhook()
	{
		return "[" + m_Name + "](https://steamcommunity.com/profiles/" + m_Steam64ID + ")";
	}

	int GetMaxPing()
	{
		return m_PingMax;
	}

	int GetMinPing()
	{
		return m_PingMin;
	}

	int GetAvgPing()
	{
		return m_PingAvg;
	}

	JMPermission GetPermissions()
	{
		return m_RootPermission;
	}

	vector GetPosition()
	{
		return m_Position;
	}

	vector GetOrientation()
	{
		return m_Orientation;
	}

	float GetHealth()
	{
		return m_Health;
	}

	float GetBlood()
	{
		return m_Blood;
	}

	float GetShock()
	{
		return m_Shock;
	}

	int GetBloodStatType()
	{
		return m_BloodStatType;
	}

	float GetEnergy()
	{
		return m_Energy;
	}

	float GetWater()
	{
		return m_Water;
	}

	float GetHeatComfort()
	{
		return m_HeatComfort;
	}

	float GetHeatBuffer()
	{
		return m_HeatBuffer;
	}

	float GetWet()
	{
		return m_Wet;
	}

	float GetTremor()
	{
		return m_Tremor;
	}

	float GetStamina()
	{
		return m_Stamina;
	}

	int GetLifeSpanState()
	{
		return m_LifeSpanState;
	}

	bool HasBloodyHands()
	{
		return m_PlayerVars[JMPlayerVariables.BLOODY_HANDS];
	}

	bool HasGodMode()
	{
		return m_PlayerVars[JMPlayerVariables.GODMODE];
	}

	bool IsFrozen()
	{
		return m_PlayerVars[JMPlayerVariables.FROZEN];
	}

	bool HasInvisibility()
	{
		return m_PlayerVars[JMPlayerVariables.INVISIBILITY];
	}

	bool HasUnlimitedAmmo()
	{
		return m_PlayerVars[JMPlayerVariables.UNLIMITED_AMMO];
	}

	bool HasUnlimitedStamina()
	{
		return m_PlayerVars[JMPlayerVariables.UNLIMITED_STAMINA];
	}

	bool HasAdminNVG()
	{
		return m_PlayerVars[JMPlayerVariables.ADMIN_NVG];
	}

	bool HasBrokenLegs()
	{
		return m_PlayerVars[JMPlayerVariables.BROKEN_LEGS];
	}

	bool GetReceiveDmgDealt()
	{
		return m_PlayerVars[JMPlayerVariables.RECEIVE_DMG_DEALT];
	}

	bool GetCannotBeTargetedByAI()
	{
		return m_PlayerVars[JMPlayerVariables.CANNOT_BE_TARGETED_BY_AI];
	}

	bool GetRemoveCollision()
	{
		return m_PlayerVars[JMPlayerVariables.REMOVE_COLLISION];
	}
};
#endif