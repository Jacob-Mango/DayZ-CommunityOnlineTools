class JMWeatherBase
{
    void Apply()
    {
    }
    
    void Log( PlayerIdentity pidentLog = NULL )
    {
    }
};

class JMWeatherStorm: JMWeatherBase
{
    float Density;
    float Threshold;
    float TimeOut;

    override void Apply()
    {
		GetGame().GetWeather().SetStorm( Density, Threshold, TimeOut );
    }

    override void Log( PlayerIdentity pidentLog = NULL )
    {
        if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Storm " + Density + ", " + Threshold + ", " + TimeOut );
		}
    }
};

class JMWeatherFog: JMWeatherBase
{
    float Forecast;
    float Time;
    float MinDuration;

    override void Apply()
    {
		GetGame().GetWeather().GetFog().Set( Forecast, Time, MinDuration );
    }

    override void Log( PlayerIdentity pidentLog = NULL )
    {
        if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Fog " + Forecast + ", " + Time + ", " + MinDuration );
		}
    }
};

class JMWeatherRain: JMWeatherBase
{
    float Forecast;
    float Time;
    float MinDuration;

    override void Apply()
    {
		GetGame().GetWeather().GetRain().Set( Forecast, Time, MinDuration );
    }

    override void Log( PlayerIdentity pidentLog = NULL )
    {
        if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Rain " + Forecast + ", " + Time + ", " + MinDuration );
		}
    }
};

class JMWeatherOvercast: JMWeatherBase
{
    float Forecast;
    float Time;
    float MinDuration;

    override void Apply()
    {
		GetGame().GetWeather().GetOvercast().Set( Forecast, Time, MinDuration );
    }

    override void Log( PlayerIdentity pidentLog = NULL )
    {
        if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Overcast " + Forecast + ", " + Time + ", " + MinDuration );
		}
    }
};

class JMWeatherWind: JMWeatherBase
{
    vector Dir;
    float Speed;
    float MaxSpeed;

    override void Apply()
    {
		GetGame().GetWeather().SetWind( Dir * Speed );
		// GetGame().GetWeather().SetWindSpeed( Speed );
		GetGame().GetWeather().SetWindMaximumSpeed( MaxSpeed );
    }

    override void Log( PlayerIdentity pidentLog = NULL )
    {
        if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Wind " + Dir + ", " + Speed + ", " + MaxSpeed );
		}
    }
};

class JMWeatherWindFunction: JMWeatherBase
{
    float Min;
    float Max;
    float Speed;

    override void Apply()
    {
		GetGame().GetWeather().SetWindFunctionParams( Min, Max, Speed );
    }

    override void Log( PlayerIdentity pidentLog = NULL )
    {
        if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "WindFunction " + Min + ", " + Max + ", " + Speed );
		}
    }
};

class JMWeatherDate: JMWeatherBase
{
    int Year;
    int Month;
    int Day;
    int Hour;
    int Minute;

    override void Apply()
    {
        GetGame().GetWorld().SetDate( Year, Month, Day, Hour, Minute );
    }

    override void Log( PlayerIdentity pidentLog = NULL )
    {
        if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Date " + Year + "/" + Month + "/" + Day + " " + Hour + ":" + Minute );
		}
    }
};

class JMWeatherRainThreshold: JMWeatherBase
{
    float OvercastMin;
    float OvercastMax;
    float Time;

    override void Apply()
    {
		GetGame().GetWeather().SetRainThresholds( OvercastMin, OvercastMax, Time );
    }

    override void Log( PlayerIdentity pidentLog = NULL )
    {
        if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "RainThreshold " + OvercastMin + ", " + OvercastMax + ", " + Time );
		}
    }
};

class JMWeatherPreset
{
    string Name;
    string Permission;

    float Time;

    autoptr JMWeatherDate Date;

    autoptr JMWeatherStorm Storm;

    autoptr JMWeatherFog PFog;
    autoptr JMWeatherOvercast POvercast;
    autoptr JMWeatherRain PRain;

    autoptr JMWeatherRainThreshold RainThreshold;

    autoptr JMWeatherWind Wind;
    autoptr JMWeatherWindFunction WindFunc;

    void JMWeatherPreset()
    {
        Date = new JMWeatherDate;
        Storm = new JMWeatherStorm;
        PFog = new JMWeatherFog;
        POvercast = new JMWeatherOvercast;
        PRain = new JMWeatherRain;
        RainThreshold = new JMWeatherRainThreshold;
        Wind = new JMWeatherWind;
        WindFunc = new JMWeatherWindFunction;
    }

    void Apply()
    {
        Date.Apply();
        Storm.Apply();
        PFog.Apply();
        POvercast.Apply();
        PRain.Apply();
        RainThreshold.Apply();
        Wind.Apply();
        WindFunc.Apply();
    }

    void Log( PlayerIdentity pidentLogPP = NULL )
    {
        if ( IsMissionHost() )
		{
            Date.Log( pidentLogPP );
            Storm.Log( pidentLogPP );
            PFog.Log( pidentLogPP );
            POvercast.Log( pidentLogPP );
            PRain.Log( pidentLogPP );
            RainThreshold.Log( pidentLogPP );
            Wind.Log( pidentLogPP );
            WindFunc.Log( pidentLogPP );
        }
    }
};