class JMLoadoutMeta
{
	ref array< string > Loadouts;

	private void JMLoadoutMeta()
	{
		Loadouts = new array< string >;
	}

	static JMLoadoutMeta Create()
	{
		return new JMLoadoutMeta();
	}

	static JMLoadoutMeta DeriveFromSettings( array< string > names )
	{
		JMLoadoutMeta meta = new JMLoadoutMeta;
		foreach(string name: names)
			meta.Loadouts.Insert( name );

		return meta;
	}
};
