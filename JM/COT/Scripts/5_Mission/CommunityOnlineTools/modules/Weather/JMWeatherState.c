// =============================================================================
//  JMWeatherState - a top-level node of the dynamic weather state machine
//
//  A state is a weather SYSTEM ("Storm front", "Fair weather") made of phases
//  ("Building", "Peak", "Clearing"). While the machine is inside a state it
//  moves between that state's phases along their edges; when a phase's edge
//  says "leave", the state's own NextStates are rolled to pick the next state,
//  and that state's Entry weights pick which of its phases it opens on.
//
//  Everything is a percentage list - see JMWeatherNextState.
// =============================================================================
class JMWeatherState
{
	string Name;

	//! Whether the random pick at start-up (and on Restart) may land here. A
	//! state meant to be reached only from other states stays out of the rotation.
	bool InRotation;

	autoptr array< ref JMWeatherPhase > Phases;

	//! To = a phase name. Which phase this state opens on; empty means the first.
	autoptr array< ref JMWeatherNextState > Entry;

	//! To = a state name. Drawn when a phase leaves the state.
	autoptr array< ref JMWeatherNextState > NextStates;

	void JMWeatherState()
	{
		Name       = "";
		InRotation = true;

		Phases     = new array< ref JMWeatherPhase >;
		Entry      = new array< ref JMWeatherNextState >;
		NextStates = new array< ref JMWeatherNextState >;
	}

	void EnsureArrays()
	{
		if ( !Phases )
			Phases = new array< ref JMWeatherPhase >;

		if ( !Entry )
			Entry = new array< ref JMWeatherNextState >;

		if ( !NextStates )
			NextStates = new array< ref JMWeatherNextState >;

		for ( int i = 0; i < Phases.Count(); i++ )
			Phases[i].EnsureArrays();
	}

	JMWeatherPhase GetPhase( string name )
	{
		if ( name == "" || !Phases )
			return NULL;

		for ( int i = 0; i < Phases.Count(); i++ )
		{
			if ( Phases[i].Name == name )
				return Phases[i];
		}

		return NULL;
	}

	bool HasPhase( string name )
	{
		return GetPhase( name ) != NULL;
	}

	//! The phase this state opens on: drawn from Entry, else the first phase.
	//! "" only when the state has no phases at all, which cannot be entered.
	string RollEntryPhase()
	{
		if ( !Phases || Phases.Count() == 0 )
			return "";

		int index = JMWeatherNextState.Roll( Entry );

		if ( index >= 0 && HasPhase( Entry[index].To ) )
			return Entry[index].To;

		return Phases[0].Name;
	}

	//! A phase name not yet taken, "Phase N".
	string NextPhaseName()
	{
		int n = 1;
		string candidate = "Phase " + n;

		while ( HasPhase( candidate ) )
		{
			n++;
			candidate = "Phase " + n;
		}

		return candidate;
	}

	//! Remove a phase and every edge that named it, so nothing is left pointing
	//! at a phase that cannot be entered.
	void RemovePhase( string name )
	{
		if ( !Phases )
			return;

		for ( int i = Phases.Count() - 1; i >= 0; i-- )
		{
			if ( Phases[i].Name == name )
				Phases.Remove( i );
		}

		JMWeatherNextState.Remove( Entry, name );

		for ( int j = 0; j < Phases.Count(); j++ )
			JMWeatherNextState.Remove( Phases[j].Edges, name );
	}

	void RenamePhase( string from, string to )
	{
		JMWeatherPhase phase = GetPhase( from );

		if ( phase )
			phase.Name = to;

		JMWeatherNextState.Rename( Entry, from, to );

		for ( int i = 0; i < Phases.Count(); i++ )
			JMWeatherNextState.Rename( Phases[i].Edges, from, to );
	}

	JMWeatherState Copy()
	{
		JMWeatherState copy = new JMWeatherState;

		copy.Name       = Name;
		copy.InRotation = InRotation;

		for ( int i = 0; i < Phases.Count(); i++ )
			copy.Phases.Insert( Phases[i].Copy() );

		copy.Entry      = JMWeatherNextState.Copy( Entry );
		copy.NextStates = JMWeatherNextState.Copy( NextStates );

		return copy;
	}
}
