class CommunityOnlineTools
{
    protected bool m_bLoaded;

    void CommunityOnlineTools()
    {
        Print("CommunityOnlineTools::CommunityOnlineTools");

        m_bLoaded = false;

        NewModuleManager();
    }

    void RegisterModules()
    {
        GetModuleManager().RegisterModule( new PlayerModule );
        GetModuleManager().RegisterModule( new ObjectModule );
        GetModuleManager().RegisterModule( new MapEditorModule );
        GetModuleManager().RegisterModule( new TeleportModule );
        GetModuleManager().RegisterModule( new CameraTool );
        GetModuleManager().RegisterModule( new GameModule );
        GetModuleManager().RegisterModule( new WeatherModule );
    }

    void ~CommunityOnlineTools()
    {
        Print("CommunityOnlineTools::~CommunityOnlineTools");
    }
    
    void OnStart()
    {
        GetModuleManager().RegisterModules();

        RegisterModules();

        GetModuleManager().OnInit();

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