// =============================================================================
//  JMWeatherMachine - the dynamic weather state machine, server side
//
//  Owns "where are we" and "when do we move": which state and phase the
//  weather is in, the single timer that ends the current phase, and the
//  persisted runtime record (WeatherState.json) that lets a restart carry on
//  from the same place. The authored configuration it walks (states, phases,
//  presets) belongs to JMWeatherSerialize and is only read here.
//
//  Movement is the same three rolls at every level, all over percentage lists:
//
//    phase --edge--> another phase of the same state       (or "leave")
//    leave --state.NextStates--> another state
//    state --Entry--> the phase that state opens on
//
//  Timing belongs to the phase being ENTERED. Its hold is drawn ONCE, when it
//  is entered, and that one figure is both the timer and the MinDuration
//  handed to the engine - the two used to be drawn separately and merely
//  agreed in scale.
//
//  Time is tracked as seconds LEFT, never as a wall-clock deadline, so time the
//  server spends down does not count against a phase: a restart resumes with
//  what was remaining when the last snapshot was written.
// =============================================================================
class JMWeatherMachine
{
	//! How often the remaining hold is snapshotted to disk while running. A crash
	//! (a clean shutdown snapshots on its own) costs at most this much of a hold.
	static const int HEARTBEAT_SECONDS = 60;

	protected JMWeatherModule m_Module;
	protected JMWeatherSerialize m_Settings;
	protected ref JMWeatherRuntime m_Runtime;

	//! A phase is current: either its timer is armed, or it is paused.
	protected bool m_Running;

	//! Freeze Time is on. Held apart from m_Paused so that a phase entered WHILE
	//! frozen (an admin advancing it by hand) comes up paused too.
	protected bool m_Frozen;
	protected bool m_Paused;

	//! An admin paused the countdown by hand. Independent of Freeze Time: either one
	//! keeps the phase held, and it moves on again only when neither does.
	protected bool m_Held;

	//! g_Game.GetTickTime() at which the current hold runs out. Meaningless while paused.
	protected float m_HoldEnd;

	void JMWeatherMachine( JMWeatherModule module, JMWeatherSerialize settings, bool frozen )
	{
		m_Module   = module;
		m_Settings = settings;
		m_Frozen   = frozen;
		m_Runtime  = new JMWeatherRuntime;
	}

	// -------------------------------------------------------------------------
	//  Reading the machine
	// -------------------------------------------------------------------------

	bool IsRunning()
	{
		return m_Running;
	}

	bool IsPaused()
	{
		return m_Paused;
	}

	string GetStateName()
	{
		if ( !m_Running )
			return "";

		return m_Runtime.State;
	}

	string GetPhaseName()
	{
		if ( !m_Running )
			return "";

		return m_Runtime.Phase;
	}

	float GetHoldTotal()
	{
		return m_Runtime.HoldTotal;
	}

	float GetRemaining()
	{
		if ( !m_Running )
			return 0;

		if ( m_Paused )
			return m_Runtime.HoldRemaining;

		return Math.Max( m_HoldEnd - g_Game.GetTickTime(), 0 );
	}

	void FillStatus( JMWeatherDynamicStatus status )
	{
		status.Enabled       = m_Settings.DynamicEnabled;
		status.Paused        = m_Paused;
		status.Held          = m_Held;
		status.State         = GetStateName();
		status.Phase         = GetPhaseName();
		status.HoldTotal     = GetHoldTotal();
		status.HoldRemaining = GetRemaining();
	}

	// -------------------------------------------------------------------------
	//  Lifecycle
	// -------------------------------------------------------------------------

	//! Server start: carry on from the recorded position when there is one that
	//! still means something, otherwise begin fresh. Does nothing while the
	//! machine is switched off.
	void Boot()
	{
		Halt();

		if ( !m_Settings.DynamicEnabled )
			return;

		m_Runtime = JMWeatherRuntime.Load();

		if ( Resume() )
			return;

		BeginRandom();
	}

	//! A clean shutdown: record how much of the hold is left.
	void Snapshot()
	{
		if ( !m_Running )
			return;

		if ( !m_Paused )
			m_Runtime.HoldRemaining = GetRemaining();

		m_Runtime.Save();
	}

	//! Stop moving. `forget` also wipes the recorded position, which is what
	//! turning the machine OFF means: switching it back on begins fresh rather
	//! than resuming a position from before it was disabled.
	void Stop( bool forget )
	{
		Halt();

		if ( forget )
		{
			m_Runtime.Clear();
			m_Runtime.Save();
		}
	}

