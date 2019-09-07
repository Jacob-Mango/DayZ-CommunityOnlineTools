class CommunityOnlineToolsBase
{
	static ref ScriptInvoker SI_OPEN = new ScriptInvoker;

    private bool m_Loaded;

    private bool m_IsActive;
    private bool m_IsOpen;

    void CommunityOnlineToolsBase()
    {
		m_Loaded = false;
    }

    void ~CommunityOnlineToolsBase()
    {
        
    }

	void RegisterModules()
	{
    }

    void OnStart()
	{
		if ( GetGame().IsServer() && GetGame().IsMultiplayer() )
		{
			GetPermissionsManager().LoadRoles();
		}

		GetModuleManager().RegisterModules();

		RegisterModules();

		GetModuleManager().OnInit();
		
		GetModuleManager().ReloadSettings();

		GetModuleManager().OnMissionStart();
	}

	void OnFinish()
	{
		GetModuleManager().OnMissionFinish();

		NewModuleManager();
	}

	void OnLoaded()
	{
		GetModuleManager().OnMissionLoaded();
	}

	void OnUpdate( float timeslice )
	{
		if( !m_Loaded && !GetDayZGame().IsLoading() )
		{
			m_Loaded = true;
			OnLoaded();
		} else {
			GetModuleManager().OnUpdate( timeslice );
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
		if ( !GetPermissionsManager().HasPermission( "COT.View" ) )
        {
			return;
        }

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
			return;
		}

        m_IsOpen = open;

        SI_OPEN.Invoke( m_IsOpen );
    }

    void ToggleOpen()
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

        m_IsOpen = !m_IsOpen;

        SI_OPEN.Invoke( m_IsOpen );
    }

    void Log( JMPlayerInstance player, string text )
    {
        text = "[COT] " + player.Data.SSteam64ID + ": " + text;

        if ( GetGame().IsServer() )
        {
            GetGame().AdminLog( text );
        }

        GetLogger().Log( text, "JM_COT_StaticFunctions" );
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
        
        GetLogger().Log( text, "JM_COT_StaticFunctions" );
    }
}

static ref CommunityOnlineToolsBase g_cotBase;

static CommunityOnlineToolsBase GetCommunityOnlineToolsBase()
{
    return g_cotBase;
}