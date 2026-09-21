class UIActionProgressBar: UIActionBase
{
	protected TextWidget  m_Label;
	protected Widget      m_Track;
	protected Widget      m_Fill;
	protected TextWidget  m_Text;
	protected float m_Value;       // current displayed fraction 0..1
	protected float m_TargetValue; // target fraction 0..1
	protected int   m_FillColor;
	static const float ANIM_SPEED = 3.0; // fraction per second

	//! Text of the caller's own in place of the percentage, for a bar whose fill
	//! means something a percentage would not say (a countdown: "12:30 / 30:00").
	protected string m_BarText;
	protected bool   m_HasBarText;

	float GetProgress()
	{
		return m_TargetValue;
	}

	void SetFillColor( int color )
	{
		m_FillColor = color;
		if ( m_Fill )
			m_Fill.SetColor( color );
	}

	void SetProgress( float value )
	{
		m_TargetValue = Math.Clamp( value, 0.0, 1.0 );
	}

	void SetShowText( bool show )
	{
		if ( m_Text )
			m_Text.Show( show );
	}

	//! Show this text on the bar instead of the percentage.
	void SetBarText( string text )
	{
		m_BarText    = text;
		m_HasBarText = true;

		if ( m_Text )
			m_Text.SetText( text );
	}

	//! Back to the percentage.
	void ClearBarText()
	{
		m_HasBarText = false;

		ApplyFill();
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
		Class.CastTo( m_Track, layoutRoot.FindAnyWidget( "action_track" ) );
		Class.CastTo( m_Fill,  layoutRoot.FindAnyWidget( "action_fill"  ) );
		Class.CastTo( m_Text,  layoutRoot.FindAnyWidget( "action"       ) );

		m_FillColor   = JMTheme.ACCENT;
		m_Value       = 0;
		m_TargetValue = 0;
		ApplyFill();
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_Label )
		{
			m_Label.SetText( text );
			if ( m_Track )
			{
				if ( text.Length() > 0 )
					SetWidgetWidth( m_Track, 0.7 );
				else
					SetWidgetWidth( m_Track, 1.0 );
			}
		}
	}

	override void SetText( string text )
	{
		if ( m_Text )
			m_Text.SetText( text );
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( Math.AbsFloat( m_Value - m_TargetValue ) < 0.001 )
		{
			if ( m_Value != m_TargetValue )
			{
				m_Value = m_TargetValue;
				ApplyFill();
			}
			return;
		}

		float delta = m_TargetValue - m_Value;
		m_Value += delta * Math.Min( ANIM_SPEED * timeSlice, 1.0 );
		ApplyFill();
	}

	protected void ApplyFill()
	{
		if ( m_Fill )
			SetWidgetWidth( m_Fill, m_Value );

		if ( !m_Text )
			return;

		if ( m_HasBarText )
			m_Text.SetText( m_BarText );
		else
			m_Text.SetText( string.Format( "%1%%", Math.Round( m_TargetValue * 100 ) ) );
	}
}
