//! Per-sender rate limit for RPCs that cost more than a switch case (log lines, file
//! appends, webhooks). A permission check does not stop a client repeating one hundreds
//! of times a second. Keyed by GUID and bucket so one action cannot starve another,
//! and bounded so a long-running server does not keep a row for every past player.
class JMRPCThrottle
{
	//! Past this the map is pruned of everything already expired. Sized for a
	//! full server times a handful of throttled buckets each.
	static const int MAX_ENTRIES = 512;

	//! Next g_Game.GetTime() at which a given guid+bucket may run again.
	protected static ref map<string, int> s_NextAllowed = new map<string, int>;

	//! May this sender run this action now? True arms the next window; false changes
	//! nothing, so a refused caller does not extend its own cooldown. An empty guid is
	//! the server or an offline host and is never rate limited.
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
