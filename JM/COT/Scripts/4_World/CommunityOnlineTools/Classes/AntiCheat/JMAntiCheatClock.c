// =============================================================================
//  JMAntiCheatClock.c
//
//  Real-world UTC seconds, for the two things g_Game.GetTime() cannot do:
//  decay a score across a server restart, and tell an admin how old a flag is.
//
//  g_Game.GetTime() is milliseconds since THIS mission started. It is the right
//  clock for everything inside a session - cooldowns, rolling windows - and the
//  wrong one for anything written to disk, because it restarts at zero.
//
//  Note the 2038 ceiling: Enforce ints are 32-bit signed, so UtcNow() overflows
//  in January 2038 like every other epoch second stored in an int. Nothing here
//  is worth widening for that; a flag file from 2038 is not a flag file anyone
//  is still reading.
// =============================================================================

class JMAntiCheatClock
{
	//! Seconds since 1970-01-01T00:00:00Z.
	static int UtcNow()
	{
		int year, month, day;
		GetYearMonthDayUTC( year, month, day );

		int hour, minute, second;
		GetHourMinuteSecondUTC( hour, minute, second );

		//! A machine with no clock set at all reports year 0, and the epoch
		//! maths below would hand back a large negative number that reads as
		//! "flagged in the bronze age". Refuse instead.
		if ( year < 1970 )
			return 0;

		int days = DaysFromCivil( year, month, day );

		return days * 86400 + hour * 3600 + minute * 60 + second;
	}

	//! Days since the epoch, by Howard Hinnant's civil-date algorithm. Integer
	//! only, and correct across leap years and centuries - which the obvious
	//! "365 * years + leaps" version is not.
	static int DaysFromCivil( int y, int m, int d )
	{
		int shifted = y;
		if ( m <= 2 )
			shifted = y - 1;

		int era = shifted / 400;
		int yoe = shifted - era * 400;
		int mp  = ( m + 9 ) % 12;
		int doy = ( 153 * mp + 2 ) / 5 + d - 1;
		int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

		return era * 146097 + doe - 719468;
	}

	//! "2026-08-23 14:05Z". Stamped into flag event lines so a list read after
	//! a restart still says when each entry happened - a g_Game.GetTime()
	//! millisecond count from a previous mission means nothing at all.
	static string StampUtc()
	{
		int year, month, day;
		GetYearMonthDayUTC( year, month, day );

		int hour, minute, second;
		GetHourMinuteSecondUTC( hour, minute, second );

		string text = year.ToString() + "-" + Pad2( month ) + "-" + Pad2( day );
		text = text + " " + Pad2( hour ) + ":" + Pad2( minute ) + "Z";

		return text;
	}

	static string Pad2( int value )
	{
		if ( value < 10 )
			return "0" + value.ToString();

		return value.ToString();
	}

	//! Human-readable age for the form. Deliberately coarse - an admin needs
	//! "3d ago", not a timestamp they have to subtract in their head.
	static string FormatAge( int seconds )
	{
		if ( seconds <= 0 )
			return "now";

		if ( seconds < 60 )
			return seconds.ToString() + "s";

		if ( seconds < 3600 )
			return ( seconds / 60 ).ToString() + "m";

		if ( seconds < 86400 )
			return ( seconds / 3600 ).ToString() + "h";

		return ( seconds / 86400 ).ToString() + "d";
	}
}
