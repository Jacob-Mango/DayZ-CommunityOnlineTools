modded class MissionServer
{
	void MissionServer()
	{		
		m_bLoaded = false;
		
		if ( !g_cotBase )
		{
			g_cotBase = new CommunityOnlineTools;
		}
	}

	void ~MissionServer()
	{
		delete g_cotBase;
	}

	override void OnInit()
	{
		super.OnInit();
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();

		GetCommunityOnlineTools().OnStart();
	}

	override void OnMissionFinish()
	{
		GetCommunityOnlineTools().OnFinish();

		super.OnMissionFinish();
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		ref array< string > data = new array< string >;
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
			GetCommunityOnlineTools().OnUpdate( timeslice );
		}
	}

	override void InvokeOnConnect( PlayerBase player, PlayerIdentity identity )
	{
		super.InvokeOnConnect( player, identity );

		for ( int i = 0; i < GetPermissionsManager().Roles.Count(); i++ )
		{
			GetCommunityOnlineTools().UpdateRole( GetPermissionsManager().Roles.GetElement( i ), identity );
		}
		
		JMPlayerInstance instance;
		GetPermissionsManager().OnClientConnected( identity, instance );
	} 

	override void PlayerDisconnected( PlayerBase player, PlayerIdentity identity, string uid )
	{
		JMPlayerInstance instance;
		if ( GetPermissionsManager().OnClientDisconnected( uid, instance ) )
		{
			GetCommunityOnlineTools().RemoveClient( uid );
		}

		super.PlayerDisconnected( player, identity, uid );
	}
}
