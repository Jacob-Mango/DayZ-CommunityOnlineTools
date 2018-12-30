modded class MissionServer
{
    protected bool m_bLoaded;

    protected ref CommunityOnlineTools m_Tool;

    void MissionServer()
    {        
        m_bLoaded = false;

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

    void OnLoaded()
    {
        ref array< string > data = new ref array< string >;
        GetPermissionsManager().RootPermission.Serialize( data );

        if ( !GetPermissionsManager().RoleExists( "everyone" ) )
        {
            GetPermissionsManager().CreateRole( "everyone", data );
        }

        if ( !GetPermissionsManager().RoleExists( "admin" ) )
        {
            for ( int i = 0; i < data.Count(); i++ )
            {
                string s = data[i];
                s.Replace( "0", "2" );
                data.Remove( i );
                data.InsertAt( s, i );
            }

            GetPermissionsManager().CreateRole( "admin", data );
        }
    }

    override void OnUpdate( float timeslice )
    {
        if( !m_bLoaded && !GetDayZGame().IsLoading() )
        {
            m_bLoaded = true;
            OnLoaded();
        } else {
            super.OnUpdate( timeslice );

            m_Tool.OnUpdate( timeslice );
        }
    }

    override void OnPreloadEvent(PlayerIdentity identity, out bool useDB, out vector pos, out float yaw, out int queueTime)
    {
        super.OnPreloadEvent( identity, useDB, pos, yaw, queueTime );

        if ( PERRMISSIONS_FRAMEWORK_DEBUG_MODE_ENABLED )
        {
            queueTime = 0;
        }
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
