#ifdef DIAG_DEVELOPER
// Example: persisting module settings as JSON in the server profile
// (same shape as COT's own JMESPSerialize: static Load(), instance Save()).
class JMCustomExampleSettings: Managed
{
	static const string FILE = JMConstants.DIR_COT + "CustomExample.json";
	int ActionsRun;

	static JMCustomExampleSettings Load()
	{
		JMCustomExampleSettings settings = new JMCustomExampleSettings();

		JMJsonFile<JMCustomExampleSettings>.Load( FILE, settings );

		return settings;
	}

	void Save()
	{
		JMJsonFile<JMCustomExampleSettings>.Save( FILE, this );
	}
}
#endif
