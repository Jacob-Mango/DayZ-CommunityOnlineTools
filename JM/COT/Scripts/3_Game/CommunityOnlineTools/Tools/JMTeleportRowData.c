//! Why does this exist? It's essentially a duplicate of JMTeleportLocation.
//! FIXME: Merge with JMTeleportLocation and remove this
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

	bool Equals(JMTeleportRowData row, bool compareId = false)
	{
		if (row.m_Name != m_Name)
			return false;

		if (row.m_Kind != m_Kind)
			return false;

		if (row.m_Position[0] != m_Position[0] || row.m_Position[1] != m_Position[1] || row.m_Position[2] != m_Position[2])
			return false;

		if (compareId && row.m_MarkerId != m_MarkerId)
			return false;

		return true;
	}
}
