// =============================================================================
//  JMCOTSideBarFooter.c
//
//  The status strip pinned to the bottom of the sidebar. Shows the handful of
//  facts an admin wants visible at all times without opening a module: which
//  server they are on, how many players are connected, the in-game clock, the
//  weather and where it is heading, server performance and the COT build.
//
//  Two layouts, one set of values. The full strip is what the expanded sidebar
//  shows; the compact rail is what survives when the sidebar collapses to its
//  60px icon column, where there is room for icons and short numbers and
//  nothing else. Both are built in sidebar_menu.layout and swapped by Show().
//
//  This is a plain helper, not a widget scriptclass: JMCOTSideBar owns it and
//  hands it the already-created Footer subtree, so there is no second script
//  init to order against the sidebar own.
// =============================================================================

class JMCOTSideBarFooter: COT_ScriptedWidgetEventHandler
{
	//! Height of the footer strip in pixels. Must match the `size 1 130` of the
	//! Footer panel in sidebar_menu.layout - JMCOTSideBar subtracts it from the
	//! screen height to size the module scroller above it.
	static const float HEIGHT = 130;

	//! Seconds between refreshes. Player count, world clock, weather and the
	//! server stats all move slowly enough that once a second looks live, and it
	//! keeps the sidebar per-frame cost to one float add and one compare.
	private static const float REFRESH_INTERVAL = 1.0;

	//! Longest server name rendered before it is cut short with an ellipsis.
	//! The name row is 254px wide at 15pt, which fits roughly this many glyphs.
	private static const int SERVER_NAME_MAX = 34;

	// --- Run-time packing ----------------------------------------------------
	//  Two rows hold text whose width is not known until it is set - the weather
	//  label, and an average that is "39 FPS" uncapped but "39 / 60 FPS" capped.
	//  Laying those out with fixed boxes leaves whatever follows stranded 40px
	//  away on the short spelling, so both rows are packed from the measured
	//  text width instead (TextWidget.GetTextSize reports rendered pixels).

	//! Gap between an icon and the value it labels.
	private static const int PAIR_GAP  = 6;
	//! Gap between one icon+value pair and the next.
	private static const int GROUP_GAP = 12;

	private Widget m_Root;

	//! Anti-cheat alarm. Sits after the player count because it is a fact about
	//! the players, and it is the only thing in the footer that is ever clicked.
	private ButtonWidget m_BtnAntiCheat;
	private ImageWidget  m_IconAntiCheat;
	private Widget       m_AntiCheatHover;

	//! The two mutually exclusive presentations.
	private Widget m_GroupFull;
	private Widget m_GroupCompact;
	private bool m_IsCompact;

	private ImageWidget m_IconTime;
	private ImageWidget m_IconWeather;
	private ImageWidget m_IconNext;
	private ImageWidget m_IconNextArrow;
	private ImageWidget m_IconLow;
	private ImageWidget m_IconHigh;

	private ImageWidget m_IconTimeCompact;
	private ImageWidget m_IconWeatherCompact;
	private ImageWidget m_IconNextCompact;
	private ImageWidget m_IconNextArrowCompact;
	private ImageWidget m_IconLowCompact;
	private ImageWidget m_IconHighCompact;

	private TextWidget m_TextServer;
	private TextWidget m_TextPlayers;
	private TextWidget m_TextTime;
	private TextWidget m_TextWeather;
	private TextWidget m_TextNext;
	private TextWidget m_TextFPS;
	private TextWidget m_TextLow;
	private TextWidget m_TextHigh;
	private TextWidget m_TextVersion;

	private TextWidget m_TextPlayersCompact;
	private TextWidget m_TextTimeCompact;
	private TextWidget m_TextFPSCompact;
	private TextWidget m_TextLowCompact;
	private TextWidget m_TextHighCompact;

	private float m_Elapsed;

	//! Only reloaded when the value actually changes: LoadImageFile re-reads the
	//! texture, and the clock icon is the same for hours at a time.
	private int m_LastTimeOfDay;
	private int m_LastWeatherKind;
	private int m_LastForecastKind;
	private bool m_LastIsNight;

