// =============================================================================
//  JMTeleportHistoryEntry
//
//  One place something was moved away FROM - a single step an undo can walk
//  back to.
// =============================================================================
class JMTeleportHistoryEntry
{
	vector Position;

	//! Client clock, in milliseconds. Only ever compared against another of
	//! these or against "now", so it does not matter that it is not a wall
	//! clock and does not survive a reconnect.
	int Time;

	void JMTeleportHistoryEntry( vector position )
	{
		Position = position;
		Time     = g_Game.GetTime();
	}

	//! Whole metres. A history row is read to recognise a PLACE - a coast, a
	//! town, the spot the admin was standing a minute ago - and three decimals
	//! of altitude only make the row too wide to read at a glance.
	string FormatPosition()
	{
		int x = ( int ) Math.Round( Position[0] );
		int y = ( int ) Math.Round( Position[1] );
		int z = ( int ) Math.Round( Position[2] );

		return string.Format( "%1 %2 %3", x, y, z );
	}

	//! How long ago this was recorded, as "12s" / "4m" / "2h".
	//!
	//! Relative rather than absolute: what an admin needs off a row is which of
	//! the five is the one they just did, and every one of them is from the
	//! current session.
	string FormatAge()
	{
		int seconds = ( g_Game.GetTime() - Time ) / 1000;

		if ( seconds < 60 )
			return string.Format( "%1s", seconds );

		if ( seconds < 3600 )
			return string.Format( "%1m", seconds / 60 );

		return string.Format( "%1h", seconds / 3600 );
	}
}
