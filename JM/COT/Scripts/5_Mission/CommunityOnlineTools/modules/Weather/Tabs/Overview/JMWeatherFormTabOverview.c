//! "Overview" tab of JMWeatherForm - live readout, quick actions, global timing
//! and Apply All. The global pickers are READ by every tab's Apply, so the form
//! keeps GlobalTransition()/GlobalDuration() as forwards here; Apply/dirty/preview
//! mechanics stay on the form (see JMWeatherFormTabSky.c header).
class JMWeatherFormTabOverview: JMFormTab
{
	protected JMWeatherForm m_Form;
	protected UIActionScroller m_ScrollerOverview;

	//! Raw widgets from weather_state_row.layout, not UIActions: "now -> next" is one
	//! line of five fixed pieces at known offsets, which spacer-based UIAction
	//! widgets (each claims a full row) cannot do.
	protected ImageWidget m_StateIconNow;
	protected TextWidget  m_StateTextNow;
	protected ImageWidget m_StateArrow;
	protected ImageWidget m_StateIconNext;
	protected TextWidget  m_StateTextNext;
	protected TextWidget  m_StateHint;

	//! Sandstorm/EVR storm badge, right of the now->next display. Neither
	//! state replicates to clients the way the vanilla forecast phenomena do
	//! (see JMWeatherModule.RequestSpecialWeatherStatus), so it is refreshed
	//! from a polled RPC rather than read straight off g_Game.GetWeather().
	protected ImageWidget m_StateSpecialIcon;
	protected TextWidget  m_StateSpecialText;

	//! g_Game.GetTickTime() timestamp of the last poll, not a countdown - Poll()
	//! takes no timeslice parameter to decrement one with.
	protected float m_LastSpecialWeatherPoll;

	//! How often the tab asks the server whether a sandstorm/EVR storm is
	//! active. Once a second is plenty for a status badge - this isn't a control
	//! an admin is dragging in real time.
	static const float SPECIAL_WEATHER_POLL_INTERVAL = 1.0;
	protected UIActionText m_StateTextClock;

	//! Range bars (now vs heading towards), mirroring the "now -> next" row. Rewritten
	//! every frame, so a drag snaps back: live output, not controls - until Easy mode
	//! is switched on, when OnChange_Easy keeps what the admin moved.
	protected UIActionSliderRange m_StateOvercast;
	protected UIActionSliderRange m_StateRain;
	protected UIActionSliderRange m_StateSnow;
	protected UIActionSliderRange m_StateFog;
	protected UIActionSliderRange m_StateWind;

	//! Easy mode: the five bars above stop being a readout and become single-value sliders
	//! that write the live world on the card's Apply. A bar left alone keeps following the
	//! forecast; one the admin moved is held (its bit in m_EasyDirtyMask) until Apply or Refresh.
	protected UIActionImageButton m_ButtonEasy;
	protected UIActionImageButton m_ButtonEasyApply;
	protected bool m_EasyMode;
	protected int  m_EasyDirtyMask;

	static const int EASY_OVERCAST = 0;
	static const int EASY_RAIN     = 1;
	static const int EASY_SNOW     = 2;
	static const int EASY_FOG      = 3;
	static const int EASY_WIND     = 4;

	protected UIActionIconGrid m_QuickWeather;
	protected UIActionIconGrid m_QuickTime;
	protected UIActionImageButton  m_ButtonRefresh;
	protected UIActionButton       m_ButtonApplyAll;
	protected UIActionToggleSwitch m_ToggleFreezeTime;
	protected UIActionToggleSwitch m_ToggleAutoRefresh;
	protected UIActionTimePicker m_PickerTransition;
	protected UIActionTimePicker m_PickerDuration;

	//! Rain/snowfall thresholds a Clear puts back. The rain and snow quick actions
	//! lower the floor to 0.5, which would keep raining at half overcast. These are
	//! the vanilla cfgweather.xml values.
	static const float CLEAR_THRESHOLD_MIN = 0.6;
	static const float CLEAR_THRESHOLD_MAX = 1.0;

	//! UIActionIconCellLabeled is authored at 110x36, but the WrapSpacer adds
	//! its own padding between children on top of that. Under-reserving it by
	//! even a pixel wraps the last chip onto a second row - 116 was too tight
	//! and put 3 chips on one line and 2 on the next.
	static const int QUICK_CELL_W = 160;

	//! Row height in pixels: the 36px chip plus breathing room. This is the
	//! number the card sizes itself to, so it has to be exact.
	static const int QUICK_ROW_H  = 44;

	void JMWeatherFormTabOverview( JMWeatherForm form )
	{
		m_Form = form;
	}

	float GetDuration()
	{
		if ( m_PickerDuration )
			return m_PickerDuration.GetTotalSeconds();

		return JMWeatherForm.DEFAULT_DURATION;
	}

	// -------------------------------------------------------------------------
	//  Global timing - read by every tab's Apply through the form
	// -------------------------------------------------------------------------

	float GetTransition()
	{
		if ( m_PickerTransition )
			return m_PickerTransition.GetTotalSeconds();

		return JMWeatherForm.DEFAULT_TRANSITION;
	}

	bool IsAutoRefreshEnabled()
	{
		return m_ToggleAutoRefresh && m_ToggleAutoRefresh.IsChecked();
	}