	//! Cached because neither value can change while the client is connected.
	private string m_ServerName;
	private int m_MaxPlayers;

	void JMCOTSideBarFooter()
	{
		//! -1 is "nothing loaded yet", so the first refresh always sets an icon.
		m_LastTimeOfDay = -1;
		m_LastWeatherKind = -1;
		m_LastForecastKind = -1;
	}

	void Init( Widget footer )
	{
		m_Root = footer;

		if ( !m_Root )
			return;

		m_GroupFull    = m_Root.FindAnyWidget( "FooterFull" );
		m_GroupCompact = m_Root.FindAnyWidget( "FooterCompact" );

		Class.CastTo( m_IconTime,       m_Root.FindAnyWidget( "ft_icon_time"       ) );
		Class.CastTo( m_IconWeather,    m_Root.FindAnyWidget( "ft_icon_weather"    ) );
		Class.CastTo( m_IconNext,       m_Root.FindAnyWidget( "ft_icon_next"       ) );
		Class.CastTo( m_IconNextArrow,  m_Root.FindAnyWidget( "ft_icon_next_arrow" ) );
		Class.CastTo( m_IconLow,        m_Root.FindAnyWidget( "ft_icon_low"        ) );
		Class.CastTo( m_IconHigh,       m_Root.FindAnyWidget( "ft_icon_high"       ) );

		Class.CastTo( m_TextServer,  m_Root.FindAnyWidget( "ft_txt_server"  ) );
		Class.CastTo( m_TextPlayers, m_Root.FindAnyWidget( "ft_txt_players" ) );
		Class.CastTo( m_TextTime,    m_Root.FindAnyWidget( "ft_txt_time"    ) );
		Class.CastTo( m_TextWeather, m_Root.FindAnyWidget( "ft_txt_weather" ) );
		Class.CastTo( m_TextNext,    m_Root.FindAnyWidget( "ft_txt_next"    ) );
		Class.CastTo( m_TextFPS,     m_Root.FindAnyWidget( "ft_txt_fps"     ) );
		Class.CastTo( m_TextLow,     m_Root.FindAnyWidget( "ft_txt_low"     ) );
		Class.CastTo( m_TextHigh,    m_Root.FindAnyWidget( "ft_txt_high"    ) );
		Class.CastTo( m_TextVersion, m_Root.FindAnyWidget( "ft_txt_version" ) );

		Class.CastTo( m_IconTimeCompact,       m_Root.FindAnyWidget( "fc_icon_time"       ) );
		Class.CastTo( m_IconWeatherCompact,    m_Root.FindAnyWidget( "fc_icon_weather"    ) );
		Class.CastTo( m_IconNextCompact,       m_Root.FindAnyWidget( "fc_icon_next"       ) );
		Class.CastTo( m_IconNextArrowCompact,  m_Root.FindAnyWidget( "fc_icon_next_arrow" ) );
		Class.CastTo( m_IconLowCompact,        m_Root.FindAnyWidget( "fc_icon_low"        ) );
		Class.CastTo( m_IconHighCompact,       m_Root.FindAnyWidget( "fc_icon_high"       ) );

		Class.CastTo( m_BtnAntiCheat,   m_Root.FindAnyWidget( "ft_btn_anticheat"   ) );
		Class.CastTo( m_IconAntiCheat,  m_Root.FindAnyWidget( "ft_icon_anticheat"  ) );
		m_AntiCheatHover = m_Root.FindAnyWidget( "ft_anticheat_hover" );

		if ( m_BtnAntiCheat )
			m_BtnAntiCheat.SetHandler( this );

		if ( m_IconAntiCheat )
			m_IconAntiCheat.LoadImageFile( 0, JMConstants.Lucide( "shield-alert" ) );

		Class.CastTo( m_TextPlayersCompact, m_Root.FindAnyWidget( "fc_txt_players" ) );
		Class.CastTo( m_TextTimeCompact,    m_Root.FindAnyWidget( "fc_txt_time"    ) );
		Class.CastTo( m_TextFPSCompact,     m_Root.FindAnyWidget( "fc_txt_fps"     ) );
		Class.CastTo( m_TextLowCompact,     m_Root.FindAnyWidget( "fc_txt_low"     ) );
		Class.CastTo( m_TextHighCompact,    m_Root.FindAnyWidget( "fc_txt_high"    ) );

		//! Icons that never change. Clock, weather and the forecast are set by
		//! Refresh instead, because all three depend on world state.
		SetIcon( "ft_icon_server",  JMConstants.Lucide( "server"        ) );
		SetIcon( "ft_icon_players", JMConstants.Lucide( "users"         ) );
		SetIcon( "ft_icon_fps",     JMConstants.Lucide( "gauge"         ) );
		SetIcon( "ft_icon_low",     JMConstants.Lucide( "trending-down" ) );
		SetIcon( "ft_icon_high",    JMConstants.Lucide( "trending-up"   ) );

		SetIcon( "fc_icon_players", JMConstants.Lucide( "users"         ) );
		SetIcon( "fc_icon_fps",     JMConstants.Lucide( "gauge"         ) );
		SetIcon( "fc_icon_low",     JMConstants.Lucide( "trending-down" ) );
		SetIcon( "fc_icon_high",    JMConstants.Lucide( "trending-up"   ) );

		if ( m_IconNextArrow )
			m_IconNextArrow.LoadImageFile( 0, JMConstants.Lucide( "move-right" ) );

		if ( m_IconNextArrowCompact )
			m_IconNextArrowCompact.LoadImageFile( 0, JMConstants.Lucide( "move-right" ) );

		CacheStaticInfo();

		if ( m_TextServer )
			m_TextServer.SetText( m_ServerName );

		if ( m_TextVersion )
			m_TextVersion.SetText( JMBuildInfo.GetFooterText() );

		SetCompact( false );

		//! Fill the live fields immediately so the first frame is never blank.
		Refresh();
	}

