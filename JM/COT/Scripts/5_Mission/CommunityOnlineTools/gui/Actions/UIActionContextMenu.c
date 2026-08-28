// =============================================================================
//  UIActionContextMenu
//
//  A right-click popup menu. Items carry a string ID, a label, an optional
//  icon and an optional label colour; clicking one fires UIEvent.CLICK and
//  GetLastClickedId() returns which. Closes on Escape or on a click outside
//  itself.
//
//  The menu draws nothing until ShowAt() is called. Its own layoutRoot is a 1px
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
//      m_Menu.SetItemEnabled( "delete", GetPermissionsManager().HasPermission( "..." ) );
//      m_Menu.ShowAt( m_Table.GetLastRightClickX(), m_Table.GetLastRightClickY() );
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

	void JMContextMenuEntry( string id, string text, string iconPath = "", int textColor = 0 )
	{
		Id        = id;
		Text      = text;
		IconPath  = iconPath;
		TextColor = textColor;
		Enabled   = true;
	}
}

class UIActionContextMenu: UIActionBase
{
	protected Widget           m_Anchor;
	protected Widget           m_Panel;
	protected GridSpacerWidget m_Grid;

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

	//! Cursor position ShowAt() was given, in screen pixels. Kept because the
	//! panel cannot be placed until its height is known, which is a frame later.
	protected float m_PendingX;
	protected float m_PendingY;

	protected float m_MenuWidth;

	static const int COLOR_ROW_NORMAL   = JMTheme.SURFACE_OVERLAY;
	static const int COLOR_ROW_HOVER    = JMTheme.ACCENT_WASH;
	static const int COLOR_ROW_DISABLED = JMTheme.SURFACE_OVERLAY;

	static const int COLOR_TEXT         = JMTheme.TEXT_PRIMARY;
	static const int COLOR_TEXT_OFF     = JMTheme.TEXT_DISABLED;

	//! Icon strip on the left of a row: 6 (position) + 14 (size) = 20, plus a
	//! 10px gap. A row without an icon pulls its label back to a plain padding
	//! so text-only menus are not indented for an icon that is not there.
	static const int TEXT_OFFSET_ICON   = 30;
	static const int TEXT_OFFSET_PLAIN  = 10;

	static const float DEFAULT_WIDTH    = 160;
	static const float OPEN_DELAY       = 0.15;

	override void OnInit()
	{
		super.OnInit();

		m_Entries       = new array<ref JMContextMenuEntry>;
		m_LastClickedId = "";
		m_HoveredRow    = -1;
		m_Open          = false;
		m_FullHeight    = 0;
		m_MenuWidth     = DEFAULT_WIDTH;
	}

	//! Create the popup panel under `anchor`. Must be called before ShowAt().
	void InitMenu( notnull Widget anchor )
	{
		m_Anchor = anchor;

		m_Panel = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionContextMenu_Panel.layout", m_Anchor );

		if ( m_Panel )
		{
			Class.CastTo( m_Grid, m_Panel.FindAnyWidget( "menu_grid" ) );
			m_Panel.Show( false );
			m_Panel.SetHandler( this );
			if ( m_Grid )
				m_Grid.SetHandler( this );
		}
	}

	//! Panel width in pixels. Menus size to their content vertically but not
	//! horizontally - there is no text measurement to size against.
	void SetMenuWidth( float pixels )
	{
		m_MenuWidth = pixels;
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
	void AddItem( string id, string label, string icon = "", int textColor = 0 )
	{
		m_Entries.Insert( new JMContextMenuEntry( id, label, icon, textColor ) );
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

	int GetItemCount()
	{
		return m_Entries.Count();
	}

	string GetLastClickedId()
	{
		return m_LastClickedId;
	}

	bool IsOpen()
	{
		return m_Open;
	}

	//! Open at a screen-space cursor position. The menu flips left and/or up
	//! when it would otherwise run off the anchor.
	void ShowAt( float screenX, float screenY )
	{
		if ( !m_Panel || m_Entries.Count() == 0 )
			return;

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
			return;

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

			TextWidget rowText;
			Class.CastTo( rowText, row.FindAnyWidget( "item_text" ) );
			if ( rowText )
			{
				rowText.SetText( Widget.TranslateString( entry.Text ) );
				if ( entry.IconPath == "" )
					rowText.SetTextOffset( TEXT_OFFSET_PLAIN, 0 );
				else
					rowText.SetTextOffset( TEXT_OFFSET_ICON, 0 );
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
			return;

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

	override bool OnClick( Widget w, int x, int y, int button )
	{
		int idx = RowIndexOf( w );
		if ( idx < 0 || idx >= m_Entries.Count() )
			return false;

		if ( !m_Entries[idx].Enabled )
			return true;

		m_LastClickedId = m_Entries[idx].Id;
		Close();
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

	protected void UpdatePlacement()
	{
		if ( !m_Panel || !m_Grid || !m_Anchor )
			return;

		float ax, ay, aw, ah;
		m_Anchor.GetScreenPos( ax, ay );
		m_Anchor.GetScreenSize( aw, ah );

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
		if ( m_PendingX + m_MenuWidth > ax + aw )
			px = m_PendingX - ax - m_MenuWidth;
		if ( m_PendingY + m_FullHeight > ay + ah )
			py = m_PendingY - ay - m_FullHeight;

		if ( px < 0 )
			px = 0;
		if ( py < 0 )
			py = 0;

		m_Panel.SetFlags( WidgetFlags.HEXACTSIZE );
		m_Panel.SetFlags( WidgetFlags.VEXACTSIZE );
		m_Panel.SetSize( m_MenuWidth, m_FullHeight );
		m_Panel.SetPos( px, py );
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
		if ( !IsFocusWidget( under ) )
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

		return RowIndexOf( widget ) >= 0;
	}
}
