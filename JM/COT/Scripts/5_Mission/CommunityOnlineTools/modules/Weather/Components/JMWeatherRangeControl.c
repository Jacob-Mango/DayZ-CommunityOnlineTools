//! One weather value that can be a single number OR a span the server rolls
//! between each time it is applied.
//!
//! It is a UIActionSliderRange - the same control the object spawner uses for
//! quantity and health - working in the units the weather stores (a 0-1
//! fraction, radians, m/s) while the bar shows the units a person reads (percent,
//! degrees, m/s). The conversion is a plain scale and offset:
//!
//!     shown = stored * scale + offset
//!
//! In single mode there is one handle and Low() == High(), so a caller that only
//! wants "the value" reads Low() and never has to ask which mode it is in; Hi()
//! is what goes in the "Hi" twin of a stored field and is 0 - "not a range" -
//! whenever the two ends coincide.
class JMWeatherRangeControl
{
	protected UIActionSliderRange m_Slider;
	protected float m_Scale;
	protected float m_Offset;

	//! `min` / `max` / `step` are in SHOWN units.
	static JMWeatherRangeControl Create( Widget parent, string label, float min, float max, float step, string format, float scale, float offset, Class instance, string callback )
	{
		JMWeatherRangeControl control = new JMWeatherRangeControl;

		control.m_Scale  = scale;
		control.m_Offset = offset;

		control.m_Slider = UIActionManager.CreateSliderRange( parent, label, min, max, instance, callback );
		control.m_Slider.SetFormat( format );
		control.m_Slider.SetStep( step );

		//! A plain value until somebody asks for a range - see JMWeatherRangeToggle.
		control.m_Slider.SetSingle( true );

		return control;
	}

	UIActionSliderRange GetAction()
	{
		return m_Slider;
	}

	void SetTooltip( string text )
	{
		if ( m_Slider )
			m_Slider.SetTooltip( text );
	}

	bool IsSingle()
	{
		return m_Slider.IsSingle();
	}

	//! Collapsing pins the span onto its top end - the bar remembers no second value.
	void SetSingle( bool single )
	{
		m_Slider.SetSingle( single );
	}

	//! A span, in STORED units.
	void Set( float low, float high )
	{
		m_Slider.SetRange( ToShown( low ), ToShown( high ) );
	}

	//! One exact value, in STORED units.
	void SetValue( float value )
	{
		Set( value, value );
	}

	float Low()
	{
		if ( m_Slider.IsSingle() )
			return ToStored( m_Slider.GetRangeHigh() );

		return ToStored( m_Slider.GetRangeLow() );
	}

	float High()
	{
		return ToStored( m_Slider.GetRangeHigh() );
	}

	//! The value for a "Hi" twin field: the top of the span, or 0 when it is only a
	//! single value (0 is what an absent field loads as, so the two agree).
	float Hi()
	{
		if ( m_Slider.IsSingle() )
			return 0;

		if ( High() > Low() )
		{
			//! 0 means "no span" in the file, so a span that really ends on 0 (a wind
			//! direction reaching straight ahead) is stored a hair above it.
			if ( High() == 0 )
				return 0.0001;

			return High();
		}

		return 0;
	}

	protected float ToShown( float stored )
	{
		return ( stored * m_Scale ) + m_Offset;
	}

	protected float ToStored( float shown )
	{
		return ( shown - m_Offset ) / m_Scale;
	}
}
