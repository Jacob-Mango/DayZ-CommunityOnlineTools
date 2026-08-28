// =============================================================================
//  UIActionSurfaceSkin
//
//  Shared painter for every skinned surface in the COT UI. It owns no events -
//  the owning UIAction feeds it state (hover / press / focus / selected /
//  enabled) and the skin repaints the fill, the bevelled border frame, the
//  accent bar and the label + icon tint from the JMUISurfaceStyle palette.
//
//  A skin binds to a *frame container*: any widget holding children named
//  "fill", "border_top", "border_left", "border_bottom", "border_right" and
//  optionally "accent". A layout can therefore carry several independent
//  skinned regions - a labelled row can skin just its input field and leave
//  the label area untouched.
//
//  Why a helper object instead of a base class: the skinned UIActions are
//  siblings under UIActionBase, not a chain, and Enforce Script has no
//  multiple inheritance. So the shared paint code lives here and each control
//  composes it.
//
//  Usage:
//      m_Skin = new UIActionSurfaceSkin();
//      m_Skin.SetRole( JMUISurfaceStyle.ROLE_INPUT );
//      m_Skin.Attach( layoutRoot.FindAnyWidget( "action_field" ) );
//      ... m_Skin.SetHovered( true ) / SetFocused( true ) / SetEnabled( false )
// =============================================================================
class UIActionSurfaceSkin
{
	protected Widget      m_Fill;
	protected Widget      m_BorderTop;
	protected Widget      m_BorderLeft;
	protected Widget      m_BorderBottom;
	protected Widget      m_BorderRight;
	protected Widget      m_Accent;
	protected Widget      m_Disable;
	protected TextWidget  m_Text;
	protected ImageWidget m_Icon;

	//! Resolved state colours - rebuilt by SetRole / SetVariant / SetBaseColor
	protected int m_FillIdle;
	protected int m_FillHover;
	protected int m_FillPress;
	protected int m_BorderIdle;
	protected int m_BorderHover;
	protected int m_AccentColor;
	protected int m_TextIdle;
	protected int m_TextHover;

	protected int  m_Role;
	protected int  m_Variant;
	protected bool m_Hovered;
	protected bool m_Pressed;
	protected bool m_Focused;
	protected bool m_Selected;
	protected bool m_Enabled;
	protected bool m_Attached;

	void UIActionSurfaceSkin()
	{
		m_Enabled   = true;
		m_Role      = JMUISurfaceStyle.ROLE_BUTTON;
		m_TextIdle  = JMUISurfaceStyle.TEXT_IDLE;
		m_TextHover = JMUISurfaceStyle.TEXT_HOVER;
		Rebuild();
	}

	//! Bind to a frame container. Every widget is optional - an image-only
	//! button has no "action" text, a row has no border frame.
	void Attach( Widget frameRoot )
	{
		if ( !frameRoot )
			return;

		m_Fill         = frameRoot.FindAnyWidget( "fill"          );
		m_BorderTop    = frameRoot.FindAnyWidget( "border_top"    );
		m_BorderLeft   = frameRoot.FindAnyWidget( "border_left"   );
		m_BorderBottom = frameRoot.FindAnyWidget( "border_bottom" );
		m_BorderRight  = frameRoot.FindAnyWidget( "border_right"  );
		m_Accent       = frameRoot.FindAnyWidget( "accent"        );

		// The disable overlay always lives on the action root, which may be an
		// ancestor of the frame container, so look for it there too.
		m_Disable = frameRoot.FindAnyWidget( "action_wrapper_disable" );

		if ( !m_Text )
			Class.CastTo( m_Text, frameRoot.FindAnyWidget( "action" ) );

		if ( !m_Icon )
		{
			if ( !Class.CastTo( m_Icon, frameRoot.FindAnyWidget( "action_icon" ) ) )
				Class.CastTo( m_Icon, frameRoot.FindAnyWidget( "action_image" ) );
		}

		if ( m_Disable )
			m_Disable.SetColor( JMUISurfaceStyle.DISABLE_OVERLAY );

		m_Attached = true;

		Apply();
	}

	//! Bind the disable overlay separately when it is not under the frame.
	void SetDisableOverlay( Widget w )
	{
		m_Disable = w;

		if ( m_Disable )
			m_Disable.SetColor( JMUISurfaceStyle.DISABLE_OVERLAY );
	}

