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
	float MinTimeBetweenLightning;

	override void Apply()
	{
		if (Density != -1)
			GetGame().GetWeather().SetStorm( Density, Threshold, MinTimeBetweenLightning );
	}

	override void SetFromWorld()
	{
		Density = 1.0;
		Threshold = 0.7;
		MinTimeBetweenLightning = 25.0;
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Storm " + Density + ", " + Threshold + ", " + MinTimeBetweenLightning );
		}
	}
};

class JMWeatherPhenomenon: JMWeatherBase
{
	float Forecast;
	float Time;
	float MinDuration;

	[NonSerialized()]
	float Actual;

	WeatherPhenomenon GetPhenomenon()
	{
		switch (Type())
		{
			case JMWeatherFog:
				return GetGame().GetWeather().GetFog();
			case JMWeatherRain:
				return GetGame().GetWeather().GetRain();
			case JMWeatherSnow:
				return GetGame().GetWeather().GetSnowfall();
			case JMWeatherWindMagnitude:
				return GetGame().GetWeather().GetWindMagnitude();
			case JMWeatherWindDirection:
				return GetGame().GetWeather().GetWindDirection();
			case JMWeatherOvercast:
				return GetGame().GetWeather().GetOvercast();
		}

		return null;
	}

	override void Apply()
	{
		if (Forecast != -1)
			GetPhenomenon().Set( Forecast, Time, MinDuration );
	}

	override void SetFromWorld()
	{
		Forecast = GetPhenomenon().GetForecast();
		Time = GetPhenomenon().GetNextChange();
		MinDuration = 240.0;
		Actual = GetPhenomenon().GetActual();
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			string type = ClassName();
			type.Replace("JMWeather", "");
			GetCommunityOnlineToolsBase().Log( pidentLog, type + " " + Forecast + ", " + Time + ", " + MinDuration );
		}
	}
};

class JMWeatherFog: JMWeatherPhenomenon
{
};

class JMWeatherDynamicFog: JMWeatherBase
{
	float Distance;
	float Height;
	float Bias;
	float Time;

	override void Apply()
	{
		if (Distance != -1)
		{
			GetGame().GetWeather().SetDynVolFogDistanceDensity( Distance, Time );
			GetGame().GetWeather().SetDynVolFogHeightDensity( Height, Time );
			GetGame().GetWeather().SetDynVolFogHeightBias( Bias, Time );
		}
	}

	override void SetFromWorld()
	{
		Distance = GetGame().GetWeather().GetDynVolFogDistanceDensity();
		Height = GetGame().GetWeather().GetDynVolFogHeightDensity();
		Bias = GetGame().GetWeather().GetDynVolFogHeightBias();
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "DynamicFog " + Distance + ", " + Height + ", " + Bias );
		}
	}
};

class JMWeatherRain: JMWeatherPhenomenon
{
};

class JMWeatherSnow: JMWeatherPhenomenon
{
};

class JMWeatherOvercast: JMWeatherPhenomenon
{
};

class JMWeatherWindMagnitude: JMWeatherPhenomenon
{
};

class JMWeatherWindDirection: JMWeatherPhenomenon
{
};

class JMWeatherWindFunction: JMWeatherBase
{
	float Min;
	float Max;
	float Speed;

	override void Apply()
	{
		if (Speed != -1)
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
		if (Year != -1)
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
		if (Time != -1)
			GetGame().GetWeather().SetRainThresholds( OvercastMin, OvercastMax, Time );
	}

	override void SetFromWorld()
	{
		OvercastMin = 0.5;
		OvercastMax = 1.0;
		Time = 120.0;
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "RainThreshold " + OvercastMin + ", " + OvercastMax + ", " + Time );
		}
	}
};

class JMWeatherSnowThreshold: JMWeatherBase
{
	float OvercastMin;
	float OvercastMax;
	float Time;

	override void Apply()
	{
		if (Time != -1)
			GetGame().GetWeather().SetSnowfallThresholds( OvercastMin, OvercastMax, Time );
	}

	override void SetFromWorld()
	{
		OvercastMin = 0.5;
		OvercastMax = 1.0;
		Time = 120.0;
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "SnowfallThreshold " + OvercastMin + ", " + OvercastMax + ", " + Time );
		}
	}
};

class JMWeatherPreset
{
	string Name;

	float Time;

	autoptr JMWeatherDate PDate;

	autoptr JMWeatherStorm Storm;
	autoptr JMWeatherOvercast POvercast;

	autoptr JMWeatherFog PFog;
	autoptr JMWeatherDynamicFog PDynFog;

	autoptr JMWeatherRain PRain;
	autoptr JMWeatherRainThreshold RainThreshold;
	
	autoptr JMWeatherSnow PSnow;
	autoptr JMWeatherSnowThreshold SnowThreshold;

	autoptr JMWeatherWindMagnitude PWindMagnitude;
	autoptr JMWeatherWindDirection PWindDirection;
	autoptr JMWeatherWindFunction WindFunc;

	void JMWeatherPreset()
	{
		PDate = new JMWeatherDate;

		Storm = new JMWeatherStorm;
		POvercast = new JMWeatherOvercast;

		PFog = new JMWeatherFog;
		PDynFog = new JMWeatherDynamicFog;

		PRain = new JMWeatherRain;
		RainThreshold = new JMWeatherRainThreshold;

		PSnow = new JMWeatherSnow;
		SnowThreshold = new JMWeatherSnowThreshold;

		PWindMagnitude = new JMWeatherWindMagnitude;
		PWindDirection = new JMWeatherWindDirection;
		WindFunc = new JMWeatherWindFunction;
	}

	void Apply()
	{
		PDate.Apply();

		Storm.Apply();
		POvercast.Apply();

		PFog.Apply();
		PDynFog.Apply();

		PRain.Apply();
		RainThreshold.Apply();

		PSnow.Apply();
		SnowThreshold.Apply();

		PWindMagnitude.Apply();
		PWindDirection.Apply();
		WindFunc.Apply();
	}

	void SetFromWorld()
	{
		PDate.SetFromWorld();

		Storm.SetFromWorld();
		POvercast.SetFromWorld();

		PFog.SetFromWorld();
		PDynFog.SetFromWorld();

		PRain.SetFromWorld();
		RainThreshold.SetFromWorld();

		PSnow.SetFromWorld();
		SnowThreshold.SetFromWorld();

		PWindMagnitude.SetFromWorld();
		PWindDirection.SetFromWorld();
		WindFunc.SetFromWorld();
	}

	void Log( PlayerIdentity pidentLogPP )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLogPP, "Start Weather Preset " + Name );

			PDate.Log( pidentLogPP );

			Storm.Log( pidentLogPP );
			POvercast.Log( pidentLogPP );

			PFog.Log( pidentLogPP );
			PDynFog.Log( pidentLogPP );

			PRain.Log( pidentLogPP );
			RainThreshold.Log( pidentLogPP );

			PSnow.Log( pidentLogPP );
			SnowThreshold.Log( pidentLogPP );

			PWindMagnitude.Log( pidentLogPP );
			PWindDirection.Log( pidentLogPP );
			WindFunc.Log( pidentLogPP );
		}
	}
};