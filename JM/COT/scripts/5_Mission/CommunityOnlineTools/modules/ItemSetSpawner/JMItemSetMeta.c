class JMItemSetMeta
{
	autoptr ref array< string > ItemSets = new array< string >;

	static ref JMItemSetMeta DeriveFromSettings( ref JMItemSetSettings settings )
	{
		ref JMItemSetMeta meta = new JMItemSetMeta;

		for ( int j = 0; j < settings.ItemSets.Count(); j++ )
		{
			meta.ItemSets.Insert( settings.ItemSets.GetKey( j ) );
		}

		return meta;
	}
}