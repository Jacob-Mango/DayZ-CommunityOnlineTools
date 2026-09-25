//! #define scope in Enforce is per-file, NOT per compiled module - see COTModule.c.
#define COT_DEBUGLOGS

// =============================================================================
//  UIActionContextMenu
//
//  A right-click popup menu. Items carry a string ID, a label, an optional
//  icon and an optional label colour; clicking one fires UIEvent.CLICK and
//  GetLastClickedId() returns which. Closes on Escape or on a click outside
//  itself.
//
//  The menu draws nothing until OpenAt() is called. Its own layoutRoot is a 1px
//  transparent stub that only exists to keep the per-frame Update() running -
//  the visible panel is a separate widget parented to the anchor passed to
//  InitMenu(), so the popup floats above whatever the host is drawing instead
//  of being clipped by it. Anchor to the window root, the way the dropdown list
//  does.
//
//  Because the panel is NOT a child of the host content widget, hiding that
//  content does not hide an open menu. A host that shows and hides panels (a
//  tab bar, a collapsible section) must call Close() when it switches away.
//
//  Usage:
//      m_Menu = UIActionManager.CreateContextMenu( parent, m_Window.GetWidgetRoot(), this, "OnClick_Menu" );
//
//      // on right-click, rebuild for whatever was hit and open at the cursor:
//      m_Menu.ClearItems();
//      m_Menu.AddItem( "take",   "Take",   JMConstants.Lucide( "hand" ) );
//      m_Menu.AddItem( "delete", "Delete", JMConstants.ICON_TRASH_CAN, JMTheme.DANGER );
//      m_Menu.SetItemEnabled( "delete", JMPermissions.Has( "..." ) );
//      m_Menu.OpenAt( m_Table.GetLastRightClickX(), m_Table.GetLastRightClickY() );
//
//      void OnClick_Menu( UIEvent eid, UIActionBase action )
//      {
//          if ( eid != UIEvent.CLICK )
//              return;
//          string id = m_Menu.GetLastClickedId();
//      }
// =============================================================================

class JMContextMenuEntry
{
	string Id;
	string Text;
	string IconPath;
	int    TextColor;
	bool   Enabled;
	bool   Submenu;
	Class  Target;
	string Callback;

	void JMContextMenuEntry( string id, string text, string iconPath = "", int textColor = 0, bool submenu = false, Class target = null, string callback = "" )
	{
		Id        = id;
		Text      = text;
		IconPath  = iconPath;
		TextColor = textColor;
		Enabled   = true;
		Submenu   = submenu;
		Target    = target;
		Callback  = callback;
	}


}

class UIActionContextMenu: UIActionBase
{
	protected Widget           m_Anchor;
	protected Widget           m_Panel;
	protected GridSpacerWidget m_Grid;

	//! Only engages when the menu is taller than the screen has room for - see
	//! UpdatePlacement. A short menu never touches the scroller at all.
	protected ScrollWidget     m_Scroller;
	protected ref array<ref JMContextMenuEntry> m_Entries;
	protected string m_LastClickedId;
	protected int    m_HoveredRow;      // -1 when nothing highlighted
	protected bool   m_Open;

	//! Seconds to ignore outside clicks after opening. The button press that
	//! opened the menu is usually still held when the first Update() runs, and
	//! without the grace period the menu closes on the frame it appears.
	protected float m_OpenDelay;

	//! Measured from the item grid once it has been laid out; 0 until then.
	protected float m_FullHeight;

	//! Cursor position OpenAt() was given, in screen pixels. Kept because the
	//! panel cannot be placed until its height is known, which is a frame later.
	protected float m_PendingX;
	protected float m_PendingY;
	protected float m_MenuWidth;

	//! Width is measured from the widest label unless a caller pinned it with
	//! SetMenuWidth(). A fixed 160 clipped "Show unconscious" halfway through
	//! the word, which is exactly the entry an admin needs to read.
	protected bool m_AutoWidth;

	//! Columns the item grid is laid out in. A long menu - the object spawner's
	//! 27 categories - is taller than the window it opens in at one column, and
	//! the panel has no scroller of its own.
	protected int m_Columns;

	//! Whether clicking an entry dismisses the menu. False for menus whose rows
	//! are toggles - a filter list is not finished being used after one click.
	protected bool m_CloseOnClick;

