modded class MissionServer
{
	protected ref CommunityOnlineTools m_COT;
	protected ref PermissionsFramework m_PF;

	void MissionServer()
	{		
		m_bLoaded = false;

		m_COT = new ref CommunityOnlineTools;

		m_PF = new ref PermissionsFramework;
	}

	void ~MissionServer()
	{
		delete m_PF;
	}

	override void OnInit()
	{
		super.OnInit();
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();

		m_COT.OnStart();

		m_PF.OnStart();
	}

	override void OnMissionFinish()
	{
		m_PF.OnFinish();

		m_COT.OnFinish();

		super.OnMissionFinish();
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

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
		super.OnUpdate( timeslice );

		if ( m_bLoaded )
		{
			m_PF.Update( timeslice );

			m_COT.OnUpdate( timeslice );
		}
	}

	override void OnPreloadEvent(PlayerIdentity identity, out bool useDB, out vector pos, out float yaw, out int queueTime)
	{
		super.OnPreloadEvent( identity, useDB, pos, yaw, queueTime );

		GetPermissionsManager().GetPlayerByIdentity( identity );

		// queueTime = 0;
	}

	override void InvokeOnConnect( PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect( player, identity );

		for ( int i = 0; i < GetPermissionsManager().Roles.Count(); i++ )
		{
			ref Role role = GetPermissionsManager().Roles[i];
			role.Serialize();
			GetRPCManager().SendRPC( "PermissionsFramework", "UpdateRole", new Param2< string, ref array< string > >( role.Name, role.SerializedData ), true, identity );
		}

		GetRPCManager().SendRPC( "PermissionsFramework", "SetClientPlayer", new Param1< ref PlayerData >( SerializePlayer( GetPermissionsManager().GetPlayerByIdentity( identity ) ) ), true, identity );

		GetGame().SelectPlayer( identity, player );
	} 

	override void InvokeOnDisconnect( PlayerBase player )
	{
		GetPermissionsManager().PlayerLeft( player.GetIdentity() );

		super.InvokeOnDisconnect( player );
	} 

}