	//! Point the skin at the label / icon it should tint. Call before Attach()
	//! when the widget names are not the conventional ones.
	void SetLabelWidget( TextWidget w )
	{
		m_Text = w;
		Apply();
	}

	void SetIconWidget( ImageWidget w )
	{
		m_Icon = w;
		Apply();
	}

	// -------------------------------------------------------------------------
	//  Palette
	// -------------------------------------------------------------------------

	//! Pick the surface role (button / input / row / surface / header).
	void SetRole( int role )
	{
		m_Role = role;

		if ( role == JMUISurfaceStyle.ROLE_SURFACE )
			m_TextIdle = JMUISurfaceStyle.TEXT_MUTED;
		else
			m_TextIdle = JMUISurfaceStyle.TEXT_IDLE;

		Rebuild();
	}

	int GetRole()
	{
		return m_Role;
	}

	//! Pick one of the JMUISurfaceStyle.VARIANT_* presets.
	void SetVariant( int variant )
	{
		m_Variant = variant;
		Rebuild();
	}

	int GetVariant()
	{
		return m_Variant;
	}

	//! Resolve the whole palette from the current role + variant.
	protected void Rebuild()
	{
		int base_color;
		if ( m_Variant == JMUISurfaceStyle.VARIANT_DEFAULT )
			base_color = JMUISurfaceStyle.FillForRole( m_Role );
		else
			base_color = JMUISurfaceStyle.FillForVariant( m_Variant );

		SetBaseColor( base_color );
	}

	//! Drive the whole palette from one custom base fill. Keeps the caller's
	//! hue while still producing a hover, a press and a matching border.
	void SetBaseColor( int color )
	{
		m_FillIdle    = color;
		m_FillHover   = JMUISurfaceStyle.Lighten( color, JMUISurfaceStyle.HOVER_LIGHTEN );
		m_FillPress   = JMUISurfaceStyle.Darken(  color, JMUISurfaceStyle.PRESS_DARKEN  );
		m_BorderIdle  = JMUISurfaceStyle.Lighten( color, JMUISurfaceStyle.BORDER_LIGHTEN );
		m_BorderHover = JMUISurfaceStyle.Lighten( color, JMUISurfaceStyle.BORDER_HOVER   );
		m_AccentColor = m_BorderHover;

		// A near-black base lightens to flat grey, which reads as "disabled"
		// rather than "interactive". Blend toward the app accent instead.
		bool neutral = m_Variant == JMUISurfaceStyle.VARIANT_DEFAULT;
		if ( neutral || m_Variant == JMUISurfaceStyle.VARIANT_GHOST )
		{
			m_FillHover   = JMUISurfaceStyle.Blend( color, JMUISurfaceStyle.ACCENT_DEFAULT, 0.45 );
			m_BorderHover = JMUISurfaceStyle.ACCENT_DEFAULT;
			m_AccentColor = JMUISurfaceStyle.ACCENT_DEFAULT;
		}

		// A transparent row has nothing to lighten, so its hover and selected
		// tints come from the palette directly.
		if ( m_Role == JMUISurfaceStyle.ROLE_ROW )
		{
			m_FillHover = JMUISurfaceStyle.ROW_HOVER;
			m_FillPress = JMUISurfaceStyle.ROW_SELECTED;
		}

		Apply();
	}

	//! Override just the hover fill, keeping the resting look. Used for ghost
	//! controls that sit transparent until pointed at - a title-bar close
	//! button that flushes red on hover, for instance.
	void SetHoverFill( int color )
	{
		m_FillHover = color;
		Apply();
	}

	//! Override the accent bar colour independently of the fill palette.
	void SetAccentColor( int color )
	{
		m_AccentColor = color;
		Apply();
	}

	//! Override the pressed fill, keeping the resting and hover looks.
	void SetPressFill( int color )
	{
		m_FillPress = color;
		Apply();
	}

	void SetTextColors( int idle, int hover )
	{
		m_TextIdle  = idle;
		m_TextHover = hover;
		Apply();
	}

	// -------------------------------------------------------------------------
	//  State
	// -------------------------------------------------------------------------

	void SetHovered( bool hovered )
	{
		if ( m_Hovered == hovered )
			return;

		m_Hovered = hovered;
		Apply();
	}

	void SetPressed( bool pressed )
	{
		if ( m_Pressed == pressed )
			return;

		m_Pressed = pressed;
		Apply();
	}