	//! The control that opens this menu, when there is one. A press on it is not
	//! treated as an outside click: closing here AND letting the button's own
	//! handler run made a dropdown button close and immediately reopen the menu,
	//! so it never toggled shut.
	protected Widget m_OwnerWidget;
	static const int COLOR_ROW_NORMAL   = JMTheme.SURFACE_OVERLAY;
	static const int COLOR_ROW_HOVER    = JMTheme.ACCENT_WASH;
	static const int COLOR_ROW_DISABLED = JMTheme.SURFACE_OVERLAY;
	static const int COLOR_TEXT         = JMTheme.TEXT_PRIMARY;
	static const int COLOR_TEXT_OFF     = JMTheme.TEXT_DISABLED;

	//! Left edge of the label, in pixels from the row's own left edge. The icon
	//! strip is 12 (position) + 16 (size) = 28, so 38 clears it with a gap. A
	//! row without an icon pulls its label back to a plain padding so text-only
	//! menus are not indented for an icon that is not there.
	//!
	//! These are widget positions rather than SetTextOffset values on purpose:
	//! the auto-width measurement adds one of them to the measured text width,
	//! and it can only do that if the number is where the text actually starts.
	static const int TEXT_LEFT_ICON     = 38;
	static const int TEXT_LEFT_PLAIN    = 12;

	//! Gap kept between the end of the longest label and the panel's right edge.
	static const int TEXT_PAD_RIGHT     = 16;

	//! Extra right-hand room a row with a trailing chevron needs, so the label
	//! of the longest submenu row does not run into its own arrow.
	static const int SUBMENU_PAD_RIGHT  = 18;

	//! Floor, not the width: a menu never gets narrower than this, however short
	//! its labels are, so a two-word popup is still a comfortable hit area.
	static const float DEFAULT_WIDTH    = 160;
	static const float OPEN_DELAY       = 0.15;

	//! Pixels per wheel notch, matching UIActionDropdown's own scroller fix.
	static const float WHEEL_PIXEL_STEP = 40.0;

	int GetItemCount()
	{
		return m_Entries.Count();
	}

	string GetLastClickedId()
	{
		return m_LastClickedId;
	}

	override bool IsOpen()
	{
		return m_Open;
	}

	//! Is `widget` the owner control or anything inside it?
	protected bool IsOwnedWidget( Widget widget )
	{
		if ( !m_OwnerWidget || !widget )
			return false;

		Widget w = widget;
		while ( w )
		{
			if ( w == m_OwnerWidget )
				return true;

			w = w.GetParent();
		}

		return false;
	}

	//! Go back to sizing from the widest label. DEFAULT_WIDTH stays the floor.
	void SetAutoWidth( bool enable )
	{
		m_AutoWidth = enable;
	}

	//! False keeps the menu up after an entry fires, for rows that are toggles
	//! rather than commands. The menu still closes on Escape or an outside click.
	void SetCloseOnClick( bool closeOnClick )
	{
		m_CloseOnClick = closeOnClick;
	}

	//! Lay the entries out in `columns` columns (1 or 2). Call it before the
	//! first AddItem.
	//!
	//! A GridSpacer's column count is fixed by its layout - there is no proto to
	//! change it - so this swaps the panel for the two-column authoring of it
	//! and rebuilds. Nothing else about the menu changes.
	void SetColumns( int columns )
	{
		if ( columns < 1 )
			columns = 1;

		if ( columns > 2 )
			columns = 2;

		if ( m_Columns == columns )
			return;

		m_Columns = columns;

		if ( !m_Anchor )
		{
			Error("[UIActionContextMenu] SetColumns failed: m_Anchor is null!");
			return;
		}

		Widget anchor = m_Anchor;

		if ( m_Panel )
		{
			JMStatics.RemoveOverlay( m_Panel );
			m_Panel.Unlink();
		}

		m_Panel = NULL;
		m_Grid  = NULL;

		InitMenu( anchor );
		RebuildItems();
	}

	//! A disabled item still draws - a menu that silently loses entries reads
	//! as a bug - but it is greyed and does not fire.
	void SetItemEnabled( string id, bool enabled )
	{
		foreach ( JMContextMenuEntry entry : m_Entries )
		{
			if ( entry.Id == id )
				entry.Enabled = enabled;
		}

		RefreshRowColors();
	}