	//! The configuration changed (switched on or off, a state edited or removed,
	//! a preset deleted). Brings the machine in line WITHOUT restarting a phase
	//! that is still valid - saving an unrelated edit must not reset the
	//! countdown that is halfway through.
	void OnConfigChanged()
	{
		if ( !m_Settings.DynamicEnabled )
		{
			if ( m_Running || m_Runtime.IsSet() )
				Stop( true );

			return;
		}

		if ( !m_Running )
		{
			BeginRandom();
			return;
		}

		JMWeatherState state = m_Settings.GetState( m_Runtime.State );

		//! The state was deleted out from under the machine.
		if ( !state )
		{
			BeginRandom();
			return;
		}

		if ( state.HasPhase( m_Runtime.Phase ) )
			return;

		//! The phase went (removed, or renamed in the edit): re-enter the state
		//! the way it opens rather than jumping to some other state entirely.
		string entry = state.RollEntryPhase();

		if ( entry == "" )
		{
			BeginRandom();
			return;
		}

		EnterPhase( state, entry );
	}

	void OnStateRenamed( string from, string to )
	{
		if ( m_Runtime.State != from )
			return;

		m_Runtime.State = to;
		m_Runtime.Save();
	}

	//! Freeze Time: hold the countdown where it is.
	void Pause()
	{
		m_Frozen = true;

		Suspend();
	}

	void Unpause()
	{
		m_Frozen = false;

		Release();
	}

	//! The pause button: hold the countdown, or let it run again. Whatever phase
	//! comes next - by hand or by Restart - stays held too, until it is let go.
	void SetHeld( bool held )
	{
		m_Held = held;

		if ( held )
			Suspend();
		else
			Release();
	}

	bool IsHeld()
	{
		return m_Held;
	}

	// -------------------------------------------------------------------------
	//  Moving
	// -------------------------------------------------------------------------

	//! Begin from a random state, as if the server had just started.
	void BeginRandom()
	{
		JMWeatherState state = RandomState();

		if ( !state )
		{
			Stop( true );
			return;
		}

		EnterPhase( state, state.RollEntryPhase() );
	}

	//! Leave the current phase now, by the roll a natural expiry would use.
	void Advance()
	{
		if ( !m_Running )
			return;

		JMWeatherState state = m_Settings.GetState( m_Runtime.State );

		if ( !state )
		{
			BeginRandom();
			return;
		}

		JMWeatherPhase phase = state.GetPhase( m_Runtime.Phase );

		if ( !phase )
		{
			BeginRandom();
			return;
		}

		int edge = JMWeatherNextState.Roll( phase.Edges );

		//! A phase with no edges stays where it is and comes back after another hold.
		if ( edge < 0 )
		{
			Hold( phase );
			return;
		}

		string to = phase.Edges[edge].To;

		if ( to != "" )
		{
			if ( state.HasPhase( to ) )
				EnterPhase( state, to );
			else
				Hold( phase );

			return;
		}

		//! Leaving the state.
		int leave = JMWeatherNextState.Roll( state.NextStates );

		if ( leave < 0 )
		{
			Hold( phase );
			return;
		}

		JMWeatherState next = m_Settings.GetState( state.NextStates[leave].To );

		if ( !next )
		{
			Hold( phase );
			return;
		}

		string entry = next.RollEntryPhase();

		if ( entry == "" )
		{
			Hold( phase );
			return;
		}

		EnterPhase( next, entry );
	}

	//! Go to a state and phase now. An empty phase means that state's own entry roll.
	bool JumpTo( string stateName, string phaseName )
	{
		if ( !m_Settings.DynamicEnabled )
			return false;

		JMWeatherState state = m_Settings.GetState( stateName );

		if ( !state )
			return false;

		string target = phaseName;

		if ( target == "" )
			target = state.RollEntryPhase();

		if ( !state.HasPhase( target ) )
			return false;

		EnterPhase( state, target );

		return true;
	}

	//! A state's weather was applied by hand: move the machine to that phase
	//! WITHOUT applying anything - the caller has already put the weather on as the
	//! admin asked, with its own timing, and the machine simply carries on from
	//! there. An empty phase means the state's entry roll. False when the machine is
	//! not running, in which case there is nothing to move.
	bool Adopt( string stateName, string phaseName )
	{
		if ( !m_Running || !m_Settings.DynamicEnabled )
			return false;

		JMWeatherState state = m_Settings.GetState( stateName );

		if ( !state )
			return false;

		string target = phaseName;

		if ( target == "" )
			target = state.RollEntryPhase();

		JMWeatherPhase phase = state.GetPhase( target );

		if ( !phase )
			return false;

		Settle( state, phase, RollHold( phase ) );

		return true;
	}

	// -------------------------------------------------------------------------
	//  Internals
	// -------------------------------------------------------------------------

	protected void Halt()
	{
		Disarm();

		m_Running = false;
		m_Paused  = false;
		m_Held    = false;
	}

	//! Stop the clock where it stands, remembering what is left of the hold.
	protected void Suspend()
	{
		if ( !m_Running || m_Paused )
			return;

		float remaining = GetRemaining();

		Disarm();

		m_Paused = true;
		m_Runtime.HoldRemaining = remaining;
		m_Runtime.Save();
	}

	//! Start the clock again - unless something else still holds it.
	protected void Release()
	{
		if ( !m_Running || !m_Paused )
			return;

		if ( m_Frozen || m_Held )
			return;

		Arm( m_Runtime.HoldRemaining );
	}

