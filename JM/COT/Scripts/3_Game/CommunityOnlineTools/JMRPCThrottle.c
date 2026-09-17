//! Per-sender rate limit for RPCs that cost more than a switch case.
//!
//! A permission check answers "may this player do it at all". It does not
//! answer "may they do it four hundred times a second", and the handlers that
//! write a log line, append to a file or post a webhook are exactly the ones
//! where the second question matters: a client that repeats the message is
//! spending the server's disk and the server's outbound HTTP budget, not its
//! own.
//!
//! Keyed by GUID and bucket so one noisy action cannot starve another, and
//! bounded so a server that has seen thousands of players over a wipe does not
//! carry a row for every one of them.
class JMRPCThrottle
{
	//! Past this the map is pruned of everything already expired. Sized for a
	//! full server times a handful of throttled buckets each.
	static const int MAX_ENTRIES = 512;

	//! Next g_Game.GetTime() at which a given guid+bucket may run again.
	protected static ref map<string, int> s_NextAllowed = new map<string, int>;

	//! May this sender run this action now?
	//!
	//! Answers true and arms the next window, or false and changes nothing -
	//! so a caller that is refused does not push its own cooldown further out
	//! by asking. An empty guid is the server or an offline host acting on its
	//! own behalf, which is never rate limited.
	static bool Allow( string guid, string bucket, int intervalMs )
	{
		if ( guid == "" || intervalMs <= 0 )
			return true;

		int now = g_Game.GetTime();

		string key = guid + "|" + bucket;

		if ( s_NextAllowed.Contains( key ) && s_NextAllowed.Get( key ) > now )
			return false;

		if ( s_NextAllowed.Count() >= MAX_ENTRIES )
			Prune( now );

		s_NextAllowed.Set( key, now + intervalMs );

		return true;
	}

	//! Forget a player's windows. Called when they disconnect: the next session
	//! under the same GUID starts clean rather than inheriting a cooldown.
	static void Clear( string guid )
	{
		if ( guid == "" )
			return;

		array<string> stale = new array<string>;

		for ( int i = 0; i < s_NextAllowed.Count(); ++i )
		{
			string key = s_NextAllowed.GetKey( i );

			if ( key.IndexOf( guid + "|" ) == 0 )
				stale.Insert( key );
		}

		for ( int j = 0; j < stale.Count(); ++j )
			s_NextAllowed.Remove( stale[j] );
	}

	//! Drop every window that has already passed.
	protected static void Prune( int now )
	{
		array<string> expired = new array<string>;

		for ( int i = 0; i < s_NextAllowed.Count(); ++i )
		{
			if ( s_NextAllowed.GetElement( i ) <= now )
				expired.Insert( s_NextAllowed.GetKey( i ) );
		}

		for ( int j = 0; j < expired.Count(); ++j )
			s_NextAllowed.Remove( expired[j] );

		//! Everything is still live - the map is genuinely that busy. Emptying
		//! it is better than growing without bound: the worst a cleared window
		//! costs is one extra allowed call per player.
		if ( s_NextAllowed.Count() >= MAX_ENTRIES )
			s_NextAllowed.Clear();
	}
}
