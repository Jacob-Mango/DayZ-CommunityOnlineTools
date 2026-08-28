// =============================================================================
//  UIActionCard
//
//  A bordered container with an optional title bar - the "category" chrome:
//  fill, 8px rounded ring, a header strip with a hairline under it, and a
//  padded body everything else goes into.
//
//  It replaces the CreateSectionHeader idiom (a title, then a 2px rule, then
//  loose rows as siblings of both). A rule under a title does not group
//  anything: with four sections stacked in one scroller there was nothing to
//  say where one ended and the next began. A box does that by existing.
//
//  Usage:
//      UIActionCard card = UIActionManager.CreateCard( parent, "#STR_..._VITALS" );
//      Widget       body = card.GetContent();
//      UIActionManager.CreateSlider( body, ... );
//
//  Pass "" as the title for a card with no header - an identity block, say,
//  where the rows are self-labelling and a title bar is just noise.
//
//  ---------------------------------------------------------------------------
//  EXACTLY ONE Size-To-Content-V IN THE CHAIN. Do not add a second.
//
//  Two of them nested resolve circularly: the outer waits on the inner, the
//  inner waits on the outer, both settle at zero, and the card renders as a
//  title bar with nothing under it. This layout got that wrong twice - first
//  with a card_body panel wrapping card_content, then with a card_wrapper grid
//  above it. Both looked like the fix and neither was, because the second
//  Size-To-Content was somewhere else in the chain each time.
//
//  So there is exactly one: card_content. The header sits at an exact y of 0
//  with an exact height, card_content at an exact y of HEADER_HEIGHT, and this
//  class gives the root a real height from what card_content laid out.
//
//  card_content is a GRID, not a panel. A panel with Size-To-Content-V measures
//  the bounding box of children whose own heights are fractions of it, so the
//  root grew, which grew the children, which grew the root - the card expanded
//  without limit. A grid derives its height from its ROWS instead, so it is
//  independent of whatever height the root currently has. Over-provisioning the
//  row count is free; empty rows cost nothing under Size-To-Content-V.
//
//  A grid is also what these sections were parented to before the card existed,
//  so the rows stack the way they always did rather than piling up at y=0.
//  ---------------------------------------------------------------------------
// =============================================================================
class UIActionCard: UIActionBase
{
	//! Must match card_header's height in the layout.
	static const int HEADER_HEIGHT = 30;

	//! Gap between the header rule and the first row.
	//!
	//! The header does not just sit above the content, it must PUSH it: a row
	//! placed at the header's own bottom edge reads as sitting on the rule, and
	//! a control that overflows its row - a 30px button in a 28px row - lands on
	//! the header itself. So the content starts a gap below the strip, and the
	//! header is the LAST sibling declared in the layout, which means it also
	//! paints over anything that still manages to reach up into it.
	//!
	//! Must match card_content's y in the layout, which is HEADER_HEIGHT + this.
	static const int CONTENT_TOP_PADDING = 6;

	//! Breathing room under the last row so it does not sit on the border.
	static const int BOTTOM_PADDING = 8;

	//! Hard ceiling on the root height.
	//!
	//! Pure insurance. Nothing here should ever approach it - the tallest card
	//! in COT is a full inventory grid - but a card that measures its own
	//! descendants can in principle feed back into itself, and the failure mode
	//! of that is a widget that grows every frame until the form is unusable.
	//! A clamp turns that into a merely wrong height.
	static const int MAX_HEIGHT = 4000;

	//! Height the root is given before anything has been measured.
	//!
	//! The layout declares the root one pixel tall. A content-sized child of a
	//! one-pixel parent can lay out to nothing and then report nothing, and this
	//! class only ever grows the root from what it measured - so the card would
	//! deadlock at zero and never recover. Starting tall breaks that: the first
	//! layout has room to resolve in, and the first SyncHeight shrinks the card
	//! to whatever it actually needs.
	static const int SEED_HEIGHT = 240;

	protected Widget     m_Header;
	protected Widget     m_Content;
	protected Widget     m_Ring;
	protected TextWidget m_Title;

	//! Right-hand strip inside the title bar. It is a sibling of the header and
	//! not a child of it: card_header takes no pointer input (nothing in it was
	//! ever meant to be clicked), and a widget cannot opt back in underneath a
	//! parent that ignores the pointer. Declared after card_ring in the layout
	//! so it also draws over the border.
	protected Widget     m_HeaderActions;

	protected bool m_HasHeader;

	//! Last height pushed onto the root. Kept so the per-frame check is a float
	//! compare and not a SetSize on every card on screen every frame.
	protected float m_AppliedHeight;