	void SetDuration( float seconds )
	{
		if ( m_PickerDuration )
			m_PickerDuration.SetTotalSeconds( seconds );
	}

	//! Paint one phenomenon as the span between now and its forecast. Ends are
	//! sorted for the range, which hides WHICH end is the forecast, so the
	//! direction is passed to the slider separately and its arrows drift to it.
	protected void SetStateRange( UIActionSliderRange range, WeatherPhenomenon phenomenon, float scale, float ceiling )
	{
		if ( !range || !phenomenon )
			return;

		float actual   = phenomenon.GetActual() * scale;
		float forecast = phenomenon.GetForecast() * scale;

		float low  = Math.Clamp( Math.Min( actual, forecast ), 0, ceiling );
		float high = Math.Clamp( Math.Max( actual, forecast ), 0, ceiling );

		range.SetRange( low, high );

		//! Settled weather has nowhere to go, and arrows on a bar that is not
		//! moving would be a lie.
		int direction = 0;

		if ( forecast > actual )
			direction = 1;
		else if ( forecast < actual )
			direction = -1;

		range.SetFlowDirection( direction );
	}

	void SetTransition( float seconds )
	{
		if ( m_PickerTransition )
			m_PickerTransition.SetTotalSeconds( seconds );
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_ScrollerOverview = UIActionManager.CreateScroller( panel );
		Widget content = m_ScrollerOverview.GetContentWidget();

		// --- Live state -------------------------------------------------------
		UIActionCard stateCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_STATE" );
		m_ButtonEasy = stateCard.AddCardHeaderAction( JMConstants.Lucide( "wand-sparkles" ), this, "OnClick_EasyMode", "#STR_COT_WEATHER_MODULE_EASYMODE" );
		m_ButtonEasyApply = stateCard.AddApplyButton( this, "OnClick_EasyApply" );
		m_ButtonEasyApply.Hide();
		m_ButtonRefresh = stateCard.AddRefreshButton( this, "OnClick_Refresh" );

		Widget stateBody = UIActionManager.CreateGridSpacer( stateCard.GetContent(), 3, 1 );

		Widget stateRow = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/weather_state_row.layout", stateBody );

		if ( stateRow )
		{
			m_StateIconNow   = ImageWidget.Cast( stateRow.FindAnyWidget( "wsr_icon_now" ) );
			m_StateTextNow   = TextWidget.Cast( stateRow.FindAnyWidget( "wsr_text_now" ) );
			m_StateArrow     = ImageWidget.Cast( stateRow.FindAnyWidget( "wsr_arrow" ) );
			m_StateIconNext  = ImageWidget.Cast( stateRow.FindAnyWidget( "wsr_icon_next" ) );
			m_StateTextNext  = TextWidget.Cast( stateRow.FindAnyWidget( "wsr_text_next" ) );
			m_StateHint      = TextWidget.Cast( stateRow.FindAnyWidget( "wsr_hint" ) );
			m_StateSpecialIcon = ImageWidget.Cast( stateRow.FindAnyWidget( "wsr_special_icon" ) );
			m_StateSpecialText = TextWidget.Cast( stateRow.FindAnyWidget( "wsr_special_text" ) );

			if ( m_StateSpecialIcon )
				m_StateSpecialIcon.Show( false );
			if ( m_StateSpecialText )
				m_StateSpecialText.Show( false );

			if ( m_StateHint )
				m_StateHint.SetColor( JMTheme.TEXT_MUTED );

			if ( m_StateArrow )
			{
				m_StateArrow.LoadImageFile( 0, JMConstants.Lucide( "move-right" ) );
				m_StateArrow.SetImage( 0 );
				m_StateArrow.SetColor( JMTheme.TEXT_MUTED );
			}

		}

		m_StateTextClock = UIActionManager.CreateText( stateBody, "#STR_COT_WEATHER_MODULE_START_TIME", "" );

		Widget bars = UIActionManager.CreateGridSpacer( stateBody, 5, 1 );

		m_StateOvercast = CreateStateRange( bars, "#STR_COT_WEATHER_MODULE_OVERCAST" );
		m_StateRain     = CreateStateRange( bars, "#STR_COT_WEATHER_MODULE_RAIN" );
		m_StateSnow     = CreateStateRange( bars, "#STR_COT_WEATHER_MODULE_SNOW" );
		m_StateFog      = CreateStateRange( bars, "#STR_COT_WEATHER_MODULE_FOG" );

		//! Wind is the one readout not measured in <0,1> - it is m/s, on the
		//! same 0-20 scale the Wind tab's slider uses.
		m_StateWind = UIActionManager.CreateSliderRange( bars, "#STR_COT_WEATHER_MODULE_WIND", 0, JMWeatherForm.WIND_SCALE_MAX, this, "OnChange_Easy" );
		m_StateWind.SetFormat( "#STR_COT_FORMAT_MPS" );
		m_StateWind.SetStep( 0.1 );

		// --- Quick actions ----------------------------------------------------
		UIActionCard quickCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_QUICKACTIONS" );

		//! Straight into the card body, NO grid spacer of its own: card_content is
		//! a GridSpacer that re-sizes children every pass, so a nested icon grid
		//! lost ForceContentWidth's size and wrapped to two rows.
		Widget quickBody = quickCard.GetContent();

		//! Each strip sits in a FIXED PIXEL HEIGHT panel, not straight in the card:
		//! ForceContentWidth leaves the grid height fractional, and card_content
		//! (Size To Content V) resolves that against the whole tab, so the card
		//! filled the page and pushed the conditions card off.
		Widget quickWeatherRow = UIActionManager.CreateRow( quickBody, QUICK_ROW_H );

		m_QuickWeather = UIActionManager.CreateIconGridHorizontal( quickWeatherRow, this, "OnClick_QuickWeather" );
		m_QuickWeather.UseLabeledCells( true );

		//! Each chip fires a command and is done; none of them is a state the
		//! form stays in. A latched blue cell would claim the weather still IS
		//! whatever was last pressed, which stops being true the moment any
		//! slider or the server's own controller moves.
		m_QuickWeather.SetMomentary( true );
		m_QuickWeather.AddIcon( "clear",  JMConstants.Lucide( "sun" ),             "#STR_COT_WEATHER_CLEAR" );
		m_QuickWeather.AddIcon( "cloudy", JMConstants.Lucide( "cloud-sun" ),       "#STR_COT_WEATHER_CLOUDY" );
		m_QuickWeather.AddIcon( "rainy",  JMConstants.Lucide( "cloud-rain" ),      "#STR_COT_WEATHER_MODULE_RAINY" );
		m_QuickWeather.AddIcon( "snowy",  JMConstants.Lucide( "cloud-snow" ),      "#STR_COT_WEATHER_MODULE_SNOWY" );
		m_QuickWeather.AddIcon( "storm",  JMConstants.Lucide( "cloud-lightning" ), "#STR_COT_WEATHER_MODULE_STORM" );

		//! Without this the inner WrapSpacer sizes itself to the whole cell it
		//! was handed and the strip grows to fill the card vertically. Has to
		//! run AFTER the icons exist. Labeled cells are authored at 110x36.
		m_QuickWeather.ForceContentWidth( QUICK_CELL_W * 5, QUICK_ROW_H );

		Widget quickTimeRow = UIActionManager.CreateRow( quickBody, QUICK_ROW_H );

		m_QuickTime = UIActionManager.CreateIconGridHorizontal( quickTimeRow, this, "OnClick_QuickTime" );
		m_QuickTime.UseLabeledCells( true );
		m_QuickTime.SetMomentary( true );
		m_QuickTime.AddIcon( "dawn",  JMConstants.Lucide( "sunrise" ), "#STR_COT_WEATHER_DAWN" );
		m_QuickTime.AddIcon( "day",   JMConstants.Lucide( "sun" ),     "#STR_COT_WEATHER_DAY" );
		m_QuickTime.AddIcon( "dusk",  JMConstants.Lucide( "sunset" ),  "#STR_COT_WEATHER_DUSK" );
		m_QuickTime.AddIcon( "night", JMConstants.Lucide( "moon" ),    "#STR_COT_WEATHER_NIGHT" );

		m_QuickTime.ForceContentWidth( QUICK_CELL_W * 4, QUICK_ROW_H );

		// --- Timing + session toggles ----------------------------------------
		UIActionCard timingCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_TIMING" );
		Widget timingBody = UIActionManager.CreateGridSpacer( timingCard.GetContent(), 5, 1 );

		//! Both are durations in seconds and both routinely run to minutes or
		//! hours, which a HH:MM:SS field states far better than a raw second
		//! count in a spinner.
		m_PickerTransition = UIActionManager.CreateTimePicker( timingBody, "#STR_COT_GENERIC_TRANSITION", this );
		m_PickerTransition.SetMaxHours( 1 );
		m_PickerTransition.SetTotalSeconds( JMWeatherForm.DEFAULT_TRANSITION );
		m_PickerTransition.SetTooltip( "#STR_COT_WEATHER_TIMING_TRANSITION_DESC" );

		m_PickerDuration = UIActionManager.CreateTimePicker( timingBody, "#STR_COT_GENERIC_DURATION", this );
		m_PickerDuration.SetMaxHours( 6 );
		m_PickerDuration.SetTotalSeconds( JMWeatherForm.DEFAULT_DURATION );
		m_PickerDuration.SetTooltip( "#STR_COT_WEATHER_TIMING_DURATION_DESC" );

		m_ToggleFreezeTime = UIActionManager.CreateToggleSwitch( timingBody, "#STR_COT_WEATHER_FREEZETIME", this, "OnClick_FreezeTime" );
		m_ToggleFreezeTime.SetTooltip( "#STR_COT_WEATHER_FREEZETIME_DESC" );
		if ( m_Form.m_Module )
			m_ToggleFreezeTime.SetChecked( m_Form.m_Module.IsTimeFrozen() );

		m_ToggleAutoRefresh = UIActionManager.CreateToggleSwitch( timingBody, "#STR_COT_WEATHER_MODULE_AUTOREFRESH", this );
		m_ToggleAutoRefresh.SetTooltip( "#STR_COT_WEATHER_AUTOREFRESH_DESC" );
		m_ToggleAutoRefresh.SetChecked( true );

		m_ButtonApplyAll = UIActionManager.CreateButton( timingBody, "#STR_COT_WEATHER_APPLY_ALL", this, "OnClick_ApplyAll" );
		m_ButtonApplyAll.SetColor( JMTheme.SUCCESS_FILL );
		m_ButtonApplyAll.SetTooltip( "#STR_COT_WEATHER_APPLY_ALL_DESC" );

		m_ScrollerOverview.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_ScrollerOverview )
			m_ScrollerOverview.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  Per-frame refresh - driven from JMWeatherForm.Update()
	// -------------------------------------------------------------------------

