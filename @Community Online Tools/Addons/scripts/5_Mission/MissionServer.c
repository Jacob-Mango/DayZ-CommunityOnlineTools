modded class MissionServer
{
    protected ref CommunityOnlineTools m_Tool;

    void MissionServer()
    {        
        m_Tool = new CommunityOnlineTools();
    }

    void ~MissionServer()
    {
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

    override void OnPreloadEvent(PlayerIdentity identity, out bool useDB, out vector pos, out float yaw, out int queueTime)
    {
        super.OnPreloadEvent( identity, useDB, pos, yaw, queueTime );

        // queueTime = 0;
    }

    override void InvokeOnConnect( PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect( player, identity );

        GetGame().SelectPlayer( identity, player );
    } 

    override void InvokeOnDisconnect( PlayerBase player )
    {
        super.InvokeOnDisconnect( player );
    } 

}
