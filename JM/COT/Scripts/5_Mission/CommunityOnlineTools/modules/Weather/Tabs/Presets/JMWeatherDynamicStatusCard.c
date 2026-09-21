//! The dynamic weather master switch and the machine's live position: which
//! preset and phase the weather is in, how long that phase has left, and where it
//! can go next.
//!
//! The controls that move the machine - pause / resume, advance, restart - are
//! icons in the card's title bar, like every other action in this tab. The body
//! is only what is being shown: the switch, where the weather is now, a bar that
//! reads "time left / hold length", and a small table of the places it can go next
//! with the odds of each.
//!
//! The position is the server's - this only ever paints what the module last
//! heard (JMWeatherModule.GetDynamicStatus), counting the clock down locally
//! between polls. The switch itself saves the moment it is flipped: there is
//! nothing else on this card to batch it with.
class JMWeatherDynamicStatusCard
{
	//! Repaint no more often than this - a countdown to the second does not need frames.
	static const float REPAINT_INTERVAL = 0.25;

	//! How many of the likeliest next steps the table spells out.
	static const int NEXT_SHOWN = 8;

	protected JMWeatherForm m_Form;
	protected UIActionScroller m_Scroller;

	protected ref UIActionCard m_Card;
	protected UIActionImageButton  m_ButtonHold;
	protected UIActionImageButton  m_ButtonAdvance;
	protected UIActionImageButton  m_ButtonRestart;
	protected UIActionToggleSwitch m_ToggleDynamic;
	protected UIActionText         m_TextNow;
	protected UIActionProgressBar  m_BarTime;

	//! The odds table: a heading, then one bar per possible next step, labelled with
	//! where it goes and filled to its chance. Rebuilt only when the odds change,
	//! which is once per phase, not once per repaint.
	protected Widget m_NextHeader;
	protected Widget m_NextHost;
	protected Widget m_NextGrid;
	protected autoptr array< ref UIActionProgressBar > m_NextBars;
	protected string m_NextSignature;

	protected float m_LastPaint;
	protected bool  m_WasRunning;

	//! Which face the hold button is wearing - pause, or play once it is held.
	protected bool m_ShownHeld;

	void JMWeatherDynamicStatusCard( JMWeatherForm form, Widget content, UIActionScroller scroller )
	{
		m_Form     = form;
		m_Scroller = scroller;
		m_NextBars = new array< ref UIActionProgressBar >;

		m_Card = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_DYN_CURRENT" );

		//! Creation order is left to right in the title bar.
		m_ButtonHold    = m_Card.AddCardHeaderAction( JMConstants.Lucide( "pause" ), this, "OnClick_Hold", "#STR_COT_WEATHER_DYN_HOLD" );
		m_ButtonAdvance = m_Card.AddCardHeaderAction( JMConstants.Lucide( "fast-forward" ), this, "OnClick_Advance", "#STR_COT_WEATHER_DYN_ADVANCE_DESC" );
		m_ButtonRestart = m_Card.AddCardHeaderAction( JMConstants.Lucide( "rotate-ccw" ), this, "OnClick_Restart", "#STR_COT_WEATHER_DYN_RESTART_DESC" );

		Widget body = m_Card.GetContent();

		m_ToggleDynamic = UIActionManager.CreateToggleSwitch( body, "#STR_COT_WEATHER_DYNAMIC_ENABLE", this, "OnChange_Enabled" );
		m_ToggleDynamic.SetTooltip( "#STR_COT_WEATHER_DYNAMIC_ENABLE_DESC" );

		m_TextNow = UIActionManager.CreateText( body, "#STR_COT_WEATHER_DYN_NOW", "" );

		m_BarTime = UIActionManager.CreateProgressBar( body, "#STR_COT_WEATHER_DYN_TIMELEFT", 0 );
		m_BarTime.SetBarText( "" );

		m_NextHeader = UIActionManager.CreateSectionHeader( body, "#STR_COT_WEATHER_DYN_NEXT_TABLE" );
		m_NextHost   = UIActionManager.CreateGridSpacer( body, 1, 1 );

		ShowNext( false );

		RefreshFromSettings();
		Paint();
	}

