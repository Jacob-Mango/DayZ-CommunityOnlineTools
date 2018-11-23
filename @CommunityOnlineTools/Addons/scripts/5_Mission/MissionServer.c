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
        super.OnKeyPress(key);

        m_Tool.OnKeyPress( key );
    }

    override void OnKeyRelease( int key )
    {
        super.OnKeyRelease( key );

        m_Tool.OnKeyRelease( key );
    }

	override void OnPreloadEvent(PlayerIdentity identity, out bool useDB, out vector pos, out float yaw, out int queueTime)
	{
        super.OnPreloadEvent( identity, useDB, pos, yaw, queueTime );

        queueTime = 1;

        GetPermissionsManager().GetPlayerByIdentity( identity );
    }

    override void InvokeOnConnect( PlayerBase player, PlayerIdentity identity)
	{
        super.InvokeOnConnect( player, identity );

        GetPermissionsManager().GetPlayerByIdentity( identity );

        GetGame().SelectPlayer( identity, player );
    } 

    override void InvokeOnDisconnect( PlayerBase player )
	{
        GetPermissionsManager().PlayerLeft( player.GetIdentity() );

        super.InvokeOnDisconnect( player );
    } 

}
