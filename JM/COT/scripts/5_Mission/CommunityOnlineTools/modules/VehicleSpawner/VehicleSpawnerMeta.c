class VehicleSpawnerMeta
{
	autoptr ref array< string > Vehicles = new ref array< string >;

	static ref VehicleSpawnerMeta DeriveFromSettings( ref VehicleSpawnerSettings settings )
	{
		ref VehicleSpawnerMeta meta = new ref VehicleSpawnerMeta;

		for ( int i = 0; i < settings.Vehicles.Count(); i++ )
		{
			meta.Vehicles.Insert( settings.Vehicles.GetKey( i ) );
		}

		return meta;
	}
}