modded class MissionServer
{
    protected ref ServerTools m_ServerTools;

    void MissionServer()
    {
        Print( "MissionServer::MissionServer()" );
        
        m_ServerTools = new ServerTools();
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

        m_ServerTools.OnStart();
    }

    override void OnMissionFinish()
    {
        m_ServerTools.OnFinish();

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
        super.OnKeyPress(key);

        m_ServerTools.OnKeyPress( key );
    }

    override void OnKeyRelease( int key )
    {
        super.OnKeyRelease( key );

        m_ServerTools.OnKeyRelease( key );
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
