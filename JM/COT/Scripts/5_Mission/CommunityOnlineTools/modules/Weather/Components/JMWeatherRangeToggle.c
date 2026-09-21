//! The button in a weather card's title bar that switches its sliders between one
//! value and a min/max span - the same control, same icon and same reasoning as
//! the object spawner's, one per card.
//!
//! It lives in the title bar rather than in a row of its own: it is a mode for
//! the whole card, not one more property of the weather, and a row would have
//! put it in the same list as the values it governs.
//!
//! Off by default when editing the live world - rolling between two bounds is the
//! advanced case, and a pair of handles reads as a range whether or not the admin
//! meant one. The form turns every toggle on while a preset is being edited,
//! where a span is the normal thing to want.
class JMWeatherRangeToggle
{
	protected JMWeatherForm m_Form;
	protected int m_Section;

	protected UIActionImageButton m_Button;
	protected bool m_Ranges;

	protected autoptr array< ref JMWeatherRangeControl > m_Controls;

	void JMWeatherRangeToggle( JMWeatherForm form, UIActionCard card, int section )
	{
		m_Form     = form;
		m_Section  = section;
		m_Controls = new array< ref JMWeatherRangeControl >;

		m_Button = card.AddCardHeaderAction( JMConstants.Lucide( "arrow-left-right" ), this, "" );

		if ( m_Button )
			m_Button.SetOnClick( this, "Click_Toggle" );

		Paint();
	}

	//! Put a control under this toggle's charge, in whatever mode it is in now.
	void Add( JMWeatherRangeControl control )
	{
		m_Controls.Insert( control );

		control.SetSingle( !m_Ranges );
	}

	bool IsRanges()
	{
		return m_Ranges;
	}

	void SetRanges( bool on )
	{
		if ( m_Ranges == on )
			return;

		m_Ranges = on;

		for ( int i = 0; i < m_Controls.Count(); i++ )
			m_Controls[i].SetSingle( !m_Ranges );

		Paint();
	}

	//! Switching mode changes what an Apply writes, so it counts as an edit -
	//! otherwise the live poll would put the world's single values straight back.
	void Click_Toggle( UIActionBase action )
	{
		SetRanges( !m_Ranges );

		if ( m_Form )
			m_Form.SetSectionDirty( m_Section );
	}

	void UpdateState( string permission )
	{
		m_Form.UpdateActionState( m_Button, permission );
	}

	//! Tint and label the button for the mode it is in.
	protected void Paint()
	{
		if ( !m_Button )
			return;

		if ( m_Ranges )
		{
			m_Button.SetColor( JMTheme.ACCENT );
			m_Button.SetTooltip( "#STR_COT_WEATHER_RANGE_ON" );
		}
		else
		{
			m_Button.SetColor( JMTheme.BUTTON_FILL );
			m_Button.SetTooltip( "#STR_COT_WEATHER_RANGE_OFF" );
		}
	}
}
