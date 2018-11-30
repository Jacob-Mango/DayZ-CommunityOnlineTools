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

        m_Tool.OnUpdate( timeslice );
    }
}
