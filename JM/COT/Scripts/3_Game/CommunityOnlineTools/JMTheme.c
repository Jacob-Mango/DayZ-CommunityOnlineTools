// =============================================================================
//  JMTheme - the single source of truth for every colour in Community Online
//  Tools.
//
//  The palette is daisyUI's "dark" theme, converted from OKLCH to sRGB once,
//  here, so the rest of the mod keeps working in plain ARGB ints:
//
//    base-100  #1D232A   base-200  #191E24   base-300  #15191E
//    base-content #ECF9FF                    neutral   #09090B
//    primary   #605DFF   secondary #F43098   accent    #00D3BB
//    info      #00BAFE   success   #00D390   warning   #FCB700   error #FF627D
//
//  Two habits carry over from daisyUI and are worth keeping:
//
//    1. Surfaces come from the base ramp, never from the accent ramp. base-300
//       is the deepest (inputs, headers), base-100 the highest (cards, buttons).
//    2. Borders and de-emphasised text are base-content at low alpha, not a
//       separate grey. That keeps an edge readable on every surface in the ramp
//       instead of only on the one it happened to be picked against.
//
//  Layers
//  ------
//    1. RAMPS      - the raw palette: base ladder, primary ladder, status set.
//                    Prefer a semantic token below.
//    2. SEMANTIC   - what a colour is FOR (surface, border, text, accent).
//    3. COMPONENT  - the handful of places that need their own named value.
//
//  Lives in 3_Game so every layer (3_Game / 4_World / 5_Mission) can read it.
//
//  @note ARGB order, as everywhere else in Enfusion: 0xAARRGGBB.
// =============================================================================
class JMTheme
{
	// -------------------------------------------------------------------------
	//  1. RAMPS
	// -------------------------------------------------------------------------

	//! Base ladder, darkest to lightest. INK_900..INK_600 are daisyUI's own
	//! neutral / base-300 / base-200 / base-100; the lighter steps are base-100
	//! mixed toward base-content, which is how daisyUI derives its own greys.
	static const int INK_900               = 0xFF09090B;  //!< neutral
	static const int INK_800               = 0xFF15191E;  //!< base-300
	static const int INK_700               = 0xFF191E24;  //!< base-200
	static const int INK_600               = 0xFF1D232A;  //!< base-100
	static const int INK_500               = 0xFF32383F;
	static const int INK_400               = 0xFF464E55;
	static const int INK_300               = 0xFF656E75;
	static const int INK_200               = 0xFF8F999F;
	static const int INK_100               = 0xFFB8C4CA;
	static const int INK_50                = 0xFFECF9FF;  //!< base-content
	static const int WHITE                 = 0xFFFFFFFF;
	static const int BLACK                 = 0xFF000000;

	//! Primary ladder. BLUE_500 is daisyUI's primary; darker steps sink toward
	//! base-300 so they stay usable as fills, lighter steps rise toward
	//! base-content. The names stay BLUE_* so existing call sites keep working -
	//! the hue is violet now.
	static const int BLUE_800              = 0xFF2C2D62;
	static const int BLUE_700              = 0xFF3A3B8E;
	static const int BLUE_600              = 0xFF4D4CC7;
	static const int BLUE_500              = 0xFF605DFF;  //!< primary
	static const int BLUE_400              = 0xFF7979FF;
	static const int BLUE_300              = 0xFF9194FF;
	static const int BLUE_200              = 0xFFADB3FF;
	static const int BLUE_100              = 0xFFEDF1FE;  //!< primary-content

	//! Status colours, straight from the theme. Brighter than the old muted set
	//! because daisyUI's dark palette expects them to carry on a dark surface.
	static const int DANGER                = 0xFFFF627D;  //!< error
	static const int DANGER_DIM            = 0xFF4D0218;  //!< error-content
	static const int DANGER_BRIGHT         = 0xFFF98FA4;
	//! Fill for a FILLED destructive control. DANGER is tuned to read as text,
	//! an icon or a badge on a dark surface; at button size that same value is a
	//! slab of bright pink next to the calm base-100 pills around it. A red
	//! button therefore fills with this darker step and lets SetColor() derive
	//! its hover / pressed / ring shades from it, which keeps the ring in the
	//! DANGER hue without the fill shouting.
	static const int DANGER_FILL           = 0xFF8C2438;
	static const int WARNING               = 0xFFFCB700;  //!< warning
	static const int WARNING_DIM           = 0xFF793205;  //!< warning-content
	static const int SUCCESS               = 0xFF00D390;  //!< success
	static const int SUCCESS_DIM           = 0xFF004C39;  //!< success-content
	//! Fill for a FILLED confirming control, the green twin of DANGER_FILL.
	//! SUCCESS is a bright mint tuned to read as text, an icon or a badge on a
	//! dark surface; at button size that same value is a slab of neon next to
	//! the calm base-100 pills around it. A green button therefore fills with
	//! this deeper step and lets SetColor() derive its hover / pressed / ring
	//! shades from it, which keeps the SUCCESS hue without the fill shouting.
	static const int SUCCESS_FILL          = 0xFF0C6B4F;
	static const int INFO                  = 0xFF00BAFE;  //!< info

