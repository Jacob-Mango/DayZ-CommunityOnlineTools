// =============================================================================
//  JMWeatherDynamicConfig
//
//  One dynamic weather edit as a single RPC payload, the same way a phenomenon
//  is sent as one JMWeatherBase rather than as loose scalars. Sending it whole
//  means the server never has to merge a half-applied edit - it either takes
//  the new configuration or keeps the old one.
//
//  It carries the master switch, which is global and always applied, and at
//  most one state, applied according to Op:
//
//    OP_TOGGLE  Enabled only. How the switch is sent with no state selected.
//    OP_SAVE    Enabled, plus State written under TargetName (new when the
//               name is empty; renamed when it differs from State.Name).
//    OP_REMOVE  Enabled, plus the state named TargetName deleted.
//
//  What the server keeps is its own validated copy - see
//  JMWeatherSerialize.NormalizeState - never the objects that arrived.
// =============================================================================
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
