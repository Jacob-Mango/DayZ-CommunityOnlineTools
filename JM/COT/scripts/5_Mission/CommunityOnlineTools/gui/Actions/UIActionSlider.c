class UIActionSlider extends UIActionBase 
{
	protected TextWidget m_Label;
	protected SliderWidget m_Slider;
	protected TextWidget m_Value;

	protected float m_Min;
	protected float m_Max;

	protected float m_StepValue;

	protected float m_Current;

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

	void SetFormat( string format )
	{
		m_Format = format;
	}

	override void SetLabel( string text )
	{
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
		m_StepValue = step;

		CalculateValue();
	}

	void SetMin( float min )
	{
		m_Min = min;

		CalculateValue();
	}

	void SetMax( float max )
	{
		m_Max = max;

		CalculateValue();
	}

	bool IsFocusWidget( Widget widget )
	{
		if ( widget == m_Slider )
			return true;

		return false;
	}

	override float GetCurrent()
	{
		return m_Current;
	}

	override void SetCurrent( float value )
	{
		if ( IsFocused() )
			return;
			
		float x = value;
		float a = m_Slider.GetMin();
		float b = m_Slider.GetMax();
		float min = m_Min;
		float max = m_Max;

		float top = ( b - a ) * ( x - min );
		float bottom = ( max - min );

		if ( bottom != 0 )
		{
			m_Slider.SetCurrent( ( top / bottom ) + a );
			CalculateValue();
		} else
		{
			m_Slider.SetCurrent( 0 );
			m_Current = 0;
		}
	}

	void CalculateValue()
	{
		float x = m_Slider.GetCurrent();
		float a = m_Min;
		float b = m_Max;
		float min = m_Slider.GetMin();
		float max = m_Slider.GetMax();

		float top = ( b - a ) * ( x - min );
		float bottom = ( max - min );

		if ( bottom != 0 )
		{
			m_Current = ( top / bottom ) + a;

			if ( m_StepValue != 0 )
			{
				m_Current = Math.Round( m_Current / m_StepValue ) * m_StepValue;
			}
		}

		m_Value.SetText( string.Format( Widget.TranslateString( m_Format ), m_Current ) );
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		CalculateValue(); 

		if ( !m_HasCallback )
			return false;

		if ( w == m_Slider )
		{
			CallEvent( UIEvent.CHANGE );
			return true;
		}
		
		return false;
	}

	override bool CallEvent( UIEvent eid )
	{
		GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionBase >( eid, this ) );

		return false;
	}
}