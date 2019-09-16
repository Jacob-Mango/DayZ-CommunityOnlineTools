class UIActionEditableText extends UIActionBase 
{
	static ref TStringArray VALID_NUMBERS = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

	protected ref TextWidget m_Label;
	protected ref EditBoxWidget m_Text;
	protected ref ButtonWidget m_Button;

	protected bool m_OnlyNumbers;

	protected string m_PreviousText;

	override void OnInit() 
	{
		super.OnInit();

		m_PreviousText = "";
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

	void HasButton( bool enabled )
	{
		Widget root = NULL;
		if ( enabled )
		{
			layoutRoot.FindAnyWidget( "action_wrapper_input" ).Show( false );

			root = layoutRoot.FindAnyWidget( "action_wrapper_check" );
			root.Show( true );

			m_Button = ButtonWidget.Cast( root.FindAnyWidget( "action_button" ) );
		} else
		{
			layoutRoot.FindAnyWidget( "action_wrapper_check" ).Show( false );

			root = layoutRoot.FindAnyWidget( "action_wrapper_input" );
			root.Show( true );
		}

		m_Label = TextWidget.Cast( root.FindAnyWidget( "action_label" ) );
		m_Text = EditBoxWidget.Cast( root.FindAnyWidget( "action" ) );
	}

	override void SetLabel( string text )
	{
		m_Label.SetText( text );
	}

	override void SetText( string text )
	{
		if ( m_Text == GetFocus() )
			return;

		m_Text.SetText( text );

		UpdateText();
	}

	override string GetText()
	{
		return m_Text.GetText();
	}

	bool UpdateText()
	{
		if ( m_OnlyNumbers )
		{
			string newText = m_Text.GetText();

			bool hasDecimal = false;
			bool failed = false;

			for ( int i = 0; i < newText.Length(); i++ )
			{
				if ( VALID_NUMBERS.Find( newText.Get( i ) ) == -1 )
				{
					if ( !hasDecimal )
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

			return !failed;
		}

		return true;
	}

	void SetOnlyNumbers( bool onlyNumbers )
	{
		m_OnlyNumbers = onlyNumbers;
	}

	bool IsOnlyNumbers()
	{
		return m_OnlyNumbers;
	}

	override void SetButton( string text )
	{
		TextWidget.Cast( layoutRoot.FindAnyWidget( "action_button_text" ) ).SetText( text );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_Text )
		{
			SetFocus( m_Text );
			return true;
		}

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_Text )
		{
			SetFocus( NULL );
			return true;
		}

		return false;
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		if ( !m_HasCallback )
			return false;

		if ( w == m_Text )
		{
			DISABLE_ALL_INPUT = true;
			if ( UpdateText() )
			{
				CallEvent( UIEvent.CHANGE );
			}

			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( this.RemoveDisableInput, 100, false );
			return true;
		}
		
		return false;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{	
		if ( !m_HasCallback ) return false;

		bool ret = false;

		if ( w == m_Button )
		{
			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}

	override bool CallEvent( UIEvent eid )
	{
		if ( !m_HasCallback ) return false;

		GetGame().GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, new Param2< UIEvent, ref UIActionBase >( eid, this ) );

		return false;
	}
}