class UIActionEditableTextPreview: UIActionEditableText 
{
	protected EditBoxWidget m_TextPreview;

	EditBoxWidget GetEditPreviewBoxWidget()
	{
		return m_TextPreview;
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

	string GetTextPreview()
	{		
		return m_TextPreview.GetText();
	}

	void SetTextPreview( string text )
	{
		text = Widget.TranslateString( text );
		
		m_TextPreview.SetText( text );
	}

	override bool OnKeyPress( Widget w, int x, int y, int key )
	{
		PrintFormat("%1::OnKeyPress %2 %3 %4 %5", ToString(), w, x, y, key);
		string preview = GetTextPreview();

		if ( preview != "" )
		{
			if ( key == KeyCode.KC_TAB || key == KeyCode.KC_RETURN || key == KeyCode.KC_NUMPADENTER )
			{
				m_Text.SetText(preview);
				return true;
			}
		}

		return super.OnKeyPress( w, x, y, key );
	}

	override void SetEditBoxWidth( float width )
	{
		super.SetEditBoxWidth(width);

		float w;
		float h;
		
		m_TextPreview.GetSize( w, h );
		m_TextPreview.SetSize( width, h );
		m_TextPreview.Update();
	}
};
