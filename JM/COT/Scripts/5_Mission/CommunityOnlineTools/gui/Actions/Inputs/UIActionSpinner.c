//  UIActionSpinner - label + value + -/+ buttons for integer/float stepping; mouse
//  wheel also steps. Fires UIEvent.CHANGE on change.
//  CreateSpinner( parent, "Quantity", 1, 100, 1, this, "OnChange_Qty" );
class UIActionSpinner: UIActionBase
{
	protected TextWidget   m_Label;
	protected EditBoxWidget m_EditBox;
	protected ButtonWidget  m_BtnDec;
	protected ButtonWidget  m_BtnInc;
	protected float m_Value;
	protected float m_Min;
	protected float m_Max;
	protected float m_Step;
	protected bool  m_IntegerOnly;

	int GetValueInt()
	{
		return ( int ) m_Value;
	}

	void SetIntegerOnly( bool intOnly )
	{
		m_IntegerOnly = intOnly;
	}

	void SetMinMax( float min, float max )
	{
		m_Min = min;
		m_Max = max;
		SetValue( m_Value );
	}

	void SetStep( float step )
	{
		if ( step > 0 )
			m_Step = step;
	}

	void SetValue( float value )
	{
		m_Value = Math.Clamp( value, m_Min, m_Max );
		if ( m_IntegerOnly )
			m_Value = Math.Round( m_Value );

		if ( m_EditBox )
		{
			if ( m_IntegerOnly )
				m_EditBox.SetText( "" + ( int ) m_Value );
			else
				m_EditBox.SetText( string.Format( "%.2f", m_Value ) );
		}
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,   layoutRoot.FindAnyWidget( "action_label"      ) );
		Class.CastTo( m_EditBox, layoutRoot.FindAnyWidget( "action"            ) );
		Class.CastTo( m_BtnDec,  layoutRoot.FindAnyWidget( "action_button_dec" ) );
		Class.CastTo( m_BtnInc,  layoutRoot.FindAnyWidget( "action_button_inc" ) );

		m_Min  = -float.MAX;
		m_Max  =  float.MAX;
		m_Step = 1.0;
	}

	override void SetLabel( string text )
	{
		if ( m_Label )
			m_Label.SetText( text );
	}

	//! Pull the typed edit box text into m_Value. OnChange( finished ) only fires on
	//! focus loss or enter, so every stepping entry point calls this first.
	//! Empty/non-numeric text is ignored, not read as 0 (which would snap to m_Min).
	protected void CommitEditBox()
	{
		if ( !m_EditBox )
			return;

		string typed = m_EditBox.GetText();
		typed.TrimInPlace();
		if ( typed == "" )
			return;

		float parsed = typed.ToFloat();
		m_Value      = Math.Clamp( parsed, m_Min, m_Max );
		if ( m_IntegerOnly )
			m_Value = Math.Round( m_Value );
	}

	override float GetCurrent()
	{
		return m_Value;
	}

	override bool IsFocusWidget( Widget widget )
	{
		return widget == m_EditBox;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_BtnDec )
		{
			CommitEditBox();
			SetValue( m_Value - m_Step );
			CallEvent( UIEvent.CHANGE );
			return true;
		}

		if ( w == m_BtnInc )
		{
			CommitEditBox();
			SetValue( m_Value + m_Step );
			CallEvent( UIEvent.CHANGE );
			return true;
		}

		if ( w == m_EditBox )
		{
			SetFocus( m_EditBox );
			return false;
		}

		return false;
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		if ( w != m_EditBox || !finished )
			return false;

		float parsed = m_EditBox.GetText().ToFloat();
		m_Value      = Math.Clamp( parsed, m_Min, m_Max );
		if ( m_IntegerOnly )
			m_Value = Math.Round( m_Value );

		CallEvent( UIEvent.CHANGE );
		return true;
	}

	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		if ( !IsFocused() )
			return false;

		float delta = m_Step * wheel;
		if ( KeyState( KeyCode.KC_LSHIFT ) )
			delta *= 10;

		CommitEditBox();
		SetValue( m_Value + delta );
		CallEvent( UIEvent.CHANGE );
		return true;
	}
}
