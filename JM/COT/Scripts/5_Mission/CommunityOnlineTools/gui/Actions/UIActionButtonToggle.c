class UIActionButtonToggle: UIActionButton 
{
	bool m_IsToggled;

	string m_TextOff;
	string m_TextOn;

	void SetButtonToggle( string textOff, string textOn )
	{
		m_TextOff = textOff;
		m_TextOn = textOn;
		
		SetButton(textOff);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		bool ret;

		if ( w == m_Button )
		{
			m_IsToggled = !m_IsToggled;

			if (m_IsToggled)
				SetButton(m_TextOn);
			else
				SetButton(m_TextOff);

			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}

	bool IsToggled()
	{
		return m_IsToggled;
	}

	void SetToggle(bool state)
	{
		m_IsToggled = state;

		if (m_IsToggled)
			SetButton(m_TextOn);
		else
			SetButton(m_TextOff);
	}
};
