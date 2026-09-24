//! "Overview" tab of JMWeatherForm - the live weather readout, quick actions,
//! the global timing fields and Apply All. Back-reference to the owning form,
//! same shape as JMPlayerRowWidget.Menu.
//!
//! The global transition/duration pickers live here but are READ by every other
//! tab's Apply, so the form keeps GlobalTransition()/GlobalDuration() as thin
//! forwards to this class. Apply/dirty-tracking/preview mechanics stay on the
//! form for the reasons given in JMWeatherFormTabSky.c's header.
class JMWeatherFormTabOverview: JMFormTab
{
	protected JMWeatherForm m_Form;
	protected UIActionScroller m_ScrollerOverview;

	//! Raw widgets from weather_state_row.layout rather than UIActions.
	//!
	//! "now -> next" is one line of five fixed-size pieces at known offsets,
	//! which is exactly what the sidebar footer does and exactly what the
	//! spacer-based UIAction widgets will not do - every one of them claims a
	//! full row.
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

	//! Range bars, not progress bars: the span between what a phenomenon is
	//! doing now and what it is heading towards says more than a single fill,
	//! and it is the same picture the "now -> next" row draws in words.
	//!
	//! They take no callback and are rewritten every frame, so a drag snaps
	//! straight back - they read as live output rather than as controls.
	protected UIActionSliderRange m_StateOvercast;
	protected UIActionSliderRange m_StateRain;
	protected UIActionSliderRange m_StateSnow;
	protected UIActionSliderRange m_StateFog;
	protected UIActionSliderRange m_StateWind;
	protected UIActionIconGrid m_QuickWeather;
	protected UIActionIconGrid m_QuickTime;
	protected UIActionImageButton  m_ButtonRefresh;
	protected UIActionButton       m_ButtonApplyAll;
	protected UIActionToggleSwitch m_ToggleFreezeTime;
	protected UIActionToggleSwitch m_ToggleAutoRefresh;
	protected UIActionTimePicker m_PickerTransition;
	protected UIActionTimePicker m_PickerDuration;

	//! Rain and snowfall thresholds a Clear puts back. The quick actions for
	//! rain and snow move the floor down to 0.5 so their precipitation starts
	//! immediately; left there, the world keeps raining at half overcast long
	//! after the admin asked for clear skies. These are the vanilla
	//! cfgweather.xml values, so Clear restores normal behaviour rather than
	//! inventing a policy of its own.
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

	//! Paint one phenomenon as the span between where it is and where it is
	//! going. A range wants its ends ordered, and a forecast is as likely to be
	//! below the current value as above it, so they are sorted before use.
	//!
	//! Sorting throws away the one thing the admin most wants to know, though -
	//! WHICH end is the forecast - because the bar looks identical either way
	//! round. So the direction is handed to the slider separately and its
	//! arrows drift toward the target.
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
		m_StateWind = UIActionManager.CreateSliderRange( bars, "#STR_COT_WEATHER_MODULE_WIND", 0, JMWeatherForm.WIND_SCALE_MAX );
		m_StateWind.SetFormat( "#STR_COT_FORMAT_MPS" );
		m_StateWind.SetStep( 0.1 );

		// --- Quick actions ----------------------------------------------------
		UIActionCard quickCard = UIActionManager.CreateCard( content, "#STR_COT_WEATHER_MODULE_QUICKACTIONS" );

		//! Straight into the card body, with NO grid spacer of its own.
		//! card_content is already a GridSpacer, and a GridSpacer re-sizes its
		//! children to the cell on every layout pass - so an icon grid nested a
		//! second time had both the exact width and the exact height that
		//! ForceContentWidth set overwritten, which is what made the strip wrap
		//! to two rows and then stretch down the card.
		Widget quickBody = quickCard.GetContent();

		//! Each strip sits in a panel of a FIXED PIXEL HEIGHT, and the icon grid
		//! goes inside that rather than straight into the card.
		//!
		//! ForceContentWidth deliberately leaves the grid root height
		//! fractional - it is written for a horizontal scroller host. card_content
		//! is a GridSpacer with Size To Content V, which resolves a fractional
		//! height against the whole tab, so the card grew to fill the page and
		//! pushed the conditions card off the top. A pixel-height panel gives the
		//! row a real number to size to.
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
	}

	//! Flag the two timing values that are legal but surprising.
	//!
	//! Neither is an error and neither is blocked - a snap change is sometimes
	//! exactly what an admin wants. They are called out because the result does
	//! not look like a setting at all: a zero transition reads as the weather
	//! glitching, and a zero hold reads as the weather ignoring what was just
	//! applied. Both are worth a sentence before the admin goes looking for a
	//! bug that is not there.
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

		//! The engine exposes exactly ONE number here - GetNextChange, the wait
		//! until the weather controller picks its next forecast. How long the
		//! CURRENT weather lasts and how long until the NEXT one are therefore
		//! the same number, so it is printed once, worded as a wait ("in 2m"),
		//! and the second line says what that wait means for the current state
		//! rather than repeating the figure.
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

		SetStateRange( m_StateOvercast, weather.GetOvercast(), 100.0, 100.0 );
		SetStateRange( m_StateRain, weather.GetRain(), 100.0, 100.0 );
		SetStateRange( m_StateSnow, weather.GetSnowfall(), 100.0, 100.0 );
		SetStateRange( m_StateFog, weather.GetFog(), 100.0, 100.0 );
		SetStateRange( m_StateWind, weather.GetWindMagnitude(), 1.0, JMWeatherForm.WIND_SCALE_MAX );

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
		UIActionSliderRange range = UIActionManager.CreateSliderRange( parent, label, 0, 100 );
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
			//! Setting rain and snow to 0 is not enough on its own. The rainy
			//! and snowy actions drop the overcast threshold to 0.5 so their
			//! precipitation starts at once, and a threshold is a standing
			//! rule - left at 0.5 the world starts raining again as soon as
			//! the cloud cover climbs back past half. Clear puts both rules
			//! back to vanilla.
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

		m_Form.LoadEditorValues();
	}

	void UpdateStates()
	{
		m_Form.UpdateActionState( m_ButtonRefresh, JMConstants.PERM_WEATHER_VIEW );
		m_Form.UpdateActionState( m_ButtonApplyAll, JMConstants.PERM_WEATHER_VIEW );
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
