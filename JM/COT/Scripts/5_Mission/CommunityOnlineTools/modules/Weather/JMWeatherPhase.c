//  JMWeatherPhase - one sub-state of a JMWeatherState: the weather values, how long it holds,
//  its fade-in, and where it can go next. Edges name another phase of the SAME state, or ""
//  for "leave the state", so ending the state is an ordinary percentage in the same list.
class JMWeatherPhase
{
	string Name;

	//! The weather put on when entered: the phase's OWN copy (shared presets made editing one move
	//! another). Fields a phase does not drive are -1 (see JMWeatherPreset.Neutral): the world's value holds.
	autoptr JMWeatherPreset Conditions;

	//! Legacy: the name of the stored preset this phase used to apply. Read once by
	//! JMWeatherSerialize.MigrateToWeather, which folds that preset into Conditions
	//! and blanks this. Stays declared so JsonFileLoader can still read it.
	string Preset;

	//! Seconds. A roll draws from between the two; equal bounds mean fixed timing.
	int DurationMin;
	int DurationMax;
	int TransitionMin;
	int TransitionMax;

	autoptr array< ref JMWeatherNextState > Edges;

	void JMWeatherPhase()
	{
		Name   = "";
		Preset = "";
		Conditions = JMWeatherPreset.Neutral( "" );

		DurationMin   = JMWeatherSerialize.DYNAMIC_DEFAULT_DURATION_MIN;
		DurationMax   = JMWeatherSerialize.DYNAMIC_DEFAULT_DURATION_MAX;
		TransitionMin = JMWeatherSerialize.DYNAMIC_DEFAULT_TRANSITION_MIN;
		TransitionMax = JMWeatherSerialize.DYNAMIC_DEFAULT_TRANSITION_MAX;

		Edges = new array< ref JMWeatherNextState >;
	}

	//! Clamp the timing into what the machine can honour: a hold never shorter
	//! than the roll floor, a fade never negative, and each range's top never
	//! below its bottom.
	void ClampTiming()
	{
		DurationMin   = Math.Max( DurationMin, JMWeatherSerialize.DYNAMIC_MIN_INTERVAL );
		DurationMax   = Math.Max( DurationMax, DurationMin );
		TransitionMin = Math.Max( TransitionMin, 0 );
		TransitionMax = Math.Max( TransitionMax, TransitionMin );
	}

	//! A JsonFileLoader-built phase can arrive with an array it never constructed.
	void EnsureArrays()
	{
		if ( !Conditions )
			Conditions = JMWeatherPreset.Neutral( "" );

		if ( !Edges )
			Edges = new array< ref JMWeatherNextState >;
	}

	JMWeatherPhase Copy()
	{
		JMWeatherPhase copy = new JMWeatherPhase;

		copy.Name   = Name;
		copy.Preset = Preset;

		if ( Conditions )
			copy.Conditions = Conditions.Copy();

		copy.DurationMin   = DurationMin;
		copy.DurationMax   = DurationMax;
		copy.TransitionMin = TransitionMin;
		copy.TransitionMax = TransitionMax;

		copy.Edges = JMWeatherNextState.Copy( Edges );

		return copy;
	}
}
