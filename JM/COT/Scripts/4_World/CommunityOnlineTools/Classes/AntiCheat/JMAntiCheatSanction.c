// =============================================================================
//  JMAntiCheatSanction.c
//
//  Short-lived "this was us" windows.
//
//  Every rule in the anti-cheat asks the same question: did the server do this,
//  or did the client? A 400m jump is a teleport cheat when a client causes it
//  and an admin tool when the server causes it, and the two look identical by
//  the time the position poll sees them - one sample here, the next one there.
//
//  The module used to answer that question by skipping anyone holding COT
//  access. That made every admin invisible to every rule, so a stolen admin
//  account was invisible too. This answers it the other way round: the code
//  performing a sanctioned action says so, and only the rule that action would
//  trip is muted, only for the player it happened to, only for a few seconds.
//  Nobody is exempt as a person.
//
//  Public API, safe for other mods:
//
//      JMAntiCheatSanction.Grant( guid, JMAntiCheatSanction.MOVEMENT, 5.0 );
//      player.SetWorldPosition( somewhere );
//
//  SetWorldPosition on PlayerBase already grants MOVEMENT for you, so a mod
//  that teleports through it needs no changes at all. Grant() is for the paths
//  that do not - a direct SetPosition, a sync juncture, an inventory fill.
//
//  Server-only in effect. Granting on a client is harmless and does nothing.
// =============================================================================

class JMAntiCheatSanction
{
	//! Rule groups. These are coarse on purpose: a sanction is a statement
	//! about what kind of thing the server just did, not about which detector
	//! happens to implement it this week.
	static const string MOVEMENT  = "movement";   //!< teleport + speed
	static const string INVENTORY = "inventory";  //!< item add-rate
	static const string DAMAGE    = "damage";     //!< immunity + health delta
	static const string AMMO      = "ammo";       //!< infinite ammo
	static const string COMBAT    = "combat";     //!< kill patterns, aim, range
	static const string RPC       = "rpc";        //!< denied-permission bursts

	//! Mutes every rule for this player for the window. Use sparingly - this
	//! is the old blanket exemption, just time-boxed, and it has all the same
	//! weaknesses while it is open.
	static const string ALL = "*";

	//! "<guid>|<bucket>" -> expiry, in g_Game.GetTime() milliseconds.
	static ref map< string, int > Windows = new map< string, int >;

	//! Set from JMAntiCheatConfig.PositionGraceS when the module loads, so the
	//! 4_World callers do not have to reach up into 5_Mission for a number.
	static float MoveGraceSeconds = 5.0;

	//! An admin filling somebody's inventory is dozens of items at once, and
	//! the inventory rule is a rate measured over a several-second window. The
	//! window has to outlive the whole spawn or the tail of it still flags.
	static const float SPAWN_GRACE_SECONDS = 30.0;

	//! A sanction is a few seconds long, so the live set is bounded by the
	//! player count in practice. This is the backstop for a caller granting in
	//! a loop, and for guids that disconnect mid-window.
	static const int MAX_ENTRIES = 1024;

	// -------------------------------------------------------------------------

	//! Declare that the server is about to do something to this player that a
	//! rule would otherwise read as a cheat. Call it BEFORE the action: the
	//! poll can land on the very next frame.
	static void Grant( string guid, string bucket, float seconds )
	{
		if ( guid == "" || seconds <= 0 )
			return;

		if ( !g_Game || !g_Game.IsServer() )
			return;

		if ( Windows.Count() >= MAX_ENTRIES )
			Prune();

		string key = guid + "|" + bucket;
		int expiry = g_Game.GetTime() + (int)( seconds * 1000 );

		//! Never shorten a window that is already open. Two overlapping
		//! sanctioned actions should leave the longer one standing, or the
		//! second one silently cancels the first one's protection.
		if ( Windows.Contains( key ) && Windows.Get( key ) > expiry )
			return;

		Windows.Set( key, expiry );
	}

	//! True while a sanctioned action of this kind is still covering the
	//! player. Expired entries are dropped as they are read, so a rule that
	//! keeps asking keeps the map clean by itself.
	static bool IsActive( string guid, string bucket )
	{
		if ( guid == "" )
			return false;

		if ( Check( guid + "|" + ALL ) )
			return true;

		return Check( guid + "|" + bucket );
	}

	//! Drop every window for one player - on disconnect, or when an admin
	//! clears their flags and wants a clean slate.
	static void Revoke( string guid )
	{
		if ( guid == "" )
			return;

		array< string > doomed = new array< string >;
		string prefix = guid + "|";

		foreach ( string key, int expiry : Windows )
		{
			if ( key.IndexOf( prefix ) == 0 )
				doomed.Insert( key );
		}

		foreach ( string dead : doomed )
			Windows.Remove( dead );
	}

	//! Drop everything already expired.
	static void Prune()
	{
		if ( !g_Game )
			return;

		int nowMs = g_Game.GetTime();

		array< string > doomed = new array< string >;

		foreach ( string key, int expiry : Windows )
		{
			if ( nowMs >= expiry )
				doomed.Insert( key );
		}

		foreach ( string dead : doomed )
			Windows.Remove( dead );
	}

	private static bool Check( string key )
	{
		if ( !Windows.Contains( key ) )
			return false;

		if ( g_Game.GetTime() < Windows.Get( key ) )
			return true;

		Windows.Remove( key );
		return false;
	}
}
