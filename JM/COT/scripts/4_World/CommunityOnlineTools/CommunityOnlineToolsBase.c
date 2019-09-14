class CommunityOnlineToolsBase
{
    private bool m_Loaded;

    private bool m_IsActive;
    private bool m_IsOpen;

    private string m_FileLogName;

    void CommunityOnlineToolsBase()
    {
		m_Loaded = false;
    }

    void ~CommunityOnlineToolsBase()
    {
    }

	private string GetDateTime()
	{
		int year, month, day;
		int hour, minute, second;
		GetYearMonthDay( year, month, day );

		GetHourMinuteSecond( hour, minute, second );
		string date = day.ToStringLen( 2 ) + "-" + month.ToStringLen( 2 ) + "-" + year.ToStringLen( 2 );
		string time = hour.ToStringLen( 2 ) + "-" + minute.ToStringLen( 2 ) + "-" + second.ToStringLen( 2 );

		return date + " " + time;
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
        m_IsOpen = false;

        JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );
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
            if ( !GetPermissionsManager().HasPermission( "COT.View" ) )
            {
                return;
            }

            if ( !GetCommunityOnlineToolsBase().IsActive() )
            {
                CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
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
            if ( !GetPermissionsManager().HasPermission( "COT.View" ) )
            {
                return;
            }

            if ( !GetCommunityOnlineToolsBase().IsActive() )
            {
                CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
                return;
            }
        }

        m_IsOpen = !m_IsOpen;

        JMScriptInvokers.COT_ON_OPEN.Invoke( m_IsOpen );
    }

    void Log( JMPlayerInstance player, string text )
    {
        if ( player == NULL )
        {
            text = "[COT] Game: " + text;
        } else if ( GetGame().IsMultiplayer() )
        {
            text = "[COT] " + player.GetSteam64ID() + ": " + text;
        } else
        {
            text = "[COT] Offline: " + text;
        }

        if ( GetGame().IsServer() )
        {
            GetGame().AdminLog( text );
        }

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
        if ( fileLog != 0 )
        {
		    FPrintln( fileLog, text );
		    CloseFile( fileLog );
        }
    }

    void Log( PlayerIdentity player, string text )
    {
        if ( player == NULL )
        {
            text = "[COT] Game: " + text;
        } else if ( GetGame().IsMultiplayer() )
        {
            text = "[COT] " + player.GetPlainId() + ": " + text;
        } else
        {
            text = "[COT] Offline: " + text;
        }

        if ( GetGame().IsServer() )
        {
            GetGame().AdminLog( text );
        }

		int fileLog = OpenFile( m_FileLogName, FileMode.APPEND );
        if ( fileLog != 0 )
        {
		    FPrintln( fileLog, text );
		    CloseFile( fileLog );
        }
    }
}

static ref CommunityOnlineToolsBase g_cotBase;

static CommunityOnlineToolsBase GetCommunityOnlineToolsBase()
{
    return g_cotBase;
}