	private void SetIcon( string widgetName, string imagePath )
	{
		ImageWidget icon;
		if ( !Class.CastTo( icon, m_Root.FindAnyWidget( widgetName ) ) )
			return;

		icon.LoadImageFile( 0, imagePath );
	}

	// -------------------------------------------------------------------------
	//  Collapsed / expanded
	//
	//  A collapsed sidebar leaves only its leftmost 60px on screen. The full
	//  strip is not narrowed to fit that - it is replaced, because a 254px
	//  server name and a weather word have nothing useful to say in 60px.
	// -------------------------------------------------------------------------
	void SetCompact( bool compact )
	{
		m_IsCompact = compact;

		if ( m_GroupFull )
			m_GroupFull.Show( !compact );

		if ( m_GroupCompact )
			m_GroupCompact.Show( compact );

		//! The hidden half stopped being updated while it was hidden, so bring
		//! whichever half just appeared back in step immediately.
		m_LastTimeOfDay = -1;
		m_LastWeatherKind = -1;
		m_LastForecastKind = -1;

		Refresh();
	}

	// -------------------------------------------------------------------------
	//  Driven from JMCOTSideBar.OnUpdate, which only calls in while the sidebar
	//  is on screen.
	// -------------------------------------------------------------------------
	void OnUpdate( float timeslice )
	{
		m_Elapsed += timeslice;

		if ( m_Elapsed < REFRESH_INTERVAL )
			return;

		m_Elapsed = 0;

		Refresh();
	}

	private void Refresh()
	{
		int hour, minute;
		GetWorldClock( hour, minute );

		bool isNight = JMWorldConditions.IsNight( hour );

		UpdateTime( hour, minute );
		UpdateWeather( isNight );
		UpdatePlayers();
		UpdateServerStats();
	}

	// -------------------------------------------------------------------------
	//  Clock
	// -------------------------------------------------------------------------
	private void UpdateTime( int hour, int minute )
	{
		string clock = Pad2( hour ) + ":" + Pad2( minute );

		if ( m_TextTime )
			m_TextTime.SetText( clock );

		if ( m_TextTimeCompact )
			m_TextTimeCompact.SetText( clock );

		int tod = JMWorldConditions.GetTimeOfDay( hour );

		if ( tod == m_LastTimeOfDay )
			return;

		m_LastTimeOfDay = tod;

		string icon = JMWorldConditions.GetTimeIcon( tod );

		if ( m_IconTime )
			m_IconTime.LoadImageFile( 0, icon );

		if ( m_IconTimeCompact )
			m_IconTimeCompact.LoadImageFile( 0, icon );
	}

