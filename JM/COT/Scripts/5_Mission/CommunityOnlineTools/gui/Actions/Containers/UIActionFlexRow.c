//  UIActionFlexRow - flexbox-style row (WrapSpacer lacks "min px but grow"): grow, basisPx
//  floor, maxPx ceiling per child. Layout() works in exact pixels only (mixing
//  spaces was the old UIActionScrollerH DPI bug); call it from OnResize.
//  Usage: CreateFlexRow, child.SetFlex( 1.0, 160 ), Add( child ).

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

	Widget GetContent()
	{
		return m_Row;
	}

	//! Override the per-gap padding allowance if a row uses a non-default
	//! WrapSpacer variant (e.g. Compact = 0). Call before Layout().
	void SetGap( float gapPx )
	{
		m_GapPx = gapPx;
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

		// 3. Leftover space for growing children. WrapSpacer adds padding between
		//    children on top of applied widths, so reserve a gap per boundary or
		//    the last child wraps to a new line.
		int gaps = m_Entries.Count() - 1;
		if ( gaps < 0 )
			gaps = 0;
		float padReserve = gaps * JMUILayout.Px( m_GapPx );

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
