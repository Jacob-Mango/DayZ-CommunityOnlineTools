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

		preset.PDate.Year = 2019;
		preset.PDate.Month = 5;
		preset.PDate.Day = 23;
		preset.PDate.Hour = 12;
		preset.PDate.Minute = 25;

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

		preset.PDate.Year = 2019;
		preset.PDate.Month = 2;
		preset.PDate.Day = 18;
		preset.PDate.Hour = 9;
		preset.PDate.Minute = 5;

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
};
