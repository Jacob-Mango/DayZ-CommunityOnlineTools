class JMServerInfoModule: JMRenderableModuleBase
{
	static ref ScriptInvoker DATA_UPDATED = new ScriptInvoker();

	protected ref JMServerInfoData m_Data;

	protected float m_TimeSinceLastRefresh;

	void JMServerInfoModule()
	{
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
	}

	void RefreshServerInformation()
	{
		float fps = GetGame().GetFps();
		if ( fps != 0 )
			m_Data.ServerFPS = 1000.0 / fps;

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
		}
	}
}