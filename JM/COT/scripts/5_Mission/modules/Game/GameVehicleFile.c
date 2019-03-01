const string GAME_VEHICLE_FOLDER = ROOT_COT_DIR + "GameModule_Vehicles";

class GameVehicleFile
{
    string Name;

    ref array< string > VehicleParts = new ref array< string >;

    static ref GameVehicleFile Load( string file )
    {
        ref GameVehicleFile settings = new ref GameVehicleFile;

        JsonFileLoader< GameVehicleFile >.JsonLoadFile( GAME_VEHICLE_FOLDER + "/" + file, settings );

        return settings;
    }

    void Save()
    {
        JsonFileLoader< GameVehicleFile >.JsonSaveFile( GAME_VEHICLE_FOLDER + "/" + Name, this );
    }

    void Defaults()
    {
    }

}