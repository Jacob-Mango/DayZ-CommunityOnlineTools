class UIActionText: UIActionBase
{
	protected string m_ActualText;

	//! Draw the value hard left, in the label's slot, instead of hard right.
	//! See SetValueLeftAligned.
	protected bool m_ValueLeftAligned;

	protected TextWidget  m_Label;
	protected TextWidget  m_Text;
	protected ImageWidget m_Icon;
	protected ImageWidget m_IconRight;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label, layoutRoot.FindAnyWidget( "action_label" ) );
		Class.CastTo( m_Text,  layoutRoot.FindAnyWidget( "action"       ) );
		Class.CastTo( m_Icon,  layoutRoot.FindAnyWidget( "action_icon"  ) );
		Class.CastTo( m_IconRight, layoutRoot.FindAnyWidget( "action_icon_right" ) );
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
		if ( m_Label )
			m_Label.SetText( text );
	}

	override void SetText( string text )
	{
		m_ActualText = Widget.TranslateString( text );

		//! Both widgets exist either way; only one of them is ever carrying
		//! text, so the other does not have to be cleared on every set.
		if ( m_ValueLeftAligned )
		{
			if ( m_Label )
				m_Label.SetText( m_ActualText );

			return;
		}

		if ( m_Text )
			m_Text.SetText( m_ActualText );
	}

	//! Put the value in the label's slot - hard left, right after the icon -
	//! rather than hard right. A label/value row wants the value on the far
	//! edge so a column of them lines up; an icon+number pair wants the number
	//! to read as belonging to the glyph beside it.
	void SetValueLeftAligned( bool enabled )
	{
		m_ValueLeftAligned = enabled;

		if ( m_Text )
			m_Text.SetText( "" );

		if ( m_Label )
			m_Label.SetText( "" );
	}

	//! How far in from the left edge the label sits. SetIcon already pushes it
	//! clear of the glyph; this is for a caller that wants a specific indent.
	void SetLabelOffset( float x )
	{
		if ( m_Label )
			m_Label.SetTextOffset( x, 0 );
	}

	override string GetText()
	{
		return m_ActualText;
	}

	override void SetIcon( string imagePath )
	{
		if ( !m_Icon )
			return;

		if ( imagePath == "" )
		{
			m_Icon.Show( false );
			if ( m_Label )
				m_Label.SetTextOffset( 10, 0 );
			return;
		}

		m_Icon.LoadImageFile( 0, imagePath );
		m_Icon.Show( true );

		// Push label text right to clear the icon (icon is at x=6, width=20 -> end at 26 + 4 gap = 30)
		if ( m_Label )
			m_Label.SetTextOffset( 30, 0 );
	}

	//! Glyph on the FAR RIGHT of the row, after the value: label - value - icon.
	//!
	//! The left slot puts the icon between the row edge and the label, which
	//! crowds the text on a narrow panel. A right-hand glyph reads as a suffix
	//! to the number instead and leaves the label column alone. The value is
	//! pushed left to clear it.
	void SetIconRight( string imagePath )
	{
		if ( !m_IconRight )
			return;

		if ( imagePath == "" )
		{
			m_IconRight.Show( false );
			if ( m_Text )
				m_Text.SetTextOffset( -6, 0 );
			return;
		}

		m_IconRight.LoadImageFile( 0, imagePath );
		m_IconRight.Show( true );

		// Icon sits 8px in from the right edge and is 14 wide -> 22px, plus a
		// 14px gap so the number does not read as part of the glyph.
		if ( m_Text )
			m_Text.SetTextOffset( -36, 0 );
	}

	void SetLabelHAlign( UIActionHAlign type )
	{
		if ( !m_Label ) return;
		switch ( type )
		{
		case UIActionHAlign.CENTER:
			m_Label.SetFlags( m_Label.GetFlags() | WidgetFlags.CENTER );
			break;
		case UIActionHAlign.RIGHT:
			m_Label.SetFlags( m_Label.GetFlags() | WidgetFlags.RALIGN );
			break;
		}
	}

	void SetLabelVAlign( UIActionVAlign type )
	{
		if ( !m_Label ) return;
		switch ( type )
		{
		case UIActionVAlign.CENTER:
			m_Label.SetFlags( m_Label.GetFlags() | WidgetFlags.VCENTER );
			break;
		}
	}

	void SetTextHAlign( UIActionHAlign type )
	{
		if ( !m_Text ) return;
		switch ( type )
		{
		case UIActionHAlign.CENTER:
			m_Text.SetFlags( m_Text.GetFlags() | WidgetFlags.CENTER );
			break;
		case UIActionHAlign.RIGHT:
			m_Text.SetFlags( m_Text.GetFlags() | WidgetFlags.RALIGN );
			break;
		}
	}

	void SetTextVAlign( UIActionVAlign type )
	{
		if ( !m_Text ) return;
		switch ( type )
		{
		case UIActionVAlign.CENTER:
			m_Text.SetFlags( m_Text.GetFlags() | WidgetFlags.VCENTER );
			break;
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if ( !m_HasCallback )
		{
			if ( w == m_Text )
			{
				g_Game.CopyToClipboard( m_ActualText );
			}
			return false;
		}

		bool ret = false;

		if ( w == m_Text )
		{
			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}
}
