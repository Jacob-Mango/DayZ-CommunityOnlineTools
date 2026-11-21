class JMTeleportRowData
{
	string m_Name;
	string m_Kind;
	vector m_Position;
	string m_MarkerId;

	void JMTeleportRowData( string name, string kind, vector pos, string markerId = "" )
	{
		m_Name = name;
		m_Kind = kind;
		m_Position = pos;
		m_MarkerId = markerId;
	}
}
