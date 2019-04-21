const string VEHICLE_SPAWNER_FOLDER = ROOT_COT_DIR + "Vehicles";

class VehicleSpawnerFile
{
	string VehicleName;
	string DisplayName;

	ref array< string > Parts = new ref array< string >;

	static ref VehicleSpawnerFile Load( string file )
	{
		ref VehicleSpawnerFile settings = new ref VehicleSpawnerFile;

		JsonFileLoader< VehicleSpawnerFile >.JsonLoadFile( VEHICLE_SPAWNER_FOLDER + "/" + file + ".json", settings );

		return settings;
	}

	void Save()
	{
		JsonFileLoader< VehicleSpawnerFile >.JsonSaveFile( VEHICLE_SPAWNER_FOLDER + "/" + VehicleName + ".json", this );
	}

	void Defaults()
	{
	}
}