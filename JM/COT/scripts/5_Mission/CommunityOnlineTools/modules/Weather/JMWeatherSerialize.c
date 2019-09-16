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
		} else 
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
		DefaultPreset();
		TestPreset();
	}

	private void DefaultPreset()
	{
		JMWeatherPreset preset = new JMWeatherPreset;

		preset.Name = "Default";
		preset.Permission = "Default";

		preset.Date.Year = 2019;
		preset.Date.Month = 5;
		preset.Date.Day = 23;
		preset.Date.Hour = 12;
		preset.Date.Minute = 25;

		preset.PFog.Forecast = 0.1;
		preset.PFog.Time = 100;
		preset.PFog.MinDuration = 100;

		preset.POvercast.Forecast = 0.1;
		preset.POvercast.Time = 100;
		preset.POvercast.MinDuration = 100;

		preset.PRain.Forecast = 0.1;
		preset.PRain.Time = 100;
		preset.PRain.MinDuration = 100;

		Presets.Insert( preset );
	}

	private void TestPreset()
	{
		JMWeatherPreset preset = new JMWeatherPreset;

		preset.Name = "Test";
		preset.Permission = "Test";

		preset.Date.Year = 2019;
		preset.Date.Month = 2;
		preset.Date.Day = 18;
		preset.Date.Hour = 9;
		preset.Date.Minute = 5;

		preset.PFog.Forecast = 0.1;
		preset.PFog.Time = 100;
		preset.PFog.MinDuration = 100;

		preset.POvercast.Forecast = 0.1;
		preset.POvercast.Time = 100;
		preset.POvercast.MinDuration = 100;

		preset.PRain.Forecast = 0.1;
		preset.PRain.Time = 100;
		preset.PRain.MinDuration = 100;

		preset.RainThreshold.OvercastMin = 0.05;
		preset.RainThreshold.OvercastMax = 1.0;
		preset.RainThreshold.Time = 5;

		Presets.Insert( preset );
	}
}