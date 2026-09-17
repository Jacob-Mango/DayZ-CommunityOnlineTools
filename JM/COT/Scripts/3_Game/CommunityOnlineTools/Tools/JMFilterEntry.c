class JMFilterEntry
{
	string m_Id;
	string m_Label;
	string m_Icon;
	int m_Color;
	Class m_Target;
	string m_Callback;

	void JMFilterEntry( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "" )
	{
		m_Id = id;
		m_Label = label;
		m_Icon = icon;
		m_Color = color;
		m_Target = target;
		m_Callback = callback;
	}
}