	//! The remaining theme roles, kept whole so a call site that genuinely wants
	//! daisyUI's secondary or accent can name it.
	static const int SECONDARY             = 0xFFF43098;  //!< secondary
	static const int SECONDARY_CONTENT     = 0xFFF9E4F0;  //!< secondary-content
	static const int ACCENT_TEAL           = 0xFF00D3BB;  //!< accent
	static const int ACCENT_TEAL_DIM       = 0xFF084D49;  //!< accent-content
	static const int NEUTRAL               = 0xFF09090B;  //!< neutral
	static const int NEUTRAL_CONTENT       = 0xFFE4E4E7;  //!< neutral-content
	static const int INFO_CONTENT          = 0xFF042E49;  //!< info-content
	static const int SUCCESS_CONTENT       = 0xFF004C39;  //!< success-content
	static const int WARNING_CONTENT       = 0xFF793205;  //!< warning-content
	static const int ERROR_CONTENT         = 0xFF4D0218;  //!< error-content

	//! Fully transparent - use instead of a bare 0 so intent is readable.
	static const int TRANSPARENT           = 0x00000000;

	// -------------------------------------------------------------------------
	//  2. SEMANTIC
	// -------------------------------------------------------------------------

	//! Surfaces, from the deepest hole to the highest floating panel. daisyUI
	//! paints these opaque; the mod keeps a little translucency because the game
	//! world sits behind every window.
	static const int SURFACE_SUNKEN        = 0xF215191E;  //!< inside an input field
	static const int SURFACE_BASE          = 0xE6191E24;  //!< a form's body
	static const int SURFACE_RAISED        = 0xF0222932;  //!< a button, a tile
	static const int SURFACE_HEADER        = 0xF215191E;  //!< section + tab headers
	static const int SURFACE_OVERLAY       = 0xF71D232A;  //!< popups, flyouts, menus

	//! Panel backing for the sidebar and its flyouts. Enfusion cannot blur what
	//! is behind a widget, so this is a near-opaque base rather than a true
	//! frosted pane - it keeps the menu legible over any backdrop.
	static const int SURFACE_GLASS         = 0xF2191E24;
	static const int SURFACE_GLASS_HEADER  = 0xF715191E;
	static const int SURFACE_GLASS_EDGE    = 0x26ECF9FF;  //!< lit hairline under the header

	//! Sidebar entry tint marking a module whose window is currently open.
	static const int MENU_ACTIVE           = 0x59605DFF;

	//! Borders and rules - base-content at low alpha, per daisyUI.
	static const int BORDER_SUBTLE         = 0x24ECF9FF;
	static const int BORDER_BASE           = 0x2EECF9FF;
	static const int BORDER_STRONG         = 0x4DECF9FF;
	static const int BORDER_ACCENT         = 0xFF605DFF;

	//! The ring around an interactive pill. daisyUI marks focus and selection
	//! with the primary colour, not with a separate hue.
	static const int BORDER_RING           = 0xFF605DFF;
	static const int BORDER_RING_HOVER     = 0xFF8384FF;

	//! Divider rules, darkest to lightest.
	static const int DIVIDER_DARK          = 0xFF15191E;
	static const int DIVIDER_MEDIUM        = 0x1AECF9FF;
	static const int DIVIDER_LIGHT         = 0x2EECF9FF;

	//! Text. Secondary / muted / disabled are base-content faded, matching how
	//! daisyUI expresses de-emphasis with opacity.
	static const int TEXT_PRIMARY          = 0xFFECF9FF;  //!< base-content
	static const int TEXT_SECONDARY        = 0xB3ECF9FF;
	static const int TEXT_MUTED            = 0x80ECF9FF;
	static const int TEXT_DISABLED         = 0x59ECF9FF;
	static const int TEXT_ON_ACCENT        = 0xFFEDF1FE;  //!< primary-content

	//! Interactive accent.
	static const int ACCENT                = 0xFF605DFF;
	static const int ACCENT_HOVER          = 0xFF7979FF;
	static const int ACCENT_PRESSED        = 0xFF4D4CC7;
	//! Translucent accent wash, for hover backgrounds over a dark surface.
	static const int ACCENT_WASH           = 0x40605DFF;
	static const int ACCENT_WASH_STRONG    = 0x73605DFF;

	//! Dim layer drawn over a disabled control.
	static const int DISABLED_OVERLAY      = 0xA615191E;

	// -------------------------------------------------------------------------
	//  3. COMPONENT
	// -------------------------------------------------------------------------

