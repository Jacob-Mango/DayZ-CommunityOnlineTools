modded class MissionGameplay
{
    protected bool m_bLoaded;

    void MissionSMissionGameplayrver()
    {
        Print( "MissionGameplay::MissionGameplay()" );
        m_bLoaded = false;
    }

    void ~MissionGameplay()
    {
        Print( "MissionGameplay::~MissionGameplay()" );
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

        GetGame().GetUIManager().CloseMenu( MENU_INGAME );
    }

    override void OnMissionFinish()
    {
        ModuleManager_OnMissionFinish();
        
        CloseAllMenus();

        DestroyAllMenus();

        GetGame().GetUIManager().CloseMenu( MENU_INGAME );

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
        
        if ( key == KeyCode.KC_PERIOD )
        {
            if ( !GetUIManager().IsMenuOpen( MENU_GESTURES ) )
            {
                GesturesMenu.OpenMenu();
            }
        }
    }

    override void OnKeyRelease( int key )
    {
        super.OnKeyRelease( key );

        GetModuleManager().OnKeyRelease( key );
    }
}
