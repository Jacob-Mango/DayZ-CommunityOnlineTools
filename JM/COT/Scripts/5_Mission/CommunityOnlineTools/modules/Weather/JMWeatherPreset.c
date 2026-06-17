class JMWeatherBase
{
	void Apply()
	{
	}

	void SetFromWorld()
	{
	}

	void StopCurrentChangeInProgress()
	{
	}

	void ResumeCurrentChangeInProgress()
	{
	}
	
	void Log( PlayerIdentity pidentLog )
	{
	}
}

class JMWeatherStorm: JMWeatherBase
{
	float Density;
	float Threshold;
	float MinTimeBetweenLightning;

	override void Apply()
	{
		if (Density != -1)
			g_Game.GetWeather().SetStorm( Density, Threshold, MinTimeBetweenLightning );
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
}

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
				return g_Game.GetWeather().GetFog();
			case JMWeatherRain:
				return g_Game.GetWeather().GetRain();
			case JMWeatherSnow:
				return g_Game.GetWeather().GetSnowfall();
			case JMWeatherWindMagnitude:
				return g_Game.GetWeather().GetWindMagnitude();
			case JMWeatherWindDirection:
				return g_Game.GetWeather().GetWindDirection();
			case JMWeatherOvercast:
				return g_Game.GetWeather().GetOvercast();
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

	override void StopCurrentChangeInProgress()
	{
		if (Actual != Forecast)
		{
			float forecast = Forecast;
			float time = Time;
			float minDuration = MinDuration;

			Forecast = Actual;
			Time = 0;
			MinDuration = GetPhenomenon().GetNextChange();

			Apply();

			Forecast = forecast;
			Time = time;
			MinDuration = minDuration;
		}
	}

	override void ResumeCurrentChangeInProgress()
	{
		if (Actual != Forecast)
		{
			float change = Forecast - Actual;
			float timeToForecast = Math.AbsFloat(change) * 100;  //! Each 1% of change = one second
			float timeUntilNextChange = GetPhenomenon().GetNextChange();

			if (timeToForecast > timeUntilNextChange)
			{
				Forecast = Actual + change * timeUntilNextChange * 0.01;
				timeToForecast = timeUntilNextChange;
			}

			float time = Time;
			float minDuration = MinDuration;

			Time = timeToForecast;
			MinDuration = timeUntilNextChange - timeToForecast;

			Apply();

			Time = time;
			MinDuration = minDuration;
			GetPhenomenon().SetNextChange(timeUntilNextChange);
		}
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
}

class JMWeatherFog: JMWeatherPhenomenon
{
}

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
			g_Game.GetWeather().SetDynVolFogDistanceDensity( Distance, Time );
			g_Game.GetWeather().SetDynVolFogHeightDensity( Height, Time );
			g_Game.GetWeather().SetDynVolFogHeightBias( Bias, Time );
		}
	}

	override void SetFromWorld()
	{
		Distance = g_Game.GetWeather().GetDynVolFogDistanceDensity();
		Height = g_Game.GetWeather().GetDynVolFogHeightDensity();
		Bias = g_Game.GetWeather().GetDynVolFogHeightBias();
	}

	override void StopCurrentChangeInProgress()
	{
		Time = 0;

		Apply();
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "DynamicFog " + Distance + ", " + Height + ", " + Bias );
		}
	}
}

class JMWeatherRain: JMWeatherPhenomenon
{
}

class JMWeatherSnow: JMWeatherPhenomenon
{
}

class JMWeatherOvercast: JMWeatherPhenomenon
{
}

class JMWeatherWindMagnitude: JMWeatherPhenomenon
{
}

class JMWeatherWindDirection: JMWeatherPhenomenon
{
}

class JMWeatherWindFunction: JMWeatherBase
{
	float Min;
	float Max;
	float Speed;

	override void Apply()
	{
		if (Speed != -1)
			g_Game.GetWeather().SetWindFunctionParams( Min, Max, Speed );
	}

