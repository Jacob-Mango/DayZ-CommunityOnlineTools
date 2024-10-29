class UIActionEditableText: UIActionBase 
{
	static ref TStringArray VALID_NUMBERS = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

	protected TextWidget m_Label;
	protected EditBoxWidget m_Text;
	protected ButtonWidget m_Button;
	protected TextWidget m_ButtonText;

	protected bool m_OnlyNumbers;
	protected bool m_OnlyIntegers;

	protected string m_PreviousText;

	protected bool m_Edited;

	override void OnInit() 
	{
		super.OnInit();

		m_PreviousText = "";

		Class.CastTo( m_Button, layoutRoot.FindAnyWidget( "action_button" ) );
		Class.CastTo( m_ButtonText, layoutRoot.FindAnyWidget( "action_button_text" ) );

		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
		Class.CastTo( m_Text, layoutRoot.FindAnyWidget( "action" ) );
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

	EditBoxWidget GetEditBoxWidget()
	{
		return m_Text;
	}

	ButtonWidget GetButtonWidget()
	{
		return m_Button;
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );

		m_Label.SetText( text );
	}

	void SetText( float num )
	{
		SetText( "" + num );
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( widget == m_Text )
			return true;

		return false;
	}

	override void SetText( string text )
	{
		if ( IsFocused() )
			return;

		m_Text.SetText( text );

		UpdateText();
	}

	override string GetText()
	{
		return m_Text.GetText();
	}

	override bool OnMouseWheel(Widget  w, int  x, int  y, int wheel)
	{
		if ( w == m_Text && IsFocusWidget(GetFocus()) )
		{
			IncrementText(wheel);

			return true;
		}

		return super.OnMouseWheel( w, x, y, wheel );
	}

	void IncrementText(int wheel)
	{
		if ( !m_OnlyNumbers )
			return;

		float multiplier = 0.1;
		if ( m_OnlyIntegers )
			multiplier = 1.0;

		if (SHIFT())
			multiplier = multiplier * 10;

		float currValue = m_Text.GetText().ToFloat();
		currValue = currValue + (wheel * multiplier);
		m_Text.SetText(currValue.ToString());
		
		UpdateText();
		m_Edited = true;
		CallEvent( UIEvent.MOUSEWHEEL );
	}

	bool UpdateText()
	{
		m_Edited = false;

		if ( m_OnlyNumbers )
		{
			string newText = m_Text.GetText();

			bool hasDecimal = false;
			bool failed = false;

			if ( newText.Length() > 0 )
			{
				int i = 0;
				if ( newText.Get( i ) == "-" )
					i = 1;

				for ( i = i; i < newText.Length(); i++ )
				{
					if ( VALID_NUMBERS.Find( newText.Get( i ) ) == -1 )
					{
						if ( !hasDecimal && !m_OnlyIntegers )
						{
							if ( newText.Get( i ) == "." || newText.Get( i ) == "," )
							{
								hasDecimal = true;
								continue;
							}
						}
						failed = true;
						break;
					}
				}

				if ( failed )
				{
					m_Text.SetText( m_PreviousText );
				} else
				{
					m_PreviousText = newText;
				}
			}

			return !failed;
		}

		return true;
	}

	void SetOnlyNumbers( bool onlyNumbers, bool onlyInts = false )
	{
		m_OnlyNumbers = onlyNumbers;

		if ( m_OnlyNumbers )
		{
			m_OnlyIntegers = onlyInts;
		} else
		{
			m_OnlyIntegers = false;
		}
	}

	bool IsOnlyNumbers()
	{
		return m_OnlyNumbers;
	}

	override void SetButton( string text )
	{
		text = Widget.TranslateString( text );
		
		m_ButtonText.SetText( text );
	}

	override bool OnKeyPress( Widget w, int x, int y, int key )
	{
		m_Edited = true;

		return super.OnKeyPress( w, x, y, key );
	}

	void SetEdited(bool edited)
	{
		m_Edited = edited;
	}

	bool IsEdited()
	{
		return m_Edited;
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		if ( !m_HasCallback )
			return false;

		if ( w == m_Text )
		{
			if ( UpdateText() )
			{
				CallEvent( UIEvent.CHANGE );
			}

			return true;
		}
		
		return false;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{	
		if ( !m_HasCallback )
			return false;

		bool ret = false;

		if ( w == m_Text )
		{
			SetFocus( m_Text );
		} else if ( w == m_Button )
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

	void SetEditBoxWidth( float width )
	{
		float w;
		float h;
		
		m_Text.GetSize( w, h );
		m_Text.SetSize( width, h );
		m_Text.Update();
	}
};
