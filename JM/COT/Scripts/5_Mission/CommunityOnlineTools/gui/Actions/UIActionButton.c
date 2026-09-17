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
	//! Flat-mode icon slot: the gap before the glyph, and the total width the
	//! glyph reserves before the label starts.
	static const float ICON_SLOT_PAD = 4.0;
	static const float ICON_SLOT_W   = 30.0;

	//! How fast the icon swings to a new resting angle. Matches the fold
	//! panel's slide so a chevron and the rows it opens move together.
	static const float ICON_ROTATE_SPEED = 14.0;

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

	//! Chrome-free mode: no pill fill or ring at rest, label hard left. Used by
	//! tree rows, where a column of pills buries the hierarchy it is meant to
	//! show. See SetFlat.
	protected bool  m_Flat;
	//! Pixels the icon and label are pushed in from the left edge, which is how
	//! a flat row encodes its depth. See SetContentIndent.
	protected float m_ContentIndent;

	protected bool  m_SpinActive;
	protected float m_SpinAngle;
	protected float m_SpinTotal;
	protected float m_SpinTarget;

	//! Resting angle of the icon, in degrees, animated. A chevron that swings
	//! from pointing right to pointing down states that the thing under it
	//! opened; swapping one glyph for another states only that something is
	//! different now. See SetIconRotation.
	protected ref JMAnimFloat m_IconAngle;

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
		m_Flat         = false;
		m_ContentIndent = 0;

		m_IconAngle = new JMAnimFloat();
		m_IconAngle.Set( 0 );

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

			if ( m_Flat )
			{
				ApplyContentIndent();
				return;
			}

			if ( m_Text )
				m_Text.SetTextOffset( 0, 0 );
			return;
		}

		m_Icon.LoadImageFile( 0, imagePath );
		m_Icon.Show( true );

		if ( m_Flat )
		{
			ApplyContentIndent();
			return;
		}

		// Shift text right so it stays visually centered in the remaining space
		if ( m_Text )
			m_Text.SetTextOffset( 16, 0 );
	}

	//! Drop the pill chrome and left-align the label, keeping the hit area and
	//! the click event intact.
	//!
	//! A tree is read by its indentation, and a filled, ringed pill per node
	//! draws a hard box around every row, which competes with that. Flat rows
	//! stay quiet at rest and light up with a faint accent wash on hover, so
	//! the affordance survives without the chrome.
	void SetFlat( bool flat )
	{
		m_Flat = flat;

		if ( flat )
		{
			m_FillColor      = JMTheme.TRANSPARENT;
			m_FillHover      = JMTheme.ACCENT_WASH;
			m_FillPressed    = JMTheme.ACCENT_WASH_STRONG;
			m_OutlineColor   = JMTheme.TRANSPARENT;
			m_OutlineHover   = JMTheme.TRANSPARENT;
			m_OutlinePressed = JMTheme.TRANSPARENT;

			if ( m_Text )
				m_Text.ClearFlags( WidgetFlags.CENTER );
		}
		else
		{
			m_FillColor      = JMTheme.BUTTON_FILL;
			m_FillHover      = JMTheme.BUTTON_FILL_HOVER;
			m_FillPressed    = JMTheme.BUTTON_FILL_PRESSED;
			m_OutlineColor   = JMTheme.BUTTON_OUTLINE;
			m_OutlineHover   = JMTheme.BUTTON_OUTLINE_HOVER;
			m_OutlinePressed = JMTheme.BUTTON_OUTLINE_PRESSED;

			if ( m_Text )
				m_Text.SetFlags( WidgetFlags.CENTER );
		}

		ApplyContentIndent();
		ApplyButtonColors();
	}

	bool IsFlat()
	{
		return m_Flat;
	}

	//! How far in from the left edge the icon and the label sit, in pixels.
	//! Only a flat button honours this; a pill centres its label by design.
	void SetContentIndent( float px )
	{
		m_ContentIndent = px;
		ApplyContentIndent();
	}

	//! The icon slot is reserved whether or not an icon is loaded, so a leaf
	//! row with no chevron still lines its label up with its siblings.
	protected void ApplyContentIndent()
	{
		if ( !m_Flat )
			return;

		if ( m_Icon )
		{
			m_Icon.SetPos( m_ContentIndent + ICON_SLOT_PAD, 0 );
			m_Icon.Update();
		}

		if ( m_Text )
			m_Text.SetTextOffset( m_ContentIndent + ICON_SLOT_W, 0 );
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

	//! Colour of the LABEL. NOT SetColor - the base and the override below both
	//! paint the pill, so a caller reaching for "grey this button out" with
	//! SetColor gets a grey pill behind unchanged text instead.
	void SetTextColor( int color )
	{
		if ( m_Text )
			m_Text.SetColor( color );
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
		if ( m_ChromeFocused && !m_IsPressed && !m_Flat )
			ringColor = JMTheme.BORDER_RING;

		if ( m_Fill )
			m_Fill.SetColor( fillColor );

		if ( m_Outline )
			m_Outline.SetColor( ringColor );
	}

	//! Turn the icon to `degrees` and leave it there.
	//!
	//! This is the icon's RESTING angle, not a one-shot: TriggerSpin borrows
	//! the icon while it runs and hands it back at this angle afterwards.
	void SetIconRotation( float degrees, bool animate = true )
	{
		if ( !m_IconAngle )
			return;

		if ( animate )
		{
			m_IconAngle.SetTarget( degrees, ICON_ROTATE_SPEED );
			return;
		}

		m_IconAngle.Set( degrees );
		ApplyIconRotation();
	}

	float GetIconRotation()
	{
		if ( !m_IconAngle )
			return 0;

		return m_IconAngle.Target;
	}

	protected void ApplyIconRotation()
	{
		// A spin is mid-flight and owns the icon; it restores this angle when
		// it lands, so writing over it here would stutter the revolution.
		if ( m_Icon && !m_SpinActive && m_IconAngle )
			m_Icon.SetRotation( 0, 0, m_IconAngle.Value );
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

		if ( m_IconAngle && m_IconAngle.Step( timeSlice ) )
			ApplyIconRotation();

		if ( !m_SpinActive )
			return;

		float step = timeSlice * 720.0;
		m_SpinAngle += step;

		if ( m_SpinAngle >= m_SpinTarget )
		{
			m_SpinAngle  = 0;
			m_SpinActive = false;

			// Hand the icon back to its resting angle rather than to zero.
			ApplyIconRotation();
			return;
		}

		if ( m_Icon )
			m_Icon.SetRotation( 0, 0, m_SpinAngle );
	}
}