	//! The switch follows the CONFIGURATION, and is only set here - never from
	//! the per-frame paint, which would flip it back for the moment between a
	//! click and the server's answer.
	void RefreshFromSettings()
	{
		if ( !m_ToggleDynamic || !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
			return;

		m_ToggleDynamic.SetChecked( m_Form.m_Module.IsDynamicEnabled() );

		UpdateStates();
		Paint();
	}

	void UpdateStates()
	{
		bool enabled = ( m_Form.m_Module && m_Form.m_Module.IsDynamicEnabled() );
		bool idle    = true;

		if ( enabled )
		{
			JMWeatherDynamicStatus status = m_Form.m_Module.GetDynamicStatus();

			if ( status.IsRunning() )
				idle = false;
		}

		m_Form.UpdateActionState( m_ToggleDynamic, JMConstants.PERM_WEATHER_PRESET_DYNAMIC );
		m_Form.UpdateActionState( m_ButtonHold, JMConstants.PERM_WEATHER_PRESET_DYNAMIC, idle );
		m_Form.UpdateActionState( m_ButtonAdvance, JMConstants.PERM_WEATHER_PRESET_DYNAMIC, idle );
		m_Form.UpdateActionState( m_ButtonRestart, JMConstants.PERM_WEATHER_PRESET_DYNAMIC, !enabled );
	}

	//! Driven from JMWeatherForm.Update().
	void Poll()
	{
		float now = g_Game.GetTickTime();

		if ( now - m_LastPaint < REPAINT_INTERVAL )
			return;

		m_LastPaint = now;

		Paint();
	}

	protected void Paint()
	{
		if ( !m_TextNow || !m_Form.m_Module || !m_Form.m_Module.HasSettings() )
			return;

		JMWeatherDynamicStatus status = m_Form.m_Module.GetDynamicStatus();

		//! The buttons that move the machine are only meaningful while a phase is
		//! running, and that changes on the server's clock rather than on any
		//! settings update, so they follow it here.
		bool running = m_Form.m_Module.IsDynamicEnabled() && status.IsRunning();

		if ( running != m_WasRunning )
		{
			m_WasRunning = running;
			UpdateStates();
		}

		PaintHoldButton( running && status.Held );

		if ( !m_Form.m_Module.IsDynamicEnabled() )
		{
			m_TextNow.SetText( Widget.TranslateString( "#STR_COT_WEATHER_DYN_OFF" ) );
			m_BarTime.SetProgress( 0 );
			m_BarTime.SetBarText( "" );
			ShowNext( false );
			return;
		}

		if ( !status.IsRunning() )
		{
			m_TextNow.SetText( Widget.TranslateString( "#STR_COT_WEATHER_DYN_IDLE" ) );
			m_BarTime.SetProgress( 0 );
			m_BarTime.SetBarText( "" );
			ShowNext( false );
			return;
		}

		string now = status.State + " > " + status.Phase;

		if ( status.Held )
			now += "  " + Widget.TranslateString( "#STR_COT_WEATHER_DYN_HELD" );
		else if ( status.Paused )
			now += "  " + Widget.TranslateString( "#STR_COT_WEATHER_DYN_PAUSED" );

		m_TextNow.SetText( now );

		float remaining = status.Remaining();
		float total     = Math.Max( status.HoldTotal, 1 );

		m_BarTime.SetProgress( remaining / total );
		m_BarTime.SetBarText( FormatClock( remaining ) + " / " + FormatClock( total ) );

		PaintNext( status );
	}

	//! Pause while the clock runs, play once it is held by hand.
	protected void PaintHoldButton( bool held )
	{
		if ( !m_ButtonHold || held == m_ShownHeld )
			return;

		m_ShownHeld = held;

		if ( held )
		{
			m_ButtonHold.SetImage( JMConstants.Lucide( "play" ) );
			m_ButtonHold.SetTooltip( "#STR_COT_WEATHER_DYN_RESUME" );
		}
		else
		{
			m_ButtonHold.SetImage( JMConstants.Lucide( "pause" ) );
			m_ButtonHold.SetTooltip( "#STR_COT_WEATHER_DYN_HOLD" );
		}
	}

	// -------------------------------------------------------------------------
	//  The odds table
	// -------------------------------------------------------------------------

	protected void ShowNext( bool show )
	{
		if ( m_NextHeader )
			m_NextHeader.Show( show );

		if ( m_NextHost )
			m_NextHost.Show( show );
	}

	//! One bar per possible next step, best first. Rebuilt only when the list
	//! differs from what is on screen.
	protected void PaintNext( JMWeatherDynamicStatus status )
	{
		JMWeatherSerialize settings = m_Form.m_Module.GetSettings();

		array<string> labels = new array<string>;
		array<float> percents = new array<float>;

		settings.GetNextOdds( status.State, status.Phase, labels, percents );

		SortOdds( labels, percents );

		string signature;
		int i;

		for ( i = 0; i < labels.Count(); i++ )
			signature += labels[i] + "=" + Math.Round( percents[i] ).ToString() + ";";

		ShowNext( labels.Count() > 0 );

		if ( signature == m_NextSignature )
			return;

		m_NextSignature = signature;

		RebuildNext( labels, percents );
	}

	//! Best first, and no more than NEXT_SHOWN.
	protected void SortOdds( array<string> labels, array<float> percents )
	{
		array<string> sortedLabels = new array<string>;
		array<float> sortedPercents = new array<float>;

		int best;
		int i;

		while ( sortedLabels.Count() < NEXT_SHOWN && labels.Count() > 0 )
		{
			best = 0;

			for ( i = 1; i < percents.Count(); i++ )
			{
				if ( percents[i] > percents[best] )
					best = i;
			}

			sortedLabels.Insert( labels[best] );
			sortedPercents.Insert( percents[best] );

			labels.Remove( best );
			percents.Remove( best );
		}

		labels.Copy( sortedLabels );
		percents.Copy( sortedPercents );
	}

	protected void RebuildNext( array<string> labels, array<float> percents )
	{
		if ( m_NextGrid )
		{
			m_NextGrid.Unlink();
			m_NextGrid = NULL;
		}

		m_NextBars.Clear();

		int count = labels.Count();

		if ( count > 0 )
		{
			m_NextGrid = UIActionManager.CreateGridSpacer( m_NextHost, count, 1 );

			UIActionProgressBar bar;

			for ( int i = 0; i < count; i++ )
			{
				bar = UIActionManager.CreateProgressBar( m_NextGrid, LabelText( labels[i] ), percents[i] * 0.01 );

				if ( bar )
					m_NextBars.Insert( bar );
			}
		}

		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}

	protected string LabelText( string label )
	{
		if ( label == JMWeatherSerialize.ODDS_STAY )
			return Widget.TranslateString( "#STR_COT_WEATHER_DYN_STAYS" );

		return label;
	}

	//! m:ss, or h:mm:ss once it is an hour or more.
	protected string FormatClock( float seconds )
	{
		int whole   = Math.Round( seconds );
		int hours   = whole / 3600;
		int minutes = ( whole % 3600 ) / 60;
		int secs    = whole % 60;

		string mm = minutes.ToString();
		string ss = secs.ToString();

		if ( secs < 10 )
			ss = "0" + ss;

		if ( hours > 0 )
		{
			if ( minutes < 10 )
				mm = "0" + mm;

			return hours.ToString() + ":" + mm + ":" + ss;
		}

		return mm + ":" + ss;
	}

	// -------------------------------------------------------------------------
	//  Handlers
	// -------------------------------------------------------------------------

	void OnChange_Enabled( UIEvent eid, UIActionBase action )
	{
		//! A toggle switch reports CLICK (it has no CHANGE), so that is what the switch is read on.
		if ( ( eid != UIEvent.CLICK && eid != UIEvent.CHANGE ) || !m_Form.m_Module || !m_ToggleDynamic )
			return;

		//! An empty selection sends the switch by itself and leaves every state alone.
		JMWeatherDynamicConfig config = new JMWeatherDynamicConfig;
		config.Enabled = m_ToggleDynamic.IsChecked();

		m_Form.m_Module.SetDynamicWeather( config );
	}

	void OnClick_Hold( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;

		JMWeatherDynamicStatus status = m_Form.m_Module.GetDynamicStatus();

		JMWeatherDynamicControl control = new JMWeatherDynamicControl;

		if ( status.Held )
			control.Op = JMWeatherDynamicControl.OP_RELEASE;
		else
			control.Op = JMWeatherDynamicControl.OP_HOLD;

		//! The face changes now rather than at the next poll, which is up to a couple
		//! of seconds away.
		//! Frozen time keeps it paused whatever the button says. What is left is pinned
		//! before the flag flips, and the countdown restarts from now on a release.
		status.HoldRemaining = status.Remaining();
		status.ReceivedAt    = g_Game.GetTickTime();
		status.Held          = !status.Held;
		status.Paused        = status.Held || m_Form.m_Module.IsTimeFrozen();

		m_Form.m_Module.SendDynamicControl( control );

		Paint();
	}

	void OnClick_Advance( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;

		action.AnimateFeedback();

		JMWeatherDynamicControl control = new JMWeatherDynamicControl;
		control.Op = JMWeatherDynamicControl.OP_ADVANCE;

		m_Form.m_Module.SendDynamicControl( control );
	}

	void OnClick_Restart( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;

		action.AnimateFeedback();

		JMWeatherDynamicControl control = new JMWeatherDynamicControl;
		control.Op = JMWeatherDynamicControl.OP_RESTART;

		m_Form.m_Module.SendDynamicControl( control );
	}
}
