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
	//! Height of the footer strip in pixels. Must match the `size 1 152` of the
	//! Footer panel in sidebar_menu.layout - JMCOTSideBar subtracts it from the
	//! screen height to size the module scroller above it.
	static const float HEIGHT = 152;

	//! Seconds between refreshes. Player count, world clock, weather and the
	//! server stats all move slowly enough that once a second looks live, and it
	//! keeps the sidebar per-frame cost to one float add and one compare.
	protected static const float REFRESH_INTERVAL = 1.0;

	//! Longest server name rendered before it is cut short with an ellipsis.
	//! The name row is 254px wide at 15pt, which fits roughly this many glyphs.
	protected static const int SERVER_NAME_MAX = 34;

	// --- Run-time packing ----------------------------------------------------
	//  Two rows hold text whose width is not known until it is set - the weather
	//  label, and an average that is "39 FPS" uncapped but "39 / 60 FPS" capped.
	//  Laying those out with fixed boxes leaves whatever follows stranded 40px
	//  away on the short spelling, so both rows are packed from the measured
	//  text width instead (TextWidget.GetTextSize reports rendered pixels).

	//! Gap between an icon and the value it labels.
	protected static const int PAIR_GAP  = 6;
	//! Gap between one icon+value pair and the next.
	protected static const int GROUP_GAP = 12;
	protected Widget m_Root;

	//! Anti-cheat alarm. Sits after the player count because it is a fact about
	//! the players, and it is the only thing in the footer that is ever clicked.
	protected ButtonWidget m_BtnAntiCheat;
	protected ImageWidget  m_IconAntiCheat;
	protected Widget       m_AntiCheatHover;

	//! Local admin position, click to copy. Full sidebar only - a collapsed
	//! sidebar drops the row entirely rather than shrinking it to an icon;
	//! there is no compact widget pair to fall back to. Hover/click only
	//! ever land on the button (icon + coordinate text) - the cardinal
	//! reading beside it is informational and sits outside the button.
	//!
	//! Click feedback fades the icon out, swaps it to a check mark, fades it
	//! back in, holds, then reverses - the same animated shape
	//! UIActionFeedbackButton uses for the GUID/Steam64 copy buttons - rather
	//! than COTFeedback.Copy's toast, which this bypasses entirely.
	//!
	//! The row shows whole-metre coordinates, but the clipboard gets the full
	//! precision reading - a copy-paste into a teleport field should not lose
	//! the fractional metre the display rounds away.
	protected ButtonWidget m_BtnPosition;
	protected TextWidget   m_TextPosition;
	protected ImageWidget  m_IconPosition;
	protected ImageWidget  m_IconCardinal;
	protected TextWidget   m_TextCardinal;

	//! Left inset of ft_txt_position inside ft_btn_position, and the extra
	//! trailing pad past the measured text - both baked into the layout's
	//! own numbers, kept here so PackPositionRow can hug the button to the
	//! text without re-deriving them.
	protected static const float POSITION_TEXT_LEFT    = 22;
	protected static const float POSITION_TRAILING_PAD = 6;
	protected static const float POSITION_ROW_HEIGHT   = 20;

	//! Feedback icon fade phases, matching UIActionFeedbackButton's own.
	protected static const int FEEDBACK_PHASE_IDLE        = 0;
	protected static const int FEEDBACK_PHASE_OUT_TO_FEED = 1;
	protected static const int FEEDBACK_PHASE_IN_FEED     = 2;
	protected static const int FEEDBACK_PHASE_HOLD        = 3;
	protected static const int FEEDBACK_PHASE_OUT_TO_REST = 4;
	protected static const int FEEDBACK_PHASE_IN_REST     = 5;

	protected static const float FEEDBACK_FADE_SECONDS = 0.10;
	protected static const float FEEDBACK_HOLD_SECONDS = 1.40;

	protected int   m_PositionFeedbackPhase;
	protected float m_PositionFeedbackTimer;

	//! Hover indication is the icon's own colour fading toward the theme
	//! accent and back, rather than a translucent white wash behind it - a
	//! wash never reads as "clickable" against this footer's own dark fill,
	//! it just looks like a rendering glitch.
	protected static const float HOVER_FADE_SECONDS = 0.12;
	protected static const int   POSITION_ICON_REST_COLOR  = 0xFF8F999F;
	protected bool  m_PositionHoverTarget;
	protected float m_PositionHoverAlpha;

	//! The two mutually exclusive presentations.
	protected Widget m_GroupFull;
	protected Widget m_GroupCompact;
	protected bool m_IsCompact;
	protected ImageWidget m_IconTime;
	protected ImageWidget m_IconWeather;
	protected ImageWidget m_IconNext;
	protected ImageWidget m_IconNextArrow;
	protected ImageWidget m_IconLow;
	protected ImageWidget m_IconHigh;
	protected ImageWidget m_IconTimeCompact;
	protected ImageWidget m_IconWeatherCompact;
	protected ImageWidget m_IconNextCompact;
	protected ImageWidget m_IconNextArrowCompact;
	protected ImageWidget m_IconLowCompact;
	protected ImageWidget m_IconHighCompact;
	protected TextWidget m_TextServer;
	protected TextWidget m_TextPlayers;
	protected TextWidget m_TextTime;
	protected TextWidget m_TextWeather;
	protected TextWidget m_TextNext;
	protected TextWidget m_TextFPS;
	protected TextWidget m_TextLow;
	protected TextWidget m_TextHigh;
	protected TextWidget m_TextVersion;
	protected TextWidget m_TextPlayersCompact;
	protected TextWidget m_TextTimeCompact;
	protected TextWidget m_TextFPSCompact;
	protected TextWidget m_TextLowCompact;
	protected TextWidget m_TextHighCompact;
	protected float m_Elapsed;

	//! Only reloaded when the value actually changes: LoadImageFile re-reads the
	//! texture, and the clock icon is the same for hours at a time.
	protected int m_LastTimeOfDay;
	protected int m_LastWeatherKind;
	protected int m_LastForecastKind;
	protected bool m_LastIsNight;

	//! Cached because neither value can change while the client is connected.
	protected string m_ServerName;
	protected int m_MaxPlayers;

	void JMCOTSideBarFooter()
	{
		//! -1 is "nothing loaded yet", so the first refresh always sets an icon.
		m_LastTimeOfDay = -1;
		m_LastWeatherKind = -1;
		m_LastForecastKind = -1;
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
	protected bool GetRunEnd( TextWidget anchor, out float x, out float y )
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

	//! Returns 0:00 before the world exists - the first refresh runs from Init,
	//! which the sidebar calls while the mission is still coming up.
	protected void GetWorldClock( out int hour, out int minute )
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

	protected void SetIcon( string widgetName, string imagePath )
	{
		ImageWidget icon;
		if ( !Class.CastTo( icon, m_Root.FindAnyWidget( widgetName ) ) )
			return;

		icon.LoadImageFile( 0, imagePath );
	}

	protected void SetIconColor( ImageWidget widget, int color )
	{
		if ( !widget )
			return;

		widget.SetColor( color );
	}

	protected void SetValue( TextWidget widget, string value, int color )
	{
		if ( !widget )
			return;

		widget.SetText( value );
		widget.SetColor( color );
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

		Class.CastTo( m_BtnPosition,   m_Root.FindAnyWidget( "ft_btn_position"   ) );
		Class.CastTo( m_TextPosition,  m_Root.FindAnyWidget( "ft_txt_position"  ) );
		Class.CastTo( m_IconPosition,  m_Root.FindAnyWidget( "ft_icon_position" ) );
		Class.CastTo( m_IconCardinal,  m_Root.FindAnyWidget( "ft_icon_cardinal" ) );
		Class.CastTo( m_TextCardinal,  m_Root.FindAnyWidget( "ft_txt_cardinal"  ) );

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

		if ( m_BtnPosition )
			m_BtnPosition.SetHandler( this );

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

		if ( m_IconPosition )
			m_IconPosition.LoadImageFile( 0, JMConstants.Lucide( "map-pin" ) );

		SetIcon( "ft_icon_cardinal", JMConstants.Lucide( "compass" ) );

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

	// -------------------------------------------------------------------------
	//  Driven from JMCOTSideBar.OnUpdate, which only calls in while the sidebar
	//  is on screen.
	// -------------------------------------------------------------------------
	void OnUpdate( float timeslice )
	{
		//! Every frame, not throttled by REFRESH_INTERVAL below - a fade that
		//! only advanced once a second would not read as a fade.
		UpdatePositionFeedback( timeslice );
		UpdatePositionHover( timeslice );

		m_Elapsed += timeslice;

		if ( m_Elapsed < REFRESH_INTERVAL )
			return;

		m_Elapsed = 0;

		Refresh();
	}

	protected void Refresh()
	{
		int hour, minute;
		GetWorldClock( hour, minute );

		bool isNight = JMWorldConditions.IsNight( hour );

		UpdateTime( hour, minute );
		UpdateWeather( isNight );
		UpdatePlayers();
		UpdateServerStats();
		UpdatePosition();
	}

	// -------------------------------------------------------------------------
	//  Clock
	// -------------------------------------------------------------------------
	protected void UpdateTime( int hour, int minute )
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

	// -------------------------------------------------------------------------
	//  Weather
	// -------------------------------------------------------------------------
	protected void UpdateWeather( bool isNight )
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
				m_TextWeather.SetText( JMWorldConditions.GetWeatherLabel( kind ) );

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
	protected void UpdateForecastIndicator( int kind, int forecast, bool isNight )
	{
		bool changing = ( forecast != kind );

		string icon = "";
		string label = "";

		if ( changing )
		{
			icon = JMWorldConditions.GetWeatherIcon( forecast, isNight );
			label = JMWorldConditions.GetWeatherLabel( forecast );
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

	protected void ShowForecastWidget( Widget widget, bool changing )
	{
		if ( !widget )
			return;

		widget.Show( changing );
	}

	protected void LoadForecastIcon( ImageWidget widget, string icon, bool changing )
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
	protected void PackWeatherRow()
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
	protected void PackPerformanceRow()
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

	//! Drop an icon at x on the row owned by `row`, vertically centred against
	//! it, and return the x just past the icon.
	protected float PlaceIcon( ImageWidget icon, float x, float y, TextWidget row )
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
	protected void UpdatePlayers()
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
	protected void UpdateAntiCheatBadge()
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
	protected void PackPlayersRow()
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
		if ( w == m_BtnAntiCheat )
		{
			OpenAntiCheatModule();
			return true;
		}

		if ( w == m_BtnPosition )
		{
			g_Game.CopyToClipboard( m_LastPositionCopyText );
			ShowPositionCopiedFeedback();
			return true;
		}

		return false;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_BtnAntiCheat && m_AntiCheatHover )
			m_AntiCheatHover.Show( true );

		if ( w == m_BtnPosition )
			m_PositionHoverTarget = true;

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_BtnAntiCheat && m_AntiCheatHover )
			m_AntiCheatHover.Show( false );

		if ( w == m_BtnPosition )
			m_PositionHoverTarget = false;

		return false;
	}

	//! Fades the icon's own tint toward JMTheme.ACCENT and back, ticked every
	//! frame from OnUpdate. Skipped while the click feedback animation owns
	//! the icon (texture swap + its own alpha fade) - the two would otherwise
	//! fight over the same widget; the hover tint simply resumes once that
	//! finishes and picks up wherever the mouse actually is by then.
	protected void UpdatePositionHover( float timeslice )
	{
		if ( !m_IconPosition || m_PositionFeedbackPhase != FEEDBACK_PHASE_IDLE )
			return;

		float target = 0.0;
		if ( m_PositionHoverTarget )
			target = 1.0;

		if ( m_PositionHoverAlpha == target )
			return;

		float step = timeslice / HOVER_FADE_SECONDS;

		if ( m_PositionHoverAlpha < target )
		{
			m_PositionHoverAlpha += step;
			if ( m_PositionHoverAlpha > target )
				m_PositionHoverAlpha = target;
		}
		else
		{
			m_PositionHoverAlpha -= step;
			if ( m_PositionHoverAlpha < target )
				m_PositionHoverAlpha = target;
		}

		m_IconPosition.SetColor( LerpColor( POSITION_ICON_REST_COLOR, JMTheme.ACCENT, m_PositionHoverAlpha ) );
	}

	//! Per-channel ARGB blend. Enforce has no built-in colour lerp.
	protected int LerpColor( int from, int to, float t )
	{
		int fa = ( from >> 24 ) & 0xFF;
		int fr = ( from >> 16 ) & 0xFF;
		int fg = ( from >> 8  ) & 0xFF;
		int fb = from & 0xFF;

		int ta = ( to >> 24 ) & 0xFF;
		int tr = ( to >> 16 ) & 0xFF;
		int tg = ( to >> 8  ) & 0xFF;
		int tb = to & 0xFF;

		int a = fa + (int) ( ( ta - fa ) * t );
		int r = fr + (int) ( ( tr - fr ) * t );
		int g = fg + (int) ( ( tg - fg ) * t );
		int b = fb + (int) ( ( tb - fb ) * t );

		return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
	}

	//! Same animated shape UIActionFeedbackButton uses for the GUID/Steam64
	//! copy buttons: the icon fades out, swaps to a check mark while
	//! invisible, fades back in, holds, then reverses - no separate toast.
	//! COTFeedback.Copy exists for controls that cannot show their own
	//! state; this one can, so it bypasses it (see COTFeedback.c's header).
	//! Ticked from OnUpdate every frame - see UpdatePositionFeedback.
	protected void ShowPositionCopiedFeedback()
	{
		if ( !m_IconPosition )
			return;

		m_PositionFeedbackPhase = FEEDBACK_PHASE_OUT_TO_FEED;
		m_PositionFeedbackTimer = 0;
	}

	protected void UpdatePositionFeedback( float timeslice )
	{
		if ( m_PositionFeedbackPhase == FEEDBACK_PHASE_IDLE || !m_IconPosition )
			return;

		m_PositionFeedbackTimer += timeslice;

		float t = m_PositionFeedbackTimer / FEEDBACK_FADE_SECONDS;
		if ( t > 1.0 )
			t = 1.0;

		if ( m_PositionFeedbackPhase == FEEDBACK_PHASE_OUT_TO_FEED )
		{
			m_IconPosition.SetAlpha( 1.0 - t );

			if ( t >= 1.0 )
			{
				m_IconPosition.LoadImageFile( 0, JMConstants.ICON_CHECK_MARK );
				m_PositionFeedbackPhase = FEEDBACK_PHASE_IN_FEED;
				m_PositionFeedbackTimer = 0;
			}

			return;
		}

		if ( m_PositionFeedbackPhase == FEEDBACK_PHASE_IN_FEED )
		{
			m_IconPosition.SetAlpha( t );

			if ( t >= 1.0 )
			{
				m_PositionFeedbackPhase = FEEDBACK_PHASE_HOLD;
				m_PositionFeedbackTimer = 0;
			}

			return;
		}

		if ( m_PositionFeedbackPhase == FEEDBACK_PHASE_HOLD )
		{
			if ( m_PositionFeedbackTimer >= FEEDBACK_HOLD_SECONDS )
			{
				m_PositionFeedbackPhase = FEEDBACK_PHASE_OUT_TO_REST;
				m_PositionFeedbackTimer = 0;
			}

			return;
		}

		if ( m_PositionFeedbackPhase == FEEDBACK_PHASE_OUT_TO_REST )
		{
			m_IconPosition.SetAlpha( 1.0 - t );

			if ( t >= 1.0 )
			{
				m_IconPosition.LoadImageFile( 0, JMConstants.Lucide( "map-pin" ) );
				m_PositionFeedbackPhase = FEEDBACK_PHASE_IN_REST;
				m_PositionFeedbackTimer = 0;
			}

			return;
		}

		if ( m_PositionFeedbackPhase == FEEDBACK_PHASE_IN_REST )
		{
			m_IconPosition.SetAlpha( t );

			if ( t >= 1.0 )
			{
				m_PositionFeedbackPhase = FEEDBACK_PHASE_IDLE;
				m_PositionFeedbackTimer = 0;
			}
		}
	}

	//! Found by class name rather than by type: the module is a 5_Mission class
	//! and this footer is 4_World, so the type is not nameable from here. The
	//! module manager list is short and this runs once per click.
	protected void OpenAntiCheatModule()
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
	protected void UpdateServerStats()
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

	// -------------------------------------------------------------------------
	//  Position
	//
	//  The viewing admin's own position - raw vector.ToString() with the
	//  "<" ">" wrapper stripped, plus which way they are facing as an
	//  8-point compass reading to its right (JMCompass, shared with the
	//  weather tab's wind heading). The compass reading sits OUTSIDE
	//  ft_btn_position, so it is never part of the button's hover/click -
	//  only the icon and the coordinate text are. Click copies the position
	//  straight to the clipboard and shows its own check-mark feedback -
	//  see ShowPositionCopiedFeedback.
	// -------------------------------------------------------------------------
	protected string m_LastPositionText;
	protected string m_LastPositionCopyText;

	protected void UpdatePosition()
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
		if ( !player )
			return;

		vector pos = player.GetPosition();
		float posXFloat = pos[0];
		float posYFloat = pos[1];
		float posZFloat = pos[2];

		//! vector.ToString() rounds to whole metres, float.ToString() only
		//! keeps a handful of significant digits, and string.Format's "%.Nf"
		//! is not actually implemented (it silently drops the value and
		//! leaves the literal ".Nf" behind) - FormatFixed builds the decimal
		//! string by hand instead, which is what actually preserves
		//! sub-millimetre precision on the clipboard.
		string copyX = FormatFixed( posXFloat, 6 );
		string copyY = FormatFixed( posYFloat, 6 );
		string copyZ = FormatFixed( posZFloat, 6 );
		m_LastPositionCopyText = copyX + ", " + copyY + ", " + copyZ;

		int posX = posXFloat;
		int posY = posYFloat;
		int posZ = posZFloat;
		m_LastPositionText = "" + posX + ", " + posY + ", " + posZ;

		if ( m_TextPosition )
			m_TextPosition.SetText( m_LastPositionText );

		vector ori = player.GetOrientation();
		if ( m_TextCardinal )
			m_TextCardinal.SetText( JMCompass.CardinalFor( ori[0] ) );

		PackPositionRow();
	}

	//! Hugs ft_btn_position to the measured text so its hover/click never
	//! reaches past the coordinate string, then places the cardinal
	//! reading - a sibling of the button, not a child of it - right after.
	//! Everything here is in ft_position_row's own local frame: the button
	//! sits at local (0,0) inside that row, so the text's position relative
	//! to the button (POSITION_TEXT_LEFT) already IS its position relative
	//! to the row, and the cardinal reading can be placed directly off the
	//! button's new width with no extra translation.
	protected void PackPositionRow()
	{
		if ( !m_TextPosition || !m_BtnPosition || !m_IconCardinal || !m_TextCardinal )
			return;

		int textWidth, textHeight;
		m_TextPosition.GetTextSize( textWidth, textHeight );

		if ( textWidth <= 0 )
			return;

		float btnWidth = POSITION_TEXT_LEFT + textWidth + POSITION_TRAILING_PAD;

		m_BtnPosition.SetSize( btnWidth, POSITION_ROW_HEIGHT );

		float cardX = btnWidth + PAIR_GAP;

		m_IconCardinal.SetPos( cardX, 2 );
		m_TextCardinal.SetPos( cardX + 13 + PAIR_GAP, 0 );
	}

	// -------------------------------------------------------------------------
	//  Values that cannot change for the lifetime of the connection.
	// -------------------------------------------------------------------------
	protected void CacheStaticInfo()
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
			m_ServerName = "#STR_COT_SIDEBAR_FOOTER_LOCAL";
	}

	// -------------------------------------------------------------------------
	//  Formatting
	// -------------------------------------------------------------------------
	protected string Pad2( int value )
	{
		if ( value < 10 )
			return "0" + value;

		return "" + value;
	}

	protected string Truncate( string text, int maxLength )
	{
		if ( text.Length() <= maxLength )
			return text;

		return text.Substring( 0, maxLength - 1 ) + "...";
	}

	//! Fixed-decimal float formatting, built by hand because string.Format's
	//! "%.Nf" is not implemented by this engine's Enforce runtime - it
	//! silently drops the value and leaves the literal ".Nf" in the string.
	protected string FormatFixed( float value, int decimals )
	{
		string sign = "";
		float absValue = value;

		if ( absValue < 0 )
		{
			sign = "-";
			absValue = -absValue;
		}

		int intPart = absValue;

		int scale = 1;
		for ( int i = 0; i < decimals; i++ )
			scale *= 10;

		int fracPart = Math.Round( ( absValue - intPart ) * scale );

		//! Rounding the fraction up to a full unit (e.g. 0.9999995 at 6
		//! decimals) carries into the integer part rather than overflowing
		//! the fractional digit count.
		if ( fracPart >= scale )
		{
			fracPart -= scale;
			intPart += 1;
		}

		string fracText = "" + fracPart;

		while ( fracText.Length() < decimals )
			fracText = "0" + fracText;

		return sign + intPart + "." + fracText;
	}
}
