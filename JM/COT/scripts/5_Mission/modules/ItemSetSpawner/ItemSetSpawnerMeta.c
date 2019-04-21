class ItemSetSpawnerMeta
{
	autoptr ref array< string > ItemSets = new ref array< string >;

	static ref ItemSetSpawnerMeta DeriveFromSettings( ref ItemSetSettings settings )
	{
		ref ItemSetSpawnerMeta meta = new ref ItemSetSpawnerMeta;

		for ( int j = 0; j < settings.ItemSets.Count(); j++ )
		{
			meta.ItemSets.Insert( settings.ItemSets.GetKey( j ) );
		}

		return meta;
	}
}