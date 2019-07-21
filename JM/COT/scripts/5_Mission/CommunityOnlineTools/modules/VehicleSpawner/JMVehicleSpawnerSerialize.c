const string VEHICLE_SPAWNER_FOLDER = ROOT_COT_DIR + "Vehicles\\";

class JMVehicleSpawnerSerialize
{
	[NonSerialized()]
	string m_FileName;

	string VehicleName;
	string DisplayName;

	ref array< string > Parts = new ref array< string >;

	static ref JMVehicleSpawnerSerialize Load( string file )
	{
		ref JMVehicleSpawnerSerialize settings = new ref JMVehicleSpawnerSerialize;
		settings.m_FileName = file;

		JsonFileLoader< JMVehicleSpawnerSerialize >.JsonLoadFile( VEHICLE_SPAWNER_FOLDER + settings.m_FileName + ".json", settings );

		return settings;
	}

	void Save()
	{
		JsonFileLoader< JMVehicleSpawnerSerialize >.JsonSaveFile( VEHICLE_SPAWNER_FOLDER + m_FileName + ".json", this );
	}

	void Defaults()
	{
	}
}