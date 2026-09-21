// =============================================================================
//  JMWeatherRuntime - where the dynamic weather machine is RIGHT NOW
//
//  Persisted on its own, apart from Weather.json. The authored configuration
//  (presets, states, phases) is rewritten only when an admin edits it; this is
//  rewritten on every phase change and on a heartbeat, and would otherwise turn
//  each of those into a rewrite of every preset the server has.
//
//  Whether the machine is enabled is NOT here - that is configuration and stays
//  in Weather.json (JMWeatherSerialize.DynamicEnabled).
// =============================================================================
class JMWeatherRuntime
{
	string State;
	string Phase;

	//! Seconds the current phase holds for in total, and how many are left as of
	//! the last snapshot. A restart resumes with HoldRemaining; time the server
	//! spent down does not count against it.
	float HoldTotal;
	float HoldRemaining;

	void JMWeatherRuntime()
	{
		Clear();
	}

	void Clear()
	{
		State         = "";
		Phase         = "";
		HoldTotal     = 0;
		HoldRemaining = 0;
	}

	bool IsSet()
	{
		return State != "" && Phase != "";
	}

	static JMWeatherRuntime Load()
	{
		JMWeatherRuntime runtime = new JMWeatherRuntime;

		JMJsonFile<JMWeatherRuntime>.Load( JMConstants.FILE_WEATHER_STATE, runtime );

		return runtime;
	}

	void Save()
	{
		JMJsonFile<JMWeatherRuntime>.Save( JMConstants.FILE_WEATHER_STATE, this );
	}
}
