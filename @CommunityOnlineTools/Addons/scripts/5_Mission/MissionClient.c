modded class MissionGameplay
{
    protected ref CommunityOnlineTools m_Tool;

    void MissionGameplay()
    {
        Print( "MissionGameplay::MissionGameplay()" );
        
        m_Tool = new CommunityOnlineTools();
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
        if ( !GetGame().IsMultiplayer() )
        {
            GetPermissionsManager().PlayerJoined( NULL );
        }

        super.OnMissionStart();
    
        m_Tool.OnStart();

        GetGame().GetUIManager().CloseMenu( MENU_INGAME );
    }

    override void OnMissionFinish()
    {
        m_Tool.OnFinish();
        
        CloseAllMenus();

        DestroyAllMenus();

        GetGame().GetUIManager().CloseMenu( MENU_INGAME );

        super.OnMissionFinish();
    }

    override void OnUpdate( float timeslice )
    {
        super.OnUpdate( timeslice );

        m_Tool.OnUpdate( timeslice );
    }

    override void OnMouseButtonRelease( int button )
    {
        super.OnMouseButtonRelease( button );

        m_Tool.OnMouseButtonRelease( button );
    }

    override void OnMouseButtonPress( int button )
    {
        super.OnMouseButtonPress( button );

        m_Tool.OnMouseButtonPress( button );
    }

    override void OnKeyPress( int key )
    {
        super.OnKeyPress( key );

        m_Tool.OnKeyPress( key );
        
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

        m_Tool.OnKeyRelease( key );
    }
}
