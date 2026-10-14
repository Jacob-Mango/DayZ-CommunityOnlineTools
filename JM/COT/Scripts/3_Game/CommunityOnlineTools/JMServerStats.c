//  JMServerStats.c - client-side mirror of the server performance numbers, plus shared
//  thresholds and formatting. Lives in 3_Game because JMServerStatsModule (5_Mission)
//  writes it and JMCOTSideBarFooter (4_World) reads it; 4_World cannot name 5_Mission
//  types, so the values pass through a layer both can see.

class JMServerStats
{
	//! Server frame rate averaged over the sampling window.
	static float Average;

	//! Mean frame rate of the slowest one percent of frames in the window - the
	//! number that actually correlates with a player noticing a stutter.
	static float OnePercentLow;

	//! Mean frame rate of the fastest one percent, for the other end of the range.
	static float OnePercentHigh;

	//! Frame cap the server is configured to run at, or 0 when it is uncapped.
	//! Read from the serverDZ.cfg `limitFPS` key - see JMServerStatsModule.
	static int MaxFPS;

	//! g_Game.GetTime() when the last broadcast landed. 0 = nothing yet.
	static int LastUpdateMs;

	// --- Health thresholds ---------------------------------------------------
	//  A DayZ server ticks at up to 60. Sustained sub-30 is where the symptoms
	//  players report - rubber-banding, slow loot spawns, late damage - start.

	//! At or above this the value is drawn in the normal text colour.
	static const float FPS_OK      = 45.0;

	//! At or above this it is drawn amber; below it, red.
	static const float FPS_CAUTION = 25.0;

	// --- Spread thresholds ---
	//  One-percent lows/highs are judged against the average, not a fixed number:
	//  a 1% low of 45 is fine next to avg 55 but alarming next to avg 90. The gap
	//  is what a player feels.

	//! Fraction of the average a one-percent value may sit away from it before
	//! it is drawn amber.
	static const float SPREAD_CAUTION = 0.30;

	//! Beyond this fraction it is drawn red.
	static const float SPREAD_BAD = 0.50;

	//! Gaps smaller than this many frames never colour, whatever the ratio says.
	//! Two guards rather than one: the ratio alone calls 29-against-30 a 3%
	//! gap (fine) but 3-against-4 a 25% gap (amber), and on a server that slow
	//! every number is within a couple of frames of every other one.
	static const float SPREAD_MIN_DELTA = 5.0;

	//! Stats older than this are stale enough not to be worth showing - the
	//! server stopped broadcasting, or this client never had the permission to
	//! receive them. Two missed broadcasts plus slack.
	protected static const int STALE_AFTER_MS = 8000;

	//! False until the first broadcast lands, and again once they stop coming.
	static bool IsValid()
	{
		if ( LastUpdateMs == 0 )
			return false;

		return ( g_Game.GetTime() - LastUpdateMs ) < STALE_AFTER_MS;
	}

	static void Set( float average, float low, float high, int maxFPS )
	{
		Average = average;
		OnePercentLow = low;
		OnePercentHigh = high;
		MaxFPS = maxFPS;
		LastUpdateMs = g_Game.GetTime();
	}

	static void Clear()
	{
		Average = 0;
		OnePercentLow = 0;
		OnePercentHigh = 0;
		MaxFPS = 0;
		LastUpdateMs = 0;
	}

	//! Colour for an fps value, from the thresholds above.
	static int ColorFor( float fps )
	{
		if ( fps >= FPS_OK )
			return JMTheme.VALUE_OK;

		if ( fps >= FPS_CAUTION )
			return JMTheme.VALUE_CAUTION;

		return JMTheme.VALUE_BAD;
	}

	//! Colour for a one-percent low or high, from its distance to the average only.
	//! Absolute thresholds deliberately play no part: a tail a frame or two from the
	//! average is the same news at 60 or 30 fps (29 vs avg 30 must not go amber just
	//! for being under FPS_OK). The average's own colour reports absolute health.
	static int ColorForSpread( float fps )
	{
		return ColorForSeverity( SpreadSeverityFor( fps ) );
	}

	//! 0 healthy, 1 caution, 2 bad - ordered so the worse of two verdicts is
	//! just the larger number.
	static int SeverityFor( float fps )
	{
		if ( fps >= FPS_OK )
			return 0;

		if ( fps >= FPS_CAUTION )
			return 1;

		return 2;
	}

	//! Severity of a value's distance from the average. Healthy when there is
	//! no average to compare against yet.
	static int SpreadSeverityFor( float fps )
	{
		if ( Average <= 0 )
			return 0;

		float delta = Math.AbsFloat( fps - Average );

		if ( delta < SPREAD_MIN_DELTA )
			return 0;

		float ratio = delta / Average;

		if ( ratio >= SPREAD_BAD )
			return 2;

		if ( ratio >= SPREAD_CAUTION )
			return 1;

		return 0;
	}

	static int ColorForSeverity( int severity )
	{
		if ( severity >= 2 )
			return JMTheme.VALUE_BAD;

		if ( severity == 1 )
			return JMTheme.VALUE_CAUTION;

		return JMTheme.VALUE_OK;
	}

	//! Rounded value, or "--" when there is nothing to show yet.
	static string Format( float fps )
	{
		if ( !IsValid() )
			return "--";

		int rounded = Math.Round( fps );
		return "" + rounded;
	}

	//! The average against its cap - "38 / 60" on a capped server, bare "38" on
	//! an uncapped one. A cap is the difference between "the server is
	//! struggling" and "the server is doing exactly what it was told to".
	static string FormatAverageAgainstCap()
	{
		string average = Format( Average );

		if ( MaxFPS <= 0 )
			return average;

		return average + " / " + MaxFPS;
	}

	//! Colour for the average. A capped server sitting on its cap is healthy
	//! whatever the absolute number is, so the fixed thresholds are replaced by
	//! a proportion of the cap once one is known.
	static int ColorForAverage()
	{
		if ( MaxFPS <= 0 )
			return ColorFor( Average );

		float ratio = Average / MaxFPS;

		if ( ratio >= 0.85 )
			return JMTheme.VALUE_OK;

		if ( ratio >= 0.5 )
			return JMTheme.VALUE_CAUTION;

		return JMTheme.VALUE_BAD;
	}
}
