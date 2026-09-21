// =============================================================================
//  JMWeatherDynamicStatus - the machine's live position, as one RPC payload
//
//  The server owns the machine; clients only ever see this. It is asked for
//  (like SpecialWeatherStatus) rather than pushed, so it costs nothing while
//  nobody has the weather form open and needs no per-client bookkeeping.
//
//  Revision is the point of the poll beyond the countdown: it changes whenever
//  the state machine's CONFIGURATION changes, so a client that sees a revision
//  it did not load simply asks for the settings again. That is what keeps a
//  second admin's form (and the editor's own after a save) current without the
//  server having to broadcast every edit.
// =============================================================================
class JMWeatherDynamicStatus
{
	bool Enabled;
	bool Paused;

	//! Paused by hand (the pause button), as opposed to by Freeze Time. Paused is true
	//! for either.
	bool Held;
	string State;
	string Phase;
	float HoldTotal;
	float HoldRemaining;
	int Revision;

	//! Client-side only, never sent: when this snapshot arrived, so the countdown
	//! can run between polls instead of stepping every couple of seconds.
	[NonSerialized()]
	float ReceivedAt;

	void JMWeatherDynamicStatus()
	{
		Enabled       = false;
		Paused        = false;
		Held          = false;
		State         = "";
		Phase         = "";
		HoldTotal     = 0;
		HoldRemaining = 0;
		Revision      = 0;
		ReceivedAt    = 0;
	}

	//! Seconds left in the current phase now, counting down from the snapshot
	//! unless the machine is paused (then the snapshot IS the answer).
	float Remaining()
	{
		if ( Paused || !Enabled || State == "" )
			return HoldRemaining;

		float elapsed = g_Game.GetTickTime() - ReceivedAt;

		return Math.Max( HoldRemaining - elapsed, 0 );
	}

	bool IsRunning()
	{
		return Enabled && State != "";
	}
}
