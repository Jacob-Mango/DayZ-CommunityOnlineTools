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

	bool Equals(JMTeleportRowData row, bool compareId = false, float tolerance = 0.1)
	{
		if (row.m_Name != m_Name)
			return false;

		if (row.m_Kind != m_Kind)
			return false;

		if (!Math.IsPointInCircle(row.m_Position, tolerance, m_Position))
			return false;

		if (compareId && row.m_MarkerId != m_MarkerId)
			return false;

		return true;
	}
}
