modded class MissionGameplay
{
    protected ref ServerTools m_ServerTools;

    void MissionGameplay()
    {
        Print( "MissionGameplay::MissionGameplay()" );
        
        m_ServerTools = new ServerTools();
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
    
        m_ServerTools.OnStart();

        GetGame().GetUIManager().CloseMenu( MENU_INGAME );
    }

    override void OnMissionFinish()
    {
        m_ServerTools.OnFinish();
        
        CloseAllMenus();

        DestroyAllMenus();

        GetGame().GetUIManager().CloseMenu( MENU_INGAME );

        super.OnMissionFinish();
    }

    override void OnUpdate( float timeslice )
    {
        super.OnUpdate( timeslice );

        m_ServerTools.OnUpdate( timeslice );
    }

    override void OnMouseButtonRelease( int button )
    {
        super.OnMouseButtonRelease( button );

        m_ServerTools.OnMouseButtonRelease( button );
    }

    override void OnMouseButtonPress( int button )
    {
        super.OnMouseButtonPress( button );

        m_ServerTools.OnMouseButtonPress( button );
    }

    override void OnKeyPress( int key )
    {
        super.OnKeyPress( key );

        m_ServerTools.OnKeyPress( key );
        
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

        m_ServerTools.OnKeyRelease( key );
    }
}
