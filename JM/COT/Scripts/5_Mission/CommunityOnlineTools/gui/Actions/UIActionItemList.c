// =============================================================================
//  UIActionItemList
//
//  A COT-styled, single-column list for sets far too large to give a widget
//  each. Only the rows that fit on screen exist; scrolling moves the DATA
//  through a fixed pool of row widgets rather than moving widgets past a
//  viewport.
//
//  This is what replaced TextListboxWidget in the object spawner. That widget
//  draws in the engine's debug style, cannot be themed, and builds one entry
//  per item - which for the ~14000 spawnable classes is 14000 widgets on every
//  keystroke. A pool of ~20 costs the same at 20 items as at 20000.
//
//  Every row carries a primary label and an optional secondary one, drawn
//  dimmed and right-aligned. The spawner uses it to show the class name beside
//  the display name, so switching to display names does not hide the thing an
//  admin actually needs to type.
//
//  The list NEVER measures itself to lay out. Its owner tells it how tall its
//  viewport is - which the owner already knows, because it is the same number
//  it used to size the list's container. A widget cannot measure itself on the
//  frame it is built, and a list that got that wrong would show one row.
//
//  Usage:
//      m_List = UIActionManager.CreateItemList( parent, this, "OnClick_Row" );
//      m_List.SetViewportHeight( listHeightInLayoutPixels );
//      m_List.SetItems( names, classnames );
//
//      void OnClick_Row( UIEvent eid, UIActionBase action )
//      {
//          if ( eid == UIEvent.CLICK )        Select( m_List.GetSelectedIndex() );
//          if ( eid == UIEvent.DOUBLE_CLICK ) Confirm();
//      }
// =============================================================================
class UIActionItemList: UIActionBase
{
	protected Widget     m_Body;
	protected Widget     m_ScrollContainer;
	protected Widget     m_ScrollHandle;
	protected TextWidget m_EmptyText;
	protected ref array<string> m_Items;
	protected ref array<string> m_Subs;

	//! Per-item selection state OWNED BY THE HOST, distinct from m_Selected
	//! (the row this widget itself last clicked/pressed). A host tracking its
	//! own multi-select data (e.g. ESP's tracked-object list) marks entries
	//! here to get a left-edge accent instead of borrowing the click-highlight,
	//! which only ever fits one row at a time.
	protected ref array<bool> m_Highlighted;

	//! The row widgets. Never more than fit on screen, never rebuilt on a data
	//! change - only repainted.
	protected ref array<Widget> m_Rows;

	//! Index of the item currently drawn in row 0.
	protected int m_Top;
	protected int m_Selected;
	protected int m_Hovered;

	//! Viewport height in LAYOUT pixels, as handed over by the owner. Layout
	//! pixels are the space SetPos and SetSize work in; they are not the screen
	//! pixels GetScreenSize and the mouse report, and mixing the two is the
	//! classic DPI trap. Everything positional here stays in layout pixels; the
	//! only place screen pixels appear is the scrollbar drag, which measures
	//! its own track at drag time so both sides of that sum are screen space.
	protected float m_ViewportH;
	protected bool m_Dragging;
	protected int  m_DragMouseY;
	protected int  m_DragTop;

	//! Time and row of the last click, for detecting a double one. The engine's
	//! own OnDoubleClick only reaches a widget the FORM handles, and these rows
	//! are handled here.
	protected int m_LastClickTime;
	protected int m_LastClickItem;

	//! Which mouse button raised the last CLICK. A host that offers a context
	//! menu needs to tell a select from a right-click, and both arrive on the
	//! same event.
	protected int m_LastClickButton;
	static const int ROW_H = 22;

	//! Scrollbar geometry as UIActionItemList.layout declares it: an 8px bar
	//! inset 4px from the right edge. A row has to clear all 12 of those plus a
	//! small gap.
	static const float SCROLL_BAR_PX    = 8.0;
	static const float SCROLL_GUTTER_PX = 14.0;

	//! Row width used until there is something to measure - the first frame,
	//! before the body has been laid out. See ApplyRowWidth for the real one.
	static const float ROW_W = 0.972;
	static const int DOUBLE_CLICK_MS = 400;

