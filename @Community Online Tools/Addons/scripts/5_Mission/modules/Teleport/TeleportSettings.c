const string TELEPORT_SETTINGS_FILE = ROOT_COT_DIR + "TeleportSettings.json";

class TeleportSettings
{
    autoptr map< string, vector > Positions = new map< string, vector >;

    static ref TeleportSettings Load()
    {
        ref TeleportSettings settings = new TeleportSettings();

        if ( FileExist( TELEPORT_SETTINGS_FILE ) )
        {
            JsonFileLoader<TeleportSettings>.JsonLoadFile( TELEPORT_SETTINGS_FILE, settings );
        } else {
            settings.Defaults();
            settings.Save();
        }

        return settings;
    }

    void Save()
    {
        JsonFileLoader<TeleportSettings>.JsonSaveFile( TELEPORT_SETTINGS_FILE, this );
    }

    void Defaults()
    {
        Positions.Insert( "Severograd", "8428.0 0.0 12767.1" );
        Positions.Insert( "Krasnostav", "11172.0 0.0 12314.1" );
        Positions.Insert( "Mogilevka", "7583.0 0.0 5159.4" );
        Positions.Insert( "Stary Sobor", "6072.0 0.0 7852.5" );
        Positions.Insert( "Msta", "11207.0 0.0 5380.54" );
        Positions.Insert( "Vybor", "3729.51 0.0 8935.56" );
        Positions.Insert( "Gorka", "9787.1 0.0 8767.19" );
        Positions.Insert( "Solnicni", "13393.1 0.0 6159.8" );
        Positions.Insert( "Airport Vybor", "4828.9 0.0 10219.5" );
        Positions.Insert( "Airport Balota", "4791.53 0.0 2557.47" );
        Positions.Insert( "Airport Krasnostav", "12004.3 0.0 12655.3" );
        Positions.Insert( "Chernogorsk Center", "6893.07 0.0 2618.13" );
        Positions.Insert( "Chernogorsk West", "6546.03 0.0 2317.56" );
        Positions.Insert( "Chernogorsk East", "7105.76 0.0 2699.53" );
        Positions.Insert( "Elektrozavodsk Center", "10459.8 0.0 2322.72" );
        Positions.Insert( "Elektrozavodsk East", "10414.5 0.0 2569.42" );
        Positions.Insert( "Berezino Center", "12296.9 0.0 9470.51" );
        Positions.Insert( "Berezino West", "12035.4 0.0 9143.49" );
        Positions.Insert( "Berezino East", "12932.3 0.0 10172.7" );
        Positions.Insert( "Svetlojarsk Center", "13835.3 0.0 13202.3" );
        Positions.Insert( "Zelenogorsk Center", "2660.99 0.0 5299.28" );
        Positions.Insert( "Zelenogorsk West", "2489.45 0.0 5080.41" );
    }
}