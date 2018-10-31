modded class MissionServer
{
	protected bool HIVE_ENABLED = true; 

    protected bool m_bLoaded;

	void MissionServer()
	{
	    Print( "MissionServer::MissionServer()" );
	    m_bLoaded = false;
	}

	void ~MissionServer()
	{
	    Print( "MissionServer::~MissionServer()" );
	}

	override void OnInit()
	{
		super.OnInit();

        InitHive();

        SetupWeather();
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();
	
        GetModuleManager().OnMissionStart();
	}

	override void OnMissionFinish()
	{
        GetModuleManager().OnMissionFinish();

		if( GetHive() )
		{
			DestroyHive();
		}

		super.OnMissionFinish();
	}

    void OnMissionLoaded()
    {
		GetModuleManager().OnMissionLoaded();
    }

	override void OnUpdate( float timeslice )
	{
	    super.OnUpdate( timeslice );

        if( !m_bLoaded && !GetDayZGame().IsLoading() )
        {
            m_bLoaded = true;
            OnMissionLoaded();
        } else {
            GetModuleManager().OnUpdate( timeslice );
        }
	}

	override void OnMouseButtonRelease( int button )
	{
		super.OnMouseButtonRelease( button );

		GetModuleManager().OnMouseButtonRelease( button );
	}

	override void OnMouseButtonPress( int button )
	{
		super.OnMouseButtonPress( button );

		GetModuleManager().OnMouseButtonPress( button );
	}

	override void OnKeyPress( int key )
	{
		super.OnKeyPress(key);

		GetModuleManager().OnKeyPress( key );
    }

    override void OnKeyRelease( int key )
	{
		super.OnKeyRelease( key );

		GetModuleManager().OnKeyRelease( key );
	}

	void InitHive()
	{
		if ( GetGame().IsClient() && GetGame().IsMultiplayer() ) return;

		// RD /s /q "storage_-1" > nul 2>&1
		if ( !HIVE_ENABLED ) return;
	
		Hive oHive = GetHive();
		
		if( !oHive )
		{
			oHive = CreateHive();
		}

		if( oHive )
		{
			oHive.InitOffline();
		}

		oHive.SetShardID("100");
		oHive.SetEnviroment("stable");
	}

    static void SetupWeather()
    {
        Weather weather = g_Game.GetWeather();

        weather.GetOvercast().SetLimits( 0.0 , 2.0 );
        weather.GetRain().SetLimits( 0.0 , 2.0 );
        weather.GetFog().SetLimits( 0.0 , 2.0 );

        weather.GetOvercast().SetForecastChangeLimits( 0.0, 0.0 );
        weather.GetRain().SetForecastChangeLimits( 0.0, 0.0 );
        weather.GetFog().SetForecastChangeLimits( 0.0, 0.0 );

        weather.GetOvercast().SetForecastTimeLimits( 1800 , 1800 );
        weather.GetRain().SetForecastTimeLimits( 600 , 600 );
        weather.GetFog().SetForecastTimeLimits( 600 , 600 );

        weather.GetOvercast().Set( 0.0, 0, 0 );
        weather.GetRain().Set( 0.0, 0, 0 );
        weather.GetFog().Set( 0.0, 0, 0 );

        weather.SetWindMaximumSpeed( 50 );
        weather.SetWindFunctionParams( 0, 0, 1 );
    }
}
