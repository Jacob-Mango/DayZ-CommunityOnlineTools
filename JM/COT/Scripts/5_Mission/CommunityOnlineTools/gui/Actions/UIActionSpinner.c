// =============================================================================
//  UIActionSpinner
//
//  Label + value display + -/+ buttons for integer/float stepping.
//  Mouse-wheel over the widget also increments/decrements.
//  Fires UIEvent.CHANGE whenever the value changes.
//
//  Usage:
//      m_Qty = UIActionManager.CreateSpinner( parent, "Quantity", 1, 100, 1, this, "OnChange_Qty" );
//      m_Qty.SetValue( 10 );
//
//      void OnChange_Qty( UIEvent eid, UIActionBase action )
//      {
//          int qty = m_Qty.GetValueInt();
//      }
// =============================================================================
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
		text = Widget.TranslateString( text );
		if ( m_Label )
			m_Label.SetText( text );
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

	void SetIntegerOnly( bool intOnly )
	{
		m_IntegerOnly = intOnly;
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

	//! Pull whatever is currently typed into the edit box into m_Value.
	//!
	//! The engine only fires OnChange( finished ) when the box loses focus or the
	//! user presses enter. Clicking +/- (or scrolling) straight after typing does
	//! neither, so the step used to be applied to the LAST committed value and the
	//! typed number visibly reverted. Every entry point that steps the value calls
	//! this first.
	//!
	//! An empty or non-numeric box is left alone rather than being read as 0,
	//! which would otherwise snap the value to m_Min the moment the user cleared
	//! the field and pressed a button.
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

	int GetValueInt()
	{
		return ( int ) m_Value;
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
