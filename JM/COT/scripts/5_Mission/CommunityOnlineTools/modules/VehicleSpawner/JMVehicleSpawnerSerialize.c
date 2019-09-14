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

		JsonFileLoader< JMVehicleSpawnerSerialize >.JsonLoadFile( JMConstants.DIR_VEHICLES + settings.m_FileName + JMConstants.EXT_VEHICLE, settings );

		return settings;
	}

	void Save()
	{
		JsonFileLoader< JMVehicleSpawnerSerialize >.JsonSaveFile( JMConstants.DIR_VEHICLES + m_FileName + JMConstants.EXT_VEHICLE, this );
	}

	void Defaults()
	{
	}
}