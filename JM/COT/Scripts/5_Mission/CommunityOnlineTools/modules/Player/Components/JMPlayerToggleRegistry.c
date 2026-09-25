class JMPlayerToggleEntryData
{
	string Name;
	string TargetFunc;
	string Permission;

	void JMPlayerToggleEntryData( string name, string targetFunc, string permission = "" )
	{
		Name       = name;
		TargetFunc = targetFunc;
		Permission = permission;
	}
}

class JMPlayerToggleRegistry
{
	protected static ref array<ref JMPlayerToggleEntryData> s_Toggles;

	static void Register( string name, string targetFunc, string permission = "" )
	{
		if ( !s_Toggles )
			s_Toggles = new array<ref JMPlayerToggleEntryData>;

		s_Toggles.Insert( new JMPlayerToggleEntryData( name, targetFunc, permission ) );
	}

	static void Populate( Widget wrapper, JMFormBase target )
	{
		if ( !s_Toggles || !wrapper )
			return;

		foreach ( JMPlayerToggleEntryData t : s_Toggles )
		{
			UIActionToggle toggle = UIActionManager.CreateToggle( wrapper, t.Name, target, t.TargetFunc );
			if ( t.Permission != "" && toggle )
				target.BindPermission( toggle, t.Permission );
		}
	}
}
