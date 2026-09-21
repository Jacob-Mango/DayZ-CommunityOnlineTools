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

	//! Defaults for a phase's timing, in seconds. A phase holds for somewhere
	//! between the two durations, then the next one fades in over somewhere
	//! between the two transitions.
	static const int DYNAMIC_DEFAULT_DURATION_MIN   = 600;
	static const int DYNAMIC_DEFAULT_DURATION_MAX   = 1800;
	static const int DYNAMIC_DEFAULT_TRANSITION_MIN = 60;
	static const int DYNAMIC_DEFAULT_TRANSITION_MAX = 180;

	//! Floor on a phase's hold. A machine that moves faster than this cannot
	//! finish the transition it just started - PRESET_TRANSITION alone is 120
	//! seconds - so it would only ever show weather mid-fade.
	static const int DYNAMIC_MIN_INTERVAL = 60;

	//! What StatesVersion reads once each migration has run.
	//!   1  the presets' legacy chain blocks were folded into states
	//!   2  the presets themselves were folded into the phases that used them, so a
	//!      phase carries its own weather and Presets is empty
	static const int STATES_VERSION_STATES  = 1;
	static const int STATES_VERSION_WEATHER = 2;
	static const int STATES_VERSION = 2;

	//! Label GetNextOdds() gives "the machine stays where it is" - the UI
	//! translates it, so it is a marker rather than display text.
	static const string ODDS_STAY = "=";

	//! The preset applied when the settings load, for servers NOT running the
	//! dynamic machine. The machine starts from a state of its own instead, so
	//! this is ignored while DynamicEnabled is set.
	string InitialPreset;
	autoptr array< ref JMWeatherPreset > Presets;

	// --- Dynamic weather -----------------------------------------------------
	//! Master switch. Persisted here with the rest of the configuration, so a
	//! server that had the machine on comes back with it on. WHERE the machine
	//! is lives in WeatherState.json - see JMWeatherRuntime.
	bool DynamicEnabled;

	//! The state machine: top-level states, each holding phases.
	autoptr array< ref JMWeatherState > States;

	//! 0 = the file predates states and its presets' legacy chain blocks still
	//! need migrating; see MigrateToStates.
	int StatesVersion;

	// --- Legacy, read once and then cleared ----------------------------------
	//! These held the flat chain's position, timing and edges before all of it
	//! moved into states and the runtime file. They stay DECLARED because
	//! JsonFileLoader can only read a field the class still has - deleting them
	//! would silently drop an existing configuration on the first load instead
	//! of migrating it.
	//!
	//! MigrateDynamic folds the timing and edges into the presets, then
	//! MigrateToStates folds those into states; both empty what they read, so a
	//! file written after that carries zeros and empty arrays.
	string CurrentPreset;
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
		States         = new array< ref JMWeatherState >;
		StatesVersion  = 0;

		CurrentPreset = "";

		//! Zero, not the defaults: a zero here is how MigrateDynamic tells an
		//! already-migrated file from one that still has timing to move.
		DurationMin   = 0;
		DurationMax   = 0;
		TransitionMin = 0;
		TransitionMax = 0;

		Transitions = new array< ref JMWeatherTransition >;
	}

	static JMWeatherSerialize Load()
	{
		JMWeatherSerialize settings = new JMWeatherSerialize();

		if ( FileExist( JMConstants.FILE_WEATHER ) )
		{
			//! An unreadable file is reported by Load() and kept as it is: falling through to
			//! Defaults() below would overwrite the admin's presets with the stock ones.
			JMJsonFile<JMWeatherSerialize>.Load( JMConstants.FILE_WEATHER, settings );

			settings.EnsureArrays();
			settings.MigrateDynamic();
			settings.MigrateToStates();
			settings.MigrateToWeather();
			settings.NormalizeStates();
		}
		else
		{
			//! First run: the bundled flow is written out, so the file is there to read and edit.
			settings.Defaults();
			settings.NormalizeStates();
			settings.Save();
		}

		return settings;
	}

	//! JsonFileLoader does NOT run the constructor for a field it does not find
	//! in the file, and an array a file never mentioned can come back null.
	//! Everything below assumes the arrays exist.
	void EnsureArrays()
	{
		if ( !Presets )
			Presets = new array< ref JMWeatherPreset >;

		if ( !States )
			States = new array< ref JMWeatherState >;

		if ( !Transitions )
			Transitions = new array< ref JMWeatherTransition >;

		int i;

		for ( i = 0; i < Presets.Count(); i++ )
		{
			if ( !Presets[i].NextStates )
				Presets[i].NextStates = new array< ref JMWeatherNextState >;
		}

		for ( i = 0; i < States.Count(); i++ )
			States[i].EnsureArrays();
	}

	//! Move a pre-per-preset dynamic configuration onto the presets it belongs
	//! to, once, and empty the fields it came from. The first of the two hops
	//! that end in states - see MigrateToStates.
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
				//! field loads as false. Every preset in it was part of the
				//! chain, so that is what it migrates to.
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

	//! Fold every preset's legacy chain block into a state of its own, once.
	//!
	//! A preset used to BE a node of the chain. It becomes a state with a single
	//! phase that applies that same preset, carrying the preset's timing and
	//! next-state list across unchanged - so an existing configuration behaves
	//! exactly as it did, and can then be split into real phases by hand.
	//!
	//! A preset that was never given a chain block (all timing zero - the
	//! constructor no longer sets defaults, and a file read by an older version
	//! got them from NormalizePresets) joins the rotation with the default
	//! timing, which is what that older normalisation gave it.
	void MigrateToStates()
	{
		if ( StatesVersion >= STATES_VERSION_STATES )
			return;

		int i;
		JMWeatherPreset preset;
		JMWeatherState state;
		JMWeatherPhase phase;
		JMWeatherNextState leave;
		bool configured;

		for ( i = 0; i < Presets.Count(); i++ )
		{
			preset = Presets[i];

			//! Nothing to migrate onto if a state of this name already exists,
			//! e.g. a file edited by hand between two versions.
			if ( HasState( preset.Name ) )
				continue;

			configured = ( preset.DurationMin > 0 || preset.DurationMax > 0 || preset.TransitionMin > 0 || preset.TransitionMax > 0 );

			state = new JMWeatherState;
			state.Name = preset.Name;

			phase = new JMWeatherPhase;
			phase.Name   = "Phase 1";
			phase.Preset = preset.Name;

			if ( configured )
			{
				state.InRotation = preset.InRotation;

				phase.DurationMin   = preset.DurationMin;
				phase.DurationMax   = preset.DurationMax;
				phase.TransitionMin = preset.TransitionMin;
				phase.TransitionMax = preset.TransitionMax;
			}

			phase.ClampTiming();

			//! A phase carries weather, never a date. A file that lacks the block loads it as
			//! zeros, and zeros are a real date to the engine.
			if ( phase.Conditions )
				phase.Conditions.ClearDate();

			//! Every state has always been left after one hold; the old chain had
			//! no notion of staying anywhere longer than that.
			leave = new JMWeatherNextState;
			leave.To     = "";
			leave.Chance = 100;
			phase.Edges.Insert( leave );

			state.Phases.Insert( phase );

			if ( preset.NextStates )
				state.NextStates = JMWeatherNextState.Copy( preset.NextStates );

			States.Insert( state );

			preset.InRotation    = false;
			preset.DurationMin   = 0;
			preset.DurationMax   = 0;
			preset.TransitionMin = 0;
			preset.TransitionMax = 0;
			preset.NextStates.Clear();
		}

		CurrentPreset = "";
		StatesVersion = STATES_VERSION_STATES;

		//! Targets are checked against the states now that all of them exist.
		NormalizeStates();
	}

	//! Fold every preset into the phases that used it, once, and empty Presets.
	//!
	//! A phase used to REFERENCE a stored preset by name; it now OWNS its weather,
	//! so a preset and a state are one thing to the person editing them and there
	//! is no second list to keep in step. Each phase gets its own copy - two phases
	//! that named the same preset stop sharing it, which is the point: editing one
	//! must not move the other. The date is dropped on the way: weather states do
	//! not move the clock.
	void MigrateToWeather()
	{
		if ( StatesVersion >= STATES_VERSION_WEATHER )
			return;

		int i;
		int j;
		int k;
		JMWeatherPhase phase;

		for ( i = 0; i < States.Count(); i++ )
		{
			for ( j = 0; j < States[i].Phases.Count(); j++ )
			{
				phase = States[i].Phases[j];

				if ( phase.Preset == "" )
					continue;

				for ( k = 0; k < Presets.Count(); k++ )
				{
					if ( Presets[k].Name != phase.Preset )
						continue;

					phase.Conditions = Presets[k].Copy();
					phase.Conditions.Name = "";
					phase.Conditions.ClearDate();
					break;
				}

				phase.Preset = "";
			}
		}

		Presets.Clear();
		InitialPreset = ResolveInitialState();

		StatesVersion = STATES_VERSION_WEATHER;

		Save();
	}

	//! InitialPreset used to name a preset; presets and states share names after the
	//! first migration, so it still names a state - unless that state is gone.
	protected string ResolveInitialState()
	{
		if ( HasState( InitialPreset ) )
			return InitialPreset;

		return "";
	}

	void NormalizeStates()
	{
		for ( int i = 0; i < States.Count(); i++ )
			NormalizeState( States[i] );
	}

	// -------------------------------------------------------------------------
	//  States
	// -------------------------------------------------------------------------

	JMWeatherState GetState( string name )
	{
		if ( name == "" || !States )
			return NULL;

		for ( int i = 0; i < States.Count(); i++ )
		{
			if ( States[i].Name == name )
				return States[i];
		}

		return NULL;
	}

	bool HasState( string name )
	{
		return GetState( name ) != NULL;
	}

	//! Make a state safe to store and to roll. Runs on the SERVER's own copy of
	//! whatever a client sent - nothing a client wrote is trusted to be
	//! well-formed.
	//!
	//! Phases get unique non-empty names (an empty name is the "leave the state"
	//! marker on an edge), timing the machine can honour, a preset that exists,
	//! and edges that lead somewhere real - every list ends up summing to 100.
	//! The state must already be in States when this runs, so that a state may
	//! name itself as a next state.
	void NormalizeState( JMWeatherState state )
	{
		if ( !state )
			return;

		state.EnsureArrays();

		int i;
		int j;
		JMWeatherPhase phase;
		string phaseName;

		for ( i = 0; i < state.Phases.Count(); i++ )
		{
			phase = state.Phases[i];

			phaseName = phase.Name.Trim();
			phase.Name = phaseName;

			for ( j = 0; j < i; j++ )
			{
				if ( state.Phases[j].Name == phase.Name )
				{
					phase.Name = "";
					break;
				}
			}

			if ( phase.Name == "" )
				phase.Name = state.NextPhaseName();

			phase.ClampTiming();
		}

		//! Edges last, once every phase name is final.
		for ( i = 0; i < state.Phases.Count(); i++ )
		{
			phase = state.Phases[i];

			for ( j = phase.Edges.Count() - 1; j >= 0; j-- )
			{
				if ( phase.Edges[j].To != "" && !state.HasPhase( phase.Edges[j].To ) )
					phase.Edges.Remove( j );
			}

			JMWeatherNextState.Normalize( phase.Edges );
		}

		for ( j = state.Entry.Count() - 1; j >= 0; j-- )
		{
			if ( !state.HasPhase( state.Entry[j].To ) )
				state.Entry.Remove( j );
		}

		JMWeatherNextState.Normalize( state.Entry );

		for ( j = state.NextStates.Count() - 1; j >= 0; j-- )
		{
			if ( !HasState( state.NextStates[j].To ) )
				state.NextStates.Remove( j );
		}

		JMWeatherNextState.Normalize( state.NextStates );
	}

	//! Store a state an admin edited. `target` is the name it has now ("" for a
	//! brand new one); incoming.Name is what it should be called afterwards, so a
	//! different one is a rename. Returns the stored copy, or NULL when refused
	//! (blank name, or a name another state already has).
	JMWeatherState SaveState( string target, JMWeatherState incoming )
	{
		if ( !incoming )
			return NULL;

		JMWeatherState copy = incoming.Copy();

		string newName = copy.Name.Trim();
		copy.Name = newName;

		if ( newName == "" )
			return NULL;

		JMWeatherState existing = GetState( target );

		if ( target != "" && !existing )
			return NULL;

		if ( newName != target && HasState( newName ) )
			return NULL;

		int i;

		if ( existing )
		{
			int index = States.Find( existing );

			//! Anything that led to the old name leads to the new one.
			if ( newName != target )
			{
				for ( i = 0; i < States.Count(); i++ )
					JMWeatherNextState.Rename( States[i].NextStates, target, newName );
			}

			States.Remove( index );
			States.InsertAt( copy, index );
		}
		else
		{
			States.Insert( copy );
		}

		NormalizeStates();

		return copy;
	}

	//! Delete a state and every edge that led to it.
	void RemoveState( string name )
	{
		JMWeatherState existing = GetState( name );

		if ( !existing )
			return;

		States.RemoveItem( existing );

		for ( int i = 0; i < States.Count(); i++ )
			JMWeatherNextState.Remove( States[i].NextStates, name );
	}

	//! Where a phase leads, as flat percentages a person can read: every leave-
	//! the-state edge is expanded through the state's next states and the entry
	//! phase of each, so "Storm > Peak 60%" is a real chance of the very next
	//! phase, not an intermediate hop. Entries that land on the same place are
	//! merged. Runs on the client too, off the configuration it already holds.
	void GetNextOdds( string stateName, string phaseName, array<string> labels, array<float> percents )
	{
		JMWeatherState state = GetState( stateName );

		if ( !state )
			return;

		JMWeatherPhase phase = state.GetPhase( phaseName );

		if ( !phase )
			return;

		int edgeTotal = JMWeatherNextState.Total( phase.Edges );

		if ( edgeTotal <= 0 )
		{
			AddOdds( labels, percents, ODDS_STAY, 100 );
			return;
		}

		int i;
		int j;
		int k;
		float share;
		float stateShare;
		float entryShare;
		int stateTotal;
		int entryTotal;
		JMWeatherState target;

		for ( i = 0; i < phase.Edges.Count(); i++ )
		{
			if ( phase.Edges[i].Chance <= 0 )
				continue;

			share = ( phase.Edges[i].Chance * 100.0 ) / edgeTotal;

			if ( phase.Edges[i].To != "" )
			{
				AddOdds( labels, percents, phase.Edges[i].To, share );
				continue;
			}

			stateTotal = JMWeatherNextState.Total( state.NextStates );

			//! Leaving with nowhere to go holds the current phase instead.
			if ( stateTotal <= 0 )
			{
				AddOdds( labels, percents, ODDS_STAY, share );
				continue;
			}

			for ( j = 0; j < state.NextStates.Count(); j++ )
			{
				if ( state.NextStates[j].Chance <= 0 )
					continue;

				target = GetState( state.NextStates[j].To );

				if ( !target || target.Phases.Count() == 0 )
				{
					AddOdds( labels, percents, ODDS_STAY, share * state.NextStates[j].Chance / stateTotal );
					continue;
				}

				stateShare = share * state.NextStates[j].Chance / stateTotal;
				entryTotal = JMWeatherNextState.Total( target.Entry );

				if ( entryTotal <= 0 )
				{
					AddOdds( labels, percents, target.Name + " > " + target.Phases[0].Name, stateShare );
					continue;
				}

				for ( k = 0; k < target.Entry.Count(); k++ )
				{
					if ( target.Entry[k].Chance <= 0 )
						continue;

					entryShare = stateShare * target.Entry[k].Chance / entryTotal;
					AddOdds( labels, percents, target.Name + " > " + target.Entry[k].To, entryShare );
				}
			}
		}
	}

	protected void AddOdds( array<string> labels, array<float> percents, string label, float value )
	{
		int existing = labels.Find( label );

		if ( existing >= 0 )
		{
			percents[existing] = percents[existing] + value;
			return;
		}

		labels.Insert( label );
		percents.Insert( value );
	}

	void Save()
	{
		JMJsonFile<JMWeatherSerialize>.Save( JMConstants.FILE_WEATHER, this );
	}

	// -------------------------------------------------------------------------
	//  The bundled weather flow
	//
	//  Six presets, each a small front of three phases (it builds, peaks, eases),
	//  and percentages that lead each one into the others - so a server that just
	//  switches the dynamic machine on gets weather that has a shape, not a coin
	//  flip. Every value is a min/max span, rolled once each time a phase is
	//  entered, so the same preset never plays out the same way twice.
	//
	//  Rain and snow are set in EVERY phase (to zero where they are not wanted):
	//  a phase that left one alone would inherit whatever the last preset put on.
	// -------------------------------------------------------------------------

	static const string PRESET_CLEAR    = "Clear and Sunny";
	static const string PRESET_CLOUDY   = "Overcast and Fog";
	static const string PRESET_RAIN     = "Frequent Rain";
	static const string PRESET_DOWNPOUR = "Heavy Rain";
	static const string PRESET_SNOW     = "Frequent Snow";
	static const string PRESET_BLIZZARD = "Heavy Snow";

	//! The machine itself starts off, as it always did - switch it on in the UI.
	void Defaults()
	{
		StatesVersion = STATES_VERSION;

		BuildClear();
		BuildCloudy();
		BuildRain();
		BuildDownpour();
		BuildSnow();
		BuildBlizzard();
	}

	//! A preset that changes NOTHING - see JMWeatherPreset.Neutral. Every phase
	//! opts into the phenomena it wants and leaves the rest.
	protected JMWeatherPreset NeutralPreset( string name )
	{
		return JMWeatherPreset.Neutral( name );
	}

	//! One phenomenon as a span. A span whose ends meet is a plain value.
	protected void Span( JMWeatherPhenomenon phenomenon, float low, float high )
	{
		phenomenon.Forecast    = low;
		phenomenon.ForecastHi  = 0;
		phenomenon.Time        = PRESET_TRANSITION;
		phenomenon.MinDuration = PRESET_MIN_DURATION;

		if ( high > low )
			phenomenon.ForecastHi = high;
	}

	//! Cloud cover, fog and the volumetric fog that goes with it.
	protected void Cover( JMWeatherPreset weather, float overcastLow, float overcastHigh, float fogLow, float fogHigh )
	{
		Span( weather.POvercast, overcastLow, overcastHigh );
		Span( weather.PFog, fogLow, fogHigh );

		//! Volumetric fog follows the fog phenomenon so a foggy phase actually looks
		//! it, and is switched off entirely for a clear one.
		weather.PDynFog.Distance   = fogLow;
		weather.PDynFog.DistanceHi = 0;
		weather.PDynFog.Height     = fogLow * 0.7;
		weather.PDynFog.HeightHi   = 0;
		weather.PDynFog.Bias       = 0;
		weather.PDynFog.BiasHi     = 0;
		weather.PDynFog.Time       = PRESET_TRANSITION;

		if ( fogHigh > fogLow )
		{
			weather.PDynFog.DistanceHi = fogHigh;
			weather.PDynFog.HeightHi   = fogHigh * 0.7;
		}
	}

	//! Rain and snow, and the cloud cover each needs before it starts.
	protected void Fall( JMWeatherPreset weather, float rainLow, float rainHigh, float snowLow, float snowHigh )
	{
		Span( weather.PRain, rainLow, rainHigh );
		Span( weather.PSnow, snowLow, snowHigh );

		//! Falling weather has to be allowed to start at moderate cloud; anything that
		//! is not falling puts the vanilla cfgweather.xml rule back.
		float rainMin = 0.6;
		float snowMin = 0.6;

		if ( rainHigh > 0 )
			rainMin = 0.5;

		if ( snowHigh > 0 )
			snowMin = 0.5;

		weather.RainThreshold.OvercastMin = rainMin;
		weather.RainThreshold.OvercastMax = 1.0;
		weather.RainThreshold.Time        = PRESET_TRANSITION;

		weather.SnowThreshold.OvercastMin = snowMin;
		weather.SnowThreshold.OvercastMax = 1.0;
		weather.SnowThreshold.Time        = PRESET_TRANSITION;
	}

	//! Lightning. A density of 0 is "none", and puts the rest back to vanilla.
	protected void Thunder( JMWeatherPreset weather, float densityLow, float densityHigh )
	{
		weather.Storm.Density     = densityLow;
		weather.Storm.DensityHi   = 0;
		weather.Storm.Threshold   = 1.0;
		weather.Storm.ThresholdHi = 0;

		weather.Storm.MinTimeBetweenLightning   = 30;
		weather.Storm.MinTimeBetweenLightningHi = 0;

		if ( densityHigh > densityLow )
			weather.Storm.DensityHi = densityHigh;

		if ( densityHigh > 0 )
		{
			weather.Storm.Threshold   = 0.6;
			weather.Storm.ThresholdHi = 0.8;

			weather.Storm.MinTimeBetweenLightning   = 10;
			weather.Storm.MinTimeBetweenLightningHi = 40;
		}
	}

	protected void Blow( JMWeatherPreset weather, float magnitudeLow, float magnitudeHigh, float gustMin, float gustMax, float gustSpeed )
	{
		Span( weather.PWindMagnitude, magnitudeLow, magnitudeHigh );

		weather.WindFunc.Min     = gustMin;
		weather.WindFunc.MinHi   = 0;
		weather.WindFunc.Max     = gustMax;
		weather.WindFunc.MaxHi   = 0;
		weather.WindFunc.Speed   = gustSpeed;
		weather.WindFunc.SpeedHi = 0;
	}

	protected JMWeatherPhase NewPhase( string name, int holdMin, int holdMax, int fadeMin, int fadeMax, JMWeatherPreset weather )
	{
		JMWeatherPhase phase = new JMWeatherPhase;

		phase.Name          = name;
		phase.Conditions    = weather;
		phase.DurationMin   = holdMin;
		phase.DurationMax   = holdMax;
		phase.TransitionMin = fadeMin;
		phase.TransitionMax = fadeMax;

		return phase;
	}

	protected void PhaseEdge( JMWeatherPhase phase, string to, int chance )
	{
		JMWeatherNextState edge = new JMWeatherNextState;

		edge.To     = to;
		edge.Chance = chance;

		phase.Edges.Insert( edge );
	}

	protected void PresetEdge( JMWeatherState state, string to, int chance )
	{
		JMWeatherNextState edge = new JMWeatherNextState;

		edge.To     = to;
		edge.Chance = chance;

		state.NextStates.Insert( edge );
	}

	//! Build -> Peak; the peak either goes on (it lists itself) or eases; the ease
	//! leaves the preset. So a front lasts a few holds, never just one. It always
	//! opens on the build.
	protected JMWeatherState NewFront( string name, JMWeatherPhase build, JMWeatherPhase peak, JMWeatherPhase ease )
	{
		JMWeatherState state = new JMWeatherState;

		state.Name       = name;
		state.InRotation = true;

		PhaseEdge( build, peak.Name, 100 );

		PhaseEdge( peak, peak.Name, 35 );
		PhaseEdge( peak, ease.Name, 65 );

		PhaseEdge( ease, "", 100 );

		state.Phases.Insert( build );
		state.Phases.Insert( peak );
		state.Phases.Insert( ease );

		JMWeatherNextState entry = new JMWeatherNextState;
		entry.To     = build.Name;
		entry.Chance = 100;
		state.Entry.Insert( entry );

		States.Insert( state );

		return state;
	}

	protected void BuildClear()
	{
		JMWeatherPreset a = NeutralPreset( "" );
		Cover( a, 0.15, 0.35, 0.0, 0.05 );
		Fall( a, 0, 0, 0, 0 );
		Thunder( a, 0, 0 );
		Blow( a, 0, 4, 0.0, 0.4, 10 );

		JMWeatherPreset b = NeutralPreset( "" );
		Cover( b, 0.0, 0.15, 0.0, 0.0 );
		Fall( b, 0, 0, 0, 0 );
		Thunder( b, 0, 0 );
		Blow( b, 0, 3, 0.0, 0.3, 8 );

		JMWeatherPreset c = NeutralPreset( "" );
		Cover( c, 0.25, 0.45, 0.0, 0.05 );
		Fall( c, 0, 0, 0, 0 );
		Thunder( c, 0, 0 );
		Blow( c, 1, 5, 0.0, 0.5, 12 );

		JMWeatherPhase build = NewPhase( "Clearing", 300, 600, 120, 240, a );
		JMWeatherPhase peak  = NewPhase( "Sunny", 900, 2100, 90, 180, b );
		JMWeatherPhase ease  = NewPhase( "Light Clouds", 300, 600, 120, 240, c );

		JMWeatherState state = NewFront( PRESET_CLEAR, build, peak, ease );

		PresetEdge( state, PRESET_CLEAR, 40 );
		PresetEdge( state, PRESET_CLOUDY, 30 );
		PresetEdge( state, PRESET_RAIN, 15 );
		PresetEdge( state, PRESET_SNOW, 15 );
	}

	protected void BuildCloudy()
	{
		JMWeatherPreset a = NeutralPreset( "" );
		Cover( a, 0.6, 0.8, 0.2, 0.4 );
		Fall( a, 0, 0, 0, 0 );
		Thunder( a, 0, 0 );
		Blow( a, 0, 4, 0.0, 0.4, 10 );

		JMWeatherPreset b = NeutralPreset( "" );
		Cover( b, 0.7, 0.9, 0.6, 0.9 );
		Fall( b, 0, 0, 0, 0 );
		Thunder( b, 0, 0 );
		Blow( b, 0, 2, 0.0, 0.2, 6 );

		JMWeatherPreset c = NeutralPreset( "" );
		Cover( c, 0.5, 0.7, 0.15, 0.35 );
		Fall( c, 0, 0, 0, 0 );
		Thunder( c, 0, 0 );
		Blow( c, 1, 5, 0.0, 0.4, 10 );

		JMWeatherPhase build = NewPhase( "Clouding Over", 300, 600, 120, 240, a );
		JMWeatherPhase peak  = NewPhase( "Thick Fog", 900, 1800, 90, 180, b );
		JMWeatherPhase ease  = NewPhase( "Fog Lifting", 300, 600, 180, 300, c );

		JMWeatherState state = NewFront( PRESET_CLOUDY, build, peak, ease );

		PresetEdge( state, PRESET_CLOUDY, 20 );
		PresetEdge( state, PRESET_CLEAR, 30 );
		PresetEdge( state, PRESET_RAIN, 20 );
		PresetEdge( state, PRESET_SNOW, 20 );
		PresetEdge( state, PRESET_DOWNPOUR, 5 );
		PresetEdge( state, PRESET_BLIZZARD, 5 );
	}

	protected void BuildRain()
	{
		JMWeatherPreset a = NeutralPreset( "" );
		Cover( a, 0.55, 0.7, 0.05, 0.2 );
		Fall( a, 0.15, 0.3, 0, 0 );
		Thunder( a, 0, 0 );
		Blow( a, 3, 7, 0.1, 0.5, 14 );

		JMWeatherPreset b = NeutralPreset( "" );
		Cover( b, 0.7, 0.85, 0.1, 0.3 );
		Fall( b, 0.35, 0.6, 0, 0 );
		Thunder( b, 0, 0 );
		Blow( b, 4, 9, 0.1, 0.6, 15 );

		JMWeatherPreset c = NeutralPreset( "" );
		Cover( c, 0.55, 0.7, 0.05, 0.2 );
		Fall( c, 0.1, 0.25, 0, 0 );
		Thunder( c, 0, 0 );
		Blow( c, 2, 6, 0.1, 0.4, 12 );

		JMWeatherPhase build = NewPhase( "Drizzle", 300, 600, 120, 240, a );
		JMWeatherPhase peak  = NewPhase( "Steady Rain", 900, 1800, 90, 180, b );
		JMWeatherPhase ease  = NewPhase( "Easing Rain", 300, 600, 180, 300, c );

		JMWeatherState state = NewFront( PRESET_RAIN, build, peak, ease );

		PresetEdge( state, PRESET_RAIN, 30 );
		PresetEdge( state, PRESET_DOWNPOUR, 20 );
		PresetEdge( state, PRESET_CLOUDY, 25 );
		PresetEdge( state, PRESET_CLEAR, 25 );
	}

	protected void BuildDownpour()
	{
		JMWeatherPreset a = NeutralPreset( "" );
		Cover( a, 0.75, 0.9, 0.1, 0.25 );
		Fall( a, 0.4, 0.6, 0, 0 );
		Thunder( a, 0.2, 0.4 );
		Blow( a, 6, 11, 0.2, 0.7, 18 );

		JMWeatherPreset b = NeutralPreset( "" );
		Cover( b, 0.9, 1.0, 0.15, 0.35 );
		Fall( b, 0.75, 1.0, 0, 0 );
		Thunder( b, 0.7, 1.0 );
		Blow( b, 9, 16, 0.3, 0.9, 22 );

		JMWeatherPreset c = NeutralPreset( "" );
		Cover( c, 0.7, 0.85, 0.1, 0.25 );
		Fall( c, 0.3, 0.5, 0, 0 );
		Thunder( c, 0.1, 0.3 );
		Blow( c, 5, 10, 0.2, 0.6, 16 );

		JMWeatherPhase build = NewPhase( "Rain Building", 300, 600, 120, 240, a );
		JMWeatherPhase peak  = NewPhase( "Downpour", 600, 1500, 60, 150, b );
		JMWeatherPhase ease  = NewPhase( "Front Passing", 300, 600, 180, 300, c );

		JMWeatherState state = NewFront( PRESET_DOWNPOUR, build, peak, ease );

		PresetEdge( state, PRESET_RAIN, 35 );
		PresetEdge( state, PRESET_CLOUDY, 25 );
		PresetEdge( state, PRESET_CLEAR, 25 );
		PresetEdge( state, PRESET_DOWNPOUR, 15 );
	}

	protected void BuildSnow()
	{
		JMWeatherPreset a = NeutralPreset( "" );
		Cover( a, 0.55, 0.7, 0.05, 0.2 );
		Fall( a, 0, 0, 0.15, 0.3 );
		Thunder( a, 0, 0 );
		Blow( a, 2, 6, 0.1, 0.4, 12 );

		JMWeatherPreset b = NeutralPreset( "" );
		Cover( b, 0.7, 0.85, 0.1, 0.3 );
		Fall( b, 0, 0, 0.35, 0.6 );
		Thunder( b, 0, 0 );
		Blow( b, 3, 8, 0.1, 0.5, 14 );

		JMWeatherPreset c = NeutralPreset( "" );
		Cover( c, 0.55, 0.7, 0.05, 0.2 );
		Fall( c, 0, 0, 0.1, 0.25 );
		Thunder( c, 0, 0 );
		Blow( c, 1, 5, 0.1, 0.4, 10 );

		JMWeatherPhase build = NewPhase( "Flurries", 300, 600, 120, 240, a );
		JMWeatherPhase peak  = NewPhase( "Steady Snow", 900, 1800, 90, 180, b );
		JMWeatherPhase ease  = NewPhase( "Snow Easing", 300, 600, 180, 300, c );

		JMWeatherState state = NewFront( PRESET_SNOW, build, peak, ease );

		PresetEdge( state, PRESET_SNOW, 30 );
		PresetEdge( state, PRESET_BLIZZARD, 20 );
		PresetEdge( state, PRESET_CLOUDY, 25 );
		PresetEdge( state, PRESET_CLEAR, 25 );
	}

	protected void BuildBlizzard()
	{
		JMWeatherPreset a = NeutralPreset( "" );
		Cover( a, 0.75, 0.9, 0.1, 0.3 );
		Fall( a, 0, 0, 0.4, 0.6 );
		Thunder( a, 0, 0 );
		Blow( a, 6, 12, 0.2, 0.7, 18 );

		JMWeatherPreset b = NeutralPreset( "" );
		Cover( b, 0.9, 1.0, 0.3, 0.5 );
		Fall( b, 0, 0, 0.8, 1.0 );
		Thunder( b, 0, 0 );
		Blow( b, 12, 20, 0.4, 1.0, 25 );

		JMWeatherPreset c = NeutralPreset( "" );
		Cover( c, 0.7, 0.85, 0.15, 0.3 );
		Fall( c, 0, 0, 0.3, 0.5 );
		Thunder( c, 0, 0 );
		Blow( c, 5, 11, 0.2, 0.6, 16 );

		JMWeatherPhase build = NewPhase( "Snow Building", 300, 600, 120, 240, a );
		JMWeatherPhase peak  = NewPhase( "Whiteout", 600, 1500, 60, 150, b );
		JMWeatherPhase ease  = NewPhase( "Blizzard Easing", 300, 600, 180, 300, c );

		JMWeatherState state = NewFront( PRESET_BLIZZARD, build, peak, ease );

		PresetEdge( state, PRESET_SNOW, 35 );
		PresetEdge( state, PRESET_CLOUDY, 25 );
		PresetEdge( state, PRESET_CLEAR, 25 );
		PresetEdge( state, PRESET_BLIZZARD, 15 );
	}
}