	void Poll()
	{
		RefreshLiveState();
		RefreshTimingWarnings();
		PollSpecialWeatherStatus();
		PollEasyGlow();
	}

	//! Flag the two legal-but-surprising timing values (not blocked): a zero
	//! transition reads as the weather glitching, a zero hold as it ignoring what
	//! was just applied. A sentence saves the admin hunting a bug.
	protected void RefreshTimingWarnings()
	{
		if ( m_PickerTransition )
			m_PickerTransition.SetWarning( m_PickerTransition.GetTotalSeconds() == 0, "#STR_COT_WEATHER_WARN_TRANSITION" );

		if ( m_PickerDuration )
			m_PickerDuration.SetWarning( m_PickerDuration.GetTotalSeconds() == 0, "#STR_COT_WEATHER_WARN_DURATION" );
	}

	// -------------------------------------------------------------------------
	//  Live state readout
	// -------------------------------------------------------------------------

	//! Current weather, what it is heading towards, and how long until the
	//! controller recomputes. All of it comes from JMWorldConditions so this
	//! panel and the sidebar can never disagree.
	protected void RefreshLiveState()
	{
		if ( !m_StateTextNow )
			return;

		int year, month, day, hour, minute;
		g_Game.GetWorld().GetDate( year, month, day, hour, minute );

		bool isNight = JMWorldConditions.IsNight( hour );

		int nowKind  = JMWorldConditions.GetWeatherKind();
		int nextKind = JMWorldConditions.GetForecastWeatherKind();

		m_StateTextNow.SetText( JMWorldConditions.GetWeatherLabel( nowKind ) );
		//! The countdown is part of the Next label rather than a widget of its
		//! own, so it stays glued to the state it belongs to however long the
		//! translated weather name turns out to be.
		string nextLabel = JMWorldConditions.GetWeatherLabel( nextKind );

		string countdown = JMWorldConditions.FormatSecondsToChange( JMWorldConditions.GetSecondsToWeatherChange() );

		//! The engine exposes ONE number, GetNextChange (wait until the next forecast),
		//! so current-weather duration and time-to-next are the same: printed once as
		//! a wait ("in 2m"); the second line says what it means, not the figure.
		if ( countdown != "" )
			nextLabel += "   " + COT_String.TranslateEx( "#STR_COT_WEATHER_IN", countdown );

		m_StateTextNext.SetText( nextLabel );

		if ( m_StateHint )
		{
			if ( countdown != "" )
				m_StateHint.SetText( "#STR_COT_WEATHER_STATE_HOLD" );
			else
				m_StateHint.SetText( "#STR_COT_WEATHER_STATE_NOCHANGE" );
		}

		LoadStateIcon( m_StateIconNow, JMWorldConditions.GetWeatherIcon( nowKind, isNight ) );
		LoadStateIcon( m_StateIconNext, JMWorldConditions.GetWeatherIcon( nextKind, isNight ) );

		int tod = JMWorldConditions.GetTimeOfDay( hour );

		string clock = JMWorldConditions.GetTimeOfDayLabel( tod );
		clock += "  " + JMWeatherFormTabTime.FormatClock( hour, minute );
		clock += "  " + day + "/" + month + "/" + year;

		//! The countdown lives on the state row now, beside the arrow it
		//! belongs to, so it is not repeated here.
		m_StateTextClock.SetText( clock );

		Weather weather = g_Game.GetWeather();
		if ( !weather )
			return;

		if ( m_EasyMode )
		{
			SetEasyValue( EASY_OVERCAST, m_StateOvercast, weather.GetOvercast(), 100.0, 100.0 );
			SetEasyValue( EASY_RAIN, m_StateRain, weather.GetRain(), 100.0, 100.0 );
			SetEasyValue( EASY_SNOW, m_StateSnow, weather.GetSnowfall(), 100.0, 100.0 );
			SetEasyValue( EASY_FOG, m_StateFog, weather.GetFog(), 100.0, 100.0 );
			SetEasyValue( EASY_WIND, m_StateWind, weather.GetWindMagnitude(), 1.0, JMWeatherForm.WIND_SCALE_MAX );
		}
		else
		{
			SetStateRange( m_StateOvercast, weather.GetOvercast(), 100.0, 100.0 );
			SetStateRange( m_StateRain, weather.GetRain(), 100.0, 100.0 );
			SetStateRange( m_StateSnow, weather.GetSnowfall(), 100.0, 100.0 );
			SetStateRange( m_StateFog, weather.GetFog(), 100.0, 100.0 );
			SetStateRange( m_StateWind, weather.GetWindMagnitude(), 1.0, JMWeatherForm.WIND_SCALE_MAX );
		}

		RefreshSpecialWeatherBadge();
	}

