class UIActionEditableTextPreview: UIActionEditableText 
{
	protected EditBoxWidget m_TextPreview;

	EditBoxWidget GetEditPreviewBoxWidget()
	{
		return m_TextPreview;
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_TextPreview, layoutRoot.FindAnyWidget( "action_preview" ) );
		m_TextPreview.SetText("");
		m_TextPreview.SetAlpha(0.5);
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

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		if ( !m_HasCallback )
			return false;

		if ( w == m_Text )
		{
			//! finished == true means RETURN/ENTER key was pressed, do autocompletion
			if (finished)
			{
				string preview = GetTextPreview();

				if (preview != "")
					m_Text.SetText(preview);
			}

			if ( UpdateText() )
			{
				g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(CallEvent);
				g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(CallEvent, 100, false, UIEvent.CHANGE);
			}

			return true;
		}
		
		return false;
	}

	override bool OnKeyPress( Widget w, int x, int y, int key )
	{
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

		// The ghost-text preview is a sibling of the field, laid out on top of
		// it, so it has to match the field's new width.
		if ( m_TextPreview )
			SetWidgetWidth( m_TextPreview, width );
	}
}
