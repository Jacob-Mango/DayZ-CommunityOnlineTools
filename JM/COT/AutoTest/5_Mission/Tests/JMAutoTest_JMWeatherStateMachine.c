#ifdef JM_COT_AUTOTEST

//! Dynamic weather state machine: the parts that are pure data - the weighted
//! roll, list normalisation, state validation, the odds flattening and the
//! legacy-preset migration. None of it touches the engine's weather, so it
//! can run at boot and assert exact outcomes.
//!
//! Position persistence and the timer are engine-backed and are covered by the
//! restart procedure in Docs/modules/weather-dynamic.md instead.

static JMWeatherNextState JMAutoTest_Edge( string to, int chance )
{
	JMWeatherNextState edge = new JMWeatherNextState;
	edge.To     = to;
	edge.Chance = chance;

	return edge;
}

static JMWeatherPreset JMAutoTest_NamedPreset( string name )
{
	JMWeatherPreset preset = new JMWeatherPreset;
	preset.Name = name;

	return preset;
}

static void JMAutoTest_JMWeatherStateMachine()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMWeatherStateMachine data logic");
	PrintFormat("================================================================");

	array< ref JMWeatherNextState > edges = new array< ref JMWeatherNextState >;
	int i;
	int index;
	int hits;

	// ---------------------------------------------------------------------
	JMAutoTest_Group("Roll: nothing to draw from is -1, not a pick");
	JMAutoTest_AssertEqualInt( "empty list", JMWeatherNextState.Roll( edges ), -1 );
	JMAutoTest_AssertEqualInt( "null list", JMWeatherNextState.Roll( NULL ), -1 );

	edges.Insert( JMAutoTest_Edge( "a", 0 ) );
	edges.Insert( JMAutoTest_Edge( "b", -5 ) );
	JMAutoTest_AssertEqualInt( "zero and negative weights are absent", JMWeatherNextState.Roll( edges ), -1 );

	// ---------------------------------------------------------------------
	JMAutoTest_Group("Roll: only positive weights can be drawn, and all of them");
	edges.Clear();
	edges.Insert( JMAutoTest_Edge( "never", 0 ) );
	edges.Insert( JMAutoTest_Edge( "sometimes", 30 ) );
	edges.Insert( JMAutoTest_Edge( "usually", 70 ) );

	hits = 0;
	bool drewNever = false;

	for ( i = 0; i < 2000; i++ )
	{
		index = JMWeatherNextState.Roll( edges );

		if ( index == 0 )
			drewNever = true;

		if ( index == 1 )
			hits++;
	}

	JMAutoTest_AssertFalse( "a zero-weight edge is never drawn", drewNever );
	JMAutoTest_AssertInRange( "30% edge drawn roughly 30% of 2000", hits, 450, 750 );

	// ---------------------------------------------------------------------
	JMAutoTest_Group("Roll: weights are drawn against their own total, not against 100");
	edges.Clear();
	edges.Insert( JMAutoTest_Edge( "x", 30 ) );
	edges.Insert( JMAutoTest_Edge( "y", 10 ) );

	hits = 0;

	for ( i = 0; i < 2000; i++ )
	{
		if ( JMWeatherNextState.Roll( edges ) == 0 )
			hits++;
	}

	JMAutoTest_AssertInRange( "30 vs 10 picks the first about 75% of the time", hits, 1350, 1650 );

	// ---------------------------------------------------------------------
	JMAutoTest_Group("Normalize: sums to exactly 100, drops zeros and duplicates");
	edges.Clear();
	edges.Insert( JMAutoTest_Edge( "a", 1 ) );
	edges.Insert( JMAutoTest_Edge( "b", 1 ) );
	edges.Insert( JMAutoTest_Edge( "c", 1 ) );
	edges.Insert( JMAutoTest_Edge( "d", 0 ) );
	edges.Insert( JMAutoTest_Edge( "a", 50 ) );

	JMWeatherNextState.Normalize( edges );

	JMAutoTest_AssertEqualInt( "zero-chance and duplicate-target edges are dropped", edges.Count(), 3 );
	JMAutoTest_AssertEqualInt( "the column adds up to 100", JMWeatherNextState.Total( edges ), 100 );

	edges.Clear();
	JMWeatherNextState.Normalize( edges );
	JMAutoTest_AssertEqualInt( "an empty list stays empty", edges.Count(), 0 );

	// ---------------------------------------------------------------------
	JMAutoTest_Group("Remove/Rename touch only the edges that name the target");
	edges.Clear();
	edges.Insert( JMAutoTest_Edge( "a", 50 ) );
	edges.Insert( JMAutoTest_Edge( "b", 50 ) );

	JMWeatherNextState.Rename( edges, "a", "z" );
	JMAutoTest_AssertEqualInt( "renamed edge found under the new name", JMWeatherNextState.Find( edges, "z" ), 0 );
	JMAutoTest_AssertEqualInt( "old name gone", JMWeatherNextState.Find( edges, "a" ), -1 );

	JMWeatherNextState.Remove( edges, "z" );
	JMAutoTest_AssertEqualInt( "removed edge takes its share with it", edges.Count(), 1 );
	JMAutoTest_AssertEqualInt( "the survivor is rescaled to 100", edges[0].Chance, 100 );

	// ---------------------------------------------------------------------
	JMAutoTest_Group("Pick: a span is rolled, anything else is the plain value");
	JMAutoTest_AssertFloatNear( "-1 stays unchanged whatever the high bound", JMWeatherRoll.Pick( -1, 5 ), -1, 0.001 );
	JMAutoTest_AssertFloatNear( "no high bound is the low value", JMWeatherRoll.Pick( 0.4, 0 ), 0.4, 0.001 );
	JMAutoTest_AssertFloatNear( "an equal high bound is the low value", JMWeatherRoll.Pick( 0.4, 0.4 ), 0.4, 0.001 );
	JMAutoTest_AssertFloatNear( "a high bound BELOW the low one is not a range", JMWeatherRoll.Pick( 0.4, 0.1 ), 0.4, 0.001 );

	bool outOfSpan = false;
	bool varied = false;
	float first = JMWeatherRoll.Pick( 0.2, 0.8 );
	float rolled;

	for ( i = 0; i < 200; i++ )
	{
		rolled = JMWeatherRoll.Pick( 0.2, 0.8 );

		if ( rolled < 0.2 || rolled > 0.8 )
			outOfSpan = true;

		if ( rolled != first )
			varied = true;
	}

	JMAutoTest_AssertFalse( "a rolled value stays inside its span", outOfSpan );
	JMAutoTest_Assert( "and it is not the same every time", varied );

	// ---------------------------------------------------------------------
	JMAutoTest_Group("NormalizeState: phases, edges and next states are made valid");
	JMWeatherSerialize settings = new JMWeatherSerialize;

	JMWeatherState state = new JMWeatherState;
	state.Name = "Front";

	JMWeatherPhase building = new JMWeatherPhase;
	building.Name   = "Building";
	building.DurationMin = 5;
	building.DurationMax = 1;
	building.Edges.Insert( JMAutoTest_Edge( "Peak", 60 ) );
	building.Edges.Insert( JMAutoTest_Edge( "Ghost", 30 ) );
	building.Edges.Insert( JMAutoTest_Edge( "", 10 ) );

	JMWeatherPhase peak = new JMWeatherPhase;
	peak.Name   = "Peak";
	peak.Preset = "NoSuchPreset";

	JMWeatherPhase unnamed = new JMWeatherPhase;
	unnamed.Name = "Peak";

	state.Phases.Insert( building );
	state.Phases.Insert( peak );
	state.Phases.Insert( unnamed );
	state.NextStates.Insert( JMAutoTest_Edge( "Front", 10 ) );
	state.NextStates.Insert( JMAutoTest_Edge( "Nowhere", 90 ) );

	settings.States.Insert( state );
	settings.NormalizeState( state );

	JMAutoTest_AssertEqualInt( "a duplicate phase name is replaced, not kept", state.Phases.Count(), 3 );
	JMAutoTest_Assert( "phase names are unique", state.Phases[1].Name != state.Phases[2].Name );
	JMAutoTest_Assert( "hold is raised to the floor", building.DurationMin >= JMWeatherSerialize.DYNAMIC_MIN_INTERVAL );
	JMAutoTest_Assert( "hold top is never below its bottom", building.DurationMax >= building.DurationMin );
	JMAutoTest_AssertEqualInt( "an edge to a phase that does not exist is dropped", building.Edges.Count(), 2 );
	JMAutoTest_AssertEqualInt( "the remaining edges add up to 100", JMWeatherNextState.Total( building.Edges ), 100 );
	JMAutoTest_AssertEqualInt( "a state may name itself; unknown states are dropped", state.NextStates.Count(), 1 );
	JMAutoTest_AssertEqualInt( "and the survivor is 100", state.NextStates[0].Chance, 100 );

	// ---------------------------------------------------------------------
	JMAutoTest_Group("SaveState: new, rename, and refusals");
	JMWeatherState incoming = new JMWeatherState;
	incoming.Name = "Calm";

	JMWeatherPhase calmPhase = new JMWeatherPhase;
	calmPhase.Name = "Phase 1";
	incoming.Phases.Insert( calmPhase );

	JMWeatherState saved = settings.SaveState( "", incoming );
	JMAutoTest_AssertNotNull( "a new state is stored", saved );
	JMAutoTest_AssertEqualInt( "and counted", settings.States.Count(), 2 );
	JMAutoTest_AssertNull( "a second state with the same name is refused", settings.SaveState( "", incoming ) );

	JMWeatherState blank = new JMWeatherState;
	JMAutoTest_AssertNull( "a blank name is refused", settings.SaveState( "", blank ) );
	JMAutoTest_AssertNull( "editing a state that does not exist is refused", settings.SaveState( "Missing", incoming ) );

	//! Front now leads to Calm; renaming Calm must carry that edge with it.
	state.NextStates.Insert( JMAutoTest_Edge( "Calm", 50 ) );

	JMWeatherState renamed = new JMWeatherState;
	renamed.Name = "Still";
	renamed.Phases.Insert( calmPhase.Copy() );

	JMAutoTest_AssertNotNull( "rename is accepted", settings.SaveState( "Calm", renamed ) );
	JMAutoTest_Assert( "old name is gone", !settings.HasState( "Calm" ) );
	JMAutoTest_Assert( "new name is there", settings.HasState( "Still" ) );
	JMAutoTest_Assert( "edges that led to the old name lead to the new one", JMWeatherNextState.Find( state.NextStates, "Still" ) >= 0 );

	// ---------------------------------------------------------------------
	JMAutoTest_Group("RemoveState leaves nothing pointing at what is gone");
	settings.RemoveState( "Still" );
	JMAutoTest_AssertEqualInt( "no edge names a removed state", JMWeatherNextState.Find( state.NextStates, "Still" ), -1 );

	// ---------------------------------------------------------------------
	JMAutoTest_Group("RollEntryPhase: entry weights when set, first phase otherwise");
	JMWeatherState opener = new JMWeatherState;
	JMAutoTest_AssertEqualStr( "no phases means nothing to enter", opener.RollEntryPhase(), "" );

	JMWeatherPhase firstPhase = new JMWeatherPhase;
	firstPhase.Name = "First";
	JMWeatherPhase second = new JMWeatherPhase;
	second.Name = "Second";
	opener.Phases.Insert( firstPhase );
	opener.Phases.Insert( second );

	JMAutoTest_AssertEqualStr( "no entry weights opens on the first phase", opener.RollEntryPhase(), "First" );

	opener.Entry.Insert( JMAutoTest_Edge( "Second", 100 ) );
	JMAutoTest_AssertEqualStr( "a single entry weight is followed", opener.RollEntryPhase(), "Second" );

	// ---------------------------------------------------------------------
	JMAutoTest_Group("GetNextOdds: leave edges expand through next states and entry phases");
	JMWeatherSerialize oddsSettings = new JMWeatherSerialize;

	JMWeatherState here = new JMWeatherState;
	here.Name = "Here";

	JMWeatherPhase hereA = new JMWeatherPhase;
	hereA.Name = "A";
	hereA.Edges.Insert( JMAutoTest_Edge( "A", 50 ) );
	hereA.Edges.Insert( JMAutoTest_Edge( "", 50 ) );
	here.Phases.Insert( hereA );
	here.NextStates.Insert( JMAutoTest_Edge( "There", 100 ) );

	JMWeatherState there = new JMWeatherState;
	there.Name = "There";

	JMWeatherPhase thereX = new JMWeatherPhase;
	thereX.Name = "X";
	JMWeatherPhase thereY = new JMWeatherPhase;
	thereY.Name = "Y";
	there.Phases.Insert( thereX );
	there.Phases.Insert( thereY );
	there.Entry.Insert( JMAutoTest_Edge( "X", 50 ) );
	there.Entry.Insert( JMAutoTest_Edge( "Y", 50 ) );

	oddsSettings.States.Insert( here );
	oddsSettings.States.Insert( there );

	array<string> labels = new array<string>;
	array<float> percents = new array<float>;

	oddsSettings.GetNextOdds( "Here", "A", labels, percents );

	JMAutoTest_AssertEqualInt( "three distinct outcomes", labels.Count(), 3 );

	int aIndex = labels.Find( "A" );
	int xIndex = labels.Find( "There > X" );

	JMAutoTest_Assert( "staying in the same phase is listed", aIndex >= 0 );
	JMAutoTest_Assert( "the far state's opening phases are listed by full name", xIndex >= 0 );

	if ( aIndex >= 0 )
		JMAutoTest_AssertFloatNear( "self edge keeps its 50%", percents[aIndex], 50, 0.5 );

	if ( xIndex >= 0 )
		JMAutoTest_AssertFloatNear( "50% leave x 50% entry = 25%", percents[xIndex], 25, 0.5 );

	JMAutoTest_CanaryCheck("JMWeatherStateMachine data logic");
}

#endif