	//! Swap one item's glyph without rebuilding the menu.
	//!
	//! For a menu of toggles rather than a list of commands: the row has to
	//! redraw the moment it is clicked, and it is still under the cursor when
	//! that happens. Rebuilding would destroy the very row the press landed on,
	//! which the engine answers by recentring the cursor - the same reason
	//! RefreshTypeFilterColors exists rather than a rebuild.
	void SetItemIcon( string id, string icon )
	{
		int index = -1;

		for ( int i = 0; i < m_Entries.Count(); i++ )
		{
			if ( m_Entries[i].Id != id )
				continue;

			index = i;
			m_Entries[i].IconPath = icon;
			break;
		}

		if ( index < 0 || !m_Grid )
			return;

		//! Rows carry their index in their name - the only per-widget storage
		//! a plain Widget offers, and what RebuildItems writes.
		Widget row = m_Grid.GetChildren();

		while ( row )
		{
			if ( row.GetName() == "" + index )
				break;

			row = row.GetSibling();
		}

		if ( !row )
			return;

		ImageWidget rowIcon;
		Class.CastTo( rowIcon, row.FindAnyWidget( "item_icon" ) );

		if ( rowIcon )
		{
			if ( icon != "" )
			{
				rowIcon.LoadImageFile( 0, icon );
				rowIcon.SetImage( 0 );
			}

			rowIcon.Show( icon != "" );
		}

		TextWidget rowText;
		Class.CastTo( rowText, row.FindAnyWidget( "item_text" ) );

		if ( !rowText )
			return;

		//! The label sits where it does because of the glyph, so a row that
		//! gains or loses one has to move it.
		if ( icon == "" )
			rowText.SetPos( TEXT_LEFT_PLAIN, 0 );
		else
			rowText.SetPos( TEXT_LEFT_ICON, 0 );
	}

	//! Recolour one entry in place. A toggle menu repaints itself on every click
	//! and must NOT do that by rebuilding rows: the row being rebuilt is the one
	//! the press landed on, and the engine answers a vanished press target by
	//! recentring the cursor.
	void SetItemTextColor( string id, int color )
	{
		foreach ( JMContextMenuEntry entry : m_Entries )
		{
			if ( entry.Id == id )
				entry.TextColor = color;
		}

		RefreshRowColors();
	}

	//! Pin the panel width in pixels. Turns the automatic measurement off - a
	//! caller that names a width means that width, not "at least that width".
	void SetMenuWidth( float pixels )
	{
		m_MenuWidth = pixels;
		m_AutoWidth = false;
	}

	//! Name the control that opens this menu so its own press is left alone -
	//! see m_OwnerWidget. Pass the button's layoutRoot; children count too.
	void SetOwnerWidget( Widget owner )
	{
		m_OwnerWidget = owner;
	}

	//! The native scrollbar track/thumb is drawn by ScrollWidget itself, not
	//! exposed as a separate child - there is no engine call to toggle it
	//! (enwidgets.c only has the getters GetScrollbarWidth/IsScrollbarVisible,
	//! no setter), and it reserves its gutter width regardless of whether
	//! there is anything to scroll.
	//!
	//! So instead of hiding it, this pushes the scroller past the panel's
	//! right edge by exactly that gutter width when it is not needed - the
	//! panel has clipchildren 1, so the reserved column lands outside the
	//! visible rect and gets cropped away instead of drawing as a bar with
	//! nothing behind it.
	protected void SetScrollbarVisible( bool visible )
	{
		if ( !m_Scroller )
			return;

		float scrollbarW = m_Scroller.GetScrollbarWidth();

		float curW, curH;
		m_Scroller.GetSize( curW, curH );

		float scrollerW = m_MenuWidth;
		if ( !visible )
			scrollerW = m_MenuWidth + scrollbarW;

		m_Scroller.SetFlags( WidgetFlags.HEXACTSIZE );
		m_Scroller.SetSize( scrollerW, curH );
		m_Scroller.SetPos( 0, 0 );
	}

	override void OnInit()
	{
		super.OnInit();

		m_Entries       = new array<ref JMContextMenuEntry>;
		m_LastClickedId = "";
		m_HoveredRow    = -1;
		m_Open          = false;
		m_FullHeight    = 0;
		m_MenuWidth     = DEFAULT_WIDTH;
		m_AutoWidth     = true;
		m_CloseOnClick  = true;
		m_Columns       = 1;
	}

