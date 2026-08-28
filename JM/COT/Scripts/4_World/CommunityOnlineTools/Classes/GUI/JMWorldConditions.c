// =============================================================================
//  JMWorldConditions.c
//
//  Reads the live world state - weather phenomena and the in-game clock - and
//  turns it into something a UI can show: a localisation key and an icon.
//
//  Kept out of the sidebar footer so anything else that wants to render "what
//  is the weather doing right now" (a server-info panel, a webhook embed, the
//  weather module's own header) classifies it the same way.
// =============================================================================

class JMWorldConditions
{
	//! Weather kinds, ordered roughly by severity. Returned by GetWeatherKind.
	static const int WEATHER_CLEAR    = 0;
	static const int WEATHER_CLOUDY   = 1;
	static const int WEATHER_OVERCAST = 2;
	static const int WEATHER_FOG      = 3;
	static const int WEATHER_RAIN     = 4;
	static const int WEATHER_STORM    = 5;
	static const int WEATHER_SNOW     = 6;

	//! Time-of-day buckets. Returned by GetTimeOfDay.
	static const int TOD_DAWN  = 0;
	static const int TOD_DAY   = 1;
	static const int TOD_DUSK  = 2;
	static const int TOD_NIGHT = 3;

	// --- Thresholds ----------------------------------------------------------
	//  Phenomena are 0..1. These are the points at which a player actually
	//  notices the effect on screen, not the points at which the value is
	//  non-zero: DayZ's default weather almost never sits at a clean 0.

	//! Snow above this is falling hard enough to see, and outranks everything
	//! else - it only happens on a map/season that has snow at all.
	private static const float SNOW_VISIBLE     = 0.10;
	//! Rain above this reads as rain rather than the odd stray drop.
	private static const float RAIN_VISIBLE     = 0.10;
	//! Rain this heavy under a near-black sky is what the weather module calls
	//! a storm; DayZ exposes no "is there lightning" getter to ask directly.
	private static const float RAIN_STORM       = 0.60;
	private static const float OVERCAST_STORM   = 0.85;
	//! Fog thick enough to cut draw distance.
	private static const float FOG_VISIBLE      = 0.30;
	//! Solid cloud lid vs. broken cloud.
	private static const float OVERCAST_SOLID   = 0.65;
	private static const float OVERCAST_BROKEN  = 0.30;

	// --- Time-of-day boundaries (24h) ---------------------------------------
	private static const int HOUR_DAWN_START = 5;
	private static const int HOUR_DAY_START  = 8;
	private static const int HOUR_DUSK_START = 18;
	private static const int HOUR_NIGHT_START = 21;

	// -------------------------------------------------------------------------
	//  Weather
	// -------------------------------------------------------------------------

	//! Classify the live weather. Falls back to CLEAR before the world exists.
	static int GetWeatherKind()
	{
		return ReadWeatherKind( false );
	}

	//! Classify where the weather is HEADING, from the forecast each phenomenon
	//! is interpolating towards. Equal to GetWeatherKind whenever the server has
	//! not queued a change, which is how callers tell "nothing known" apart from
	//! "something coming" - there is no separate has-forecast flag to read.
	static int GetForecastWeatherKind()
	{
		return ReadWeatherKind( true );
	}

	//! Seconds until the weather system recomputes its forecast, or 0 when there
	//! is no world yet. Note this is when the NEXT forecast is decided, not when
	//! the current one finishes interpolating.
	static float GetSecondsToWeatherChange()
	{
		Weather weather = g_Game.GetWeather();
		if ( !weather )
			return 0;

		//! The soonest of the four phenomena that can change what the weather is
		//! called. Reading overcast alone reports "nothing coming" on a server
		//! whose cloud lid is parked while rain is minutes away.
		float soonest = 0;

		soonest = Soonest( soonest, weather.GetOvercast() );
		soonest = Soonest( soonest, weather.GetRain() );
		soonest = Soonest( soonest, weather.GetFog() );
		soonest = Soonest( soonest, weather.GetSnowfall() );

		return soonest;
	}

	//! Fold one phenomenon's next-change time into the running minimum. 0 means
	//! "nothing pending" on both sides, so it never wins.
	private static float Soonest( float current, WeatherPhenomenon phenomenon )
	{
		if ( !phenomenon )
			return current;

		float next = phenomenon.GetNextChange();

		if ( next <= 0 )
			return current;

		if ( current <= 0 )
			return next;

		if ( next < current )
			return next;

		return current;
	}

	//! The wait until the weather controller picks its next forecast, as
	//! something a footer can print: "45s", "12m", "1h 20m". Empty when there is
	//! no world yet or nothing pending, which is the caller's cue to draw
	//! nothing rather than a zero.
	static string FormatSecondsToChange( float seconds )
	{
		if ( seconds <= 0 )
			return "";

		int total = Math.Round( seconds );

		if ( total < 60 )
			return "" + total + "s";

		int minutes = total / 60;

		if ( minutes < 60 )
			return "" + minutes + "m";

		int hours     = minutes / 60;
		int remainder = minutes - ( hours * 60 );

		if ( remainder == 0 )
			return "" + hours + "h";

		return "" + hours + "h " + remainder + "m";
	}