	//! Window title bar, focused / unfocused.
	static const int TITLE_FOCUSED         = 0xFF15191E;
	static const int TITLE_UNFOCUSED       = 0xFF09090B;
	//! Close button hover - the one destructive affordance, so it stays red.
	static const int TITLE_CLOSE_HOVER     = 0xCCFF627D;

	//! The rounded pill, shared by every clickable surface in the mod: action
	//! buttons, tab-bar tabs, filter chips, icon-grid cells. The SHAPE is what
	//! makes them one family; the colour separates "you can click this" from
	//! "this one is on".
	//!
	//!   SELECTED_*  the primary violet - reserved for the one thing that is ON
	//!               (current tab, active filter, chosen icon).
	//!   BUTTON_*    daisyUI's default button: a base-100 surface with a faint
	//!               base-content border. A form is mostly buttons, so these stay
	//!               calm and let the primary pill carry the meaning.
	static const int SELECTED_FILL         = 0xFF605DFF;
	static const int SELECTED_FILL_HOVER   = 0xFF7979FF;
	static const int SELECTED_OUTLINE      = 0xFF9194FF;
	//! An unselected pill in a group draws no fill at all, so only the ON one
	//! carries weight.
	static const int SELECTED_FILL_OFF     = 0x0015191E;

	static const int BUTTON_FILL           = 0xF0222932;
	static const int BUTTON_FILL_HOVER     = 0xF73A424C;
	//! Press has to read at a glance, so it drops well below base-300 rather
	//! than one step down from the resting fill, and the ring goes primary.
	static const int BUTTON_FILL_PRESSED   = 0xFF12171D;
	static const int BUTTON_OUTLINE_PRESSED = 0xFF605DFF;
	static const int BUTTON_OUTLINE        = 0x2EECF9FF;
	static const int BUTTON_OUTLINE_HOVER  = 0xFF605DFF;
	static const int BUTTON_TEXT           = 0xFFECF9FF;
	static const int BUTTON_TEXT_INACTIVE  = 0x80ECF9FF;

	//! Pin toggle.
	static const int PIN_ACTIVE            = 0xFFFCB700;
	static const int PIN_INACTIVE          = 0x80ECF9FF;

	//! Sidebar rows: primary wash behind, hard primary bar on the left.
	static const int SIDEBAR_HOVER_FILL    = 0x4D605DFF;
	static const int SIDEBAR_PRESS_FILL    = 0x734D4CC7;
	static const int SIDEBAR_ACCENT        = 0xFF7979FF;
	static const int SIDEBAR_TEXT_IDLE     = 0xFFECF9FF;
	static const int SIDEBAR_TEXT_HOVER    = 0xFFEDF1FE;

	//! Value readouts that grade good -> caution -> bad (health, blood, ...).
	static const int VALUE_OK              = 0xFFECF9FF;
	static const int VALUE_CAUTION         = 0xFFFCB700;
	static const int VALUE_BAD             = 0xFFFF627D;
	// -------------------------------------------------------------------------
	//  Helpers
	// -------------------------------------------------------------------------

	//! Same colour at a different opacity. alpha is 0..255.
	static int Alpha( int color, int alpha )
	{
		return ( color & 0x00FFFFFF ) | ( ( alpha & 0xFF ) << 24 );
	}

	//! Same colour at a fractional opacity, 0.0 .. 1.0.
	static int Fade( int color, float opacity )
	{
		int a = (int)Math.Clamp( opacity * 255, 0, 255 );
		return Alpha( color, a );
	}

	//! Linear interpolate every channel (alpha included) of two ARGB colours.
	//! t = 0 returns color, t = 1 returns target.
	//!
	//! @note the channel unpack MUST stay in int locals. Enforce resolves an
	//! operator against the type of the assignment target, so a float target
	//! makes the shift unresolvable ("Unknown operator '>>'").
	static int Mix( int color, int target, float t )
	{
		int ca = ( color  >> 24 ) & 0xFF;
		int cr = ( color  >> 16 ) & 0xFF;
		int cg = ( color  >>  8 ) & 0xFF;
		int cb =   color          & 0xFF;

		int ta = ( target >> 24 ) & 0xFF;
		int tr = ( target >> 16 ) & 0xFF;
		int tg = ( target >>  8 ) & 0xFF;
		int tb =   target         & 0xFF;

		int a = (int)Math.Clamp( ca + ( ta - ca ) * t, 0, 255 );
		int r = (int)Math.Clamp( cr + ( tr - cr ) * t, 0, 255 );
		int g = (int)Math.Clamp( cg + ( tg - cg ) * t, 0, 255 );
		int b = (int)Math.Clamp( cb + ( tb - cb ) * t, 0, 255 );

		return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
	}

	static int Lighten( int color, float t )
	{
		return Mix( color, WHITE, t );
	}

	static int Darken( int color, float t )
	{
		return Mix( color, BLACK, t );
	}
}
