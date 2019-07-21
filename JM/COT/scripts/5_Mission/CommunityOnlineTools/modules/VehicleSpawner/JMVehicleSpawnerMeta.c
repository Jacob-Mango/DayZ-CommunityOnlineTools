class JMVehicleSpawnerMeta
{
	autoptr ref array< string > Vehicles = new ref array< string >;

	static ref JMVehicleSpawnerMeta DeriveFromSettings( ref JMVehicleSpawnerSettings settings )
	{
		ref JMVehicleSpawnerMeta meta = new ref JMVehicleSpawnerMeta;

		for ( int i = 0; i < settings.Vehicles.Count(); i++ )
		{
			meta.Vehicles.Insert( settings.Vehicles.GetKey( i ) );
		}

		return meta;
	}
}