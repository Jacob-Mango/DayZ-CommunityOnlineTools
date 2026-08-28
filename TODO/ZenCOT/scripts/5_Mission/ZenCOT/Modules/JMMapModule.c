class JMMapTempMark
{
	vector 		m_Pos;
	string 		m_Label;
	int    		m_Color 	= ARGB(255, 0, 200, 255); // teal
	string 		m_Icon  	= JM_COT_ICON_DOT + ".paa";
}

modded class JMMapModule 
{
	protected ref array<ref JMMapTempMark> m_TempItemMarks;

	void SetTempItemMarks(array<ref JMMapTempMark> marks, string title = "")
	{
		m_TempItemMarks = marks;
	}

	array<ref JMMapTempMark> GetTempItemMarks()
	{
		return m_TempItemMarks;
	}
}