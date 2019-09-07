const string VEHICLE_SPAWNER_FOLDER = JMConstants.DIR_COT + "Vehicles\\";

class JMVehicleSpawnerSerialize
{
	[NonSerialized()]
	string m_FileName;

	string VehicleName;
	string DisplayName;

	ref array< string > Parts = new array< string >;

	static ref JMVehicleSpawnerSerialize Load( string file )
	{
		ref JMVehicleSpawnerSerialize settings = new JMVehicleSpawnerSerialize;
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