	protected string PanelLayout()
	{
		if ( m_Columns == 2 )
			return "JM/COT/GUI/layouts/uiactions/UIActionContextMenu_Panel2.layout";

		return "JM/COT/GUI/layouts/uiactions/UIActionContextMenu_Panel.layout";
	}

	//! Create the popup panel under `anchor`. Must be called before OpenAt().
	void InitMenu( notnull Widget anchor )
	{
		m_Anchor = anchor;

		m_Panel = g_Game.GetWorkspace().CreateWidgets( PanelLayout(), m_Anchor );

		if ( m_Panel )
		{
			Class.CastTo( m_Grid, m_Panel.FindAnyWidget( "menu_grid" ) );
			Class.CastTo( m_Scroller, m_Panel.FindAnyWidget( "menu_scroller" ) );
			m_Panel.Show( false );
			m_Panel.SetHandler( this );
			if ( m_Grid )
				m_Grid.SetHandler( this );
			if ( m_Scroller )
				m_Scroller.SetHandler( this );

			// The panel floats outside its anchor's normal hierarchy (see the
			// class note above) so COTModule's "was this click on COT UI?"
			// ancestor walk cannot find a window above it - without this every
			// click on the menu reads as a world click and the game grabs the
			// mouse back, which is what looks like "the menu/sidebar closed".
			JMStatics.AddOverlay( m_Panel );
		}
		else
		{
			Error("[UIActionContextMenu] InitMenu failed: Could not create widgets for panel layout " + PanelLayout());
		}
	}

	void ClearItems()
	{
		m_Entries.Clear();
		m_HoveredRow = -1;

		if ( m_Grid )
		{
			Widget child = m_Grid.GetChildren();
			while ( child )
			{
				Widget next = child.GetSibling();
				child.Unlink();
				child = next;
			}
		}

		m_FullHeight = 0;
	}

	//! textColor 0 means "use the default", so callers can pass JMTheme.DANGER
	//! for a destructive entry without every other call site naming a colour.
	void AddItem( string id, string label, string icon = "", int textColor = 0, bool submenu = false, Class target = null, string callback = "" )
	{
		m_Entries.Insert( new JMContextMenuEntry( id, label, icon, textColor, submenu, target, callback ) );
		RebuildItems();
	}

	//! Remove one item and redraw the menu. Like AddItem / ClearItems this rebuilds every row, so
	//! never call it from the click handler of the menu's own row - defer it a tick.
	void RemoveItemById( string id )
	{
		for ( int i = 0; i < m_Entries.Count(); i++ )
		{
			if ( m_Entries[i].Id == id )
			{
				m_Entries.Remove( i );
				RebuildItems();
				return;
			}
		}
	}

	//! The base class's IsVisible() reads layoutRoot, which for this control is
	//! a 1px stub that only exists to keep Update() running (see class note) -
	//! it is not the actual popup panel and does not track open/closed. m_Open
	//! does, so the Escape priority chain (JMFormBase.HasOpenOverlay) sees the
	//! real state instead of the always-visible stub.
	override bool IsVisible()
	{
		return m_Open;
	}

	//! Open at a screen-space cursor position. The menu flips left and/or up
	//! when it would otherwise run off the anchor.
	//! DEPRECATED - use OpenAt
	void ShowAt( float screenX, float screenY )
	{
		JMDeprecated.WarnOnce( this, "UIActionContextMenu.OpenAt() is deprecated. Please use OpenAt()." );

		OpenAt( screenX, screenY );
	}

	//! Open at the cursor - what a right-click menu wants.
	void OpenAtMouse()
	{
		int mx, my;
		GetMousePos( mx, my );
		OpenAt( mx, my );
	}

	void OpenAt( float screenX, float screenY )
	{
		if ( !m_Panel )
		{
			Error("[UIActionContextMenu] OpenAt failed: m_Panel is null!");
			return;
		}

		if ( m_Entries.Count() == 0 )
			return;

		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] UIActionContextMenu.OpenAt(" + screenX + "," + screenY + ") entries=" + m_Entries.Count());
		#endif

		m_PendingX  = screenX;
		m_PendingY  = screenY;
		m_Open      = true;
		m_OpenDelay = OPEN_DELAY;

		m_HoveredRow = -1;
		RefreshRowColors();

		m_Panel.Show( true );
		m_Panel.SetSort( 9999, true );

