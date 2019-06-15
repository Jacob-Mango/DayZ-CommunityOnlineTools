class CommunityOnlineTools
{
	protected bool m_bLoaded;

	void CommunityOnlineTools()
	{
		Print("CommunityOnlineTools::CommunityOnlineTools");

		m_bLoaded = false;

		NewModuleManager();
	}

	void RegisterModules()
	{
		GetModuleManager().RegisterModule( new DebugModule );
		GetModuleManager().RegisterModule( new ServerInformationModule );
		GetModuleManager().RegisterModule( new PlayerModule );
		GetModuleManager().RegisterModule( new ObjectModule );
		GetModuleManager().RegisterModule( new ESPModule );
		GetModuleManager().RegisterModule( new MapEditorModule );
		GetModuleManager().RegisterModule( new TeleportModule );
		GetModuleManager().RegisterModule( new CameraTool );
		GetModuleManager().RegisterModule( new ItemSetSpawnerModule );
		GetModuleManager().RegisterModule( new VehicleSpawnerModule );
		GetModuleManager().RegisterModule( new WeatherModule );
		GetModuleManager().RegisterModule( new COTMapModule );
	}

	void ~CommunityOnlineTools()
	{
		Print("CommunityOnlineTools::~CommunityOnlineTools");
	}
	
	void OnStart()
	{
		GetModuleManager().RegisterModules();

		RegisterModules();

		GetModuleManager().OnInit();
		
		GetModuleManager().ReloadSettings();

		GetModuleManager().OnMissionStart();
	}

	void OnFinish()
	{
		GetModuleManager().OnMissionFinish();

		NewModuleManager();
	}

	void OnLoaded()
	{
		GetModuleManager().OnMissionLoaded();
	}

	void OnUpdate( float timeslice )
	{
		if( !m_bLoaded && !GetDayZGame().IsLoading() )
		{
			m_bLoaded = true;
			OnLoaded();
		} else {
			GetModuleManager().OnUpdate( timeslice );
		}
	}
}