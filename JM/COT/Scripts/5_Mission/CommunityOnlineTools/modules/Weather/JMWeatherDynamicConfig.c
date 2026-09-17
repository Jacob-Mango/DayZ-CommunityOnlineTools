// =============================================================================
//  JMWeatherDynamicConfig
//
//  One dynamic weather edit as a single RPC payload, the same way a phenomenon
//  is sent as one JMWeatherBase rather than as loose scalars. Sending it whole
//  means the server never has to merge a half-applied edit - it either takes
//  the new configuration or keeps the old one.
//
//  It carries two things that are edited in two different places:
//
//    - Enabled, the master switch, which is global and always applied.
//    - One preset's dynamic block, applied only when PresetName names a preset
//      that actually exists. An empty PresetName is how the master switch is
//      sent on its own, with no preset selected to speak for.
// =============================================================================
class JMWeatherDynamicConfig
{
	bool Enabled;

	//! "" = update the master switch only.
	string PresetName;

	bool InRotation;

	int DurationMin;
	int DurationMax;
	int TransitionMin;
	int TransitionMax;

	autoptr array< ref JMWeatherNextState > NextStates;

	void JMWeatherDynamicConfig()
	{
		Enabled    = false;
		PresetName = "";
		InRotation = true;

		DurationMin   = JMWeatherSerialize.DYNAMIC_DEFAULT_DURATION_MIN;
		DurationMax   = JMWeatherSerialize.DYNAMIC_DEFAULT_DURATION_MAX;
		TransitionMin = JMWeatherSerialize.DYNAMIC_DEFAULT_TRANSITION_MIN;
		TransitionMax = JMWeatherSerialize.DYNAMIC_DEFAULT_TRANSITION_MAX;

		NextStates = new array< ref JMWeatherNextState >;
	}
}
