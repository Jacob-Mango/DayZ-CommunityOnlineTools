class JMWeatherSerialize
{
	autoptr array< ref JMWeatherPreset > Presets;

	void JMWeatherSerialize()
	{
		Presets = new array< ref JMWeatherPreset >;
	}

	static JMWeatherSerialize Load()
	{
		JMWeatherSerialize settings = new JMWeatherSerialize();

		if ( FileExist( JMConstants.FILE_WEATHER ) )
		{
			JsonFileLoader<JMWeatherSerialize>.JsonLoadFile( JMConstants.FILE_WEATHER, settings );
		}
		else
		{
			settings.Defaults();
			settings.Save();
		}

		return settings;
	}

	void Save()
	{
		JsonFileLoader<JMWeatherSerialize>.JsonSaveFile( JMConstants.FILE_WEATHER, this );
	}

	void Defaults()
	{
		PresetNoRain();
		PresetNoSnow();
		PresetNoFog();
		PresetNoRainAndFog();
	}

	private void PresetNoRain()
	{
		JMWeatherPreset preset = new JMWeatherPreset;

		preset.Name = "No Rain";

		preset.PDate.Year = -1;
		preset.PDate.Month = -1;
		preset.PDate.Day = -1;
		preset.PDate.Hour = -1;
		preset.PDate.Minute = -1;
		
		preset.Storm.Density = -1;
		preset.Storm.Threshold = -1;
		preset.Storm.MinTimeBetweenLightning = -1;

		preset.POvercast.Forecast = 0.0;
		preset.POvercast.Time = 120;
		preset.POvercast.MinDuration = 21600;

		preset.PRain.Forecast = 0.0;
		preset.PRain.Time = 120;
		preset.PRain.MinDuration = 21600;

		preset.RainThreshold.OvercastMin = -1;
		preset.RainThreshold.OvercastMax = -1;
		preset.RainThreshold.Time = -1;

		preset.PSnow.Forecast = 0.0;
		preset.PSnow.Time = 120;
		preset.PSnow.MinDuration = 21600;

		preset.SnowThreshold.OvercastMin = -1;
		preset.SnowThreshold.OvercastMax = -1;
		preset.SnowThreshold.Time = -1;

		preset.PWindMagnitude.Forecast = 0.0;
		preset.PWindMagnitude.Time = 120;
		preset.PWindMagnitude.MinDuration = 21600;

		preset.PWindDirection.Forecast = 0.0;
		preset.PWindDirection.Time = 120;
		preset.PWindDirection.MinDuration = 21600;

		preset.WindFunc.Min = -1;
		preset.WindFunc.Max = -1;
		preset.WindFunc.Speed = -1;

		Presets.Insert( preset );
	}

	private void PresetNoSnow()
	{
		JMWeatherPreset preset = new JMWeatherPreset;

		preset.Name = "No Snow";

		preset.PDate.Year = -1;
		preset.PDate.Month = -1;
		preset.PDate.Day = -1;
		preset.PDate.Hour = -1;
		preset.PDate.Minute = -1;
		
		preset.Storm.Density = -1;
		preset.Storm.Threshold = -1;
		preset.Storm.MinTimeBetweenLightning = -1;

		preset.POvercast.Forecast = 0.0;
		preset.POvercast.Time = 120;
		preset.POvercast.MinDuration = 21600;

		preset.PRain.Forecast = -1;
		preset.PRain.Time = -1;
		preset.PRain.MinDuration = -1;

		preset.RainThreshold.OvercastMin = -1;
		preset.RainThreshold.OvercastMax = -1;
		preset.RainThreshold.Time = -1;

		preset.PSnow.Forecast = 0.0;
		preset.PSnow.Time = 120;
		preset.PSnow.MinDuration = 21600;

		preset.SnowThreshold.OvercastMin = -1;
		preset.SnowThreshold.OvercastMax = -1;
		preset.SnowThreshold.Time = -1;

		preset.PWindMagnitude.Forecast = 0.0;
		preset.PWindMagnitude.Time = 120;
		preset.PWindMagnitude.MinDuration = 21600;

		preset.PWindDirection.Forecast = 0.0;
		preset.PWindDirection.Time = 120;
		preset.PWindDirection.MinDuration = 21600;

		preset.WindFunc.Min = -1;
		preset.WindFunc.Max = -1;
		preset.WindFunc.Speed = -1;

		Presets.Insert( preset );
	}

	private void PresetNoFog()
	{
		JMWeatherPreset preset = new JMWeatherPreset;

		preset.Name = "No Fog";

		preset.PDate.Year = -1;
		preset.PDate.Month = -1;
		preset.PDate.Day = -1;
		preset.PDate.Hour = -1;
		preset.PDate.Minute = -1;
		
		preset.Storm.Density = -1;
		preset.Storm.Threshold = -1;
		preset.Storm.MinTimeBetweenLightning = -1;

		preset.POvercast.Forecast = 0.0;
		preset.POvercast.Time = 120.0;
		preset.POvercast.MinDuration = 21600;

		preset.PRain.Forecast = -1;
		preset.PRain.Time = -1;
		preset.PRain.MinDuration = -1;

		preset.RainThreshold.OvercastMin = -1;
		preset.RainThreshold.OvercastMax = -1;
		preset.RainThreshold.Time = -1;

		preset.PSnow.Forecast = 0.0;
		preset.PSnow.Time = 120;
		preset.PSnow.MinDuration = 21600;

		preset.SnowThreshold.OvercastMin = -1;
		preset.SnowThreshold.OvercastMax = -1;
		preset.SnowThreshold.Time = -1;

		preset.PWindMagnitude.Forecast = 0.0;
		preset.PWindMagnitude.Time = 120;
		preset.PWindMagnitude.MinDuration = 21600;

		preset.PWindDirection.Forecast = 0.0;
		preset.PWindDirection.Time = 120;
		preset.PWindDirection.MinDuration = 21600;

		preset.WindFunc.Min = -1;
		preset.WindFunc.Max = -1;
		preset.WindFunc.Speed = -1;

		preset.PFog.Forecast = 0.0;
		preset.PFog.Time = 120.0;
		preset.PFog.MinDuration = 21600;

		preset.PDynFog.Distance = 0;
		preset.PDynFog.Height = 0;
		preset.PDynFog.Bias = 0;


		Presets.Insert( preset );
	}

	private void PresetNoRainAndFog()
	{
		JMWeatherPreset preset = new JMWeatherPreset;

		preset.Name = "No Rain and Fog";

		preset.PDate.Year = -1;
		preset.PDate.Month = -1;
		preset.PDate.Day = -1;
		preset.PDate.Hour = -1;
		preset.PDate.Minute = -1;
		
		preset.Storm.Density = -1;
		preset.Storm.Threshold = -1;
		preset.Storm.MinTimeBetweenLightning = -1;

		preset.POvercast.Forecast = 0.0;
		preset.POvercast.Time = 120.0;
		preset.POvercast.MinDuration = 21600;

		preset.PRain.Forecast = -1;
		preset.PRain.Time = -1;
		preset.PRain.MinDuration = -1;

		preset.RainThreshold.OvercastMin = -1;
		preset.RainThreshold.OvercastMax = -1;
		preset.RainThreshold.Time = -1;

		preset.PSnow.Forecast = 0.0;
		preset.PSnow.Time = 120;
		preset.PSnow.MinDuration = 21600;

		preset.SnowThreshold.OvercastMin = -1;
		preset.SnowThreshold.OvercastMax = -1;
		preset.SnowThreshold.Time = -1;

		preset.PWindMagnitude.Forecast = 0.0;
		preset.PWindMagnitude.Time = 120;
		preset.PWindMagnitude.MinDuration = 21600;

		preset.PWindDirection.Forecast = 0.0;
		preset.PWindDirection.Time = 120;
		preset.PWindDirection.MinDuration = 21600;

		preset.WindFunc.Min = -1;
		preset.WindFunc.Max = -1;
		preset.WindFunc.Speed = -1;

		preset.PFog.Forecast = 0.0;
		preset.PFog.Time = 120;
		preset.PFog.MinDuration = 21600;

		preset.PDynFog.Distance = 0;
		preset.PDynFog.Height = 0;
		preset.PDynFog.Bias = 0;

		Presets.Insert( preset );
	}
};
