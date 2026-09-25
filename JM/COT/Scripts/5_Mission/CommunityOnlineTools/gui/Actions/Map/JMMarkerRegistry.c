class JMMarkerEntryData
{
	string Layer;
	string Id;
	vector Position;
	string Name;
	int Color;
	string IconPath;

	void JMMarkerEntryData( string layer, string id, vector pos, string name, string icon, int color )
	{
		Layer = layer;
		Id    = id;
		Position = pos;
		Name  = name;
		IconPath = icon;
		Color = color;
	}
}

class JMMarkerRegistry
{
	protected static ref array<ref JMMarkerEntryData> s_Markers;

	static void RegisterMarker( string layer, string id, vector pos, string name, string icon, int color )
	{
		if ( !s_Markers )
			s_Markers = new array<ref JMMarkerEntryData>;

		s_Markers.Insert( new JMMarkerEntryData( layer, id, pos, name, icon, color ) );
	}

	static void PopulateMap( UIActionMap actionMap )
	{
		if ( !s_Markers || !actionMap )
			return;

		foreach ( JMMarkerEntryData m : s_Markers )
		{
			actionMap.AddMarker( m.Id, m.Position, m.Name, m.Color, m.IconPath, m.Layer );
		}
	}
}