		UpdatePlacement();
	}

	override void Close()
	{
		#ifdef COT_DEBUGLOGS
		if ( m_Open )
			Print("[COT_DBG] UIActionContextMenu.Close() called, was open");
		#endif

		m_Open       = false;
		m_HoveredRow = -1;

		if ( m_Panel )
			m_Panel.Show( false );
	}

	//! UIActionBase.Hide() also tears down the update loop, which this control
	//! needs in order to notice the click that should dismiss it. Closing the
	//! popup is what a caller means by "hide" here.
	override void Hide()
	{
		Close();
	}

	protected void RebuildItems()
	{
		if ( !m_Grid )
		{
			Error("[UIActionContextMenu] RebuildItems failed: m_Grid is null!");
			return;
		}

		Widget child = m_Grid.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		for ( int i = 0; i < m_Entries.Count(); i++ )
		{
			JMContextMenuEntry entry = m_Entries[i];

			Widget row = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionContextMenuItem.layout", m_Grid );

			if ( !row )
				continue;

			row.SetHandler( this );

			ImageWidget rowIcon;
			Class.CastTo( rowIcon, row.FindAnyWidget( "item_icon" ) );
			if ( rowIcon )
			{
				if ( entry.IconPath != "" )
				{
					rowIcon.LoadImageFile( 0, entry.IconPath );
					rowIcon.SetImage( 0 );
					rowIcon.Show( true );
				}
				else
				{
					rowIcon.Show( false );
				}
			}

			ImageWidget rowArrow;
			Class.CastTo( rowArrow, row.FindAnyWidget( "item_arrow" ) );
			if ( rowArrow )
			{
				if ( entry.Submenu )
				{
					rowArrow.LoadImageFile( 0, JMConstants.Lucide( "chevron-right" ) );
					rowArrow.SetImage( 0 );
					rowArrow.Show( true );
				}
				else
				{
					rowArrow.Show( false );
				}
			}

			TextWidget rowText;
			Class.CastTo( rowText, row.FindAnyWidget( "item_text" ) );
			if ( rowText )
			{
				rowText.SetText( entry.Text );
				if ( entry.IconPath == "" )
					rowText.SetPos( TEXT_LEFT_PLAIN, 0 );
				else
					rowText.SetPos( TEXT_LEFT_ICON, 0 );
			}

			// The row name carries its index, same as the dropdown rows - it is
			// the only per-widget storage a plain Widget offers.
			row.SetName( "" + i );
		}

		RefreshRowColors();
	}

	protected void RefreshRowColors()
	{
		if ( !m_Grid )
		{
			Error("[UIActionContextMenu] RefreshRowColors failed: m_Grid is null!");
			return;
		}

		Widget child = m_Grid.GetChildren();
		int i = 0;
		while ( child )
		{
			bool enabled = true;
			if ( i < m_Entries.Count() )
				enabled = m_Entries[i].Enabled;

			int color = COLOR_ROW_NORMAL;
			if ( !enabled )
				color = COLOR_ROW_DISABLED;
			else if ( i == m_HoveredRow )
				color = COLOR_ROW_HOVER;

			child.SetColor( color );

			TextWidget rowText;
			Class.CastTo( rowText, child.FindAnyWidget( "item_text" ) );
			if ( rowText )
			{
				int textColor = COLOR_TEXT;
				if ( i < m_Entries.Count() && m_Entries[i].TextColor != 0 )
					textColor = m_Entries[i].TextColor;
				if ( !enabled )
					textColor = COLOR_TEXT_OFF;
				rowText.SetColor( textColor );
			}

			ImageWidget rowIcon;
			Class.CastTo( rowIcon, child.FindAnyWidget( "item_icon" ) );
			if ( rowIcon )
			{
				int iconColor = COLOR_TEXT;
				if ( i < m_Entries.Count() && m_Entries[i].TextColor != 0 )
					iconColor = m_Entries[i].TextColor;
				if ( !enabled )
					iconColor = COLOR_TEXT_OFF;
				rowIcon.SetColor( iconColor );
			}

			//! The chevron stays neutral whatever colour the label is - it says
			//! "there is more behind this", not what kind of thing it is.
			ImageWidget rowArrow;
			Class.CastTo( rowArrow, child.FindAnyWidget( "item_arrow" ) );
			if ( rowArrow )
			{
				int arrowColor = COLOR_TEXT;
				if ( !enabled )
					arrowColor = COLOR_TEXT_OFF;
				rowArrow.SetColor( arrowColor );
			}

			child = child.GetSibling();
			i++;
		}
	}

	//! Row index for a row widget or one of its children; -1 for anything else.
	protected int RowIndexOf( Widget w )
	{
		if ( !m_Grid || !w )
			return -1;
		if ( w.GetParent() == m_Grid )
			return w.GetName().ToInt();
		Widget p = w.GetParent();
		if ( p && p.GetParent() == m_Grid )
			return p.GetName().ToInt();
		return -1;
	}

	//! Left button only. A menu is usually raised BY a right-click, and the
	//! button is often still held when the rows appear under the cursor - so a
	//! right press landing on a row must not count as picking it. The row is
	//! still swallowed rather than passed on, so the press cannot reach
	//! whatever the menu is covering either.
	override bool OnClick( Widget w, int x, int y, int button )
	{
		int idx = RowIndexOf( w );
		if ( idx < 0 || idx >= m_Entries.Count() )
			return false;

		if ( button != MouseState.LEFT )
			return true;

		if ( !m_Entries[idx].Enabled )
			return true;

		JMContextMenuEntry entry = m_Entries[idx];
		m_LastClickedId = entry.Id;

		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] UIActionContextMenu.OnClick: row id=" + m_LastClickedId + " m_CloseOnClick=" + m_CloseOnClick.ToString());
		#endif

		if ( m_CloseOnClick )
			Close();

		if ( entry.Target && entry.Callback != "" )
		{
			GetGame().GameScript.CallFunctionParams( entry.Target, entry.Callback, null, new Param1<string>( entry.Id ) );
		}

		CallEvent( UIEvent.CLICK );
		return true;

	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		super.OnMouseEnter( w, x, y );
		int idx = RowIndexOf( w );
		if ( idx >= 0 )
		{
			m_HoveredRow = idx;
			RefreshRowColors();
		}
		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		super.OnMouseLeave( w, enterW, x, y );
		int idx = RowIndexOf( w );
		if ( idx >= 0 && idx == m_HoveredRow )
		{
			m_HoveredRow = -1;
			RefreshRowColors();
		}
		return false;
	}

	override bool OnKeyPress( Widget w, int x, int y, int key )
	{
		if ( m_Open && key == KeyCode.KC_ESCAPE )
		{
			Close();
			return true;
		}
		return super.OnKeyPress( w, x, y, key );
	}

	//! Widest label plus its own left inset plus a trailing gap, floored at
	//! DEFAULT_WIDTH. Re-run every frame the menu is open rather than once at
	//! build time: GetTextSize answers 0 until the row has actually been laid
	//! out, which is a frame or two after the widgets exist.
	protected void MeasureWidth()
	{
		if ( !m_Grid )
			return;

		float widest = 0;
		int   textW, textH;
		float left;

		Widget child = m_Grid.GetChildren();
		int i = 0;
		while ( child )
		{
			TextWidget rowText;
			Class.CastTo( rowText, child.FindAnyWidget( "item_text" ) );
			if ( rowText )
			{
				rowText.GetTextSize( textW, textH );

				left = TEXT_LEFT_PLAIN;
				if ( i < m_Entries.Count() && m_Entries[i].IconPath != "" )
					left = TEXT_LEFT_ICON;

				float pad = TEXT_PAD_RIGHT;
				if ( i < m_Entries.Count() && m_Entries[i].Submenu )
					pad = TEXT_PAD_RIGHT + SUBMENU_PAD_RIGHT;

				if ( left + textW + pad > widest )
					widest = left + textW + pad;
			}

			child = child.GetSibling();
			i++;
		}

		// Nothing measurable yet - keep whatever width is already up rather than
		// snapping the panel to the floor for a frame and back out again.
		if ( widest <= 0 )
			return;

		if ( widest < DEFAULT_WIDTH )
			widest = DEFAULT_WIDTH;

		// `widest` is one COLUMN. The panel has to hold all of them.
		m_MenuWidth = widest * m_Columns;
	}

	protected void UpdatePlacement()
	{
		if ( !m_Panel || !m_Grid || !m_Anchor )
			return;

		if ( m_AutoWidth )
			MeasureWidth();

		float ax, ay, aw, ah;
		m_Anchor.GetScreenPos( ax, ay );
		m_Anchor.GetScreenSize( aw, ah );

		//! The panel floats above the anchor's own container (see the class note
		//! on CreateContextMenu), so it is not clipped to the anchor's box - a
		//! form window shorter than the screen is not a ceiling for the menu.
		//! Flip and fit are measured against the real screen instead, or a menu
		//! opened near the bottom of a small window got a scrollbar it did not
		//! need, with empty screen below the window doing nothing about it.
		float screenW, screenH;
		g_Game.GetWorkspace().GetScreenSize( screenW, screenH );

		float gw, gh;
		m_Grid.GetScreenSize( gw, gh );
		if ( gh >= 1 )
			m_FullHeight = gh;

		if ( m_FullHeight <= 0 )
			return;

		float px = m_PendingX - ax;
		float py = m_PendingY - ay;

		// Flip rather than clamp: a menu shoved back inside the edge would cover
		// the row it belongs to, while a flipped one still points at it.
		if ( m_PendingX + m_MenuWidth > screenW )
			px = m_PendingX - ax - m_MenuWidth;
		if ( m_PendingY + m_FullHeight > screenH )
			py = m_PendingY - ay - m_FullHeight;

		if ( px < 0 )
			px = 0;
		if ( py < 0 )
			py = 0;

		// A menu taller than the screen still does not fit after flipping -
		// there is nowhere left to flip it TO. Capping the panel at the room
		// actually available and leaving the scroller to hold the rest beats
		// running the bottom rows off the edge with no way to reach them,
		// which is what a plain flip alone left it doing.
		float drawHeight = m_FullHeight;
		float available = screenH - ay - py - 8;
		if ( available > 0 && drawHeight > available )
			drawHeight = available;

		m_Panel.SetFlags( WidgetFlags.HEXACTSIZE );
		m_Panel.SetFlags( WidgetFlags.VEXACTSIZE );
		m_Panel.SetSize( m_MenuWidth, drawHeight );
		m_Panel.SetPos( px, py );

		// The layout enables the scrollbar unconditionally so a menu CAN
		// scroll once it grows past the screen; a short menu never gets
		// clipped, so it has nothing to scroll and the bar/track should not
		// draw at all.
		SetScrollbarVisible( drawHeight < m_FullHeight - 1 );
	}

	//! Only fires anything when the panel is actually shorter than the grid -
	//! GetContentHeight()/GetScreenSize() agreeing means there is nothing to
	//! scroll, and the event is left for whatever is behind the menu.
	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		if ( !m_Open || !m_Scroller )
			return super.OnMouseWheel( w, x, y, wheel );

		float contentH = m_Scroller.GetContentHeight();
		float rootW, rootH;
		m_Scroller.GetScreenSize( rootW, rootH );

		if ( contentH <= rootH )
			return super.OnMouseWheel( w, x, y, wheel );

		float pos = m_Scroller.GetVScrollPos();
		pos -= wheel * WHEEL_PIXEL_STEP;
		m_Scroller.VScrollToPos( pos );

		return true;
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( !m_Open )
			return;

		UpdatePlacement();

		if ( m_OpenDelay > 0 )
		{
			m_OpenDelay -= timeSlice;
			return;
		}

		bool leftDown  = ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) != 0;
		bool rightDown = ( GetMouseState( MouseState.RIGHT ) & MB_PRESSED_MASK ) != 0;

		if ( !leftDown && !rightDown )
			return;

		Widget under = GetWidgetUnderCursor();
		if ( IsFocusWidget( under ) || IsOwnedWidget( under ) )
			return;

		#ifdef COT_DEBUGLOGS
		string underName = "NULL";
		if ( under )
			underName = under.GetName() + " (" + under.ClassName() + ")";
		Print("[COT_DBG] UIActionContextMenu.Update: closing self - cursor over " + underName + " which is neither IsFocusWidget nor IsOwnedWidget (leftDown=" + leftDown.ToString() + " rightDown=" + rightDown.ToString() + ")");
		#endif

		Close();
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( !widget )
			return false;
		if ( widget == m_Panel )
			return true;
		if ( widget == m_Grid )
			return true;
		if ( widget == m_Scroller )
			return true;

		// The scrollbar thumb/track are children of the scroller but not of
		// the grid, so RowIndexOf below never sees them - dragging the thumb
		// would otherwise read as a click outside the menu and close it.
		if ( m_Scroller )
		{
			Widget w = widget;
			while ( w )
			{
				if ( w == m_Scroller )
					return true;
				w = w.GetParent();
			}
		}

		return RowIndexOf( widget ) >= 0;
	}
}
