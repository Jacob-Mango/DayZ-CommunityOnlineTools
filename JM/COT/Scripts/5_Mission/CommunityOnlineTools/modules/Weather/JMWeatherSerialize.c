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
		PresetNoRain();
		PresetNoRainAndFog();
		PresetPissingCows();
	}

	private void PresetNoRain()
	{
		JMWeatherPreset preset = new JMWeatherPreset;

		preset.Name = "No Rain";

		preset.POvercast.Forecast = 0.0;
		preset.POvercast.Time = 180;
		preset.POvercast.MinDuration = 21600;

		preset.PRain.Forecast = 0.0;
		preset.PRain.Time = 180;
		preset.PRain.MinDuration = 21600;

		Presets.Insert( preset );
	}

	private void PresetNoRainAndFog()
	{
		JMWeatherPreset preset = new JMWeatherPreset;

		preset.Name = "No Rain and Fog";

		preset.PFog.Forecast = 0.0;
		preset.PFog.Time = 180;
		preset.PFog.MinDuration = 21600;

		preset.POvercast.Forecast = 0.0;
		preset.POvercast.Time = 180;
		preset.POvercast.MinDuration = 21600;

		preset.PRain.Forecast = 0.0;
		preset.PRain.Time = 180;
		preset.PRain.MinDuration = 21600;

		Presets.Insert( preset );
	}

	private void PresetPissingCows()
	{
		JMWeatherPreset preset = new JMWeatherPreset;

		preset.Name = "Heavy RainThunder";

		preset.PFog.Forecast = 0.0;
		preset.PFog.Time = 180;
		preset.PFog.MinDuration = 21600;

		preset.POvercast.Forecast = 1.0;
		preset.POvercast.Time = 180;
		preset.POvercast.MinDuration = 21600;

		preset.PRain.Forecast = 1.0;
		preset.PRain.Time = 180;
		preset.PRain.MinDuration = 21600;

		Presets.Insert( preset );
	}
};
