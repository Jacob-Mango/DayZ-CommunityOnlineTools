// =============================================================================
//  JMAntiCheatStatus.c
//
//  Client-side mirror of one number: how many players the anti-cheat currently
//  has flagged hard enough to be worth looking at.
//
//  The anti-cheat module keeps its full flag table server-side and ships it to
//  a client only while the AntiCheat form is open and asking. That is the right
//  shape for a table of scores and event lists, and the wrong shape for a
//  footer badge, which has to know something is wrong before anyone thinks to
//  open the form. So the count - and only the count - rides along with the
//  server-stats broadcast that already runs on a timer to the same audience.
//
//  Lives in 3_Game for the same reason JMServerStats does: the producer is a
//  5_Mission module and the consumer is the 4_World sidebar footer, and those
//  two cannot name each other's types.
// =============================================================================

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
	private static const int STALE_AFTER_MS = 8000;

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

	//! Static ref arrays are not constructed for us, and every entry point here
	//! can be the first one called.
	private static void EnsureList()
	{
		if ( !FlaggedGuids )
			FlaggedGuids = new array< string >;
	}

	//! False until the first broadcast lands, and again once they stop coming.
	static bool IsValid()
	{
		if ( LastUpdateMs == 0 )
			return false;

		return ( g_Game.GetTime() - LastUpdateMs ) < STALE_AFTER_MS;
	}

	//! The one question the footer asks: is there anything to raise a badge for?
	static bool HasFlags()
	{
		if ( !IsValid() )
			return false;

		return FlaggedCount > 0;
	}
}
