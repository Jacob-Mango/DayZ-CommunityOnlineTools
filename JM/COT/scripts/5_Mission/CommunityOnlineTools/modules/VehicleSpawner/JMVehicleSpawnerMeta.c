class JMVehicleSpawnerMeta
{
	autoptr ref array< string > Vehicles;

	private void JMVehicleSpawnerMeta()
	{
		Vehicles = new array< string >;
	}

	void ~JMVehicleSpawnerMeta()
	{
		delete Vehicles;
	}

	static ref JMVehicleSpawnerMeta DeriveFromSettings( ref JMVehicleSpawnerSettings settings )
	{
		ref JMVehicleSpawnerMeta meta = new JMVehicleSpawnerMeta;

		for ( int i = 0; i < settings.Vehicles.Count(); i++ )
		{
			meta.Vehicles.Insert( settings.Vehicles.GetKey( i ) );
		}

		return meta;
	}
}