	//! Asks the server whether a sandstorm or (Namalsk) EVR storm is active,
	//! throttled to once a second - see JMWeatherModule.RequestSpecialWeatherStatus.
	protected void PollSpecialWeatherStatus()
	{
		if ( !m_Form.m_Module )
			return;

		float now = g_Game.GetTickTime();
		if ( now - m_LastSpecialWeatherPoll < SPECIAL_WEATHER_POLL_INTERVAL )
			return;

		m_LastSpecialWeatherPoll = now;
		m_Form.m_Module.RequestSpecialWeatherStatus();
	}

	//! Paints the badge from whatever JMWeatherModule last heard back from
	//! PollSpecialWeatherStatus's RPC - hidden when neither is active.
	protected void RefreshSpecialWeatherBadge()
	{
		if ( !m_StateSpecialIcon || !m_StateSpecialText || !m_Form.m_Module )
			return;

		string label = "";
		string icon = "";

		//! EVR takes the row when both happen to be active at once - it is
		//! the more dangerous of the two and the one an admin is less likely
		//! to already know about from the sky itself.
		if ( m_Form.m_Module.m_LastEVRStormActive )
		{
			label = "#STR_COT_WEATHER_EVR_ACTIVE";
			icon = JMConstants.Lucide( "radiation" );
		}
		else if ( m_Form.m_Module.m_LastSandstormActive )
		{
			label = "#STR_COT_WEATHER_SANDSTORM_ACTIVE";
			icon = JMConstants.Lucide( "wind" );
		}

		bool active = label != "";

		m_StateSpecialIcon.Show( active );
		m_StateSpecialText.Show( active );

		if ( !active )
			return;

		m_StateSpecialText.SetText( label );
		m_StateSpecialText.SetColor( JMTheme.WARNING );
		m_StateSpecialIcon.LoadImageFile( 0, icon );
		m_StateSpecialIcon.SetImage( 0 );
		m_StateSpecialIcon.SetColor( JMTheme.WARNING );
	}

