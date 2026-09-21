//! One row a mod adds to a COT filter menu - see JMFilterRegistry.
class JMFilterEntry
{
	string m_Id;
	string m_Label;
	string m_Icon;
	int m_Color;
	Class m_Target;

	//! `void Fn( string id )` - called when the row is clicked.
	string m_Callback;

	//! Optional `bool Fn()` - when set the row is a checkbox that shows its current state.
	string m_StateCallback;

	void JMFilterEntry( string id, string label, string icon = "", int color = 0, Class target = null, string callback = "", string stateCallback = "" )
	{
		m_Id = id;
		m_Label = label;
		m_Icon = icon;
		m_Color = color;
		m_Target = target;
		m_Callback = callback;
		m_StateCallback = stateCallback;
	}

	//! Current state of a checkbox row (always false for a plain row).
	bool IsOn()
	{
		if ( !m_Target || m_StateCallback == "" )
			return false;

		bool on = false;
		g_Script.CallFunction( m_Target, m_StateCallback, on, null );
		return on;
	}
}