	//! Returns 0:00 before the world exists - the first refresh runs from Init,
	//! which the sidebar calls while the mission is still coming up.
	private void GetWorldClock( out int hour, out int minute )
	{
		hour = 0;
		minute = 0;

		World world = g_Game.GetWorld();
		if ( !world )
			return;

		int year, month, day;
		world.GetDate( year, month, day, hour, minute );
	}

	// -------------------------------------------------------------------------
	//  Weather
	// -------------------------------------------------------------------------
	private void UpdateWeather( bool isNight )
	{
		int kind = JMWorldConditions.GetWeatherKind();
		int forecast = JMWorldConditions.GetForecastWeatherKind();

		//! Night only matters to the icon, but it changes which icon a given
		//! kind maps to, so it has to be part of the "did anything change" test.
		bool changed = ( kind != m_LastWeatherKind ) || ( forecast != m_LastForecastKind ) || ( isNight != m_LastIsNight );

		if ( changed )
		{
			m_LastWeatherKind = kind;
			m_LastForecastKind = forecast;
			m_LastIsNight = isNight;

			if ( m_TextWeather )
				m_TextWeather.SetText( Widget.TranslateString( JMWorldConditions.GetWeatherLabel( kind ) ) );

			string icon = JMWorldConditions.GetWeatherIcon( kind, isNight );

			if ( m_IconWeather )
				m_IconWeather.LoadImageFile( 0, icon );

			if ( m_IconWeatherCompact )
				m_IconWeatherCompact.LoadImageFile( 0, icon );
		}

		//! Deliberately outside the guard above: the tail counts down, so it has
		//! something new to say on every refresh even when the weather kind and
		//! the forecast are both sitting still.
		UpdateForecastIndicator( kind, forecast, isNight );
	}

	//! Arrow, icon, then label - the reading order of a transition.
	//!
	//! Two things can follow the arrow. When the forecast is heading for a
	//! different kind of weather, it is that kind: icon and name. When it is
	//! not - which is most of the time, since a forecast that drifts from
	//! overcast 0.4 to 0.55 never leaves "cloudy" - naming the same weather
	//! twice says nothing, so the slot carries the wait until the weather
	//! controller picks its next forecast instead. Either way the row answers
	//! "and then what", which is the only reason it exists.
	//!
	//! The collapsed rail has no text at all, so it shows the arrow only for a
	//! real change of kind; a bare arrow pointing at nothing is worse than no
	//! arrow.
	private void UpdateForecastIndicator( int kind, int forecast, bool isNight )
	{
		bool changing = ( forecast != kind );

		string icon = "";
		string label = "";

		if ( changing )
		{
			icon = JMWorldConditions.GetWeatherIcon( forecast, isNight );
			label = Widget.TranslateString( JMWorldConditions.GetWeatherLabel( forecast ) );
		}
		else
		{
			label = JMWorldConditions.FormatSecondsToChange( JMWorldConditions.GetSecondsToWeatherChange() );
		}

		bool show = ( label != "" );

		//! Nothing to draw means the client is holding neither a forecast that
		//! differs from the current weather nor a next-change time. Whether that
		//! is an idle weather system or weather data that never reached this
		//! client is not something the widgets can show, so a diag build says it.
		#ifdef JM_COT_DIAG_LOGGING
		if ( !show )
			Print( "[COT] footer forecast idle - kind " + kind + ", forecast " + forecast + ", next change " + JMWorldConditions.GetSecondsToWeatherChange() + "s" );
		#endif

		ShowForecastWidget( m_IconNextArrow, show );
		ShowForecastWidget( m_IconNextArrowCompact, changing );

		if ( m_TextNext )
		{
			m_TextNext.Show( show );

			if ( show )
				m_TextNext.SetText( label );
		}

		LoadForecastIcon( m_IconNext, icon, changing );
		LoadForecastIcon( m_IconNextCompact, icon, changing );

		if ( show )
			PackWeatherRow();
	}

