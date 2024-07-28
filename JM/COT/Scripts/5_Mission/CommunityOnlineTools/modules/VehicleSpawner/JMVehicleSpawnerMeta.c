class JMVehicleSpawnerMeta
{
	autoptr ref array< string > Vehicles;
	autoptr ref array< string > VehiclesName;

	private void JMVehicleSpawnerMeta()
	{
		Vehicles = new array< string >;
		VehiclesName = new array< string >;
	}

	static JMVehicleSpawnerMeta Create()
	{
		return new JMVehicleSpawnerMeta();
	}

	static JMVehicleSpawnerMeta DeriveFromSettings( JMVehicleSpawnerSettings settings )
	{
		JMVehicleSpawnerMeta meta = new JMVehicleSpawnerMeta;

		for ( int i = 0; i < settings.Vehicles.Count(); i++ )
		{
			string classname = settings.Vehicles.GetKey(i);
			JMVehicleSpawnerSerialize VSserialize = settings.Vehicles.Get(classname);
			meta.Vehicles.Insert( classname );
			meta.VehiclesName.Insert( VSserialize.DisplayName );
		}

		return meta;
	}
};
