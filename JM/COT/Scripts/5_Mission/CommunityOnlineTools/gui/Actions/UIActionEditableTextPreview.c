class UIActionEditableTextPreview: UIActionBase 
{
	static ref TStringArray VALID_NUMBERS = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

	protected TextWidget m_Label;
	protected EditBoxWidget m_TextPreview;
	protected EditBoxWidget m_Text;
	protected ButtonWidget m_Button;

	protected bool m_OnlyNumbers;
	protected bool m_OnlyIntegers;

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

	EditBoxWidget GetEditPreviewBoxWidget()
	{
		return m_TextPreview;
	}

	ButtonWidget GetButtonWidget()
	{
		return m_Button;
	}

	void HasButton( bool enabled )
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::HasButton" );
		#endif

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

		Class.CastTo( m_Label, root.FindAnyWidget( "action_label" ) );
		Class.CastTo( m_Text, root.FindAnyWidget( "action" ) );
		Class.CastTo( m_TextPreview, root.FindAnyWidget( "action_preview" ) );
		m_TextPreview.SetText("");
		m_TextPreview.SetAlpha(0.5);

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::HasButton" );
		#endif
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

	string GetTextPreview()
	{		
		return m_TextPreview.GetText();
	}

	void SetTextPreview( string text )
	{
		text = Widget.TranslateString( text );
		
		m_TextPreview.SetText( text );
	}

	bool UpdateText()
	{
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
		
		TextWidget.Cast( layoutRoot.FindAnyWidget( "action_button_text" ) ).SetText( text );
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
		
		m_TextPreview.GetSize( w, h );
		m_TextPreview.SetSize( width, h );
		m_TextPreview.Update();
	}
};