	//! Take the recorded position back up. False when there is none, or it names
	//! something that no longer exists - the caller then begins fresh.
	protected bool Resume()
	{
		if ( !m_Runtime.IsSet() )
			return false;

		JMWeatherState state = m_Settings.GetState( m_Runtime.State );

		if ( !state )
			return false;

		JMWeatherPhase phase = state.GetPhase( m_Runtime.Phase );

		if ( !phase )
			return false;

		//! Always at least a second, so a snapshot taken right at expiry still
		//! comes back through the normal roll rather than being dropped.
		float remaining = Math.Max( m_Runtime.HoldRemaining, 1 );

		//! The world does not remember what it was told before the restart, so the
		//! phase's weather goes back on - at once, there is nothing to fade from.
		Apply( phase, 0, remaining );

		m_Runtime.HoldRemaining = remaining;
		m_Runtime.HoldTotal     = Math.Max( m_Runtime.HoldTotal, remaining );

		Arm( remaining );

		m_Module.OnMachineEntered( state.Name, phase.Name, true );

		return true;
	}

	//! Enter a phase properly: draw its timing once, apply its preset with that
	//! fade and hold, and start the timer.
	protected void EnterPhase( JMWeatherState state, string phaseName )
	{
		JMWeatherPhase phase = state.GetPhase( phaseName );

		if ( !phase )
			return;

		float hold = RollHold( phase );
		float fade = RandomBetween( phase.TransitionMin, phase.TransitionMax );

		Apply( phase, fade, hold );

		Settle( state, phase, hold );

		m_Module.OnMachineEntered( state.Name, phase.Name, false );
	}

	//! Record a phase as current and start its timer, without applying it.
	protected void Settle( JMWeatherState state, JMWeatherPhase phase, float hold )
	{
		m_Runtime.State         = state.Name;
		m_Runtime.Phase         = phase.Name;
		m_Runtime.HoldTotal     = hold;
		m_Runtime.HoldRemaining = hold;

		Arm( hold );

		m_Runtime.Save();
	}

	//! The phase stays as it is and is given another hold. Nothing is re-applied:
	//! the weather already is what the phase says.
	protected void Hold( JMWeatherPhase phase )
	{
		float hold = RollHold( phase );

		m_Runtime.HoldTotal     = hold;
		m_Runtime.HoldRemaining = hold;

		Arm( hold );

		m_Runtime.Save();
	}

	protected void Apply( JMWeatherPhase phase, float fade, float hold )
	{
		if ( phase.Conditions )
			phase.Conditions.ApplyTimed( fade, hold );
	}

	//! Arm the phase timer for `seconds`. While frozen nothing is scheduled - the
	//! phase is entered paused, holding its full duration until Unpause().
	protected void Arm( float seconds )
	{
		Disarm();

		m_Running = true;

		if ( m_Frozen || m_Held )
		{
			m_Paused = true;
			m_Runtime.HoldRemaining = seconds;
			return;
		}

		m_Paused  = false;
		m_HoldEnd = g_Game.GetTickTime() + seconds;

		g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( OnHoldExpired, seconds * 1000, false );
		g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( OnHeartbeat, HEARTBEAT_SECONDS * 1000, true );
	}

	protected void Disarm()
	{
		g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( OnHoldExpired );
		g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Remove( OnHeartbeat );
	}

	protected void OnHoldExpired()
	{
		Advance();
	}

	protected void OnHeartbeat()
	{
		Snapshot();
	}

	protected float RollHold( JMWeatherPhase phase )
	{
		float hold = RandomBetween( phase.DurationMin, phase.DurationMax );

		return Math.Max( hold, JMWeatherSerialize.DYNAMIC_MIN_INTERVAL );
	}

	protected int RandomBetween( int low, int high )
	{
		if ( high <= low )
			return low;

		//! RandomInt's upper bound is exclusive, so the top of the range would
		//! otherwise never come up.
		return Math.RandomInt( low, high + 1 );
	}

	//! A random state from the rotation.
	//!
	//! States can opt out, which is how one meant only to be reached from other
	//! states stays off the start-of-session dice. If NOTHING is in rotation the
	//! flag is treated as unset rather than as "no weather at all". A state with
	//! no phases cannot be entered and is never picked.
	protected JMWeatherState RandomState()
	{
		array<JMWeatherState> pool = new array<JMWeatherState>;

		int i;

		for ( i = 0; i < m_Settings.States.Count(); i++ )
		{
			if ( m_Settings.States[i].InRotation && m_Settings.States[i].Phases.Count() > 0 )
				pool.Insert( m_Settings.States[i] );
		}

		if ( pool.Count() == 0 )
		{
			for ( i = 0; i < m_Settings.States.Count(); i++ )
			{
				if ( m_Settings.States[i].Phases.Count() > 0 )
					pool.Insert( m_Settings.States[i] );
			}
		}

		if ( pool.Count() == 0 )
			return NULL;

		return pool[ Math.RandomInt( 0, pool.Count() ) ];
	}
}