	//! The one classifier, run over either the actual values or the forecast
	//! ones. Keeping it single-sourced is the point: a current state and a
	//! predicted state that disagreed about what counts as "rain" would show an
	//! arrow pointing at the weather already on screen.
	private static int ReadWeatherKind( bool forecast )
	{
		Weather weather = g_Game.GetWeather();
		if ( !weather )
			return WEATHER_CLEAR;

		float snow = ReadPhenomenon( weather.GetSnowfall(), forecast );

		if ( snow > SNOW_VISIBLE )
			return WEATHER_SNOW;

		float rain = ReadPhenomenon( weather.GetRain(), forecast );
		float overcast = ReadPhenomenon( weather.GetOvercast(), forecast );

		if ( rain > RAIN_STORM && overcast > OVERCAST_STORM )
			return WEATHER_STORM;

		if ( rain > RAIN_VISIBLE )
			return WEATHER_RAIN;

		float fogAmount = ReadPhenomenon( weather.GetFog(), forecast );

		if ( fogAmount > FOG_VISIBLE )
			return WEATHER_FOG;

		if ( overcast > OVERCAST_SOLID )
			return WEATHER_OVERCAST;

		if ( overcast > OVERCAST_BROKEN )
			return WEATHER_CLOUDY;

		return WEATHER_CLEAR;
	}

	private static float ReadPhenomenon( WeatherPhenomenon phenomenon, bool forecast )
	{
		if ( !phenomenon )
			return 0;

		if ( forecast )
			return phenomenon.GetForecast();

		return phenomenon.GetActual();
	}

	//! Stringtable key for a weather kind. Resolve with Widget.TranslateString.
	static string GetWeatherLabel( int kind )
	{
		if ( kind == WEATHER_SNOW )
			return "#STR_COT_WEATHER_MODULE_SNOW";

		if ( kind == WEATHER_STORM )
			return "#STR_COT_WEATHER_MODULE_STORM";

		if ( kind == WEATHER_RAIN )
			return "#STR_COT_WEATHER_MODULE_RAIN";

		if ( kind == WEATHER_FOG )
			return "#STR_COT_WEATHER_MODULE_FOG";

		if ( kind == WEATHER_OVERCAST )
			return "#STR_COT_WEATHER_MODULE_OVERCAST";

		if ( kind == WEATHER_CLOUDY )
			return "#STR_COT_WEATHER_CLOUDY";

		return "#STR_COT_WEATHER_CLEAR";
	}

	//! Icon for a weather kind. The clear and broken-cloud icons come in a day
	//! and a night variant, because a sun icon at 02:00 reads as a bug.
	static string GetWeatherIcon( int kind, bool isNight )
	{
		if ( kind == WEATHER_SNOW )
			return JMConstants.Lucide( "cloud-snow" );

		if ( kind == WEATHER_STORM )
			return JMConstants.Lucide( "cloud-lightning" );

		if ( kind == WEATHER_RAIN )
		{
			if ( isNight )
				return JMConstants.Lucide( "cloud-moon-rain" );

			return JMConstants.Lucide( "cloud-sun-rain" );
		}

		if ( kind == WEATHER_FOG )
			return JMConstants.Lucide( "cloud-fog" );

		if ( kind == WEATHER_OVERCAST )
			return JMConstants.Lucide( "cloudy" );

		if ( kind == WEATHER_CLOUDY )
		{
			if ( isNight )
				return JMConstants.Lucide( "cloud-moon" );

			return JMConstants.Lucide( "cloud-sun" );
		}

		if ( isNight )
			return JMConstants.Lucide( "moon" );

		return JMConstants.Lucide( "sun" );
	}

	// -------------------------------------------------------------------------
	//  Time of day
	// -------------------------------------------------------------------------

	//! Bucket an hour (0-23) into dawn / day / dusk / night.
	static int GetTimeOfDay( int hour )
	{
		if ( hour >= HOUR_NIGHT_START )
			return TOD_NIGHT;

		if ( hour >= HOUR_DUSK_START )
			return TOD_DUSK;

		if ( hour >= HOUR_DAY_START )
			return TOD_DAY;

		if ( hour >= HOUR_DAWN_START )
			return TOD_DAWN;

		return TOD_NIGHT;
	}

	static bool IsNight( int hour )
	{
		int tod = GetTimeOfDay( hour );
		return tod == TOD_NIGHT;
	}

	//! Icon for a time-of-day bucket.
	static string GetTimeIcon( int tod )
	{
		if ( tod == TOD_DAWN )
			return JMConstants.Lucide( "sunrise" );

		if ( tod == TOD_DUSK )
			return JMConstants.Lucide( "sunset" );

		if ( tod == TOD_NIGHT )
			return JMConstants.Lucide( "moon" );

		return JMConstants.Lucide( "sun" );
	}

	//! Stringtable key for a time-of-day bucket.
	static string GetTimeOfDayLabel( int tod )
	{
		if ( tod == TOD_DAWN )
			return "#STR_COT_WEATHER_DAWN";

		if ( tod == TOD_DUSK )
			return "#STR_COT_WEATHER_DUSK";

		if ( tod == TOD_NIGHT )
			return "#STR_COT_WEATHER_NIGHT";

		return "#STR_COT_WEATHER_DAY";
	}
}
