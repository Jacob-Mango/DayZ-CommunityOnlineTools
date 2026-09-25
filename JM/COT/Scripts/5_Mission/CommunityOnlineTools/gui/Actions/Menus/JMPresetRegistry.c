class JMPresetEntryData
{
	string Category;
	string Name;
	ref TStringArray Items;

	void JMPresetEntryData( string category, string name, TStringArray items )
	{
		Category = category;
		Name     = name;
		Items    = items;
	}
}

class JMPresetRegistry
{
	protected static ref array<ref JMPresetEntryData> s_Presets;

	static void Register( string category, string name, TStringArray items )
	{
		if ( !s_Presets )
			s_Presets = new array<ref JMPresetEntryData>;

		s_Presets.Insert( new JMPresetEntryData( category, name, items ) );
	}

	static array<ref JMPresetEntryData> GetPresets()
	{
		if ( !s_Presets )
			s_Presets = new array<ref JMPresetEntryData>;

		return s_Presets;
	}
}