	//! Items the selection moves per wheel notch.
	static const int WHEEL_STEP = 1;
	static const int COLOR_ROW_NORMAL   = 0x00000000;

	//! Banding is a hint, not a border. The style this sits on already carries
	//! its own tint, so the wash on top of it is barely there on purpose - at
	//! anything stronger the list reads as stripes rather than as rows.
	static const int COLOR_ROW_ALT      = 0x08ECF9FF;
	static const int COLOR_ROW_HOVER    = JMTheme.ACCENT_WASH;

	//! The selected row is a solid accent bar with light text on it, not a
	//! wash. A wash over an alternating band reads as "slightly different"
	//! rather than "this one", which in a list this long is the one thing the
	//! row has to say.
	static const int COLOR_ROW_SELECTED = JMTheme.SELECTED_FILL;

	//! Host-driven per-item highlight (row_accent), separate from the fill
	//! colours above.
	static const int COLOR_ACCENT_NONE      = 0x00000000;
	static const int COLOR_ACCENT_HIGHLIGHT = JMTheme.SUCCESS;

	int GetItemCount()
	{
		return m_Items.Count();
	}

	//! Which mouse button raised the CLICK being handled. MouseState.LEFT or
	//! MouseState.RIGHT.
	int GetLastClickButton()
	{
		return m_LastClickButton;
	}

	int GetSelectedIndex()
	{
		return m_Selected;
	}

	string GetSelectedItem()
	{
		if ( m_Selected < 0 || m_Selected >= m_Items.Count() )
			return "";

		return m_Items[m_Selected];
	}

	bool IsHighlighted( int index )
	{
		if ( index < 0 || index >= m_Highlighted.Count() )
			return false;

		return m_Highlighted[index];
	}

	void SetEmptyText( string text )
	{
		if ( m_EmptyText )
			m_EmptyText.SetText( text );
	}

	//! Host-driven highlight for `index` - independent of the click-selection
	//! this widget tracks itself. Does not scroll or otherwise touch selection.
	void SetHighlighted( int index, bool highlighted )
	{
		if ( index < 0 || index >= m_Highlighted.Count() )
			return;

		if ( m_Highlighted[index] == highlighted )
			return;

		m_Highlighted[index] = highlighted;
		Refresh();
	}

	// -------------------------------------------------------------------------
	//  Data
	// -------------------------------------------------------------------------

	//! `subs` is optional; pass NULL for a single-column list. Where it is
	//! given it must be the same length as `items` - it is the same row.
	void SetItems( notnull array<string> items, array<string> subs = NULL )
	{
		m_Items.Clear();
		m_Subs.Clear();
		m_Highlighted.Clear();

		for ( int i = 0; i < items.Count(); i++ )
		{
			m_Items.Insert( items[i] );
			m_Highlighted.Insert( false );
		}

		if ( subs )
		{
			for ( int j = 0; j < subs.Count(); j++ )
				m_Subs.Insert( subs[j] );
		}

		m_Selected = -1;
		m_Hovered  = -1;
		m_Top      = 0;

		Refresh();
	}

	void SetSelectedIndex( int index, bool scrollTo = true )
	{
		if ( index < -1 || index >= m_Items.Count() )
			return;

		m_Selected = index;

		if ( scrollTo && index >= 0 )
			ScrollTo( index );

		Refresh();
	}

	// -------------------------------------------------------------------------
	//  Geometry
	// -------------------------------------------------------------------------

	//! Tell the list how tall it is, in layout pixels. Call it from wherever the
	//! container's height is decided - the same number, not a re-measurement.
	//!
	//! This also APPLIES that height to the control. The layout authors the
	//! root as `size 1 1` with vexactsize 0, i.e. a fraction of its parent, so
	//! a list left to itself claims the parent's full height no matter how many
	//! rows it holds. Under a parent that sizes itself to its content - a
	//! UIActionCard - that resolves as "card fits list, list fills card" and
	//! the card grows without bound. Callers that size a wrapper and pass its
	//! height are unaffected: the number they pass is the height it already had.
	void SetViewportHeight( float pixels )
	{
		if ( pixels <= 0 )
			return;

		if ( m_ViewportH == pixels )
			return;

		m_ViewportH = pixels;

		SetFixedHeight( pixels );

		SyncRowPool();
		Refresh();
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Body,            layoutRoot.FindAnyWidget( "list_body" ) );
		Class.CastTo( m_ScrollContainer, layoutRoot.FindAnyWidget( "list_scroll_container" ) );
		Class.CastTo( m_ScrollHandle,    layoutRoot.FindAnyWidget( "list_scroll_handle" ) );
		Class.CastTo( m_EmptyText,       layoutRoot.FindAnyWidget( "list_empty" ) );