	//! A 0-100 percentage range bar for one phenomenon.
	protected UIActionSliderRange CreateStateRange( Widget parent, string label )
	{
		UIActionSliderRange range = UIActionManager.CreateSliderRange( parent, label, 0, 100, this, "OnChange_Easy" );
		range.SetFormat( "#STR_COT_FORMAT_PERCENTAGE" );
		range.SetStep( 1 );
		return range;
	}

	protected void LoadStateIcon( ImageWidget widget, string path )
	{
		if ( !widget )
			return;

		widget.LoadImageFile( 0, path );
		widget.SetImage( 0 );
	}

	// -------------------------------------------------------------------------
	//  Easy mode - edit the Current Conditions bars directly
	// -------------------------------------------------------------------------

	protected bool IsEasyDirty( int bar )
	{
		return ( m_EasyDirtyMask & ( 1 << bar ) ) != 0;
	}

	//! The FROM end always follows the live value and cannot be dragged, so it creeps toward
	//! the target as the weather fades. The TARGET end follows the forecast until the admin
	//! moves it, and is then held.
	protected void SetEasyValue( int bar, UIActionSliderRange range, WeatherPhenomenon phenomenon, float scale, float ceiling )
	{
		if ( !range || !phenomenon )
			return;

		range.SetAnchor( Math.Clamp( phenomenon.GetActual() * scale, 0, ceiling ) );

		if ( !IsEasyDirty( bar ) )
			range.SetTarget( Math.Clamp( phenomenon.GetForecast() * scale, 0, ceiling ) );
	}

	protected void SetEasyAnchored( bool anchored )
	{
		if ( m_StateOvercast ) m_StateOvercast.SetAnchored( anchored );
		if ( m_StateRain )     m_StateRain.SetAnchored( anchored );
		if ( m_StateSnow )     m_StateSnow.SetAnchored( anchored );
		if ( m_StateFog )      m_StateFog.SetAnchored( anchored );
		if ( m_StateWind )     m_StateWind.SetAnchored( anchored );
	}

	//! Per frame, from Poll(): the Apply icon glows while a bar is held and unapplied.
	protected void PollEasyGlow()
	{
		if ( m_ButtonEasyApply )
			m_ButtonEasyApply.SetGlow( m_EasyMode && m_EasyDirtyMask != 0 );
	}

	void OnChange_Easy( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || !m_EasyMode )
			return;

