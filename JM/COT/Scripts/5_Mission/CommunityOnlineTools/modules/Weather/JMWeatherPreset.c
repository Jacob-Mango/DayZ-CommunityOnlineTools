class JMWeatherBase
{
	void Apply()
	{
	}

	void SetFromWorld()
	{

	}
	
	void Log( PlayerIdentity pidentLog )
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

	override void SetFromWorld()
	{
		Density = 0.0;
		Threshold = 0.5;
		TimeOut = 60.0;
	}

	override void Log( PlayerIdentity pidentLog )
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

	override void SetFromWorld()
	{
		Forecast = GetGame().GetWeather().GetFog().GetForecast();
		Time = GetGame().GetWeather().GetFog().GetNextChange();
		MinDuration = 4000.0;
	}

	override void Log( PlayerIdentity pidentLog )
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

	override void SetFromWorld()
	{
		Forecast = GetGame().GetWeather().GetRain().GetForecast();
		Time = GetGame().GetWeather().GetRain().GetNextChange();
		MinDuration = 4000.0;
	}

	override void Log( PlayerIdentity pidentLog )
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

	override void SetFromWorld()
	{
		Forecast = GetGame().GetWeather().GetOvercast().GetForecast();
		Time = GetGame().GetWeather().GetOvercast().GetNextChange();
		MinDuration = 4000.0;
	}

	override void Log( PlayerIdentity pidentLog )
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

	override void SetFromWorld()
	{
		Dir = GetGame().GetWeather().GetWind();
		Speed = Dir.Normalize();
		MaxSpeed = GetGame().GetWeather().GetWindMaximumSpeed();
	}

	override void Log( PlayerIdentity pidentLog )
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

	override void SetFromWorld()
	{
		GetGame().GetWeather().GetWindFunctionParams( Min, Max, Speed );
	}

	override void Log( PlayerIdentity pidentLog )
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

	override void SetFromWorld()
	{
		GetGame().GetWorld().GetDate( Year, Month, Day, Hour, Minute );
	}

	override void Log( PlayerIdentity pidentLog )
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

	override void SetFromWorld()
	{
		OvercastMin = 0.75;
		OvercastMax = 1.0;
		Time = 30.0;
	}

	override void Log( PlayerIdentity pidentLog )
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

	float Time;

	autoptr JMWeatherDate PDate;

	autoptr JMWeatherStorm Storm;

	autoptr JMWeatherFog PFog;
	autoptr JMWeatherOvercast POvercast;
	autoptr JMWeatherRain PRain;

	autoptr JMWeatherRainThreshold RainThreshold;

	autoptr JMWeatherWind Wind;
	autoptr JMWeatherWindFunction WindFunc;

	void JMWeatherPreset()
	{
		PDate = new JMWeatherDate;
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
		PDate.Apply();
		Storm.Apply();
		PFog.Apply();
		POvercast.Apply();
		PRain.Apply();
		RainThreshold.Apply();
		Wind.Apply();
		WindFunc.Apply();
	}

	void SetFromWorld()
	{
		PDate.SetFromWorld();
		Storm.SetFromWorld();
		PFog.SetFromWorld();
		POvercast.SetFromWorld();
		PRain.SetFromWorld();
		RainThreshold.SetFromWorld();
		Wind.SetFromWorld();
		WindFunc.SetFromWorld();
	}

	void Log( PlayerIdentity pidentLogPP )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLogPP, "Start Weather Preset " + Name );

			PDate.Log( pidentLogPP );
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