// =============================================================================
//  JMWeatherDynamicControl - a one-shot command to the running machine
//
//  Separate from JMWeatherDynamicConfig on purpose: that edits the machine's
//  configuration, this only moves it. Neither touches the other's data.
// =============================================================================
class JMWeatherDynamicControl
{
	//! Leave the current phase now, by the same roll a natural expiry would use.
	static const int OP_ADVANCE = 0;

	//! Start over from a random entry, as if the server had just started.
	static const int OP_RESTART = 1;

	//! Go to State/Phase now. An empty Phase means that state's own entry roll.
	static const int OP_JUMP = 2;

	//! Put the weather of State / Phase on NOW, exactly as authored - the old "use
	//! preset". An empty Phase means the state's entry roll. Not gated on the machine
	//! running.
	static const int OP_APPLY = 3;

	//! Hold the countdown where it is, and let it run again.
	static const int OP_HOLD    = 4;
	static const int OP_RELEASE = 5;

	int Op;
	string State;
	string Phase;

	void JMWeatherDynamicControl()
	{
		Op    = OP_ADVANCE;
		State = "";
		Phase = "";
	}
}
