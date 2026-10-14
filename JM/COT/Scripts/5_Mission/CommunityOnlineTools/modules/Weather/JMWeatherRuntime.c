// JMWeatherRuntime - where the dynamic weather machine is RIGHT NOW. Persisted apart from
// Weather.json: configuration is rewritten only on admin edits, while this is rewritten on every
// phase change and heartbeat. Whether the machine is enabled stays in Weather.json (DynamicEnabled).
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
