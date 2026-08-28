// =============================================================================
//  JMUIConstants.c
//
//  Central registry of UI constants shared across the COT window/form system.
//  Use these instead of inline magic numbers so changes propagate everywhere.
// =============================================================================

class JMUIColors
{
	//! Pin button: active (pinned) - warm yellow
	static const int PIN_ACTIVE   = JMTheme.PIN_ACTIVE;
	//! Pin button: inactive (unpinned) - neutral grey
	static const int PIN_INACTIVE = JMTheme.PIN_INACTIVE;

	//! Window background alpha+RGB when focused / unfocused
	static const float BG_FOCUSED_A   = 0.9995;
	static const float BG_FOCUSED_R   = 0.06;
	static const float BG_FOCUSED_G   = 0.08;
	static const float BG_FOCUSED_B   = 0.11;

	//! Unfocused windows are meant to read as "behind" the one being worked in.
	//! At 0.95 the drop was invisible - the tab strip and the empty parts of a
	//! form sit directly on `background`, so that widget's alpha IS the window's
	//! transparency, and it has to fall far enough to let the world through.
	static const float BG_UNFOCUSED_A = 0.62;
	static const float BG_UNFOCUSED_R = 0.042;
	static const float BG_UNFOCUSED_G = 0.056;
	static const float BG_UNFOCUSED_B = 0.077;

	//! Title bar alpha+RGB when focused / unfocused
	static const float TITLE_FOCUSED_A   = 1.0;
	static const float TITLE_FOCUSED_R   = 0.04;
	static const float TITLE_FOCUSED_G   = 0.04;
	static const float TITLE_FOCUSED_B   = 0.12;

	static const float TITLE_UNFOCUSED_A = 0.72;
	static const float TITLE_UNFOCUSED_R = 0.02;
	static const float TITLE_UNFOCUSED_G = 0.02;
	static const float TITLE_UNFOCUSED_B = 0.06;
}

class JMUIAnimations
{
	//! Duration (seconds) of the window minimize/restore animation
	static const float WINDOW_MINIMIZE_TIME = 0.2;

	//! Duration (seconds) of the sidebar slide in/out animation
	static const float SIDEBAR_ANIMATE_TIME = 0.35;
}

class JMUILayout
{
	//! Pixels a newly opened window is offset from the previous one (cascade)
	static const float WINDOW_CASCADE_OFFSET = 20;

	//! Maximum cascade displacement before wrapping back
	static const float WINDOW_CASCADE_MAX = 200;

	//! Compact label column - narrower than LABEL_WIDTH (0.30), used in dense rows
	static const float LABEL_COMPACT      = 0.22;

	//! Standard two-button row: wide dominant button on the left
	static const float BTN_PAIR_WIDE      = 0.72;
	//! Standard two-button row: narrow secondary button on the right
	static const float BTN_PAIR_NARROW    = 0.26;
	//! X offset of the narrow button (= BTN_PAIR_WIDE + tiny gap)
	static const float BTN_PAIR_NARROW_X  = 0.74;

	//! Search-bar width when paired with a small refresh/action button
	static const float SEARCH_BAR_W       = 0.78;
	//! Small refresh/action button width when paired with a search bar
	static const float REFRESH_BTN_W      = 0.22;

	// --- Workspace draw order -----------------------------------------------
	//  Every COT screen widget is a direct child of the workspace, and so is the
	//  in-game HUD. Siblings draw in sort order, and the layout files' `priority`
	//  field is NOT that order - it only weights input routing. Left at the
	//  default sort the HUD wins, which is why the stance/quickbar arrows drew on
	//  top of the sidebar. These are set explicitly with SetSort().
	//
	//  Ordering rationale: the sidebar is the floor, a category flyout must clear
	//  the sidebar it springs from, and a module window must clear both.
	static const int SORT_SIDEBAR = 900;
	static const int SORT_FLYOUT  = 910;
	static const int SORT_WINDOW  = 920;
}

