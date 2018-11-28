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
        
        GetRPCManager().SendRPC( "COT_Admin", "UpdatePlayers", new Param, true );
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

        // PlayerControlEnable();
    }
}
