class JMItemSetMeta
{
	ref array< string > ItemSets;

	private void JMItemSetMeta()
	{
		ItemSets = new array< string >;
	}

	void ~JMItemSetMeta()
	{
		delete ItemSets;
	}

	static JMItemSetMeta DeriveFromSettings( JMItemSetSettings settings )
	{
		JMItemSetMeta meta = new JMItemSetMeta;

		for ( int j = 0; j < settings.ItemSets.Count(); j++ )
		{
			meta.ItemSets.Insert( settings.ItemSets.GetKey( j ) );
		}

		return meta;
	}
};