// =============================================================================
//  JMUIStyle - visual style constants for form content.
//  Use these in UIActionManager.CreatePanel / CreateSectionHeader calls so the
//  whole mod's form chrome changes from one place.
// =============================================================================
class JMUIStyle
{
	//! Section divider line colors (pass to CreatePanel / CreateSectionHeader)
	static const int DIVIDER_DARK   = JMTheme.DIVIDER_DARK;
	static const int DIVIDER_MEDIUM = JMTheme.DIVIDER_MEDIUM;
	static const int DIVIDER_LIGHT  = JMTheme.DIVIDER_LIGHT;

	//! Fully transparent panel (invisible spacer/background)
	static const int TRANSPARENT    = JMTheme.TRANSPARENT;

	//! Default fractional width of the label column in CreateLabeledRow (0..1)
	static const float LABEL_WIDTH  = 0.30;
}

// =============================================================================
//  JMUISurfaceStyle - one palette for every skinned surface in the mod.
//
//  Every control paints the same four layers: a fill, a 1px border frame with
//  a bevel (one edge lit, the opposite sunk), an optional accent bar, and the
//  label/icon tint. What changes between controls is the ROLE:
//
//    ROLE_BUTTON   raised   - lit top edge, sunk bottom. "Click me."
//    ROLE_INPUT    recessed - sunk top edge, lit bottom, darker fill, accent
//                             border while focused. "Type here."
//    ROLE_ROW      flat     - transparent at rest, tinted on hover, accent
//                             fill + bar when selected. List and table rows.
//    ROLE_SURFACE  flat     - static panel background, no hover response.
//    ROLE_HEADER   flat     - slightly lifted surface with an accent underline.
//
//  Variants (default / primary / danger / success / ghost) pick a base fill.
//  Every other colour is derived from that base with Blend(), so a caller
//  passing a custom SetColor( ARGB(...) ) gets the same treatment for free.
// =============================================================================
class JMUISurfaceStyle
{
	//! Roles - pass to UIActionSurfaceSkin.SetRole()
	static const int ROLE_BUTTON  = 0;
	static const int ROLE_INPUT   = 1;
	static const int ROLE_ROW     = 2;
	static const int ROLE_SURFACE = 3;
	static const int ROLE_HEADER  = 4;

	//! Variants - pass to UIActionSurfaceSkin.SetVariant()
	static const int VARIANT_DEFAULT = 0;
	static const int VARIANT_PRIMARY = 1;
	static const int VARIANT_DANGER  = 2;
	static const int VARIANT_SUCCESS = 3;
	static const int VARIANT_GHOST   = 4;

	//! Base fill per variant. Everything else is derived from these.
	static const int FILL_DEFAULT = JMTheme.SURFACE_RAISED;
	static const int FILL_PRIMARY = JMTheme.BLUE_600;
	static const int FILL_DANGER  = JMTheme.DANGER_DIM;
	static const int FILL_SUCCESS = JMTheme.SUCCESS_DIM;
	static const int FILL_GHOST   = JMTheme.Alpha( JMTheme.INK_600, 0x33 );

	//! Base fill per role, used when the variant is DEFAULT. A recessed input
	//! sits darker than the form behind it; a row starts fully transparent.
	static const int FILL_INPUT   = JMTheme.SURFACE_SUNKEN;
	static const int FILL_ROW     = JMTheme.TRANSPARENT;
	static const int FILL_SURFACE = JMTheme.Alpha( JMTheme.INK_700, 0xB3 );
	static const int FILL_HEADER  = JMTheme.SURFACE_HEADER;

	//! Label / icon tint
	static const int TEXT_IDLE     = JMTheme.TEXT_PRIMARY;
	static const int TEXT_HOVER    = JMTheme.TEXT_ON_ACCENT;
	static const int TEXT_DISABLED = JMTheme.TEXT_DISABLED;
	//! Dimmer label for static surfaces, where nothing is clickable
	static const int TEXT_MUTED    = JMTheme.TEXT_MUTED;

	//! Dim overlay drawn over a disabled control
	static const int DISABLE_OVERLAY = JMTheme.DISABLED_OVERLAY;

	//! Accent used by the neutral variant - the same blue as the tab bar, so
	//! hovering a plain control reads as "same family" as an active tab.
	static const int ACCENT_DEFAULT = JMTheme.ACCENT;

