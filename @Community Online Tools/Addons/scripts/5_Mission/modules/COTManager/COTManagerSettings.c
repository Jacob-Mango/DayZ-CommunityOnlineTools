const string COT_SETTINGS_FILE = ROOT_COT_DIR + "CommunityOnlineTools.json";

class COTManagerSettings
{
    void COTManagerSettings()
    {
    }

    static ref COTManagerSettings Load()
    {
        ref COTManagerSettings settings = new COTManagerSettings();

        if ( FileExist( COT_SETTINGS_FILE ) )
        {
            JsonFileLoader<COTManagerSettings>.JsonLoadFile( COT_SETTINGS_FILE, settings );
        } else {
            settings.Defaults();
            settings.Save();
        }

        return settings;
    }

    void Save()
    {
        JsonFileLoader<COTManagerSettings>.JsonSaveFile( COT_SETTINGS_FILE, this );
    }

    void Defaults()
    {
    }
}