	override void SetFromWorld()
	{
		g_Game.GetWeather().GetWindFunctionParams( Min, Max, Speed );
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "WindFunction " + Min + ", " + Max + ", " + Speed );
		}
	}
}

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
			g_Game.GetWorld().SetDate( Year, Month, Day, Hour, Minute );
	}

	override void SetFromWorld()
	{
		g_Game.GetWorld().GetDate( Year, Month, Day, Hour, Minute );
	}

	override void Log( PlayerIdentity pidentLog )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLog, "Date " + Year + "/" + Month + "/" + Day + " " + Hour + ":" + Minute );
		}
	}
}

class JMWeatherRainThreshold: JMWeatherBase
{
	float OvercastMin;
	float OvercastMax;
	float Time;

	override void Apply()
	{
		if (Time != -1)
			g_Game.GetWeather().SetRainThresholds( OvercastMin, OvercastMax, Time );
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
}

class JMWeatherSnowThreshold: JMWeatherBase
{
	float OvercastMin;
	float OvercastMax;
	float Time;

	override void Apply()
	{
		if (Time != -1)
			g_Game.GetWeather().SetSnowfallThresholds( OvercastMin, OvercastMax, Time );
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
}

class JMWeatherPreset
{
	string Name;

	float Time;

	autoptr JMWeatherDate PDate = new JMWeatherDate;

	autoptr JMWeatherStorm Storm = new JMWeatherStorm;
	autoptr JMWeatherOvercast POvercast = new JMWeatherOvercast;

	autoptr JMWeatherFog PFog = new JMWeatherFog;
	autoptr JMWeatherDynamicFog PDynFog = new JMWeatherDynamicFog;

	autoptr JMWeatherRain PRain = new JMWeatherRain;
	autoptr JMWeatherRainThreshold RainThreshold = new JMWeatherRainThreshold;
	
	autoptr JMWeatherSnow PSnow = new JMWeatherSnow;
	autoptr JMWeatherSnowThreshold SnowThreshold = new JMWeatherSnowThreshold;

	autoptr JMWeatherWindMagnitude PWindMagnitude = new JMWeatherWindMagnitude;
	autoptr JMWeatherWindDirection PWindDirection = new JMWeatherWindDirection;
	autoptr JMWeatherWindFunction WindFunc = new JMWeatherWindFunction;

	array<JMWeatherBase> GetPhenomena()
	{
		array<JMWeatherBase> phenomena = {};

		typename e = Type();
		int cnt = e.GetVariableCount();
		JMWeatherBase phenomenom;

		for (int i = 0; i < cnt; ++i)
		{
			if (e.GetVariableValue(this, i, phenomenom))
				phenomena.Insert(phenomenom);
		}

		return phenomena;
	}

	void Apply()
	{
		array<JMWeatherBase> phenomena = GetPhenomena();

		foreach (JMWeatherBase phenomenom: phenomena)
		{
			phenomenom.Apply();
		}
	}

	void SetFromWorld()
	{
		array<JMWeatherBase> phenomena = GetPhenomena();

		foreach (JMWeatherBase phenomenom: phenomena)
		{
			phenomenom.SetFromWorld();
		}
	}

	void StopCurrentChangesInProgress()
	{
		array<JMWeatherBase> phenomena = GetPhenomena();

		foreach (JMWeatherBase phenomenom: phenomena)
		{
			phenomenom.StopCurrentChangeInProgress();
		}
	}

	void ResumeCurrentChangesInProgress()
	{
		array<JMWeatherBase> phenomena = GetPhenomena();

		foreach (JMWeatherBase phenomenom: phenomena)
		{
			phenomenom.ResumeCurrentChangeInProgress();
		}
	}

	void Log( PlayerIdentity pidentLogPP )
	{
		if ( IsMissionHost() )
		{
			GetCommunityOnlineToolsBase().Log( pidentLogPP, "Start Weather Preset " + Name );

			array<JMWeatherBase> phenomena = GetPhenomena();

			foreach (JMWeatherBase phenomenom: phenomena)
			{
				phenomenom.Log(pidentLogPP);
			}
		}
	}
}