	private void ShowForecastWidget( Widget widget, bool changing )
	{
		if ( !widget )
			return;

		widget.Show( changing );
	}

	private void LoadForecastIcon( ImageWidget widget, string icon, bool changing )
	{
		if ( !widget )
			return;

		widget.Show( changing );

		if ( !changing )
			return;

		widget.LoadImageFile( 0, icon );
	}

	// -------------------------------------------------------------------------
	//  Run-time packing
	//
	//  Both of these rows sit behind text whose rendered width is not knowable
	//  from the layout: a weather label is "Fog" or "Overcast", an average is
	//  "39 FPS" or "39 / 60 FPS". Fixed boxes sized for the longest spelling
	//  leave the rest of the row stranded on the short one, so what follows is
	//  positioned from the measured width instead.
	// -------------------------------------------------------------------------
	private void PackWeatherRow()
	{
		if ( !m_TextWeather || !m_IconNextArrow || !m_IconNext || !m_TextNext )
			return;

		float x, y;
		if ( !GetRunEnd( m_TextWeather, x, y ) )
			return;

		x = PlaceIcon( m_IconNextArrow, x, y, m_TextWeather ) + PAIR_GAP;

		//! The countdown has no icon of its own, so its slot is closed up rather
		//! than left as a gap the arrow points across.
		if ( m_IconNext.IsVisible() )
			x = PlaceIcon( m_IconNext, x, y, m_TextWeather ) + PAIR_GAP;

		m_TextNext.SetPos( x, y );
	}

	//! The two tails follow the average on the same rhythm the gauge icon and
	//! the average itself use, rather than sitting at fixed columns with a gap
	//! wide enough to read as a separate row.
	private void PackPerformanceRow()
	{
		if ( !m_TextFPS || !m_IconLow || !m_TextLow || !m_IconHigh || !m_TextHigh )
			return;

		float x, y;
		if ( !GetRunEnd( m_TextFPS, x, y ) )
			return;

		x = PlaceIcon( m_IconLow, x, y, m_TextFPS ) + PAIR_GAP;
		m_TextLow.SetPos( x, y );

		//! Continue from the x just written rather than reading it back: the low
		//! value was placed a line ago and GetPos would only confirm what is
		//! already known, at the cost of depending on SetPos having flushed.
		int lowWidth, lowHeight;
		m_TextLow.GetTextSize( lowWidth, lowHeight );

		if ( lowWidth <= 0 )
			return;

		x += lowWidth + GROUP_GAP;

		x = PlaceIcon( m_IconHigh, x, y, m_TextFPS ) + PAIR_GAP;
		m_TextHigh.SetPos( x, y );
	}

	// -------------------------------------------------------------------------
	//  Packing primitives
	//
	//  Everything here reads the ANCHOR widget position rather than working from
	//  the row coordinates in the layout file. Those two do not agree: laying the
	//  forecast and the tails out from the layout `position` values put both
	//  groups exactly one row above the text they belong to. GetPos and SetPos
	//  are symmetric, so measuring and placing in the same frame is correct
	//  whatever that frame turns out to be.
	// -------------------------------------------------------------------------

	//! Position just past the end of a widget rendered text, and the row y to
	//! place at. False when the widget has not been laid out yet - GetTextSize
	//! reports 0 until the first layout pass, and packing against that would
	//! stack the whole row on the left margin.
	private bool GetRunEnd( TextWidget anchor, out float x, out float y )
	{
		x = 0;
		y = 0;

		int textWidth, textHeight;
		anchor.GetTextSize( textWidth, textHeight );

		if ( textWidth <= 0 )
			return false;

		float anchorX, anchorY;
		anchor.GetPos( anchorX, anchorY );

		x = anchorX + textWidth + GROUP_GAP;
		y = anchorY;

		return true;
	}

