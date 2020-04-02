class CommunityOnlineToolsBase
{
	private bool m_Loaded;

	private bool m_IsActive;
	private bool m_IsOpen;

	private string m_FileLogName;

	void CommunityOnlineToolsBase()
	{
		m_Loaded = false;

		GetDayZGame().Event_OnRPC.Insert( OnRPC );
	}

	void ~CommunityOnlineToolsBase()
	{
		array< ref JMRenderableModuleBase > cotModules = GetModuleManager().GetCOTModules();
		for ( int i = 0; i < cotModules.Count(); i++ )
		{
			cotModules[i].Hide();
		}

		delete g_cot_WindowManager;

		GetDayZGame().Event_OnRPC.Remove( OnRPC );
	}

	private string GetDateTime()
	{
		int year, month, day;
		int hour, minute, second;
		GetYearMonthDay( year, month, day );

		GetHourMinuteSecond( hour, minute, second );
		string date = day.ToStringLen( 2 ) + "-" + month.ToStringLen( 2 ) + "-" + year.ToStringLen( 2 );
		string time = hour.ToStringLen( 2 ) + "-" + minute.ToStringLen( 2 ) + "-" + second.ToStringLen( 2 );

		return date + "-" + time;
	}

	void CreateNewLog()
	{
		if ( !FileExist( JMConstants.DIR_LOGS ) )
		{
			MakeDirectory( JMConstants.DIR_LOGS );
		}

		m_FileLogName = JMConstants.DIR_LOGS + "cot-" + GetDateTime() + JMConstants.EXT_LOG;
		int fileLog = OpenFile( m_FileLogName, FileMode.WRITE );

		if ( fileLog != 0 )
		{
			CloseFile( fileLog );
		}
	}

	void CloseLog()
	{
		m_FileLogName = "";
	}

	void OnStart()
	{
		m_IsOpen = false;

		JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );
		
		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			GetPermissionsManager().LoadRoles();
		}

		CreateNewLog();
	}

	void OnFinish()
	{
		m_IsOpen = false;

		JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );

		CloseLog();
	}

	void OnLoaded()
	{
	}

	void OnUpdate( float timeslice )
	{
		if ( !m_Loaded && !GetDayZGame().IsLoading() )
		{
			m_Loaded = true;
			OnLoaded();
		}

	}

	bool IsActive()
	{
		return m_IsActive;
	}

	void SetActive( bool active )
	{
		m_IsActive = active;
	}

	void ToggleActive()
	{
		m_IsActive = !m_IsActive;
	}

	bool IsOpen()
	{
		return m_IsOpen;
	}

	void SetOpen( bool open )
	{
		if ( open )
		{
			if ( GetGame().GetUIManager().GetMenu() )
			{
				return;
			}

			if ( !GetPermissionsManager().HasPermission( "COT.View", NULL ) )
			{
				return;
			}

			if ( !GetCommunityOnlineToolsBase().IsActive() )
			{
				CreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLED_OFF" ) );
				return;
			}
		}

		m_IsOpen = open;

		JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );
	}

	void ToggleOpen()
	{
		if ( !m_IsOpen )
		{
			if ( GetGame().GetUIManager().GetMenu() )
			{
				return;
			}

			if ( !GetPermissionsManager().HasPermission( "COT.View", NULL ) )
			{
				return;
			}

			if ( !GetCommunityOnlineToolsBase().IsActive() )
			{
				CreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIF_TOGGLED_OFF" ) );
				return;
			}
		}

		m_IsOpen = !m_IsOpen;

		JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );
	}

	void LogServer( string text )
	{
		if ( GetGame().IsServer() )
		{
			GetGame().AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + GetDateTime() + "] " + text );
			CloseFile( fileLog );
		}
	}

	void Log( JMPlayerInstance logInstacPlyer, string text )
	{
		if ( GetGame().IsMultiplayer() )
		{
			text = "" + logInstacPlyer.GetSteam64ID() + ": " + text;
		} else
		{
			text = "Offline: " + text;
		}

		if ( GetGame().IsServer() )
		{
			GetGame().AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + GetDateTime() + "] " + text );
			CloseFile( fileLog );
		}
	}

	void Log( PlayerIdentity logIdentPlyer, string text )
	{
		if ( GetGame().IsMultiplayer() )
		{
			text = "" + logIdentPlyer.GetPlainId() + ": " + text;
		} else
		{
			text = "Offline: " + text;
		}

		if ( GetGame().IsServer() )
		{
			GetGame().AdminLog( "[COT] " + text );
		}

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
		if ( fileLog != 0 )
		{
			FPrintln( fileLog, "[COT " + GetDateTime() + "] " + text );
			CloseFile( fileLog );
		}
	}

	void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
	}

	void UpdateRole( JMRole role, PlayerIdentity toSendTo )
	{
	}

	void UpdateClient( string guid, PlayerIdentity sendTo )
	{
	}

	void RemoveClient( string guid )
	{
	}

	void RefreshClients()
	{
	}

	void SetClient( JMPlayerInstance player )
	{
	}

	void SetClient( JMPlayerInstance player, PlayerIdentity identity )
	{
	}
}

static ref CommunityOnlineToolsBase g_cotBase;

static CommunityOnlineToolsBase GetCommunityOnlineToolsBase()
{
	return g_cotBase;
}