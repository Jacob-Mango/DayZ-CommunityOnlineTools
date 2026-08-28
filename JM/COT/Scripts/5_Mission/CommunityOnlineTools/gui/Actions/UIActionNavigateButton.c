class UIActionNavigateButton: UIActionBase
{
	protected TextWidget  m_Text;
	protected ImageWidget m_Icon;

	protected ImageWidget  m_Image_Left;
	protected ImageWidget  m_Image_Right;

	protected ButtonWidget m_Button_Left;
	protected ButtonWidget m_Button_Right;

	//! Same rounded pill as UIActionButton - see that class for why the colour
	//! lands on the images rather than on the button widgets.
	protected Widget      m_Fill;
	protected Widget      m_Outline;

	//! Resting colours plus the two feedback states derived from them. A caller
	//! that recolours the button with SetColor() gets matching hover / pressed
	//! shades for free.
	protected int  m_FillColor;
	protected int  m_FillHover;
	protected int  m_FillPressed;
	protected int  m_OutlineColor;
	protected int  m_OutlineHover;
	protected int  m_OutlinePressed;
	protected bool m_IsHovered;
	protected bool m_IsPressed;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Text,         layoutRoot.FindAnyWidget( "action"              ) );
		Class.CastTo( m_Icon,         layoutRoot.FindAnyWidget( "action_icon"         ) );
		Class.CastTo( m_Image_Left,   layoutRoot.FindAnyWidget( "image_left"          ) );
		Class.CastTo( m_Image_Right,  layoutRoot.FindAnyWidget( "image_right"         ) );
		Class.CastTo( m_Button_Left,  layoutRoot.FindAnyWidget( "action_button_left"  ) );
		Class.CastTo( m_Button_Right, layoutRoot.FindAnyWidget( "action_button_right" ) );
		Class.CastTo( m_Fill,         layoutRoot.FindAnyWidget( "fill"                ) );
		Class.CastTo( m_Outline,      layoutRoot.FindAnyWidget( "outline"             ) );

		m_FillColor    = JMTheme.BUTTON_FILL;
		m_FillHover    = JMTheme.BUTTON_FILL_HOVER;
		m_FillPressed  = JMTheme.BUTTON_FILL_PRESSED;
		m_OutlineColor = JMTheme.BUTTON_OUTLINE;
		m_OutlineHover = JMTheme.BUTTON_OUTLINE_HOVER;
		m_OutlinePressed = JMTheme.BUTTON_OUTLINE_PRESSED;
		m_IsHovered    = false;
		m_IsPressed    = false;

		ApplyButtonColors();
	}

	override void OnShow()
	{
	}

	override void OnHide() 
	{
	}

	override void SetButton( string text )
	{
		text = Widget.TranslateString( text );
		
		m_Text.SetText( text );
	}

	void SetImages( string imageL, string imageR )
	{
		m_Button_Left.SetText("");
		m_Button_Right.SetText("");
	}

	override void SetIcon( string imagePath )
	{
		if ( !m_Icon )
			return;

		if ( imagePath == "" )
		{
			m_Icon.Show( false );
			if ( m_Text )
				m_Text.SetTextOffset( 0, 0 );
			return;
		}

		m_Icon.LoadImageFile( 0, imagePath );
		m_Icon.Show( true );

		if ( m_Text )
			m_Text.SetTextOffset( 16, 0 );
	}

	override string GetButton()
	{
		return "";
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		bool ret = false;

		if ( w == m_Button_Left )
		{
			ret = CallEvent( UIEvent.CLICK_LEFTSIDE );
		}
		else if ( w == m_Button_Right )
		{
			ret = CallEvent( UIEvent.CLICK_RIGHTSIDE );
		}

		return ret;
	}

	override void SetColor( int color )
	{
		m_FillColor    = color;
		m_FillHover    = JMTheme.Lighten( color, 0.18 );
		m_FillPressed  = JMTheme.Darken( color, 0.55 );
		m_OutlineColor = JMTheme.Lighten( color, 0.35 );
		m_OutlineHover = JMTheme.Lighten( color, 0.60 );
		m_OutlinePressed = JMTheme.Lighten( color, 0.60 );

		ApplyButtonColors();
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		super.OnMouseEnter( w, x, y );

		m_IsHovered = true;
		ApplyButtonColors();

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		super.OnMouseLeave( w, enterW, x, y );

		// Both arrows sit inside the same pill, so moving from one arrow to the
		// other must not flicker the pill back to its resting colour.
		if ( enterW != m_Button_Left && enterW != m_Button_Right )
			m_IsHovered = false;

		m_IsPressed = false;
		ApplyButtonColors();

		return false;
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( button == MouseState.LEFT )
		{
			m_IsPressed = true;
			ApplyButtonColors();
		}

		return false;
	}

	override bool OnMouseButtonUp( Widget w, int x, int y, int button )
	{
		if ( button == MouseState.LEFT )
		{
			m_IsPressed = false;
			ApplyButtonColors();
		}

		return false;
	}

	protected void ApplyButtonColors()
	{
		int fillColor = m_FillColor;
		int ringColor = m_OutlineColor;

		if ( m_IsPressed )
		{
			fillColor = m_FillPressed;
			ringColor = m_OutlinePressed;
		}
		else if ( m_IsHovered )
		{
			fillColor = m_FillHover;
			ringColor = m_OutlineHover;
		}

		// This painter owns the ring, so the base class's focus ring would be
		// overwritten the next time any other state changed. Honour it here.
		if ( m_ChromeFocused && !m_IsPressed )
			ringColor = JMTheme.BORDER_RING;

		if ( m_Fill )
			m_Fill.SetColor( fillColor );

		if ( m_Outline )
			m_Outline.SetColor( ringColor );
	}
}
