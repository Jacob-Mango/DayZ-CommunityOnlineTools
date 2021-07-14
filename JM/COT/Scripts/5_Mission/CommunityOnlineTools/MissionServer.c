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

		g_cotBase.OnStart();
	}

	override void OnMissionFinish()
	{
		g_cotBase.OnFinish();

		super.OnMissionFinish();
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		g_cotBase.OnUpdate( timeslice );
	}
}
