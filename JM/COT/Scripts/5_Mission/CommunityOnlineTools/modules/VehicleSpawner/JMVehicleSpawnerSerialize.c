class JMVehicleSpawnerSerialize : Managed
{
	[NonSerialized()]
	string m_FileName;

	string VehicleName;
	string DisplayName;

	ref array< string > Parts;
	
	void JMVehicleSpawnerSerialize()
	{
		Parts = new array< string >;
	}

	void ~JMVehicleSpawnerSerialize()
	{
		delete Parts;
	}

	static JMVehicleSpawnerSerialize Create()
	{
		return new JMVehicleSpawnerSerialize;
	}

	static JMVehicleSpawnerSerialize Load( string file )
	{
		JMVehicleSpawnerSerialize settings = new JMVehicleSpawnerSerialize;
		settings.m_FileName = file;

		JsonFileLoader< JMVehicleSpawnerSerialize >.JsonLoadFile( JMConstants.DIR_VEHICLES + settings.m_FileName + JMConstants.EXT_VEHICLE, settings );

		if ( !settings )
		{
			Print("[Community Online Tools] Vehicle Spawner Module - Cannot read the file "+ file);
			return NULL;
		}

		return settings;
	}

	void Save()
	{
		JsonFileLoader< JMVehicleSpawnerSerialize >.JsonSaveFile( JMConstants.DIR_VEHICLES + m_FileName + JMConstants.EXT_VEHICLE, this );
	}

	void Defaults()
	{
	}
};
