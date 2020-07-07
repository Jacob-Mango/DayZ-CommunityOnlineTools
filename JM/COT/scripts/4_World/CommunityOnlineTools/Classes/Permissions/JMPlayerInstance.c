class JMPlayerInstance : Managed
{
	private ref JMPermission m_RootPermission;
	private ref array< string > m_Roles;

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

	private float m_Wet;
	private float m_Tremor;
	private float m_Stamina;

	private int m_LifeSpanState;
	private bool m_BloodyHands;
	private bool m_GodMode;
	private bool m_Invisibility;

	private ref JMPlayerSerialize m_PlayerFile;

	void JMPlayerInstance( PlayerIdentity identity )
	{
		PlayerObject = NULL;

		if ( identity && GetGame().IsServer() )
		{
			m_GUID = identity.GetId();
			m_Steam64ID = identity.GetPlainId();
			m_Name = identity.GetName();
		} else if ( IsMissionOffline() )
		{
			m_GUID = JMConstants.OFFLINE_GUID;
			m_Steam64ID = JMConstants.OFFLINE_STEAM;
			m_Name = JMConstants.OFFLINE_NAME;
		}

		m_RootPermission = new JMPermission( JMConstants.PERM_ROOT );
		m_Roles = new array< string >();
		m_PlayerFile = new JMPlayerSerialize();
	}

	void ~JMPlayerInstance()
	{
		Assert_Null( m_RootPermission );
		Assert_Null( m_Roles );
		Assert_Null( m_PlayerFile );

		delete m_RootPermission;
		delete m_Roles;
		delete m_PlayerFile;
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
				m_Wet = PlayerObject.GetStatWet().Get();
				m_Tremor = PlayerObject.GetStatTremor().Get();
				m_Stamina = PlayerObject.GetStatStamina().Get();
				m_LifeSpanState = PlayerObject.GetLifeSpanState();
				m_BloodyHands = PlayerObject.HasBloodyHands();
				m_GodMode = PlayerObject.HasGodMode();
				m_Invisibility = PlayerObject.IsInvisible();
			}
		}
	}

	void CopyPermissions( ref JMPermission copy )
	{
		Assert_Null( m_RootPermission );
		Assert_Null( copy );

		array< string > data = new array< string >;

		copy.Serialize( data );

		m_RootPermission.Deserialize( data );
	}

	void ClearPermissions()
	{
		Assert_Null( m_RootPermission );

		m_RootPermission.Clear();
	}

	void LoadPermissions( array< string > permissions )
	{
		Assert_Null( m_RootPermission );

		m_RootPermission.Deserialize( permissions );
		Save();
	}

	void AddPermission( string permission, JMPermissionType type = JMPermissionType.INHERIT )
	{
		Assert_Null( m_RootPermission );

		m_RootPermission.AddPermission( permission, type );
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
	}

	bool HasRole( string role )
	{
		return m_Roles.Find( role ) >= 0;
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

	void OnSend( ref ParamsWriteContext ctx /* param here for helper class so only necessary data is filled based on permissions */ )
	{
		ctx.Write( m_GUID );
		ctx.Write( m_Steam64ID );
		ctx.Write( m_Name );

		OnSendPermissions( ctx );
		OnSendPosition( ctx );
		OnSendOrientation( ctx );
		OnSendHealth( ctx );
	}

	void OnRecieve( ref ParamsReadContext ctx )
	{
		ctx.Read( m_GUID );
		ctx.Read( m_Steam64ID );
		ctx.Read( m_Name );

		OnRecievePermissions( ctx );
		OnRecievePosition( ctx );
		OnRecieveOrientation( ctx );
		OnRecieveHealth( ctx );
	}

	void OnSendPermissions( ref ParamsWriteContext ctx )
	{
		Assert_Null( m_RootPermission );

		array< string > permissions = new array< string >;
		m_RootPermission.Serialize( permissions );

		ctx.Write( permissions );
		ctx.Write( m_Roles );
	}

	void OnRecievePermissions( ref ParamsReadContext ctx )
	{
		Assert_Null( m_RootPermission );
		
		array< string > permissions = new array< string >;
		array< string > roles = new array< string >;

		ctx.Read( permissions );
		ctx.Read( roles );

		m_RootPermission.Deserialize( permissions );
		
		ClearRoles();
		for ( int j = 0; j < roles.Count(); j++ )
			AddRole( roles[j] );
	}
	
	void OnSendPosition( ref ParamsWriteContext ctx )
	{
		ctx.Write( m_Position );
	}

	void OnRecievePosition( ref ParamsReadContext ctx )
	{
		ctx.Read( m_Position );
	}
	
	void OnSendOrientation( ref ParamsWriteContext ctx )
	{
		ctx.Write( m_Orientation );
	}

	void OnRecieveOrientation( ref ParamsReadContext ctx )
	{
		ctx.Read( m_Orientation );
	}
	
	void OnSendHealth( ref ParamsWriteContext ctx )
	{
		ctx.Write( m_Health );
		ctx.Write( m_Blood );
		ctx.Write( m_Shock );
		ctx.Write( m_BloodStatType );
		ctx.Write( m_Energy );
		ctx.Write( m_Water );
		ctx.Write( m_HeatComfort );
		ctx.Write( m_Wet );
		ctx.Write( m_Tremor );
		ctx.Write( m_Stamina );
		ctx.Write( m_LifeSpanState );
		ctx.Write( m_BloodyHands );
		ctx.Write( m_GodMode );
		ctx.Write( m_Invisibility );
	}

	void OnRecieveHealth( ref ParamsReadContext ctx )
	{
		ctx.Read( m_Health );
		ctx.Read( m_Blood );
		ctx.Read( m_Shock );
		ctx.Read( m_BloodStatType );
		ctx.Read( m_Energy );
		ctx.Read( m_Water );
		ctx.Read( m_HeatComfort );
		ctx.Read( m_Wet );
		ctx.Read( m_Tremor );
		ctx.Read( m_Stamina );
		ctx.Read( m_LifeSpanState );
		ctx.Read( m_BloodyHands );
		ctx.Read( m_GodMode );
		ctx.Read( m_Invisibility );
	}

	void Save()
	{
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
		Assert_Null( m_RootPermission );

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
		return m_BloodyHands;
	}

	bool HasGodMode()
	{
		return m_GodMode;
	}

	bool HasInvisibility()
	{
		return m_Invisibility;
	}

}