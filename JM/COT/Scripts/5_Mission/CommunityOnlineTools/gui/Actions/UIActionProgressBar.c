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

	void SetProgress( float value )
	{
		m_TargetValue = Math.Clamp( value, 0.0, 1.0 );
	}

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

	void SetShowText( bool show )
	{
		if ( m_Text )
			m_Text.Show( show );
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

	private void ApplyFill()
	{
		if ( m_Fill )
			SetWidgetWidth( m_Fill, m_Value );

		if ( m_Text )
			m_Text.SetText( string.Format( "%1%%", Math.Round( m_TargetValue * 100 ) ) );
	}
}
