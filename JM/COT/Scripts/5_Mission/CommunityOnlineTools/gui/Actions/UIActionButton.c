// =============================================================================
//  UIActionButton
//
//  The mod's standard button: a rounded pill - a filled surface plus a ring -
//  which is the same shape the tab bar, the filter bar and the icon grid use,
//  so everything clickable in the mod reads as one family.
//
//  At rest the pill is deliberately quiet (raised surface, half-lit ring): a
//  form is mostly buttons, and painting them all in the selected-blue turns the
//  window into a slab of colour. Hover lights the ring; JMTheme owns both.
//
//  The ButtonWidget itself paints nothing; it is only the hit area. Every
//  colour lands on the "fill" / "outline" images, which is also why
//  SetColor() retints the fill instead of the button.
// =============================================================================
class UIActionButton: UIActionBase
{
	protected TextWidget  m_Text;
	protected ButtonWidget m_Button;
	protected ImageWidget  m_Icon;

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

	protected bool  m_SpinActive;
	protected float m_SpinAngle;
	protected float m_SpinTotal;
	protected float m_SpinTarget;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Button, layoutRoot.FindAnyWidget( "action_button" ) );
		Class.CastTo( m_Text,   layoutRoot.FindAnyWidget( "action"        ) );
		Class.CastTo( m_Icon,   layoutRoot.FindAnyWidget( "action_icon"   ) );
		Class.CastTo( m_Fill,    layoutRoot.FindAnyWidget( "fill"    ) );
		Class.CastTo( m_Outline, layoutRoot.FindAnyWidget( "outline" ) );

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
		if ( m_Button )
			m_Button.SetName( text );
		if ( m_Text )
			m_Text.SetText( text );
	}

	override string GetButton()
	{
		if ( m_Button )
			return m_Button.GetName();
		return "";
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

		// Shift text right so it stays visually centered in the remaining space
		if ( m_Text )
			m_Text.SetTextOffset( 16, 0 );
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		bool ret = false;

		if ( w == m_Button )
		{
			ret = CallEvent( UIEvent.CLICK );
		}

		return ret;
	}

	//! Retint the pill. The ring follows the new fill so a red / green button
	//! still looks like the same component, just in another colour.
	override void SetColor(int color)
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

		m_IsHovered = false;
		// A press that ends outside the button never gets an OnMouseButtonUp
		// here, so drop the pressed look on the way out as well.
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

	void TriggerSpin( int revolutions )
	{
		if ( !m_Icon || !m_Icon.IsVisible() )
			return;

		m_SpinAngle  = 0;
		m_SpinTarget = revolutions * 360.0;
		m_SpinTotal  = m_SpinTarget;
		m_SpinActive = true;
	}

	override void AnimateSpin( float revolutions = 1.0 )
	{
		TriggerSpin( Math.Max( 1, revolutions ) );
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( !m_SpinActive )
			return;

		float step = timeSlice * 720.0;
		m_SpinAngle += step;

		if ( m_SpinAngle >= m_SpinTarget )
		{
			m_SpinAngle  = 0;
			m_SpinActive = false;
		}

		if ( m_Icon )
			m_Icon.SetRotation( 0, 0, m_SpinAngle );
	}
}