		if ( action == m_StateOvercast )
			m_EasyDirtyMask = m_EasyDirtyMask | ( 1 << EASY_OVERCAST );
		else if ( action == m_StateRain )
			m_EasyDirtyMask = m_EasyDirtyMask | ( 1 << EASY_RAIN );
		else if ( action == m_StateSnow )
			m_EasyDirtyMask = m_EasyDirtyMask | ( 1 << EASY_SNOW );
		else if ( action == m_StateFog )
			m_EasyDirtyMask = m_EasyDirtyMask | ( 1 << EASY_FOG );
		else if ( action == m_StateWind )
			m_EasyDirtyMask = m_EasyDirtyMask | ( 1 << EASY_WIND );
	}

	//! Switching either way drops whatever was held: the readout takes the bars back.
	void OnClick_EasyMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_EasyMode      = !m_EasyMode;
		m_EasyDirtyMask = 0;

		SetEasyAnchored( m_EasyMode );

		if ( m_EasyMode )
		{
			m_ButtonEasy.SetColor( JMTheme.ACCENT );
			m_ButtonEasyApply.Show();
		}
		else
		{
			m_ButtonEasy.SetColor( JMTheme.BUTTON_FILL );
			m_ButtonEasyApply.Hide();
		}

		PollEasyGlow();
	}

	//! Writes the bars the admin moved to the live world, each under its own permission,
	//! with the Transition / Duration from the Timing card.
	void OnClick_EasyApply( UIEvent eid, UIActionBase action )
	{
		JMWeatherModule module = m_Form.m_Module;

		if ( eid != UIEvent.CLICK || !module || !m_EasyMode )
			return;

		action.AnimateFeedback();

		float transition = m_Form.GlobalTransition();
		float duration   = m_Form.GlobalDuration();

		if ( IsEasyDirty( EASY_OVERCAST ) && JMPermissions.Has( JMConstants.PERM_WEATHER_OVERCAST ) )
			module.SetOvercast( m_StateOvercast.GetTarget() * 0.01, transition, duration );

		//! Rain and snow only fall while the overcast sits inside their thresholds (vanilla
		//! 0.6 - 1.0), so a bare amount does nothing under a lighter sky. Opened fully so the
		//! amount the admin asked for is what shows, as the Rainy / Snowy quick actions do.
		//! Rain and snow are one or the other, as the Rainy / Snowy quick actions treat them: rain
		//! left falling hides the snow (and the reverse). So switching one on sends the other to
		//! zero, unless the admin moved that bar too.
		bool rainDirty = IsEasyDirty( EASY_RAIN );
		bool snowDirty = IsEasyDirty( EASY_SNOW );

		if ( rainDirty && JMPermissions.Has( JMConstants.PERM_WEATHER_RAIN ) )
		{
			module.SetRain( m_StateRain.GetTarget() * 0.01, transition, duration );

			if ( m_StateRain.GetTarget() > 0 && JMPermissions.Has( JMConstants.PERM_WEATHER_RAIN_THRESHOLDS ) )
				module.SetRainThresholds( 0.0, 1.0, transition );

			if ( m_StateRain.GetTarget() > 0 && !snowDirty && JMPermissions.Has( JMConstants.PERM_WEATHER_SNOW ) )
				module.SetSnow( 0, transition, duration );
		}

		if ( snowDirty && JMPermissions.Has( JMConstants.PERM_WEATHER_SNOW ) )
		{
			module.SetSnow( m_StateSnow.GetTarget() * 0.01, transition, duration );

			if ( m_StateSnow.GetTarget() > 0 && JMPermissions.Has( JMConstants.PERM_WEATHER_SNOW_THRESHOLDS ) )
				module.SetSnowThresholds( 0.0, 1.0, transition );

			if ( m_StateSnow.GetTarget() > 0 && !rainDirty && JMPermissions.Has( JMConstants.PERM_WEATHER_RAIN ) )
				module.SetRain( 0, transition, duration );
		}

		if ( IsEasyDirty( EASY_FOG ) && JMPermissions.Has( JMConstants.PERM_WEATHER_FOG ) )
			module.SetFog( m_StateFog.GetTarget() * 0.01, transition, duration );

		if ( IsEasyDirty( EASY_WIND ) && JMPermissions.Has( JMConstants.PERM_WEATHER_WIND ) )
			module.SetWindMagnitude( m_StateWind.GetTarget(), transition, duration );

		m_EasyDirtyMask = 0;
	}

	// -------------------------------------------------------------------------
	//  Apply All
	// -------------------------------------------------------------------------

	//! Writes every section the admin has touched, and only those. A section
	//! nobody edited is left to the mission's own weather controller.
	void OnClick_ApplyAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module )
			return;

		action.AnimateFeedback();

		JMWeatherFormTabSky           sky           = m_Form.m_TabSkyCtrl;
		JMWeatherFormTabPrecipitation precipitation = m_Form.m_TabPrecipitationCtrl;
		JMWeatherFormTabWind          wind          = m_Form.m_TabWindCtrl;
		JMWeatherFormTabTime          timeTab       = m_Form.m_TabTimeCtrl;

		//! Dirty tracking exists so a live Apply All does not write sections
		//! the admin never touched. A preset preview has no such risk - it
		//! changes nothing but this client's own view - and the admin asked to
		//! see the preset, which means all of it.
		if ( m_Form.IsPresetMode() )
		{
			if ( sky && JMPermissions.Has( JMConstants.PERM_WEATHER_OVERCAST ) )        sky.ApplyOvercast();
			if ( sky && JMPermissions.Has( JMConstants.PERM_WEATHER_FOG ) )             sky.ApplyFog();
			if ( sky && JMPermissions.Has( JMConstants.PERM_WEATHER_STORM ) )           sky.ApplyStorm();
			if ( sky && JMPermissions.Has( JMConstants.PERM_WEATHER_SANDSTORM ) )       sky.ApplySandstorm();
			if ( precipitation && JMPermissions.Has( JMConstants.PERM_WEATHER_RAIN ) )  precipitation.ApplyRain();
			if ( precipitation && JMPermissions.Has( JMConstants.PERM_WEATHER_SNOW ) )  precipitation.ApplySnow();
			if ( wind && JMPermissions.Has( JMConstants.PERM_WEATHER_WIND ) )           wind.ApplyWind();
			if ( wind && JMPermissions.Has( JMConstants.PERM_WEATHER_WIND_FUNCPARAMS ) ) wind.ApplyWindFunction();
			if ( timeTab && JMPermissions.Has( JMConstants.PERM_WEATHER_DATE ) )           timeTab.ApplyTime();
			return;
		}

		if ( sky && m_Form.IsSectionDirty( JMWeatherForm.SECTION_OVERCAST ) && JMPermissions.Has( JMConstants.PERM_WEATHER_OVERCAST ) )        sky.ApplyOvercast();
		if ( sky && m_Form.IsSectionDirty( JMWeatherForm.SECTION_FOG ) && JMPermissions.Has( JMConstants.PERM_WEATHER_FOG ) )                  sky.ApplyFog();
		if ( sky && m_Form.IsSectionDirty( JMWeatherForm.SECTION_STORM ) && JMPermissions.Has( JMConstants.PERM_WEATHER_STORM ) )              sky.ApplyStorm();
		if ( sky && m_Form.IsSectionDirty( JMWeatherForm.SECTION_SANDSTORM ) && JMPermissions.Has( JMConstants.PERM_WEATHER_SANDSTORM ) )      sky.ApplySandstorm();
		if ( precipitation && m_Form.IsSectionDirty( JMWeatherForm.SECTION_RAIN ) && JMPermissions.Has( JMConstants.PERM_WEATHER_RAIN ) )      precipitation.ApplyRain();
		if ( precipitation && m_Form.IsSectionDirty( JMWeatherForm.SECTION_SNOW ) && JMPermissions.Has( JMConstants.PERM_WEATHER_SNOW ) )      precipitation.ApplySnow();
		if ( wind && m_Form.IsSectionDirty( JMWeatherForm.SECTION_WIND ) && JMPermissions.Has( JMConstants.PERM_WEATHER_WIND ) )               wind.ApplyWind();
		if ( wind && m_Form.IsSectionDirty( JMWeatherForm.SECTION_WIND_FUNCTION ) && JMPermissions.Has( JMConstants.PERM_WEATHER_WIND_FUNCPARAMS ) ) wind.ApplyWindFunction();
		if ( timeTab && m_Form.IsSectionDirty( JMWeatherForm.SECTION_TIME ) && JMPermissions.Has( JMConstants.PERM_WEATHER_DATE ) )               timeTab.ApplyTime();
	}

	// -------------------------------------------------------------------------
	//  Quick actions
	// -------------------------------------------------------------------------

	void OnClick_QuickWeather( UIEvent eid, UIActionBase action )
	{
		JMWeatherModule module = m_Form.m_Module;

		if ( eid != UIEvent.CLICK || !module || !m_QuickWeather )
			return;

		string id = m_QuickWeather.GetLastClickedId();

		float transition = m_Form.GlobalTransition();
		float duration   = m_Form.GlobalDuration();

		JMWeatherFormTabSky sky = m_Form.m_TabSkyCtrl;

		//! Clear is the undo button of this strip, and an undo that fades is
		//! not an undo. It lands instantly whatever the Transition field says,
		//! so the rain the admin is trying to stop stops now rather than over
		//! the next few minutes.
		if ( id == "clear" )
			transition = 0;

		//! Enforce has no block scope, so every quick action's working values
		//! are named once here rather than per branch.
		float overcast = 0;
		float rain     = 0;
		float snow     = 0;
		float fog      = 0;
		float density  = 0;
		float windMin  = Math.RandomFloatInclusive( 0, 0.8 );

		if ( id == "clear" )
		{
			overcast = 0;
		}
		else if ( id == "cloudy" )
		{
			overcast = Math.RandomFloatInclusive( 0.5, 0.8 );
			if ( sky ) density = sky.ReadStormDensity();
		}
		else if ( id == "rainy" )
		{
			overcast = Math.RandomFloatInclusive( 0.7, 1.0 );
			rain     = Math.RandomFloatInclusive( 0.5, 1.0 );
			fog      = Math.RandomFloatInclusive( 0.0, 1.0 - windMin );
			if ( sky ) density = sky.ReadStormDensity();
		}
		else if ( id == "snowy" )
		{
			overcast = Math.RandomFloatInclusive( 0.7, 1.0 );
			snow     = Math.RandomFloatInclusive( 0.5, 1.0 );
			fog      = Math.RandomFloatInclusive( 0.0, 1.0 - windMin );
			if ( sky ) density = sky.ReadStormDensity();
		}
		else if ( id == "storm" )
		{
			overcast = Math.RandomFloatInclusive( 0.7, 1.0 );
			rain     = Math.RandomFloatInclusive( 0.5, 1.0 );
			fog      = Math.RandomFloatInclusive( 0.0, 1.0 - windMin );
			density  = 1.0;
		}
		else
		{
			return;
		}

		module.SetOvercast( overcast, transition, duration );
		module.SetRain( rain, transition, duration );
		module.SetSnow( snow, transition, duration );
		module.SetFog( fog, transition, duration );
		module.SetDynamicFog( 0, 0, 0 );

		if ( id == "rainy" )
			module.SetRainThresholds( 0.5, 1.0, transition );

		if ( id == "snowy" )
			module.SetSnowThresholds( 0.5, 1.0, transition );

		float lightningInterval = JMWeatherFormTabSky.DEFAULT_LIGHTNING_INTERVAL;
		float stormThreshold    = 0;
		if ( sky )
		{
			lightningInterval = sky.StormLightningInterval();
			stormThreshold    = sky.ReadStormThreshold();
		}

		if ( id == "storm" )
			module.SetStorm( density, 0.7, lightningInterval );
		else if ( density > 0 )
			module.SetStorm( density, Math.Max( stormThreshold, 0.7 ), lightningInterval );

		if ( id == "clear" )
		{
			//! Rain/snow 0 is not enough: the rainy/snowy actions drop the overcast
			//! threshold to 0.5, a standing rule that restarts rain once cloud cover
			//! passes half. Clear puts both thresholds back to vanilla.
			module.SetRainThresholds( CLEAR_THRESHOLD_MIN, CLEAR_THRESHOLD_MAX, transition );
			module.SetSnowThresholds( CLEAR_THRESHOLD_MIN, CLEAR_THRESHOLD_MAX, transition );

			//! Storm is a separate system with no getter, so nothing else here
			//! would ever turn the lightning off again.
			module.SetStorm( 0, 1.0, lightningInterval );

			module.SetWindMagnitude( 0, transition, duration );
			module.SetWindDirection( 0, transition, duration );
			module.SetWindFunctionParams( 0, 1, 30 );
		}
		else
		{
			module.SetWindMagnitude( Math.RandomFloatInclusive( 0, 20 ), transition, duration );

			//! Wind direction is an ANGLE IN RADIANS, documented as <-PI, +PI>.
			module.SetWindDirection( Math.RandomFloatInclusive( -Math.PI, Math.PI ), transition, duration );

			module.SetWindFunctionParams( windMin, Math.RandomFloatInclusive( windMin, 1 ), Math.RandomFloatInclusive( 0, 30 ) );
		}

		//! A quick action is a live-world command even in preset mode, but the
		//! controls are holding a preset there - so the world it just set is
		//! not read back into them.
		if ( !m_Form.IsPresetMode() )
			m_Form.RefreshValues();
	}

	void OnClick_QuickTime( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Form.m_Module || !m_QuickTime )
			return;

		string id = m_QuickTime.GetLastClickedId();

		int hour = -1;

		if ( id == "dawn" )
			hour = 6;
		else if ( id == "day" )
			hour = 12;
		else if ( id == "dusk" )
			hour = 18;
		else if ( id == "night" )
			hour = 0;

		if ( hour < 0 )
			return;

		//! Only the time of day changes - the calendar date is whatever the
		//! world already says, not whatever the Time tab happens to be showing.
		int year, month, day, currentHour, minute;
		g_Game.GetWorld().GetDate( year, month, day, currentHour, minute );

		m_Form.m_Module.SetDate( year, month, day, hour, 0 );

		if ( !m_Form.IsPresetMode() )
			m_Form.RefreshValues();
	}

	// -------------------------------------------------------------------------
	//  Session toggles
	// -------------------------------------------------------------------------

	void OnClick_FreezeTime( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE && eid != UIEvent.CLICK )
			return;

		if ( m_Form.m_Module && m_ToggleFreezeTime )
			m_Form.m_Module.SetFreezeTime( m_ToggleFreezeTime.IsChecked() );
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		//! An explicit refresh discards local edits rather than protecting
		//! them - and reloads whatever the tabs are meant to be showing, which
		//! in preset mode is the stored preset, not the server.
		m_Form.ClearDirty();
		m_EasyDirtyMask = 0;

		m_Form.LoadEditorValues();
	}

	void UpdateStates()
	{
		m_Form.UpdateActionState( m_ButtonRefresh, JMConstants.PERM_WEATHER_VIEW );
		m_Form.UpdateActionState( m_ButtonApplyAll, JMConstants.PERM_WEATHER_VIEW );
		m_Form.UpdateActionState( m_ButtonEasy, JMConstants.PERM_WEATHER_VIEW );
		m_Form.UpdateActionState( m_ButtonEasyApply, JMConstants.PERM_WEATHER_VIEW );
		m_Form.UpdateActionState( m_ToggleFreezeTime, JMConstants.PERM_WEATHER_FREEZETIME );
		m_Form.UpdateActionState( m_ToggleAutoRefresh, JMConstants.PERM_WEATHER_VIEW );

		m_Form.UpdateActionState( m_PickerTransition, JMConstants.PERM_WEATHER_VIEW );
		m_Form.UpdateActionState( m_PickerDuration, JMConstants.PERM_WEATHER_VIEW );

		//! The quick-action grids answer to their own per-action permissions,
		//! which the grid cannot express per cell - so the whole strip follows
		//! the umbrella permission.
		m_Form.UpdateActionState( m_QuickWeather, JMConstants.PERM_WEATHER_QUICKACTION );
		m_Form.UpdateActionState( m_QuickTime, JMConstants.PERM_WEATHER_QUICKACTION_DATE );
	}
}
