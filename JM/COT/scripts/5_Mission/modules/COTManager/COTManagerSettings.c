const string COT_MANAGER_SETTINGS_FILE = ROOT_COT_DIR + "COTManager.json";

class COTManagerSettings
{
	bool DebugMode;

	void COTManagerSettings()
	{
	}

	static ref COTManagerSettings Load()
	{
		ref COTManagerSettings settings = new COTManagerSettings();

		if ( FileExist( COT_MANAGER_SETTINGS_FILE ) )
		{
			JsonFileLoader<COTManagerSettings>.JsonLoadFile( COT_MANAGER_SETTINGS_FILE, settings );
		} else {
			settings.Defaults();
			settings.Save();
		}

		return settings;
	}

	void RegisterSettings()
	{
		COT_DEBUG_MODE = DebugMode;
	}

	void Save()
	{
		JsonFileLoader<COTManagerSettings>.JsonSaveFile( COT_MANAGER_SETTINGS_FILE, this );

		RegisterSettings();
	}

	void Defaults()
	{
		DebugMode = false;
	}
}