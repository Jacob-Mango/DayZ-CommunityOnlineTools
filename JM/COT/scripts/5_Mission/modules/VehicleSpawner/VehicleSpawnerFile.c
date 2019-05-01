const string VEHICLE_SPAWNER_FOLDER = ROOT_COT_DIR + "Vehicles/";

class VehicleSpawnerFile
{
	[NonSerialized()]
	string m_FileName;

	string VehicleName;
	string DisplayName;

	ref array< string > Parts = new ref array< string >;

	static ref VehicleSpawnerFile Load( string file )
	{
		ref VehicleSpawnerFile settings = new ref VehicleSpawnerFile;
		settings.m_FileName = file;

		JsonFileLoader< VehicleSpawnerFile >.JsonLoadFile( VEHICLE_SPAWNER_FOLDER + settings.m_FileName + ".json", settings );

		return settings;
	}

	void Save()
	{
		JsonFileLoader< VehicleSpawnerFile >.JsonSaveFile( VEHICLE_SPAWNER_FOLDER + m_FileName + ".json", this );
	}

	void Defaults()
	{
	}
}