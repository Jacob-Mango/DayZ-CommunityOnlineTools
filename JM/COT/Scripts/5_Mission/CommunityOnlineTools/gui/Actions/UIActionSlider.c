class UIActionSlider: UIActionBase 
{
	protected TextWidget m_Label;
	protected SliderWidget m_Slider;
	protected TextWidget m_Value;

	protected string m_Format;

	override void OnInit() 
	{
		super.OnInit();
		
		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
		Class.CastTo( m_Slider, layoutRoot.FindAnyWidget( "action" ) );
		Class.CastTo( m_Value, layoutRoot.FindAnyWidget( "action_value" ) );
	}

	override void OnShow()
	{
	}

	override void OnHide() 
	{
	}

	TextWidget GetLabelWidget()
	{
		return m_Label;
	}

	SliderWidget GetSliderWidget()
	{
		return m_Slider;
	}

	TextWidget GetValueWidget()
	{
		return m_Value;
	}

	override void SetText( string text )
	{
		m_Value.SetText(text);
	}

	void SetFormat( string format )
	{
		m_Format = format;
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		
		m_Label.SetText( text );

		if ( text.Length() > 0 )
		{
			SetWidgetWidth( m_Slider, 0.7 );
		} else
		{
			SetWidgetWidth( m_Slider, 1.0 );
		}
	}

	void SetStepValue( float step )
	{
		if (step > 0)
		{
			m_Slider.SetStep(step);

			SetCurrent(GetCurrent());
		}
		else
		{
			Error("Invalid step value " + step);
		}
	}

	float GetStepValue()
	{
		return m_Slider.GetStep();
	}

	void SetMin( float min )
	{
		ErrorEx("DEPRECATED, use SetMinMax", ErrorExSeverity.WARNING);

		SetMinMax(min, GetMax());
	}

	void SetMax( float max )
	{
		ErrorEx("DEPRECATED, use SetMinMax", ErrorExSeverity.WARNING);

		SetMinMax(GetMin(), max);
	}

	void SetMinMax(float min, float max)
	{
		float newRange = max - min;

		if (newRange > 0)
		{
			float oldRange = GetMax() - GetMin();
			float oldValue = GetCurrent() - GetMin();

			m_Slider.SetMinMax(min, max);
			m_Slider.Update();

			float value = min + (oldValue / oldRange) * newRange;
			SetCurrent(value);
		}
		else
		{
			Error("Invalid slider range " + newRange);
		}
	}

	float GetMin()
	{
		return m_Slider.GetMin();
	}

	float GetMax()
	{
		return m_Slider.GetMax();
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( widget == m_Slider )
			return true;

		return false;
	}

	override float GetCurrent()
	{
		return m_Slider.GetCurrent();
	}

	override void SetCurrent( float value )
	{
		float stepValue = GetStepValue();

		m_Slider.SetCurrent(Math.Round(Math.Clamp(value, GetMin(), GetMax()) / stepValue) * stepValue);

		UpdateValue();
	}

	void UpdateValue()
	{
		m_Value.SetText( string.Format( Widget.TranslateString( m_Format ), m_Slider.GetCurrent() ) );
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		UpdateValue(); 

		if ( !m_HasCallback )
			return false;

		if ( w == m_Slider )
		{
			CallEvent( UIEvent.CHANGE );
			return true;
		}
		
		return false;
	}

	void SetSliderWidth( float width )
	{
		float w;
		float h;
		m_Slider.GetSize( w, h );
		m_Slider.SetSize( width, h );
		m_Slider.Update();
	}

	void SetColor( int color )
	{
		m_Slider.SetColor(color);
	}

	void SetAlpha( float alpha )
	{
		m_Slider.SetAlpha(alpha);
	}
};
