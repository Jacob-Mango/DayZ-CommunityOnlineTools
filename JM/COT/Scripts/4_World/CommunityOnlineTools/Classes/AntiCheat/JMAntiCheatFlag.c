// =============================================================================
//  JMAntiCheatFlag.c
//
//  Per-player rolling flag state. Server-only. The form shows the current
//  score plus a recent-events list.
//
//  The score decays. A player who tripped one uncertain rule six hours ago and
//  nothing since is not the same as one who tripped it six seconds ago, and a
//  score that only ever goes up eventually flags everybody who has played on
//  the server long enough - at which point the number means "hours played".
//  Decay is what keeps the score a statement about the present.
//
//  Everything time-related here is UTC seconds, not g_Game.GetTime(), because
//  this class is written to disk and read back after a restart.
// =============================================================================

class JMAntiCheatFlag
{
	string PlayerName;
	string Guid;
	int TotalScore;

	//! Fractional part of the decay that has not yet cost a whole point. Decay
	//! is a few points an hour and the poll runs at 1 Hz, so without a carry
	//! every tick would truncate to zero and the score would never move.
	float DecayDebt;

	//! UTC seconds. LastEventUnix is when the score last went UP - the age an
	//! admin actually cares about. LastDecayUnix is bookkeeping.
	int LastEventUnix;
	int LastDecayUnix;

	//! Session-relative, deliberately NOT persisted in any meaningful way: an
	//! auto-kick cooldown from before a restart is not a cooldown any more.
	int LastActionTimeMs;

	ref array< string > RecentEvents;

	void JMAntiCheatFlag()
	{
		TotalScore = 0;
		DecayDebt = 0;
		LastEventUnix = 0;
		LastDecayUnix = 0;
		LastActionTimeMs = 0;
		RecentEvents = new array< string >;
	}

	void AddEvent( int weight, string text, int nowMs )
	{
		TotalScore += weight;

		int nowUnix = JMAntiCheatClock.UtcNow();
		LastEventUnix = nowUnix;

		//! Restart the decay clock from here rather than letting it bill for
		//! the quiet hours before the event: the score just became current.
		LastDecayUnix = nowUnix;
		DecayDebt = 0;

		string entry = "[" + JMAntiCheatClock.StampUtc() + "] (+" + weight.ToString() + ") " + text;
		RecentEvents.Insert( entry );

		// Cap the list - older entries fall off the back
		while ( RecentEvents.Count() > 25 )
			RecentEvents.Remove( 0 );
	}

	//! Bleed the score down at pointsPerHour. Returns true if the score moved,
	//! so the caller knows the on-disk copy is now stale.
	//!
	//! Safe to call every poll: it charges for real elapsed time, so calling it
	//! once an hour and 3600 times an hour cost the same points.
	bool ApplyDecay( int nowUnix, float pointsPerHour )
	{
		if ( pointsPerHour <= 0 || TotalScore <= 0 )
			return false;

		if ( LastDecayUnix == 0 || nowUnix <= LastDecayUnix )
		{
			//! First call, or the wall clock moved backwards (NTP correction,
			//! a host with a bad RTC). Re-anchor rather than bill a negative.
			LastDecayUnix = nowUnix;
			return false;
		}

		float elapsedHours = ( nowUnix - LastDecayUnix ) / 3600.0;
		LastDecayUnix = nowUnix;

		DecayDebt += elapsedHours * pointsPerHour;

		int whole = (int)DecayDebt;
		if ( whole <= 0 )
			return false;

		DecayDebt -= whole;
		TotalScore -= whole;

		if ( TotalScore < 0 )
		{
			TotalScore = 0;
			DecayDebt = 0;
		}

		return true;
	}

	//! Seconds since the score last went up. Zero when it never has.
	int AgeSeconds( int nowUnix )
	{
		if ( LastEventUnix == 0 )
			return 0;

		return nowUnix - LastEventUnix;
	}

	void Clear()
	{
		TotalScore = 0;
		DecayDebt = 0;
		LastEventUnix = 0;
		LastDecayUnix = 0;
		RecentEvents.Clear();
	}
}
