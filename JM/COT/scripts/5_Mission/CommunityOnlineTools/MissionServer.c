modded class MissionServer
{
	static ref ScriptInvoker EVENT_INVOKER = new ScriptInvoker();

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

	override void OnEvent(EventType eventTypeId, Param params) 
	{
		EVENT_INVOKER.Invoke( eventTypeId, params );

		super.OnEvent( eventTypeId, params );
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		if ( m_bLoaded )
		{
			GetCommunityOnlineTools().OnUpdate( timeslice );
		}
	}

	override void InvokeOnConnect( PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect( player, identity );

		for ( int i = 0; i < GetPermissionsManager().Roles.Count(); i++ )
		{
			JMRole role = GetPermissionsManager().Roles[i];
			role.Serialize();
			GetRPCManager().SendRPC( "COT", "UpdateRole", new Param2< string, ref array< string > >( role.Name, role.SerializedData ), true, identity );
		}

		GetRPCManager().SendRPC( "COT", "SetClientPlayer", new Param2< ref JMPlayerInformation, PlayerIdentity >( SerializePlayer( GetPermissionsManager().PlayerJoined( identity ) ), identity ), true, identity );

		GetGame().SelectPlayer( identity, player );
	} 

	override void InvokeOnDisconnect( PlayerBase player )
	{
		super.InvokeOnDisconnect( player );
	} 

}