	//! Drop an icon at x on the row owned by `row`, vertically centred against
	//! it, and return the x just past the icon.
	private float PlaceIcon( ImageWidget icon, float x, float y, TextWidget row )
	{
		float iconW, iconH;
		icon.GetSize( iconW, iconH );

		float rowW, rowH;
		row.GetSize( rowW, rowH );

		icon.SetPos( x, y + ( ( rowH - iconH ) / 2 ) );

		//! Just past the icon - the caller adds whichever gap it wants next.
		return x + iconW;
	}

	// -------------------------------------------------------------------------
	//  Players
	// -------------------------------------------------------------------------

	//! COT keeps its own synchronised roster, so this is the same number the
	//! Players module lists rather than a guess from the local entity list.
	private void UpdatePlayers()
	{
		int online = 0;

		JMPermissionManager permissions = GetPermissionsManager();
		if ( permissions )
			online = permissions.GetPlayers().Count();

		//! The collapsed rail has room for the count and nothing else; the slot
		//! total only appears where it fits.
		if ( m_TextPlayersCompact )
			m_TextPlayersCompact.SetText( "" + online );

		if ( !m_TextPlayers )
			return;

		if ( m_MaxPlayers > 0 )
			m_TextPlayers.SetText( "" + online + " / " + m_MaxPlayers );
		else
			m_TextPlayers.SetText( "" + online );

		UpdateAntiCheatBadge();
	}

	// -------------------------------------------------------------------------
	//  Anti-cheat alarm
	//
	//  Hidden unless the server has someone flagged at or above the anti-cheat
	//  module's own warning score, which is also the only state worth a badge:
	//  a footer that is red all the time is a footer nobody reads. The count
	//  arrives with the server-stats broadcast, so it is live whether or not the
	//  AntiCheat form has ever been opened, and it is 0 for admins without the
	//  permission to be told.
	// -------------------------------------------------------------------------
	private void UpdateAntiCheatBadge()
	{
		if ( !m_BtnAntiCheat )
			return;

		bool alarm = JMAntiCheatStatus.HasFlags();

		m_BtnAntiCheat.Show( alarm );

		if ( !alarm )
		{
			if ( m_AntiCheatHover )
				m_AntiCheatHover.Show( false );

			return;
		}

		PackPlayersRow();
	}

	//! The badge follows the player count rather than sitting at a fixed column,
	//! for the same reason the weather and performance tails do: "3 / 60" and
	//! "12" are not the same width.
	private void PackPlayersRow()
	{
		if ( !m_TextPlayers || !m_BtnAntiCheat )
			return;

		float x, y;
		if ( !GetRunEnd( m_TextPlayers, x, y ) )
			return;

		float rowW, rowH;
		m_TextPlayers.GetSize( rowW, rowH );

		float btnW, btnH;
		m_BtnAntiCheat.GetSize( btnW, btnH );

		m_BtnAntiCheat.SetPos( x, y + ( ( rowH - btnH ) / 2 ) );
	}

