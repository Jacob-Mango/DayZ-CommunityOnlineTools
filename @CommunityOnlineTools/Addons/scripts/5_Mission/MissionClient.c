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
        super.OnMissionStart();
    
        m_Tool.OnStart();
    }

    override void OnMissionFinish()
    {
        m_Tool.OnFinish();

        super.OnMissionFinish();
    }

    override void OnUpdate( float timeslice )
    {
        super.OnUpdate( timeslice );

		UIScriptedMenu menu = m_UIManager.GetMenu();
        Input input = GetGame().GetInput();

        m_Tool.OnUpdate( timeslice );

        if ( DISABLE_ALL_INPUT )
        {            
            if( input.GetActionDown(UAUIQuickbarRadialOpen, false) )
            {
                if ( GetUIManager().IsMenuOpen( MENU_RADIAL_QUICKBAR ) )
                {
                    RadialQuickbarMenu.CloseMenu();
                }
            }
        }
    }
    
    override void ShowInventory()
    {
        if ( DISABLE_ALL_INPUT ) return;

        super.ShowInventory();
    }

    override void ShowChat()
    {
        if ( DISABLE_ALL_INPUT ) return;

        super.ShowChat();
    }
}
