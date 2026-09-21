class JMWeatherSerialize
{
	//! Seconds a bundled preset holds a phenomenon before the mission's own
	//! weather controller is allowed to move it again.
	//!
	//! This was 21600 - SIX HOURS. WeatherPhenomenon.Set's third argument is
	//! the minimum time the change will last, so one click of a bundled preset
	//! took the phenomenon away from cfgweather.xml for a quarter of a day.
	//! That is the single biggest reason COT looked like it was "ignoring
	//! weather.xml". 240 matches what the form's Easy Mode already used.
	//!
	//! An admin who genuinely wants a long hold can still say so - these are
	//! only the defaults written into Weather.json on first run, and the file
	//! is theirs to edit afterwards.
	static const float PRESET_MIN_DURATION = 240;

	//! Seconds to fade to the requested value.
	static const float PRESET_TRANSITION = 120;

	//! Defaults for the dynamic chain's timing, in seconds. A state holds for
	//! somewhere between the two durations, then fades into the next one over
	//! somewhere between the two transitions.
	static const int DYNAMIC_DEFAULT_DURATION_MIN   = 600;
	static const int DYNAMIC_DEFAULT_DURATION_MAX   = 1800;
	static const int DYNAMIC_DEFAULT_TRANSITION_MIN = 60;
	static const int DYNAMIC_DEFAULT_TRANSITION_MAX = 180;

	//! Floor on the roll interval. A chain that fires faster than this cannot
	//! finish the transition it just started - PRESET_TRANSITION alone is 120
	//! seconds - so it would only ever show weather mid-fade.
	static const int DYNAMIC_MIN_INTERVAL = 60;

	//! The preset applied when the settings load, for servers NOT running the
	//! dynamic chain. The chain starts from a random preset instead, so this is
	//! ignored while DynamicEnabled is set.
	string InitialPreset;
	autoptr array< ref JMWeatherPreset > Presets;

	// --- Dynamic weather -----------------------------------------------------
	//! Master switch. Everything else the chain needs lives on the presets
	//! themselves - see JMWeatherPreset's dynamic block.
	bool DynamicEnabled;

	//! Where the chain currently sits. Written out with the rest of the
	//! settings, but NOT used as a seed - every server start picks a fresh
	//! random preset, so this only records where the running chain got to.
	string CurrentPreset;

	// --- Legacy, read once and then cleared ----------------------------------
	//! These held the chain's timing and its edges before both moved onto the
	//! presets. They stay DECLARED because JsonFileLoader can only read a field
	//! the class still has - deleting them would silently drop an existing
	//! configuration on the first load instead of migrating it.
	//!
	//! MigrateDynamic folds them into the presets and empties them, so a file
	//! written after that point carries them as zero and an empty array.
	int DurationMin;
	int DurationMax;
	int TransitionMin;
	int TransitionMax;
	autoptr array< ref JMWeatherTransition > Transitions;

	void JMWeatherSerialize()
	{
		InitialPreset = "";
		Presets = new array< ref JMWeatherPreset >;

		DynamicEnabled = false;
		CurrentPreset  = "";

		//! Zero, not the defaults: a zero here is how MigrateDynamic tells an
		//! already-migrated file from one that still has timing to move.
		DurationMin   = 0;
		DurationMax   = 0;
		TransitionMin = 0;
		TransitionMax = 0;

		Transitions = new array< ref JMWeatherTransition >;
	}

	//! Fade one phenomenon to `forecast` and hold it for PRESET_MIN_DURATION.
	protected void SetPhenomenon( JMWeatherPhenomenon phenomenon, float forecast )
	{
		phenomenon.Forecast = forecast;
		phenomenon.Time = PRESET_TRANSITION;
		phenomenon.MinDuration = PRESET_MIN_DURATION;
	}

	static JMWeatherSerialize Load()
	{
		JMWeatherSerialize settings = new JMWeatherSerialize();

		if ( FileExist( JMConstants.FILE_WEATHER ) )
		{
			//! An unreadable file is reported by Load() and kept as it is: falling through to
			//! Defaults() below would overwrite the admin's presets with the stock ones.
			JMJsonFile<JMWeatherSerialize>.Load( JMConstants.FILE_WEATHER, settings );

			settings.MigrateDynamic();
			settings.NormalizePresets();
		}
		else
		{
			settings.Defaults();
			settings.Save();
		}

		return settings;
	}

	//! Fill in the dynamic block of any preset that has never had one.
	//!
	//! JsonFileLoader does NOT run the constructor for a field it does not find
	//! in the file - an absent int loads as 0 and an absent bool as false, so
	//! the defaults JMWeatherPreset sets never reach a preset read out of a
	//! file written before that block existed. Left alone, every such preset
	//! would come back out of rotation with a zero-length hold.
	//!
	//! The whole timing block being zero is the signal: the module clamps a
	//! saved DurationMin to at least DYNAMIC_MIN_INTERVAL, so a preset the
	//! admin has actually configured can never look like this. One that has
	//! been configured is left exactly as written - including a rotation flag
	//! deliberately turned off.
	void NormalizePresets()
	{
		if ( !Presets )
			return;

		bool changed = false;

		JMWeatherPreset preset;

		for ( int i = 0; i < Presets.Count(); i++ )
		{
			preset = Presets[i];

			if ( !preset.NextStates )
			{
				preset.NextStates = new array< ref JMWeatherNextState >;
				changed = true;
			}

			if ( preset.DurationMin > 0 || preset.DurationMax > 0 )
				continue;

			if ( preset.TransitionMin > 0 || preset.TransitionMax > 0 )
				continue;

			preset.InRotation = true;

			preset.DurationMin   = DYNAMIC_DEFAULT_DURATION_MIN;
			preset.DurationMax   = DYNAMIC_DEFAULT_DURATION_MAX;
			preset.TransitionMin = DYNAMIC_DEFAULT_TRANSITION_MIN;
			preset.TransitionMax = DYNAMIC_DEFAULT_TRANSITION_MAX;

			changed = true;
		}

		if ( changed )
			Save();
	}

	//! Move a pre-per-preset dynamic configuration onto the presets it belongs
	//! to, once, and empty the fields it came from.
	//!
	//! No-ops on a file that has already been through it, and on one that never
	//! had a chain configured at all.
	void MigrateDynamic()
	{
		bool hasLegacyTiming = ( DurationMin > 0 || DurationMax > 0 || TransitionMin > 0 || TransitionMax > 0 );
		bool hasLegacyEdges  = ( Transitions && Transitions.Count() > 0 );

		if ( !hasLegacyTiming && !hasLegacyEdges )
			return;

		int i;
		int j;
		JMWeatherPreset preset;

		if ( hasLegacyTiming )
		{
			for ( i = 0; i < Presets.Count(); i++ )
			{
				preset = Presets[i];

				//! Only presets that have not been given their own timing yet -
				//! a partly hand-edited file keeps whatever it already says.
				if ( preset.DurationMin > 0 || preset.DurationMax > 0 )
					continue;

				preset.DurationMin   = DurationMin;
				preset.DurationMax   = DurationMax;
				preset.TransitionMin = TransitionMin;
				preset.TransitionMax = TransitionMax;

				//! A file this old has no rotation flag at all, and an absent
				//! field loads as false - see NormalizePresets. Every preset in
				//! it was part of the chain, so that is what it migrates to.
				preset.InRotation = true;
			}
		}

		//! Each legacy edge goes to the preset it leaves, which is what its
		//! From field named. An edge whose source no longer exists has nowhere
		//! to land and goes out with the rest of the list.
		JMWeatherNextState moved;

		for ( i = 0; i < Transitions.Count(); i++ )
		{
			for ( j = 0; j < Presets.Count(); j++ )
			{
				if ( Presets[j].Name != Transitions[i].From )
					continue;

				moved = new JMWeatherNextState;
				moved.To     = Transitions[i].To;
				moved.Chance = Transitions[i].Chance;

				Presets[j].NextStates.Insert( moved );
				break;
			}
		}

		DurationMin   = 0;
		DurationMax   = 0;
		TransitionMin = 0;
		TransitionMax = 0;

		Transitions.Clear();

		Save();
	}

	void Save()
	{
		JMJsonFile<JMWeatherSerialize>.Save( JMConstants.FILE_WEATHER, this );
	}

	void Defaults()
	{
		PresetNoRain();
		PresetNoSnow();
		PresetNoFog();
		PresetNoRainAndFog();
	}

	//! A preset that changes NOTHING, for the callers below to opt into the one
	//! or two phenomena they are actually named after.
	//!
	//! Every marker field is -1, which is what each JMWeatherBase.Apply checks
	//! before it writes. The bundled presets used to set every field on every
	//! preset, so "No Rain" also forced overcast, snow, wind magnitude and wind
	//! direction to zero and held all of them - a preset doing four things its
	//! name does not mention.
	protected JMWeatherPreset NeutralPreset( string name )
	{
		JMWeatherPreset preset = new JMWeatherPreset;

		preset.Name = name;

		preset.PDate.Year = -1;
		preset.PDate.Month = -1;
		preset.PDate.Day = -1;
		preset.PDate.Hour = -1;
		preset.PDate.Minute = -1;

		preset.Storm.Density = -1;
		preset.Storm.Threshold = -1;
		preset.Storm.MinTimeBetweenLightning = -1;

		preset.PSandstorm.Enabled = -1;
		preset.PSandstorm.Duration = -1;

		preset.POvercast.Forecast = -1;
		preset.POvercast.Time = -1;
		preset.POvercast.MinDuration = -1;

		preset.PFog.Forecast = -1;
		preset.PFog.Time = -1;
		preset.PFog.MinDuration = -1;

		preset.PDynFog.Distance = -1;
		preset.PDynFog.Height = -1;
		preset.PDynFog.Bias = -1;
		preset.PDynFog.Time = -1;

		preset.PRain.Forecast = -1;
		preset.PRain.Time = -1;
		preset.PRain.MinDuration = -1;

		preset.RainThreshold.OvercastMin = -1;
		preset.RainThreshold.OvercastMax = -1;
		preset.RainThreshold.Time = -1;

		preset.PSnow.Forecast = -1;
		preset.PSnow.Time = -1;
		preset.PSnow.MinDuration = -1;

		preset.SnowThreshold.OvercastMin = -1;
		preset.SnowThreshold.OvercastMax = -1;
		preset.SnowThreshold.Time = -1;

		preset.PWindMagnitude.Forecast = -1;
		preset.PWindMagnitude.Time = -1;
		preset.PWindMagnitude.MinDuration = -1;

		preset.PWindDirection.Forecast = -1;
		preset.PWindDirection.Time = -1;
		preset.PWindDirection.MinDuration = -1;

		preset.WindFunc.Min = -1;
		preset.WindFunc.Max = -1;
		preset.WindFunc.Speed = -1;

		return preset;
	}

	protected void PresetNoRain()
	{
		JMWeatherPreset preset = NeutralPreset( "No Rain" );

		SetPhenomenon( preset.PRain, 0.0 );

		Presets.Insert( preset );
	}

	protected void PresetNoSnow()
	{
		JMWeatherPreset preset = NeutralPreset( "No Snow" );

		SetPhenomenon( preset.PSnow, 0.0 );

		Presets.Insert( preset );
	}

	protected void PresetNoFog()
	{
		JMWeatherPreset preset = NeutralPreset( "No Fog" );

		SetPhenomenon( preset.PFog, 0.0 );

		//! Volumetric fog is a separate system from the fog phenomenon, and a
		//! preset named "No Fog" is expected to clear both.
		preset.PDynFog.Distance = 0;
		preset.PDynFog.Height = 0;
		preset.PDynFog.Bias = 0;
		preset.PDynFog.Time = PRESET_TRANSITION;

		Presets.Insert( preset );
	}

	protected void PresetNoRainAndFog()
	{
		JMWeatherPreset preset = NeutralPreset( "No Rain and Fog" );

		SetPhenomenon( preset.PRain, 0.0 );
		SetPhenomenon( preset.PFog, 0.0 );

		preset.PDynFog.Distance = 0;
		preset.PDynFog.Height = 0;
		preset.PDynFog.Bias = 0;
		preset.PDynFog.Time = PRESET_TRANSITION;

		Presets.Insert( preset );
	}
}