	// -------------------------------------------------------------------------
	//  Input
	// -------------------------------------------------------------------------

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w != m_BtnAntiCheat )
			return false;

		OpenAntiCheatModule();
		return true;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_BtnAntiCheat && m_AntiCheatHover )
			m_AntiCheatHover.Show( true );

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_BtnAntiCheat && m_AntiCheatHover )
			m_AntiCheatHover.Show( false );

		return false;
	}

	//! Found by class name rather than by type: the module is a 5_Mission class
	//! and this footer is 4_World, so the type is not nameable from here. The
	//! module manager list is short and this runs once per click.
	private void OpenAntiCheatModule()
	{
		JMModuleManager manager = GetModuleManager();
		if ( !manager )
			return;

		array< JMRenderableModuleBase > modules = manager.GetCOTModules();
		if ( !modules )
			return;

		foreach ( JMRenderableModuleBase module: modules )
		{
			if ( !module )
				continue;

			if ( module.ClassName() != "JMAntiCheatModule" )
				continue;

			module.ToggleShow();
			return;
		}
	}

	// -------------------------------------------------------------------------
	//  Server performance
	//
	//  These are the SERVER numbers, sampled by JMServerStatsModule on the host
	//  and broadcast to admin clients - not this client render rate, which says
	//  nothing about whether the server is keeping up.
	//
	//  The one-percent low is the number worth watching: a server holding a
	//  healthy average while one tick in a hundred takes 200ms is the server
	//  whose players report rubber-banding, and only the low shows it.
	// -------------------------------------------------------------------------
	private void UpdateServerStats()
	{
		bool valid = JMServerStats.IsValid();

		int colorAverage = JMServerStats.ColorForAverage();
		//! Judged against the average rather than on their own: the tails only
		//! colour once they pull far enough away from it to be a stutter.
		int colorLow     = JMServerStats.ColorForSpread( JMServerStats.OnePercentLow );
		int colorHigh    = JMServerStats.ColorForSpread( JMServerStats.OnePercentHigh );

		//! Nothing has arrived yet (or this client cannot receive it) - do not
		//! paint "--" red as though the server were on fire.
		if ( !valid )
		{
			colorAverage = JMTheme.TEXT_MUTED;
			colorLow     = JMTheme.TEXT_MUTED;
			colorHigh    = JMTheme.TEXT_MUTED;
		}

		string average = JMServerStats.FormatAverageAgainstCap();
		string low     = JMServerStats.Format( JMServerStats.OnePercentLow );
		string high    = JMServerStats.Format( JMServerStats.OnePercentHigh );

		SetValue( m_TextFPS,  average + " FPS", colorAverage );
		SetValue( m_TextLow,  low,              colorLow  );
		SetValue( m_TextHigh, high,             colorHigh );

		//! The collapsed rail drops the unit and the cap - three characters of
		//! run-up is all it has - and leans on the icon colours instead.
		SetValue( m_TextFPSCompact,  JMServerStats.Format( JMServerStats.Average ), colorAverage );
		SetValue( m_TextLowCompact,  low,  colorLow  );
		SetValue( m_TextHighCompact, high, colorHigh );

		//! The tail icons are tinted with their own value, so the rail reads at a
		//! glance without the numbers having to be legible.
		SetIconColor( m_IconLow,         colorLow  );
		SetIconColor( m_IconHigh,        colorHigh );
		SetIconColor( m_IconLowCompact,  colorLow  );
		SetIconColor( m_IconHighCompact, colorHigh );

		PackPerformanceRow();
	}

	private void SetValue( TextWidget widget, string value, int color )
	{
		if ( !widget )
			return;

		widget.SetText( value );
		widget.SetColor( color );
	}

	private void SetIconColor( ImageWidget widget, int color )
	{
		if ( !widget )
			return;

		widget.SetColor( color );
	}

	// -------------------------------------------------------------------------
	//  Values that cannot change for the lifetime of the connection.
	// -------------------------------------------------------------------------
	private void CacheStaticInfo()
	{
		m_ServerName = "";
		m_MaxPlayers = 0;

		string hostName = g_Game.GetHostName();
		if ( hostName != "" )
			m_ServerName = Truncate( hostName, SERVER_NAME_MAX );

		//! Only populated when the client joined through the server browser -
		//! a direct connect or a local test session leaves it null.
		GetServersResultRow hostData = g_Game.GetHostData();
		if ( hostData )
		{
			m_MaxPlayers = hostData.m_MaxPlayers;

			if ( m_ServerName == "" && hostData.m_Name != "" )
				m_ServerName = Truncate( hostData.m_Name, SERVER_NAME_MAX );
		}

		//! TextWidget.SetText does not resolve stringtable keys the way the
		//! `text` field of a layout does, so the lookup has to be explicit here.
		if ( m_ServerName == "" )
			m_ServerName = Widget.TranslateString( "#STR_COT_SIDEBAR_FOOTER_LOCAL" );
	}

	// -------------------------------------------------------------------------
	//  Formatting
	// -------------------------------------------------------------------------
	private string Pad2( int value )
	{
		if ( value < 10 )
			return "0" + value;

		return "" + value;
	}

	private string Truncate( string text, int maxLength )
	{
		if ( text.Length() <= maxLength )
			return text;

		return text.Substring( 0, maxLength - 1 ) + "...";
	}
}
