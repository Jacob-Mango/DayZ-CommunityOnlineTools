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

		float w;
		float h;
		
		m_TextPreview.GetSize( w, h );
		m_TextPreview.SetSize( width, h );
		m_TextPreview.Update();
	}
};
