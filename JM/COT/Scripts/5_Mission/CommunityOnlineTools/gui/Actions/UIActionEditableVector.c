class UIActionEditableVector: UIActionBase 
{
	protected TextWidget m_Label;
	protected EditBoxWidget m_TextX;
	protected EditBoxWidget m_TextY;
	protected EditBoxWidget m_TextZ;
	protected ButtonWidget m_Button;

	override void OnInit() 
	{
		super.OnInit();

		m_Button = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "action_button" ) );

		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
		Class.CastTo( m_TextX, layoutRoot.FindAnyWidget( "action_x" ) );
		Class.CastTo( m_TextY, layoutRoot.FindAnyWidget( "action_y" ) );
		Class.CastTo( m_TextZ, layoutRoot.FindAnyWidget( "action_z" ) );

		SetValue( vector.Zero );
	}

	override void OnShow()
	{
	}

	override void OnHide() 
	{
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );

		m_Label.SetText( text );
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( widget == m_TextX )
			return true;
		if ( widget == m_TextY )
			return true;
		if ( widget == m_TextZ )
			return true;

		return false;
	}

	override void SetValue( vector v )
	{
		if ( IsFocused() )
			return;

		m_TextX.SetText( v[0].ToString() );
		m_TextY.SetText( v[1].ToString() );
		m_TextZ.SetText( v[2].ToString() );
	}

	override vector GetValue()
	{
		return Vector( m_TextX.GetText().ToFloat(), m_TextY.GetText().ToFloat(), m_TextZ.GetText().ToFloat() );
	}

	override bool OnMouseWheel(Widget  w, int  x, int  y, int wheel)
	{
		if ( (w == m_TextX || w == m_TextY || w == m_TextZ) && IsFocusWidget(GetFocus()) )
		{
			EditBoxWidget editw = EditBoxWidget.Cast(w);
			IncrementValue(editw, wheel);

			return true;
		}

		return super.OnMouseWheel( w, x, y, wheel );
	}

	void IncrementValue(EditBoxWidget w, int wheel)
	{
		float multiplier = 0.1;
		if (SHIFT())
			multiplier = multiplier * 10;

		float currValue = w.GetText().ToFloat();
		currValue = currValue + (wheel * multiplier);
		w.SetText(currValue.ToString());
		
		CallEvent( UIEvent.MOUSEWHEEL );
	}

	override void SetButton( string text )
	{
		text = Widget.TranslateString( text );
		
		TextWidget.Cast( layoutRoot.FindAnyWidget( "action_button_text" ) ).SetText( text );
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		if ( !m_HasCallback )
			return false;

		if ( w == m_TextX || w == m_TextY || w == m_TextZ )
		{
			CallEvent( UIEvent.CHANGE );
			
			return true;
		}
		
		return false;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{	
		if ( !m_HasCallback )
			return false;

		bool ret = false;

		if ( w == m_Button )
		{
			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}

	override bool CallEvent( UIEvent eid )
	{
		if ( !m_HasCallback )
			return false;

		GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionBase >( eid, this ) );

		return false;
	}
};
