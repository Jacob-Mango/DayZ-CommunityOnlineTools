//  JMWeatherDynamicConfig - one dynamic weather edit as a single RPC payload (no
//  half-applied merges). Master switch always applied, plus at most one state per
//  Op: OP_TOGGLE none; OP_SAVE State under TargetName (new if empty, rename if
//  differs); OP_REMOVE delete it. Server keeps its own NormalizeState copy.
class JMWeatherDynamicConfig
{
	static const int OP_TOGGLE = 0;
	static const int OP_SAVE   = 1;
	static const int OP_REMOVE = 2;

	bool Enabled;
	int Op;

	//! The state being edited, by the name it has now on the server. "" = new.
	string TargetName;

	autoptr JMWeatherState State;

	void JMWeatherDynamicConfig()
	{
		Enabled    = false;
		Op         = OP_TOGGLE;
		TargetName = "";
		State      = new JMWeatherState;
	}
}