	//! Keyboard focus - inputs light their border while being typed into.
	void SetFocused( bool focused )
	{
		if ( m_Focused == focused )
			return;

		m_Focused = focused;
		Apply();
	}

	//! Sticky "on" look: a toggled button, a selected row, the active tab.
	void SetSelected( bool selected )
	{
		if ( m_Selected == selected )
			return;

		m_Selected = selected;
		Apply();
	}

	bool IsSelected()
	{
		return m_Selected;
	}

	void SetEnabled( bool enabled )
	{
		if ( m_Enabled == enabled )
			return;

		m_Enabled = enabled;

		if ( !enabled )
		{
			m_Hovered = false;
			m_Pressed = false;
			m_Focused = false;
		}

		Apply();
	}

	// -------------------------------------------------------------------------
	//  Paint
	// -------------------------------------------------------------------------

	//! Repaint every layer for the current state.
	void Apply()
	{
		if ( !m_Attached )
			return;

		int   fill        = m_FillIdle;
		int   border      = m_BorderIdle;
		int   text        = m_TextIdle;
		float accentAlpha = JMUISurfaceStyle.ACCENT_IDLE_ALPHA;

		bool interactive = JMUISurfaceStyle.IsInteractive( m_Role );

		if ( !m_Enabled )
		{
			text = JMUISurfaceStyle.TEXT_DISABLED;
		}
		else if ( interactive && m_Pressed )
		{
			fill        = m_FillPress;
			border      = m_BorderHover;
			text        = m_TextHover;
			accentAlpha = 1.0;
		}
		else if ( interactive && m_Hovered )
		{
			fill        = m_FillHover;
			border      = m_BorderHover;
			text        = m_TextHover;
			accentAlpha = 1.0;
		}
		else if ( m_Focused )
		{
			// Focus keeps the resting fill - only the frame lights up, so the
			// text stays readable while typing.
			border      = m_BorderHover;
			text        = m_TextHover;
			accentAlpha = 1.0;
		}
		else if ( m_Selected )
		{
			if ( m_Role == JMUISurfaceStyle.ROLE_ROW )
				fill = JMUISurfaceStyle.ROW_SELECTED;
			else
				fill = m_FillHover;

			border      = m_BorderHover;
			text        = m_TextHover;
			accentAlpha = 0.85;
		}
		else if ( m_Variant != JMUISurfaceStyle.VARIANT_DEFAULT && m_Variant != JMUISurfaceStyle.VARIANT_GHOST )
		{
			// Semantic controls (primary / danger / success) keep a dimmed
			// accent bar at rest so they are identifiable without hovering.
			accentAlpha = 0.7;
		}
		else if ( m_Role == JMUISurfaceStyle.ROLE_HEADER )
		{
			// A header's accent is its underline - always on.
			accentAlpha = 0.9;
		}

		// Bevel: one edge lit, the opposite sunk. Raised roles run lit-top;
		// inputs invert it so the field reads as a hole in the form. A pressed
		// control inverts again, so it visibly pushes in.
		bool raised = JMUISurfaceStyle.IsRaised( m_Role );
		if ( m_Pressed && interactive )
			raised = !raised;

		int bevelTop;
		int bevelBottom;
		if ( raised )
		{
			bevelTop    = JMUISurfaceStyle.Lighten( border, JMUISurfaceStyle.BEVEL_LIT  );
			bevelBottom = JMUISurfaceStyle.Darken(  border, JMUISurfaceStyle.BEVEL_SUNK );
		}
		else
		{
			bevelTop    = JMUISurfaceStyle.Darken(  border, JMUISurfaceStyle.BEVEL_SUNK );
			bevelBottom = JMUISurfaceStyle.Lighten( border, JMUISurfaceStyle.BEVEL_LIT  );
		}

		if ( m_Fill )
			m_Fill.SetColor( fill );

		if ( m_BorderTop )
			m_BorderTop.SetColor( bevelTop );

		if ( m_BorderBottom )
			m_BorderBottom.SetColor( bevelBottom );

		if ( m_BorderLeft )
			m_BorderLeft.SetColor( border );

		if ( m_BorderRight )
			m_BorderRight.SetColor( border );

		if ( m_Accent )
			m_Accent.SetColor( JMUISurfaceStyle.WithAlpha( m_AccentColor, (int)( accentAlpha * 255 ) ) );

		if ( m_Text )
			m_Text.SetColor( text );

		if ( m_Icon )
			m_Icon.SetColor( text );
	}
}