	//! Blend factors used to derive the state colours from the base fill
	static const float HOVER_LIGHTEN   = 0.22;  //!< fill -> white on hover
	static const float PRESS_DARKEN    = 0.28;  //!< fill -> black while held
	static const float BORDER_LIGHTEN  = 0.18;  //!< idle border brightness
	static const float BORDER_HOVER    = 0.45;  //!< hover border brightness
	static const float BEVEL_LIT       = 0.30;  //!< extra lift on the lit edge
	static const float BEVEL_SUNK      = 0.55;  //!< extra sink on the dark edge

	//! Alpha the accent bar fades to when the control is at rest.
	static const float ACCENT_IDLE_ALPHA = 0.0;

	// --- Sidebar ------------------------------------------------------------
	//! The sidebar deliberately breaks from the blue accent: its hover state is
	//! red, matching the COT brand mark, so the menu never looks like part of
	//! a form. A soft red wash behind the row plus a hard red bar on the left.
	static const int SIDEBAR_HOVER_FILL = JMTheme.SIDEBAR_HOVER_FILL;
	static const int SIDEBAR_PRESS_FILL = JMTheme.SIDEBAR_PRESS_FILL;
	static const int SIDEBAR_ACCENT     = JMTheme.SIDEBAR_ACCENT;
	static const int SIDEBAR_TEXT_IDLE  = JMTheme.SIDEBAR_TEXT_IDLE;
	static const int SIDEBAR_TEXT_HOVER = JMTheme.SIDEBAR_TEXT_HOVER;

	//! Width in px of the red bar down the left edge of a hovered sidebar row.
	static const int SIDEBAR_ACCENT_W = 4;

	//! Hover tint strength for ROLE_ROW, which has no fill of its own.
	static const int ROW_HOVER    = JMTheme.ACCENT_WASH;
	static const int ROW_SELECTED = JMTheme.Alpha( JMTheme.BLUE_600, 0x66 );

	// -------------------------------------------------------------------------
	//  Blend - linear interpolate every channel (alpha included) of two ARGB
	//  colours. t = 0 returns color, t = 1 returns target.
	//
	//  @note the channel unpacks MUST land in int locals. Enforce resolves an
	//  operator against the type of the assignment target, so
	//  `float x = ( color >> 24 ) & 0xFF;` fails to compile with
	//  "Unknown operator '>>'" - there is no float shift. Unpack as int, then
	//  let the int -> float promotion happen inside the arithmetic.
	// -------------------------------------------------------------------------
	static int Blend( int color, int target, float t )
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

	//! Lighten toward opaque white.
	static int Lighten( int color, float t )
	{
		return Blend( color, 0xFFFFFFFF, t );
	}

	//! Darken toward opaque black.
	static int Darken( int color, float t )
	{
		return Blend( color, 0xFF000000, t );
	}

	//! Replace a colour's alpha channel (0..255) leaving RGB untouched.
	static int WithAlpha( int color, int alpha )
	{
		return ( color & 0x00FFFFFF ) | ( ( alpha & 0xFF ) << 24 );
	}

	//! Base fill for a variant id. Unknown ids fall back to DEFAULT.
	static int FillForVariant( int variant )
	{
		if ( variant == VARIANT_PRIMARY )
			return FILL_PRIMARY;
		if ( variant == VARIANT_DANGER )
			return FILL_DANGER;
		if ( variant == VARIANT_SUCCESS )
			return FILL_SUCCESS;
		if ( variant == VARIANT_GHOST )
			return FILL_GHOST;
		return FILL_DEFAULT;
	}

	//! Base fill for a role when the variant is DEFAULT.
	static int FillForRole( int role )
	{
		if ( role == ROLE_INPUT )
			return FILL_INPUT;
		if ( role == ROLE_ROW )
			return FILL_ROW;
		if ( role == ROLE_SURFACE )
			return FILL_SURFACE;
		if ( role == ROLE_HEADER )
			return FILL_HEADER;
		return FILL_DEFAULT;
	}

	//! True when the role's bevel runs lit-top / sunk-bottom (raised). Inputs
	//! invert it so the field reads as a hole punched into the form.
	static bool IsRaised( int role )
	{
		return role == ROLE_BUTTON || role == ROLE_HEADER;
	}

	//! True when the role reacts to the cursor at all.
	static bool IsInteractive( int role )
	{
		return role != ROLE_SURFACE && role != ROLE_HEADER;
	}
}
