//! Runtime deprecation notice for APIs that still work but have a replacement.
//!
//! Follows the same convention as DayZ Expansion (`EXError.WarnOnce`): the old
//! method stays as a thin forwarding wrapper, warns exactly once per message,
//! then calls the new one. Use `[Obsolete("Use X")]` instead when the old
//! signature no longer does anything.
//!
//!     //! DEPRECATED - use BindPermission
//!     void UpdatePermission( UIActionBase c, string p )
//!     {
//!         JMDeprecated.WarnOnce( this, "UpdatePermission() is deprecated. Please use BindPermission()." );
//!         BindPermission( c, p );
//!     }
//!
//! Every deprecation is also listed in Docs/systems/deprecations.md.
class JMDeprecated
{
	//! Distinct messages are a handful; the cap only stops a caller that builds
	//! its message from runtime data from growing the set without bound.
	static const int MAX_MESSAGES = 128;
	protected static ref map<string, int> s_Seen = new map<string, int>;

	//! Warn once per unique message, with the caller's class name for context.
	//! Later identical messages only increment a counter.
	static void WarnOnce( Class instance, string msg )
	{
		int count;
		if ( s_Seen.Find( msg, count ) )
		{
			s_Seen[msg] = count + 1;
			return;
		}

		if ( s_Seen.Count() >= MAX_MESSAGES )
			return;

		s_Seen[msg] = 1;

		string owner = "COT";
		if ( instance )
			owner = instance.ClassName();

		PrintFormat( "[COT] WARNING (%1): %2", owner, msg );
	}
}
