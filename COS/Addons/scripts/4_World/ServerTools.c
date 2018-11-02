class ServerTools
{
    protected bool m_bLoaded;

    void ServerTools()
    {
        Print("ServerTools::ServerTools");

        m_bLoaded = false;

        NewModuleManager();
    }

    void ~ServerTools()
    {
        Print("ServerTools::~ServerTools");
    }
    
    void OnStart()
    {
        GetModuleManager().RegisterModules();

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
        if( !m_bLoaded && !GetDayZGame().IsLoading() )
        {
            m_bLoaded = true;
            OnLoaded();
        } else {
            GetModuleManager().OnUpdate( timeslice );
        }
    }

    void OnMouseButtonRelease( int button )
    {
        GetModuleManager().OnMouseButtonRelease( button );
    }

    void OnMouseButtonPress( int button )
    {
        GetModuleManager().OnMouseButtonPress( button );
    }

    void OnKeyPress( int key )
    {
        GetModuleManager().OnKeyPress( key );
    }

    void OnKeyRelease( int key )
    {
        GetModuleManager().OnKeyRelease( key );
    }
}