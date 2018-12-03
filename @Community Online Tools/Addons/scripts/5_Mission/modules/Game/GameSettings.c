const string GAME_SETTINGS_FILE = ROOT_COT_DIR + "GameSettings.json";

class GameSettings
{
    autoptr map< string, ref array< string > > Vehicles = new map< string, ref array< string > >;

    static ref GameSettings Load()
    {
        ref GameSettings settings = new GameSettings();

        if ( FileExist( GAME_SETTINGS_FILE ) )
        {
            JsonFileLoader<GameSettings>.JsonLoadFile( GAME_SETTINGS_FILE, settings );
        } else {
            settings.Defaults();
            settings.Save();
        }

        return settings;
    }

    void Save()
    {
        JsonFileLoader<GameSettings>.JsonSaveFile( GAME_SETTINGS_FILE, this );
    }

    void Defaults()
    {
        // DefaultVan();
        // DefaultSedan();
        DefaultHatchback();
        // DefaultV3SCargo();
        // DefaultV3S();
    }

    void DefaultVan()
    {
        ref array< string> attArr = new ref array< string>;

        attArr.Insert("CivVanDoors_TrumpUp");
        attArr.Insert("CivVanDoors_BackRight");
        attArr.Insert("CivVanDoors_TrumpDown");
        attArr.Insert("CivVanDoors_CoDriver");
        attArr.Insert("CivVanTrunk");
        attArr.Insert("CivVanWheel");
        attArr.Insert("CivVanWheel");
        attArr.Insert("CivVanWheel");
        attArr.Insert("CivVanWheel");
        attArr.Insert("CarBattery");
        attArr.Insert("CarRadiator");
        attArr.Insert("EngineBelt");
        attArr.Insert("SparkPlug");

        Vehicles.Insert( "CivilianVan", attArr );
    }

    void DefaultSedan()
    {
        ref array< string> attArr = new ref array< string>;

        attArr.Insert("CivSedanHood");
        attArr.Insert("CivSedanTrunk");
        attArr.Insert("CivSedanDoors_Driver");
        attArr.Insert("CivSedanDoors_BackRight");
        attArr.Insert("CivSedanDoors_BackLeft");
        attArr.Insert("CivSedanDoors_CoDriver");
        attArr.Insert("CivSedanWheel");
        attArr.Insert("CivSedanWheel");
        attArr.Insert("CivSedanWheel");
        attArr.Insert("CivSedanWheel");
        attArr.Insert("CarBattery");
        attArr.Insert("CarRadiator");
        attArr.Insert("EngineBelt");
        attArr.Insert("SparkPlug");

        Vehicles.Insert( "CivilianSedan", attArr );
    }

    void DefaultHatchback()
    {
        ref array< string> attArr = new ref array< string>;

        attArr.Insert("HatchbackHood");
        attArr.Insert("HatchbackTrunk");
        attArr.Insert("HatchbackDoors_Driver");
        attArr.Insert("HatchbackDoors_CoDriver");
        attArr.Insert("HatchbackWheel");
        attArr.Insert("HatchbackWheel");
        attArr.Insert("HatchbackWheel");
        attArr.Insert("HatchbackWheel");
        attArr.Insert("CarBattery");
        attArr.Insert("CarRadiator");
        attArr.Insert("EngineBelt");
        attArr.Insert("SparkPlug");

        Vehicles.Insert( "OffroadHatchback", attArr );
    }

    void DefaultV3SCargo()
    {
        ref array< string> attArr = new ref array< string>;

        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("TruckBattery");
        attArr.Insert("TruckRadiator");
        attArr.Insert("EngineBelt");
        attArr.Insert("GlowPlug");
        attArr.Insert("V3SHood");
        attArr.Insert("V3SDoors_Driver_Blue");
        attArr.Insert("V3SDoors_CoDriver_Blue");

        Vehicles.Insert( "V3S_Cargo_Blue", attArr );
    }

    void DefaultV3S()
    {
        ref array< string> attArr = new ref array< string>;

        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheel");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("V3SWheelDouble");
        attArr.Insert("TruckBattery");
        attArr.Insert("TruckRadiator");
        attArr.Insert("EngineBelt");
        attArr.Insert("GlowPlug");
        attArr.Insert("V3SHood");
        attArr.Insert("V3SDoors_Driver_Blue");
        attArr.Insert("V3SDoors_CoDriver_Blue");

        Vehicles.Insert( "V3S_Chassis_Blue", attArr );
    }
}