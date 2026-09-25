class JMPlayerStatEntryData
{
	string Name;
	string Getter;

	void JMPlayerStatEntryData( string name, string getter )
	{
		Name   = name;
		Getter = getter;
	}
}

class JMPlayerStatRegistry
{
	protected static ref array<ref JMPlayerStatEntryData> s_Stats;

	static void Register( string name, string getter )
	{
		if ( !s_Stats )
			s_Stats = new array<ref JMPlayerStatEntryData>;

		s_Stats.Insert( new JMPlayerStatEntryData( name, getter ) );
	}

	static void Populate( Widget container )
	{
		if ( !s_Stats || !container )
			return;

		foreach ( JMPlayerStatEntryData stat : s_Stats )
		{
			UIActionManager.CreateText( container, stat.Name + ": Active", "" );
		}
	}
}
