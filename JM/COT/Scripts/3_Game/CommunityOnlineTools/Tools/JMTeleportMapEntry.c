//! One entry shared by JMTeleportForm's list-row builders and map-marker
//! refreshers for the same data source (heli crashes, toxic zones, Expansion
//! map markers), so both walk the source exactly once instead of twice.
class JMTeleportMapEntry
{
	string m_Id;
	vector m_Position;
	string m_Label;
	string m_SubLabel;
	string m_Icon;
	int m_Color;

	void JMTeleportMapEntry( string id, vector position, string label, string subLabel = "", string icon = "", int color = 0 )
	{
		m_Id = id;
		m_Position = position;
		m_Label = label;
		m_SubLabel = subLabel;
		m_Icon = icon;
		m_Color = color;
	}
}
