modded class MissionServer
{
    protected bool m_bLoaded;

    void MissionServer()
    {
        Print( "MissionServer::MissionServer()" );
        
        m_bLoaded = false;
    }

    void ~MissionServer()
    {
        Print( "MissionServer::~MissionServer()" );
    }

    override void OnInit()
    {
        super.OnInit();
    }

    override void OnMissionStart()
    {
        super.OnMissionStart();

        GetModuleManager().RegisterModules();
    
        GetModuleManager().OnMissionStart();
    }

    override void OnMissionFinish()
    {
        ModuleManager_OnMissionFinish();

        super.OnMissionFinish();
    }

    void OnMissionLoaded()
    {
        GetModuleManager().OnMissionLoaded();
    }

    override void OnUpdate( float timeslice )
    {
        super.OnUpdate( timeslice );

        if( !m_bLoaded && !GetDayZGame().IsLoading() )
        {
            m_bLoaded = true;
            OnMissionLoaded();
        } else {
            GetModuleManager().OnUpdate( timeslice );
        }
    }

    override void OnMouseButtonRelease( int button )
    {
        super.OnMouseButtonRelease( button );

        GetModuleManager().OnMouseButtonRelease( button );
    }

    override void OnMouseButtonPress( int button )
    {
        super.OnMouseButtonPress( button );

        GetModuleManager().OnMouseButtonPress( button );
    }

    override void OnKeyPress( int key )
    {
        super.OnKeyPress(key);

        GetModuleManager().OnKeyPress( key );
    }

    override void OnKeyRelease( int key )
    {
        super.OnKeyRelease( key );

        GetModuleManager().OnKeyRelease( key );
    }
}