	override void OnInit()
	{
		super.OnInit();

		m_Header  = layoutRoot.FindAnyWidget( "card_header"  );
		m_Content = layoutRoot.FindAnyWidget( "card_content" );
		m_Ring    = layoutRoot.FindAnyWidget( "card_ring"    );

		m_HeaderActions = layoutRoot.FindAnyWidget( "card_header_actions" );

		// Packed against the right edge and centred in the 30px strip, so a
		// caller only has to create its buttons in reading order.
		WrapSpacerWidget actions;
		if ( Class.CastTo( actions, m_HeaderActions ) )
		{
			actions.SetContentAlignmentH( WidgetAlignment.WA_RIGHT );
			actions.SetContentAlignmentV( WidgetAlignment.WA_CENTER );
		}

		Class.CastTo( m_Title, layoutRoot.FindAnyWidget( "card_title" ) );

		m_HasHeader     = true;
		m_AppliedHeight = 0;

		float rw, rh;
		layoutRoot.GetSize( rw, rh );
		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE, true );
		layoutRoot.SetSize( rw, SEED_HEIGHT );
	}

	//! The widget callers put their rows into.
	Widget GetContent()
	{
		return m_Content;
	}

	//! Right-aligned strip in the title bar, for controls that act on the card
	//! as a whole - a refresh, a destructive action. Children are packed toward
	//! the right edge in creation order, so the first one created sits leftmost.
	//! Size them explicitly: the strip is 30px tall and gives its children no
	//! width of their own.
	Widget GetHeaderActions()
	{
		return m_HeaderActions;
	}

	// ---------------------------------------------------------------------------
	//  Convenience Premade Card Header Action Creators
	// ---------------------------------------------------------------------------
	UIActionImageButton AddRefreshButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_REFRESH" )
	{
		return UIActionManager.CreateRefreshButton( GetHeaderActions(), instance, callback, tooltip );
	}

	UIActionImageButton AddDeleteButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_DELETE" )
	{
		return UIActionManager.CreateDeleteButton( GetHeaderActions(), instance, callback, tooltip );
	}

	UIActionImageButton AddSaveButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_SAVE" )
	{
		return UIActionManager.CreateSaveButton( GetHeaderActions(), instance, callback, tooltip );
	}

	UIActionImageButton AddApplyButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_APPLY" )
	{
		return UIActionManager.CreateApplyButton( GetHeaderActions(), instance, callback, tooltip );
	}

	UIActionImageButton AddCopyButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_COPY" )
	{
		return UIActionManager.CreateCopyButton( GetHeaderActions(), instance, callback, tooltip );
	}

	UIActionImageButton AddPasteButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_PASTE" )
	{
		return UIActionManager.CreatePasteButton( GetHeaderActions(), instance, callback, tooltip );
	}

	UIActionImageButton AddAddButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_ADD" )
	{
		return UIActionManager.CreateAddButton( GetHeaderActions(), instance, callback, tooltip );
	}

	UIActionImageButton AddEditButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_EDIT" )
	{
		return UIActionManager.CreateEditButton( GetHeaderActions(), instance, callback, tooltip );
	}

	UIActionImageButton AddSearchButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_SEARCH" )
	{
		return UIActionManager.CreateSearchButton( GetHeaderActions(), instance, callback, tooltip );
	}

	UIActionImageButton AddLockButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_LOCK" )
	{
		return UIActionManager.CreateLockButton( GetHeaderActions(), instance, callback, tooltip );
	}

	UIActionImageButton AddVisibilityButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_VIEW" )
	{
		return UIActionManager.CreateVisibilityButton( GetHeaderActions(), instance, callback, tooltip );
	}

	//! Escape hatch behind the named helpers above: a header action with an
	//! arbitrary Lucide icon, for the cases the premade set does not cover.
	//! Prefer a named helper where one fits - the icon and tooltip are then
	//! consistent across every module that uses it.
	//!
	//!     card.AddCardHeaderAction( JMConstants.Lucide( "download" ), this, "OnClick_Export", "#STR_..." );
	UIActionImageButton AddCardHeaderAction( string iconPath, Class instance, string callback, string tooltip = "" )
	{
		UIActionImageButton btn = UIActionManager.CreateIconButton( GetHeaderActions(), iconPath, instance, callback );

		if ( btn )
		{
			btn.SetFixedSize( JMFormBase.HEADER_ACTION_PX, JMFormBase.HEADER_ACTION_PX );

			if ( tooltip != "" )
				btn.SetTooltip( tooltip );
		}

		return btn;
	}

	override void SetLabel( string text )
	{
		m_HasHeader = ( text != "" );

		if ( m_Header )
			m_Header.Show( m_HasHeader );

		// A headerless card has no title bar to hang controls in.
		if ( m_HeaderActions )
			m_HeaderActions.Show( m_HasHeader );

		// A headerless card starts its body at the top edge instead of leaving
		// an empty strip where the title bar would have been.
		if ( m_Content )
		{
			if ( m_HasHeader )
				m_Content.SetPos( 0, HEADER_HEIGHT + CONTENT_TOP_PADDING, true );
			else
				m_Content.SetPos( 0, CONTENT_TOP_PADDING, true );
		}

		if ( m_HasHeader && m_Title )
			m_Title.SetText( Widget.TranslateString( text ) );
	}

	void SetTitleColor( int color )
	{
		if ( m_Title )
			m_Title.SetColor( color );
	}

	//! Tint the border. A destructive section gets a red ring rather than a red
	//! heading, so the whole block reads as dangerous and not just its name.
	void SetRingColor( int color )
	{
		if ( m_Ring )
			m_Ring.SetColor( color );
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		SyncHeight();
	}

	//! Give the root a height: the header, plus whatever card_content laid out,
	//! plus the same margin at the bottom that the header gives at the top.
	protected void SyncHeight()
	{
		if ( !layoutRoot || !m_Content )
			return;

		float cw, ch;
		m_Content.GetScreenSize( cw, ch );

		if ( ch <= 0 )
			return;

		float height = ch + CONTENT_TOP_PADDING + BOTTOM_PADDING;
		if ( m_HasHeader )
			height = height + HEADER_HEIGHT;

		// Float compare against the last value we set, not against the root's
		// current size: reading the root back gives layout units, and these are
		// physical pixels.
		if ( Math.AbsFloat( height - m_AppliedHeight ) < 1 )
			return;

		height = Math.Clamp( height, 0, MAX_HEIGHT );

		m_AppliedHeight = height;

		float rw, rh;
		layoutRoot.GetSize( rw, rh );

		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE, true );
		layoutRoot.SetSize( rw, height );
		layoutRoot.Update();
	}
}
