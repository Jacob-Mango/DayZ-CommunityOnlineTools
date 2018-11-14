modded class MissionServer
{
    protected ref CommunityOnlineTools m_Tool;

    void MissionServer()
    {
        Print( "MissionServer::MissionServer()" );
        
        m_Tool = new CommunityOnlineTools();
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

    override void InvokeOnConnect( PlayerBase player, PlayerIdentity identity)
	{
        GetPermissionsManager().PlayerJoined( identity );

        super.InvokeOnConnect( player, identity );
    } 

    override void InvokeOnDisconnect( PlayerBase player )
	{
        GetPermissionsManager().PlayerLeft( player.GetIdentity() );

        super.InvokeOnDisconnect( player );
    } 
}
