class JMServerInfoModule: JMRenderableModuleBase
{
	static ref ScriptInvoker DATA_UPDATED = new ScriptInvoker();

	protected ref JMServerInfoData m_Data;

	protected float m_TimeSinceLastRefresh;

	void JMServerInfoModule()
	{
		GetRPCManager().AddRPC( "COT_ServerInformation", "LoadData", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT_ServerInformation", "Restart", this, SingeplayerExecutionType.Server );

		GetPermissionsManager().RegisterPermission( "Admin.View" );
		GetPermissionsManager().RegisterPermission( "Admin.Restart" );

		m_Data = new JMServerInfoData;

		//MissionServer.EVENT_INVOKER.Insert( OnEvent );
	}

	void ~JMServerInfoModule()
	{
		//MissionServer.EVENT_INVOKER.Remove( OnEvent );
	}

	override void RegisterKeyMouseBindings() 
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Admin.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/serverinfo_form.layout";
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		//if ( GetGame().IsServer() )
		//	GetGame().ChatPlayer( "#login" );

		if ( GetGame().IsClient() )
			GetRPCManager().SendRPC( "COT_ServerInformation", "LoadData", new Param, false );
	}

	void OnEvent( EventType eventTypeId, Param params ) 
	{
		switch( eventTypeId )
		{
		case ChatMessageEventTypeID:
		{
			ChatMessageEventParams chat_params;
			if ( Class.CastTo( chat_params, params ) )
			{
				Print( "int: " + chat_params.param1 + " string: " + chat_params.param2 + " string: " + chat_params.param3 + " string: " + chat_params.param4 );
			}
			break;
		}
		}
	}

	void RefreshServerInformation()
	{
		if ( GetGame().IsServer() )
		{
			GetGame().ChatPlayer( "#monitor 1" );
		}

		float fps = GetGame().GetFps();
		if ( fps != 0 )
			m_Data.ServerFPS = 1000 / fps;

		m_Data.ActiveAI = GetNumActiveEntities( GetPlayer() );
		m_Data.GameTickTime = GetGame().GetTickTime();
		m_Data.MissionTickTime = GetGame().GetTime();
	}

	override void OnUpdate( float timeslice )
	{
		if ( m_TimeSinceLastRefresh + 1 < GetGame().GetTickTime() )
		{
			m_TimeSinceLastRefresh = GetGame().GetTickTime();
			RefreshServerInformation();

			//! Only for testing, remove later
			if ( GetGame().IsServer() )
			{
				GetRPCManager().SendRPC( "COT_ServerInformation", "LoadData", new Param1< ref JMServerInfoData >( m_Data ), false, NULL );
			}
		}
	}

	void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Server )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.View", sender ) )
				return;
		
			GetRPCManager().SendRPC( "COT_ServerInformation", "LoadData", new Param1< ref JMServerInfoData >( m_Data ), false, sender );
		}

		if ( type == CallType.Client )
		{
			Param1< ref JMServerInfoData > data;
			if ( !ctx.Read( data ) ) return;

			m_Data = data.param1;

			JMServerInfoModule.DATA_UPDATED.Invoke( m_Data );
		}
	}

	void Restart( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Server )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Restart", sender ) )
				return;
		
			GetGame().RestartMission();
		}
	}	
}