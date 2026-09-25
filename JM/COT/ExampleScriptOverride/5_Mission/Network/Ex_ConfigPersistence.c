#ifdef JM_CommunityOnlineTools
// Example: persisting module settings as JSON in the server profile
// (same shape as COT's own JMESPSerialize: static Load(), instance Save()).
class JMCustomExampleSettings: Managed
{
	static const string FILE = JMConstants.DIR_COT + "CustomExample.json";
	int ActionsRun;

	static JMCustomExampleSettings Load()
	{
		JMCustomExampleSettings settings = new JMCustomExampleSettings();

		if ( FileExist( FILE ) )
			JsonFileLoader<JMCustomExampleSettings>.JsonLoadFile( FILE, settings );

		return settings;
	}

	void Save()
	{
		MakeDirectory( JMConstants.DIR_COT );
		JsonFileLoader<JMCustomExampleSettings>.JsonSaveFile( FILE, this );
	}
}
#endif