		m_Items = new array<string>;
		m_Subs  = new array<string>;
		m_Rows  = new array<Widget>;
		m_Highlighted = new array<bool>;

		m_Top       = 0;
		m_Selected  = -1;
		m_Hovered   = -1;
		m_ViewportH = 0;

		m_LastClickTime = 0;
		m_LastClickItem = -1;

		//! Every widget of the control answers to this script, so a wheel notch
		//! is caught wherever in the list it lands - over a row, over the empty
		//! space under the last one, or over the frame itself.
		layoutRoot.SetHandler( this );

		if ( m_Body )
			m_Body.SetHandler( this );

		Widget frame = layoutRoot.FindAnyWidget( "list_frame" );

		if ( frame )
			frame.SetHandler( this );

		if ( m_ScrollHandle )
			m_ScrollHandle.SetHandler( this );

		if ( m_ScrollContainer )
			m_ScrollContainer.SetHandler( this );
	}

	override void OnShow() {}

	override void OnHide() {}

	//! How many rows fit. One less than the pool would leave a strip of the
	//! frame showing at the bottom, so the pool holds the partial row too and
	//! the body clips it.
	protected int VisibleRows()
	{
		if ( m_ViewportH <= 0 )
			return 0;

		int rows = ( int ) Math.Ceil( m_ViewportH / ROW_H );

		if ( rows < 1 )
			return 1;

		return rows;
	}

	//! Create or destroy row widgets so the pool matches the viewport. Only
	//! ever runs on a resize - a data change repaints what is already here.
	protected void SyncRowPool()
	{
		if ( !m_Body )
			return;

		int wanted = VisibleRows();

		while ( m_Rows.Count() > wanted )
		{
			Widget doomed = m_Rows[m_Rows.Count() - 1];

			if ( doomed )
				doomed.Unlink();

			m_Rows.Remove( m_Rows.Count() - 1 );
		}

		while ( m_Rows.Count() < wanted )
		{
			Widget row = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionItemListRow.layout", m_Body );

			if ( !row )
				return;

			row.SetHandler( this );

			ImageWidget selectedIcon;
			if ( Class.CastTo( selectedIcon, row.FindAnyWidget( "row_selected_icon" ) ) )
			{
				selectedIcon.LoadImageFile( 0, JMConstants.Lucide( "check" ) );
				selectedIcon.SetImage( 0 );
			}

			// The row's own index in the POOL, not in the data. It is the only
			// per-widget storage a plain Widget offers, and it is what turns a
			// click back into an item once the top of the list is known.
			row.SetName( "" + m_Rows.Count() );

			m_Rows.Insert( row );
		}

		for ( int i = 0; i < m_Rows.Count(); i++ )
		{
			m_Rows[i].SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE | WidgetFlags.HEXACTPOS, true );
			m_Rows[i].ClearFlags( WidgetFlags.HEXACTSIZE );
			m_Rows[i].SetPos( 0, i * ROW_H );
		}

		ApplyRowWidth();
	}

	//! How far short of the right edge a row stops, measured rather than
	//! assumed.
	//!
	//! The gutter the scrollbar needs is a FIXED number of layout pixels, so
	//! expressing it as a fixed FRACTION of the row was wrong: 0.028 of the
	//! object spawner's wide column clears the bar, and 0.028 of the teleport
	//! form's narrow one does not - which is why only the narrow list had its
	//! right-hand column sliced by the bar. The bar measures itself here, so
	//! the fraction is derived per list instead of guessed once. Both sides of
	//! the division are screen space, so UI scaling cancels out of it.
	protected void ApplyRowWidth()
	{
		float rowW = ROW_W;

		if ( m_Body && m_ScrollContainer )
		{
			float bw, bh;
			m_Body.GetScreenSize( bw, bh );

			float cw, ch;
			m_ScrollContainer.GetScreenSize( cw, ch );

			if ( bw > 1 && cw > 0 )
				rowW = ( bw - ( cw * ( SCROLL_GUTTER_PX / SCROLL_BAR_PX ) ) ) / bw;
		}

		//! A list too narrow to hold a scrollbar and a row would compute a
		//! negative width; the authored fraction is the better wrong answer.
		if ( rowW <= 0 || rowW > 1 )
			rowW = ROW_W;

		for ( int i = 0; i < m_Rows.Count(); i++ )
		{
			m_Rows[i].SetSize( rowW, ROW_H );
			m_Rows[i].Update();
		}
	}

	void Clear()
	{
		m_Items.Clear();
		m_Subs.Clear();
		m_Highlighted.Clear();

		m_Selected = -1;
		m_Hovered  = -1;
		m_Top      = 0;

		Refresh();
	}

	//! Bring `index` into view, moving as little as possible.
	void ScrollTo( int index )
	{
		int rows = VisibleRows();

		if ( rows <= 0 )
			return;

		if ( index < m_Top )
			m_Top = index;
		else if ( index >= m_Top + rows )
			m_Top = index - rows + 1;

		ClampTop();
	}

	protected void ClampTop()
	{
		int maxTop = m_Items.Count() - VisibleRows();

		if ( maxTop < 0 )
			maxTop = 0;

		if ( m_Top > maxTop )
			m_Top = maxTop;

		if ( m_Top < 0 )
			m_Top = 0;
	}

	// -------------------------------------------------------------------------
	//  Painting
	// -------------------------------------------------------------------------

	//! Repaint the pool against the current data and scroll position. This is
	//! the whole of what a data change costs.
	void Refresh()
	{
		//! Re-measured here, not only on a resize. SyncRowPool runs before the
		//! body has ever been laid out, where GetScreenSize answers 0 and the
		//! width falls back to the authored fraction; a data change is the
		//! first moment there is anything real to measure.
		ApplyRowWidth();

		ClampTop();

		if ( m_EmptyText )
			m_EmptyText.Show( m_Items.Count() == 0 );

		for ( int i = 0; i < m_Rows.Count(); i++ )
		{
			int item = m_Top + i;

			if ( item >= m_Items.Count() )
			{
				m_Rows[i].Show( false );
				continue;
			}

			m_Rows[i].Show( true );

			TextWidget rowText;
			Class.CastTo( rowText, m_Rows[i].FindAnyWidget( "row_text" ) );

			if ( rowText )
				rowText.SetText( m_Items[item] );

			TextWidget rowSub;
			Class.CastTo( rowSub, m_Rows[i].FindAnyWidget( "row_sub" ) );

			if ( rowSub )
			{
				if ( item < m_Subs.Count() )
					rowSub.SetText( m_Subs[item] );
				else
					rowSub.SetText( "" );
			}

			PaintRow( i, item );
		}

		UpdateScrollbar();
	}

	//! Colour goes on the ROW WIDGET itself, the way UIActionDataTable bands its
	//! rows. An inner fill panel under a ButtonWidget root drew nothing here,
	//! and the button also swallowed the mouse wheel before the handler saw it -
	//! a plain panel raises OnClick just as well and lets the wheel through.
	protected void PaintRow( int poolIndex, int item )
	{
		Widget fill = m_Rows[poolIndex];

		int color = COLOR_ROW_NORMAL;

		// Alternating bands read against the DATA index, not the pool index -
		// banding that followed the pool would crawl as the list scrolled.
		if ( item - ( item / 2 ) * 2 == 1 )
			color = COLOR_ROW_ALT;

		if ( item == m_Hovered )
			color = COLOR_ROW_HOVER;

		if ( item == m_Selected )
			color = COLOR_ROW_SELECTED;

		fill.SetColor( color );

		bool highlighted = item < m_Highlighted.Count() && m_Highlighted[item];

		Widget rowAccent = m_Rows[poolIndex].FindAnyWidget( "row_accent" );

		if ( rowAccent )
		{
			if ( highlighted )
				rowAccent.SetColor( COLOR_ACCENT_HIGHLIGHT );
			else
				rowAccent.SetColor( COLOR_ACCENT_NONE );
		}

		Widget rowSelectedIcon = m_Rows[poolIndex].FindAnyWidget( "row_selected_icon" );

		if ( rowSelectedIcon )
			rowSelectedIcon.Show( highlighted );

		TextWidget rowText;
		Class.CastTo( rowText, m_Rows[poolIndex].FindAnyWidget( "row_text" ) );

		if ( rowText )
		{
			if ( item == m_Selected )
				rowText.SetColor( JMTheme.TEXT_ON_ACCENT );
			else
				rowText.SetColor( JMTheme.TEXT_SECONDARY );
		}

		TextWidget rowSub;
		Class.CastTo( rowSub, m_Rows[poolIndex].FindAnyWidget( "row_sub" ) );

		if ( rowSub )
		{
			//! Dimmed against the row it is on, whichever that is - on the
			//! accent bar the disabled grey all but vanishes.
			if ( item == m_Selected )
				rowSub.SetColor( JMTheme.TEXT_ON_ACCENT );
			else
				rowSub.SetColor( JMTheme.TEXT_DISABLED );
		}
	}

	protected void UpdateScrollbar()
	{
		if ( !m_ScrollContainer || !m_ScrollHandle )
			return;

		int rows  = VisibleRows();
		int count = m_Items.Count();

		if ( count <= rows || m_ViewportH <= 0 )
		{
			m_ScrollContainer.Show( false );
			return;
		}

		m_ScrollContainer.Show( true );

		// The track is the viewport minus the 4px inset the layout gives it at
		// each end.
		float track = m_ViewportH - 8;

		if ( track < 1 )
			return;

		float handle = track * rows / count;

		if ( handle < 20 )
			handle = 20;

		float travel = track - handle;
		float t = 0;

		if ( count - rows > 0 )
			t = ( 1.0 * m_Top ) / ( count - rows );

		m_ScrollHandle.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		m_ScrollHandle.SetSize( 1, handle );
		m_ScrollHandle.SetPos( 0, travel * t );
	}

	// -------------------------------------------------------------------------
	//  Input
	// -------------------------------------------------------------------------

	//! Pool index for a row widget or any child of one; -1 for anything else.
	protected int RowIndexOf( Widget w )
	{
		if ( !m_Body || !w )
			return -1;

		if ( w.GetParent() == m_Body )
			return w.GetName().ToInt();

		Widget p = w.GetParent();

		if ( p && p.GetParent() == m_Body )
			return p.GetName().ToInt();

		return -1;
	}

	//! Selection runs off the PRESS, not off OnClick.
	//!
	//! OnClick is a ButtonWidget event. These rows are plain panels - which is
	//! what lets the mouse wheel through to the list at all, since a button
	//! consumes it - and a panel never raises a click. The press is the event
	//! every widget gets, and it is what UIActionDataTable already uses to hit
	//! its own non-button rows.
	//!
	//! Selecting on the way down rather than the way up is also what a list is
	//! expected to do: press, drag, release elsewhere still selects what was
	//! pressed.
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		//! A right press selects the row and reports itself, so a host can open
		//! a context menu on the row the pointer is actually over. It never
		//! drags the scrollbar and never counts toward a double click.
		if ( button == MouseState.RIGHT )
			return SelectRowUnder( w, MouseState.RIGHT );

		if ( button != MouseState.LEFT )
			return false;

		if ( w == m_ScrollHandle )
		{
			m_Dragging = true;
			m_DragTop  = m_Top;

			int mx;
			GetMousePos( mx, m_DragMouseY );

			return true;
		}

		int idx = RowIndexOf( w );

		if ( idx < 0 || idx >= m_Rows.Count() )
			return false;

		int item = m_Top + idx;

		if ( item < 0 || item >= m_Items.Count() )
			return true;

		m_Selected = item;
		m_LastClickButton = MouseState.LEFT;
		Refresh();

		int now = g_Game.GetTime();

		if ( item == m_LastClickItem && now - m_LastClickTime <= DOUBLE_CLICK_MS )
		{
			m_LastClickTime = 0;
			m_LastClickItem = -1;

			CallEvent( UIEvent.CLICK );
			CallEvent( UIEvent.DOUBLE_CLICK );

			return true;
		}

		m_LastClickTime = now;
		m_LastClickItem = item;

		CallEvent( UIEvent.CLICK );

		return true;
	}

	//! Move the selection to the row under `w` and raise one CLICK for it.
	//! Shared by the right press; the left press has its own copy because it
	//! also has to feed the double-click timer.
	protected bool SelectRowUnder( Widget w, int button )
	{
		int idx = RowIndexOf( w );

		if ( idx < 0 || idx >= m_Rows.Count() )
			return false;

		int item = m_Top + idx;

		if ( item < 0 || item >= m_Items.Count() )
			return true;

		m_Selected        = item;
		m_LastClickButton = button;

		Refresh();

		CallEvent( UIEvent.CLICK );

		return true;
	}

	//! The wheel moves the SELECTION, and the view follows it.
	//!
	//! It does not pan the list out from under the selected row. Scrolling walks
	//! the highlight down one entry at a time, and only once that reaches the
	//! last visible row does the list itself start moving - which is what makes
	//! a wheel notch mean the same thing as an arrow key.
	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		if ( m_Items.Count() == 0 )
			return false;

		int next = m_Selected;

		//! Nothing picked yet: start from whatever is at the top of the view so
		//! the first notch lands somewhere the user can see.
		if ( next < 0 )
			next = m_Top;

		next = next - wheel * WHEEL_STEP;

		if ( next < 0 )
			next = 0;

		if ( next >= m_Items.Count() )
			next = m_Items.Count() - 1;

		if ( next == m_Selected )
			return true;

		m_Selected = next;

		//! A wheel notch is a LEFT-button-equivalent select, and saying so
		//! matters: the flag is sticky, so a host that opens a context menu on
		//! a right press would keep opening it on every scroll after the first
		//! right-click.
		m_LastClickButton = MouseState.LEFT;

		//! Moves the view only when the new row is outside it.
		ScrollTo( next );
		Refresh();

		//! The owner hears the same thing it hears from a click, so whatever is
		//! highlighted is also what is previewed and what would be spawned.
		CallEvent( UIEvent.CLICK );

		return true;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		super.OnMouseEnter( w, x, y );

		int idx = RowIndexOf( w );

		if ( idx < 0 )
			return false;

		m_Hovered = m_Top + idx;
		Refresh();

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		super.OnMouseLeave( w, enterW, x, y );

		int idx = RowIndexOf( w );

		if ( idx < 0 )
			return false;

		if ( m_Hovered == m_Top + idx )
		{
			m_Hovered = -1;
			Refresh();
		}

		return false;
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( !m_Dragging )
			return;

		if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) == 0 )
		{
			m_Dragging = false;
			return;
		}

		if ( !m_ScrollContainer )
			return;

		int rows  = VisibleRows();
		int count = m_Items.Count();

		if ( count <= rows )
			return;

		// Both sides of this sum are SCREEN pixels: the mouse delta, and a track
		// measured now rather than converted from the layout-space viewport. The
		// list is on screen by the time anyone can drag its handle, so measuring
		// here is safe in a way that measuring at build time is not.
		float trackW;
		float trackH;
		m_ScrollContainer.GetScreenSize( trackW, trackH );

		if ( trackH <= 1 )
			return;

		int mx;
		int my;
		GetMousePos( mx, my );

		float moved = ( my - m_DragMouseY ) / trackH;

		m_Top = m_DragTop + ( int ) Math.Round( moved * ( count - rows ) );

		ClampTop();
		Refresh();
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( !widget )
			return false;

		Widget w = widget;

		while ( w )
		{
			if ( w == layoutRoot )
				return true;

			w = w.GetParent();
		}

		return false;
	}
}
