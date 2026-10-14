//  JMAntiCheatStatus - client mirror of the flagged-player count. The flag table
//  is only sent while the form is open, so the count rides the server-stats
//  broadcast for the footer badge. In 3_Game as 5_Mission and 4_World cannot
//  name each other's types (same as JMServerStats).

class JMAntiCheatStatus
{
	//! Players at or above the module's own "worth a warning" score. 0 means a
	//! quiet server, or a client without permission to be told.
	static int FlaggedCount;

	//! GUIDs behind that count, so a list row can ask about one player rather
	//! than only knowing a total. Short by construction - this is the set of
	//! players an admin is meant to go and look at, not every stray reading.
	static ref array< string > FlaggedGuids;

	//! g_Game.GetTime() when the last broadcast landed. 0 = nothing yet.
	static int LastUpdateMs;

	//! Matches JMServerStats: two missed broadcasts plus slack. A client that
	//! stops hearing from the server shows no badge rather than a stale one.
	protected static const int STALE_AFTER_MS = 8000;

	//! The one question the footer asks: is there anything to raise a badge for?
	static bool HasFlags()
	{
		if ( !IsValid() )
			return false;

		return FlaggedCount > 0;
	}

	//! Is this one player flagged? False on a stale or never-received mirror,
	//! which is the safe answer: a badge nobody can justify is worse than none.
	static bool IsFlagged( string guid )
	{
		if ( guid == "" )
			return false;

		if ( !IsValid() )
			return false;

		if ( !FlaggedGuids )
			return false;

		return FlaggedGuids.Find( guid ) >= 0;
	}

	//! False until the first broadcast lands, and again once they stop coming.
	static bool IsValid()
	{
		if ( LastUpdateMs == 0 )
			return false;

		return ( g_Game.GetTime() - LastUpdateMs ) < STALE_AFTER_MS;
	}

	static void Set( int flagged, array< string > guids = NULL )
	{
		FlaggedCount = flagged;
		LastUpdateMs = g_Game.GetTime();

		EnsureList();
		FlaggedGuids.Clear();

		if ( !guids )
			return;

		foreach ( string guid: guids )
			FlaggedGuids.Insert( guid );
	}

	static void Clear()
	{
		FlaggedCount = 0;
		LastUpdateMs = 0;

		EnsureList();
		FlaggedGuids.Clear();
	}

	//! Static ref arrays are not constructed for us, and every entry point here
	//! can be the first one called.
	protected static void EnsureList()
	{
		if ( !FlaggedGuids )
			FlaggedGuids = new array< string >;
	}
}
