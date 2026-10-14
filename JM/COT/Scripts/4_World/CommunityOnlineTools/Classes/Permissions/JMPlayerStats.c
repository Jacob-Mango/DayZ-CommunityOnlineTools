//  JMPlayerStats - per-player session history (playtime, connects, first/last
//  seen, deaths, life durations), stored in the per-GUID player JSON to avoid a
//  second file's I/O; older files load at defaults. Wall-clock time is
//  JMAntiCheatClock.UtcNow(): g_Game.GetTime() restarts at zero each mission.

class JMPlayerStats : Managed
{
	int TotalPlaytimeSec;
	int SessionCount;
	int FirstSeenUnix;
	int LastSeenUnix;
	int Deaths;
	int LongestLifeSec;

	//! Unix time the current life began, or 0 when the player is dead or has no
	//! character. Persisted rather than in-memory so a life survives a reconnect.
	int CurrentLifeStartUnix;

	//! Engine time (ms) the current session began. Not serialized: it means
	//! nothing across a restart, and a stale value would add garbage to
	//! TotalPlaytimeSec on the next disconnect.
	[NonSerialized()]
	int SessionStartMs;

	void JMPlayerStats()
	{
		TotalPlaytimeSec     = 0;
		SessionCount         = 0;
		FirstSeenUnix        = 0;
		LastSeenUnix         = 0;
		Deaths               = 0;
		LongestLifeSec       = 0;
		CurrentLifeStartUnix = 0;
		SessionStartMs       = 0;
	}

	//! Seconds the current life has been running, or 0 when there is none.
	int GetCurrentLifeSeconds()
	{
		if ( CurrentLifeStartUnix <= 0 )
			return 0;

		int now = JMAntiCheatClock.UtcNow();
		if ( now <= CurrentLifeStartUnix )
			return 0;

		return now - CurrentLifeStartUnix;
	}

	//! Total playtime including the session currently in progress, which is the
	//! number an admin looking at a connected player expects to see.
	int GetLivePlaytimeSeconds()
	{
		if ( SessionStartMs <= 0 )
			return TotalPlaytimeSec;

		return TotalPlaytimeSec + ( g_Game.GetTime() - SessionStartMs ) / 1000;
	}

	//! True when nothing has ever been recorded. The save path uses this to
	//! decide whether the player file is worth keeping at all - without it, an
	//! ordinary player with default roles has their file deleted on every save
	//! and their playtime with it.
	bool IsEmpty()
	{
		if ( TotalPlaytimeSec > 0 ) return false;
		if ( SessionCount > 0 )     return false;
		if ( FirstSeenUnix > 0 )    return false;
		if ( LastSeenUnix > 0 )     return false;
		if ( Deaths > 0 )           return false;
		if ( LongestLifeSec > 0 )   return false;

		return true;
	}

	//! Begin a session. Safe to call twice: a reconnect without a matching
	//! disconnect must not count the session or the life twice.
	void OnSessionStart()
	{
		int now = JMAntiCheatClock.UtcNow();

		SessionStartMs = g_Game.GetTime();
		SessionCount++;

		if ( FirstSeenUnix == 0 )
			FirstSeenUnix = now;

		LastSeenUnix = now;

		// A life that is already open survived a reconnect and keeps running.
		if ( CurrentLifeStartUnix == 0 )
			CurrentLifeStartUnix = now;
	}

	//! Fold the open session into the total. Called on disconnect and from the
	//! periodic flush, so it has to be idempotent - it re-bases SessionStartMs
	//! rather than clearing it.
	void AccumulateSession()
	{
		if ( SessionStartMs <= 0 )
			return;

		int nowMs = g_Game.GetTime();
		int elapsed = ( nowMs - SessionStartMs ) / 1000;

		if ( elapsed > 0 )
		{
			TotalPlaytimeSec = TotalPlaytimeSec + elapsed;
			SessionStartMs = nowMs;
		}

		LastSeenUnix = JMAntiCheatClock.UtcNow();
	}

	void OnSessionEnd()
	{
		AccumulateSession();
		SessionStartMs = 0;
	}

	//! Close the current life and count the death.
	void OnDeath()
	{
		Deaths++;

		int lifeLength = GetCurrentLifeSeconds();
		if ( lifeLength > LongestLifeSec )
			LongestLifeSec = lifeLength;

		CurrentLifeStartUnix = 0;
	}

	//! "3d 04h 12m" / "4h 12m" / "12m 30s". A duration an admin reads at a
	//! glance rather than a raw second count.
	static string FormatDuration( int seconds )
	{
		if ( seconds <= 0 )
			return "-";

		int days = seconds / 86400;
		int rem  = seconds - days * 86400;
		int hours = rem / 3600;
		rem = rem - hours * 3600;
		int minutes = rem / 60;
		int secs = rem - minutes * 60;

		if ( days > 0 )
			return days.ToString() + "d " + hours.ToString() + "h " + minutes.ToString() + "m";

		if ( hours > 0 )
			return hours.ToString() + "h " + minutes.ToString() + "m";

		if ( minutes > 0 )
			return minutes.ToString() + "m " + secs.ToString() + "s";

		return secs.ToString() + "s";
	}

	//! How long ago a timestamp was, as "3d" / "4h" / "now". Shown instead of a
	//! calendar date because "last seen 3d" is the question being asked, and it
	//! avoids reimplementing civil-date maths that already exists once.
	static string FormatSince( int unix )
	{
		if ( unix <= 0 )
			return "-";

		int now = JMAntiCheatClock.UtcNow();
		if ( now <= unix )
			return "now";

		return JMAntiCheatClock.FormatAge( now - unix );
	}
}
