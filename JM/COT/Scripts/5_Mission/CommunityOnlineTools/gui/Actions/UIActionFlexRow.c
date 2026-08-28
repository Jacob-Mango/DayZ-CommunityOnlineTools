// -----------------------------------------------------------------------------
//  UIActionFlexRow - CSS-flexbox-style row layout for COT UI.
//
//  WHY THIS EXISTS
//  The DayZ WrapSpacer engine gives you two width modes per child:
//    * SetFixedSize(px)   - never grows, never shrinks
//    * SetWidth(fraction) - grows/shrinks with the row, but has NO floor
//  There is no way to say "at least N px, but grow to fill leftover space"
//  (CSS `min-width` + `flex-grow`). A pure fraction collapses to nothing on a
//  narrow form; a pure fixed size never fills a wide one. This class adds that
//  missing behaviour on top of the engine, without fighting it.
//
//  MODEL (mirrors flexbox)
//    Each flex child has:  grow (weight), basisPx (min width floor), maxPx (ceiling).
//    Layout pass:
//      1. Measure the row's real inner width in EXACT pixels (GetScreenSize).
//      2. Reserve every child's basisPx floor. Sum = reserved.
//      3. leftover = rowWidth - reserved  (clamped >= 0).
//      4. Distribute leftover across children by grow weight.
//      5. Each child width = clamp( basisPx + share, basisPx, maxPx ).
//      6. Apply via ApplyFlexWidthPx (exact-pixel space - same space we measured,
//         so no screen/layout unit mismatch, i.e. no DPI trap).
//
//  DPI SAFETY
//  We only ever mix values from the SAME coordinate space: GetScreenSize gives
//  exact screen pixels, ApplyFlexWidthPx sets exact-size pixels, and the row's
//  own children are measured and set in that one space. We never back-compute a
//  fractional-fill dimension from a screen measurement (that was the historical
//  UIActionScrollerH bug).
//
//  USAGE
//    In OnInit:
//      m_Footer = UIActionManager.CreateFlexRow( actionsWrap );
//      UIActionConfirmInline del = UIActionManager.CreateConfirmInline( m_Footer.GetContent(), ... );
//      del.SetFixedSize( 32, 32 );                 // non-flex child: fixed
//      m_Footer.Add( del );                         // reserved at its fixed width
//      m_SpawnBtn.SetFlex( 1.0, 160 );              // grow=1, min 160px, unbounded
//      m_Footer.Add( m_SpawnBtn );
//    In OnResize:
//      if ( m_Footer ) m_Footer.Layout();
// -----------------------------------------------------------------------------
class UIActionFlexRow
{
	protected Widget m_Row;                         // the WrapSpacer this row lays out
	protected ref array<ref UIActionFlexEntry> m_Entries;

	// Per-inter-child gap allowance (layout px). The WrapSpacer adds padding
	// between children on top of the widths we apply; we subtract this per gap
	// so the total (widths + padding) stays within the row and the last child
	// does not wrap. Matches UIWrapSpacer.layout's default child spacing.
	protected float m_GapPx = 6;

	void UIActionFlexRow( Widget row )
	{
		m_Row = row;
		m_Entries = new array<ref UIActionFlexEntry>();
	}

	//! Override the per-gap padding allowance if a row uses a non-default
	//! WrapSpacer variant (e.g. Compact = 0). Call before Layout().
	void SetGap( float gapPx )
	{
		m_GapPx = gapPx;
	}

	Widget GetContent()
	{
		return m_Row;
	}

	//! Register a flex child. If the action called SetFlex() its spec is honoured;
	//! otherwise it is treated as a fixed-width child reserved at its current width
	//! (measured at Layout time), so mixed fixed + flex rows work transparently.
	void Add( UIActionBase action )
	{
		if ( !action )
			return;
		m_Entries.Insert( new UIActionFlexEntry( action ) );
	}

	//! Recompute and apply widths. Call from the form's OnResize (and once after
	//! initial creation). Safe to call before first render - if the row has no
	//! measurable width yet it no-ops and waits for the next resize.
	void Layout()
	{
		if ( !m_Row || m_Entries.Count() == 0 )
			return;

		float rowW;
		float rowH;
		m_Row.GetScreenSize( rowW, rowH );

		// Before first render GetScreenSize returns 0 - bail; OnResize fires again
		// once the form is on screen with real dimensions.
		if ( rowW < 1 )
			return;

		// 1+2. Reserve each child's floor. Flex children reserve basisPx; non-flex
		//      children reserve their current measured (fixed) width.
		float reserved = 0;
		float totalGrow = 0;

		foreach ( UIActionFlexEntry e : m_Entries )
		{
			if ( e.action.HasFlex() )
			{
				e.floorPx = e.action.GetFlexBasis();
				totalGrow += e.action.GetFlexGrow();
			}
			else
			{
				// Fixed child: reserve whatever width it currently occupies.
				float cw, ch;
				e.action.GetLayoutRoot().GetScreenSize( cw, ch );
				e.floorPx = cw;
			}
			reserved += e.floorPx;
		}

		// 3. Leftover space to distribute among growing children.
		//    The WrapSpacer inserts padding BETWEEN children on top of the widths
		//    we apply, so if we distributed the full rowW the total (widths +
		//    padding) would exceed the row and the last child would wrap to a new
		//    line. Reserve a gap allowance per inter-child boundary so the applied
		//    widths plus padding stay within rowW.
		int gaps = m_Entries.Count() - 1;
		if ( gaps < 0 )
			gaps = 0;
		float padReserve = gaps * m_GapPx;

		float leftover = rowW - reserved - padReserve;
		if ( leftover < 0 )
			leftover = 0;

		// 4+5. Distribute leftover by grow weight, clamp to [basis, max], apply.
		//      A second pass would be needed for perfect max-clamp redistribution,
		//      but one pass covers every real COT row (few children, generous maxes).
		foreach ( UIActionFlexEntry entry : m_Entries )
		{
			if ( !entry.action.HasFlex() )
				continue; // fixed children keep their width

			float width = entry.floorPx;

			if ( totalGrow > 0 && entry.action.GetFlexGrow() > 0 )
				width += leftover * ( entry.action.GetFlexGrow() / totalGrow );

			float maxPx = entry.action.GetFlexMax();
			if ( maxPx > 0 && width > maxPx )
				width = maxPx;

			if ( width < entry.floorPx )
				width = entry.floorPx;

			entry.action.ApplyFlexWidthPx( width );
		}

		m_Row.Update();
	}
}

//! Internal per-child record for UIActionFlexRow.
class UIActionFlexEntry
{
	UIActionBase action;
	float        floorPx;

	void UIActionFlexEntry( UIActionBase a )
	{
		action = a;
	}
}
