// =============================================================================
//  UIActionDropdown
//
//  A dropdown that supports entries with an optional icon + label text.
//  The selected entry is shown in the collapsed header.
//  Fires UIEvent.CHANGE when the selection changes.
//
//  Usage:
//      m_Drop = UIActionManager.CreateDropdown( parent, "Status", this, "OnChange_Status" );
//      m_Drop.AddEntry( "Healthy",   JMConstants.ICON_HEALTH_NORMAL,  0xFF44FF44 );
//      m_Drop.AddEntry( "Wounded",   JMConstants.ICON_HEALTH_DECREASE, 0xFFFF8800 );
//      m_Drop.AddEntry( "Critical",  JMConstants.ICON_HEALTH_DECREASE, 0xFFFF3333 );
//      m_Drop.SetSelection( 0 );
// =============================================================================

class JMDropdownEntry
{
	string Text;
	string IconPath;
	int    IconColor;

	void JMDropdownEntry( string text, string iconPath = "", int iconColor = 0xFFFFFFFF )
	{
		Text      = text;
		IconPath  = iconPath;
		IconColor = iconColor;
	}
}

class UIActionDropdown: UIActionBase
{
	protected TextWidget   m_Label;
	protected Widget       m_Field;
	protected ImageWidget  m_SelectedIcon;
	protected TextWidget   m_SelectedText;
	protected ButtonWidget m_Toggle;
	protected ImageWidget  m_ToggleImage;
	protected Widget m_ListAnchor;
	protected Widget           m_ListPanel;
	protected ScrollWidget     m_Scroller;
	protected GridSpacerWidget m_List;
	protected ref array<ref JMDropdownEntry> m_Entries;
	protected int   m_SelectedIndex;
	protected int   m_HoveredRow;     // -1 when nothing highlighted
	protected bool  m_Open;
	protected bool  m_JustOpened;
	protected float             m_OpenDelay;
	protected ref JMAnimFloat   m_AnimT;
	protected float             m_FullHeight;

	//! Gap kept between an open list and the edges of its anchor.
	static const float LIST_EDGE_MARGIN = 4;
	static const int COLOR_ROW_NORMAL = JMTheme.SURFACE_OVERLAY;
	static const int COLOR_ROW_HOVER  = JMTheme.ACCENT_WASH;
	static const int COLOR_ROW_SELECT = JMTheme.SELECTED_FILL;

	// Where the label glyphs start, in layout pixels. Both the field's
	// action_selected_icon and the row's row_icon sit at x=4 and are 16 wide, so
	// their right edge is at 20. TEXT_OFFSET_ICON must clear that edge by
	// TEXT_ICON_GAP; the original value of 28 left only 8px and the icon visibly
	// ran into the text.
	//
	// These override whatever "text offset" the .layout declares, so change them
	// here - editing the layout alone has no effect at runtime.
	static const int ICON_RIGHT_EDGE   = 20;   // 4 (position) + 16 (size)
	static const int TEXT_ICON_GAP     = 20;
	static const int TEXT_OFFSET_ICON  = ICON_RIGHT_EDGE + TEXT_ICON_GAP;   // 40
	static const int TEXT_OFFSET_PLAIN = 8;    // no icon: just a left padding

	//! Pixels per wheel notch. A direct GetVScrollPos/VScrollToPos move rather
	//! than VScrollStep: VScrollStep's own step size is engine-controlled and
	//! landed several rows at once per notch, which read as the list jumping
	//! rather than scrolling; this also sidesteps VScrollStep's step direction
	//! disagreeing with `wheel`'s own sign, which is what had the wheel scroll
	//! backwards from what it showed - scrolling up moved the list down.
	static const float WHEEL_PIXEL_STEP = 40.0;

	//! The open list is parented to the ANCHOR, not to this control, so that it
	//! floats above whatever the host is drawing instead of being clipped by
	//! it. That also means unlinking the control does not take the panel with
	//! it - a dropdown built inside a section that gets rebuilt would leave one
	//! orphan panel behind per rebuild, still handled by a dead script. Take it
	//! down here.
	void ~UIActionDropdown()
	{
		if ( !g_Game )
			return;

		DestroyWidget(m_ListPanel);
	}

	string GetSelectedText()
	{
		if ( m_SelectedIndex < 0 || m_SelectedIndex >= m_Entries.Count() )
			return "";
		return m_Entries[ m_SelectedIndex ].Text;
	}

	//! Is `widget` any part of the collapsed control - the label, the field, the
	//! selected icon or text, the chevron? The open list is NOT part of it: it
	//! is parented to the list anchor rather than to layoutRoot, so the walk
	//! below never reaches it.
	protected bool IsHeaderWidget( Widget widget )
	{
		if ( !widget || !layoutRoot )
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

	protected bool IsListWidget( Widget widget )
	{
		if ( !widget || !m_ListPanel )
			return false;

		Widget w = widget;
		while ( w )
		{
			if ( w == m_ListPanel )
				return true;

			w = w.GetParent();
		}

		return false;
	}

	override bool IsOpen()
	{
		return m_Open;
	}

	void SetEntries( notnull array<ref JMDropdownEntry> entries )
	{
		m_Entries.Clear();
		foreach ( JMDropdownEntry e : entries )
			m_Entries.Insert( e );

		m_SelectedIndex = -1;
		RebuildList();
	}

	protected void SetHoveredRow( int idx )
	{
		m_HoveredRow = idx;
		RefreshRowColors();
	}

	// Convenience for plain string lists (parity with UIActionDropdownList /
	// UIActionSelectBox APIs).
	void SetItems( notnull array<string> items )
	{
		m_Entries.Clear();
		foreach ( string s : items )
			m_Entries.Insert( new JMDropdownEntry( s ) );

		m_SelectedIndex = -1;
		RebuildList();
		UpdateHeader();
	}

	protected void SetOpen( bool open )
	{
		m_Open = open;

		if ( m_ListPanel )
		{
			if ( open )
			{
				m_OpenDelay = 0.15;
				m_ListPanel.Show( true );
				m_ListPanel.SetSort( 9999, true );
				// Start keyboard-nav highlight at the current selection so
				// the user's first arrow press moves from a sensible place.
				m_HoveredRow = m_SelectedIndex;
				RefreshRowColors();
			}
			// Hide is deferred until animation completes (handled in Update).
		}

		if ( m_ToggleImage )
		{
			if ( open )
				m_ToggleImage.SetRotation( 0, 0, 180 );
			else
				m_ToggleImage.SetRotation( 0, 0, 0 );
		}
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,        layoutRoot.FindAnyWidget( "action_label"         ) );
		Class.CastTo( m_Field,        layoutRoot.FindAnyWidget( "action_field"         ) );
		Class.CastTo( m_SelectedIcon, layoutRoot.FindAnyWidget( "action_selected_icon" ) );
		Class.CastTo( m_SelectedText, layoutRoot.FindAnyWidget( "action_selected_text" ) );
		Class.CastTo( m_Toggle,       layoutRoot.FindAnyWidget( "action_toggle"        ) );
		Class.CastTo( m_ToggleImage,  layoutRoot.FindAnyWidget( "action_toggle_image"  ) );

		m_Entries       = new array<ref JMDropdownEntry>;
		m_SelectedIndex = -1;
		m_HoveredRow    = -1;
		m_Open          = false;
		m_FullHeight    = 0;
		m_AnimT         = new JMAnimFloat();
		m_AnimT.Set( 0 );

		if ( m_ToggleImage )
		{
			m_ToggleImage.LoadImageFile( 0, JMConstants.ICON_CHEVRON_DOWN );
			m_ToggleImage.SetImage( 0 );
		}
	}

	void InitList( notnull Widget listAnchor )
	{
		m_ListAnchor = listAnchor;

		m_ListPanel = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionDropdown_List.layout", m_ListAnchor );

		if ( m_ListPanel )
		{
			Class.CastTo( m_Scroller, m_ListPanel.FindAnyWidget( "action_list_scroller" ) );
			Class.CastTo( m_List,     m_ListPanel.FindAnyWidget( "action_list_grid" ) );
			m_ListPanel.Show( false );
			m_ListPanel.SetHandler( this );
			if ( m_List )
				m_List.SetHandler( this );
		}
	}

	//! Take the open list down with the control.
	//!
	//! The list panel is parented to the ANCHOR, not to this control, so hiding
	//! the control does not hide it. Neither does the deferred hide in Update()
	//! - a hidden action is off the update queue, so the frame that would have
	//! closed the list never arrives and the panel is left floating over
	//! whatever the host put in this control's place. Close it here, without
	//! the animation, because there is nothing left on screen to animate from.
	override void OnHide()
	{
		super.OnHide();

		m_Open      = false;
		m_OpenDelay = 0;

		if ( m_AnimT )
			m_AnimT.Set( 0 );

		if ( m_ListPanel )
			m_ListPanel.Show( false );

		if ( m_ToggleImage )
			m_ToggleImage.SetRotation( 0, 0, 0 );
	}

	override void SetLabel( string text )
	{
		if ( m_Label )
			m_Label.SetText( text );
	}

	void AddEntry( string text, string iconPath = "", int iconColor = 0xFFFFFFFF )
	{
		m_Entries.Insert( new JMDropdownEntry( text, iconPath, iconColor ) );
		RebuildList();
	}

	void ClearEntries()
	{
		m_Entries.Clear();
		m_SelectedIndex = -1;

		if ( m_List )
		{
			Widget child = m_List.GetChildren();
			while ( child )
			{
				Widget next = child.GetSibling();
				child.Unlink();
				child = next;
			}
		}

		UpdateHeader();
	}

	override int GetSelection()
	{
		return m_SelectedIndex;
	}

	//! The base class's IsVisible() reads layoutRoot, which here is the dropdown
	//! button itself - visible whenever the control is on screen, whether or
	//! not its list is expanded. m_Open tracks the list, which is what the
	//! Escape priority chain needs (a closed dropdown is not an open popup).
	override bool IsVisible()
	{
		return m_Open;
	}

	//! Close the popup from outside the control.
	//!
	//! The list panel is parented to the anchor passed to InitList(), which for
	//! every current caller is the window root rather than the control's own
	//! parent. Hiding the container the dropdown sits in therefore does not hide
	//! the open list - a tab switch would leave it floating over the new tab. A
	//! host that hides its content has to close the dropdown explicitly, and
	//! SetOpen is private, so this is the way in.
	override void Close()
	{
		SetOpen( false );
	}

	override void SetSelection( int i, bool sendEvent = true )
	{
		if ( m_Entries.Count() == 0 )
			return;

		if ( i < 0 )              i = 0;
		if ( i >= m_Entries.Count() ) i = m_Entries.Count() - 1;

		if ( i == m_SelectedIndex )
			return;

		m_SelectedIndex = i;
		UpdateHeader();
		RefreshRowColors();
		SetOpen( false );

		if ( sendEvent )
			CallEvent( UIEvent.CHANGE );
	}

	protected void UpdateHeader()
	{
		if ( m_SelectedIndex < 0 || m_SelectedIndex >= m_Entries.Count() )
		{
			if ( m_SelectedText ) m_SelectedText.SetText( "" );
			if ( m_SelectedIcon ) m_SelectedIcon.Show( false );
			return;
		}

		JMDropdownEntry entry = m_Entries[ m_SelectedIndex ];

		if ( m_SelectedText )
			m_SelectedText.SetText( entry.Text );

		if ( m_SelectedIcon )
		{
			if ( entry.IconPath != "" )
			{
				m_SelectedIcon.LoadImageFile( 0, entry.IconPath );
				m_SelectedIcon.SetImage( 0 );
				m_SelectedIcon.SetColor( entry.IconColor );
				m_SelectedIcon.Show( true );
				if ( m_SelectedText ) m_SelectedText.SetTextOffset( TEXT_OFFSET_ICON, 0 );
			}
			else
			{
				m_SelectedIcon.Show( false );
				if ( m_SelectedText ) m_SelectedText.SetTextOffset( TEXT_OFFSET_PLAIN, 0 );
			}
		}
	}

	protected void RebuildList()
	{
		if ( !m_List )
			return;

		Widget child = m_List.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		for ( int i = 0; i < m_Entries.Count(); i++ )
		{
			JMDropdownEntry entry = m_Entries[ i ];

			Widget row = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionDropdown_Row.layout", m_List );

			if ( !row )
				continue;

			row.SetHandler( this );

			ImageWidget rowIcon;
			Class.CastTo( rowIcon, row.FindAnyWidget( "row_icon" ) );
			if ( rowIcon )
			{
				if ( entry.IconPath != "" )
				{
					rowIcon.LoadImageFile( 0, entry.IconPath );
					rowIcon.SetImage( 0 );
					rowIcon.SetColor( entry.IconColor );
					rowIcon.Show( true );
				}
				else
				{
					rowIcon.Show( false );
				}
			}

			TextWidget rowText;
			Class.CastTo( rowText, row.FindAnyWidget( "row_text" ) );
			if ( rowText )
			{
				rowText.SetText( entry.Text );
				if ( entry.IconPath == "" )
					rowText.SetTextOffset( TEXT_OFFSET_PLAIN, 0 );
				else
					rowText.SetTextOffset( TEXT_OFFSET_ICON, 0 );
			}

			// Store index in the row widget name for retrieval on click
			row.SetName( "" + i );
		}

		RefreshRowColors();
	}

	override bool OnKeyPress( Widget w, int x, int y, int key )
	{
		if ( !m_Open || m_Entries.Count() == 0 )
			return super.OnKeyPress( w, x, y, key );

		if ( key == KeyCode.KC_DOWN )
		{
			int next = m_HoveredRow + 1;
			if ( next >= m_Entries.Count() )
				next = 0;
			SetHoveredRow( next );
			if ( m_Scroller )
				m_Scroller.VScrollStep( 1 );
			return true;
		}
		if ( key == KeyCode.KC_UP )
		{
			int prev = m_HoveredRow - 1;
			if ( prev < 0 )
				prev = m_Entries.Count() - 1;
			SetHoveredRow( prev );
			if ( m_Scroller )
				m_Scroller.VScrollStep( -1 );
			return true;
		}
		if ( key == KeyCode.KC_RETURN || key == KeyCode.KC_NUMPADENTER )
		{
			if ( m_HoveredRow >= 0 )
			{
				SetSelection( m_HoveredRow );
				return true;
			}
		}
		if ( key == KeyCode.KC_ESCAPE )
		{
			SetOpen( false );
			return true;
		}
		return super.OnKeyPress( w, x, y, key );
	}

	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		if ( m_Open && m_Scroller && m_ListPanel && m_ListPanel.IsVisible() )
		{
			float pos = m_Scroller.GetVScrollPos();
			pos -= wheel * WHEEL_PIXEL_STEP;
			m_Scroller.VScrollToPos( pos );

			return true;
		}

		return super.OnMouseWheel( w, x, y, wheel );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		super.OnMouseEnter( w, x, y );
		int idx = RowIndexOf( w );
		if ( idx >= 0 )
			SetHoveredRow( idx );
		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		super.OnMouseLeave( w, enterW, x, y );
		int idx = RowIndexOf( w );
		if ( idx >= 0 && idx == m_HoveredRow )
			SetHoveredRow( -1 );
		return false;
	}

	// Returns the row index for any widget that's a row or a row descendant.
	protected int RowIndexOf( Widget w )
	{
		if ( !m_List || !w )
			return -1;
		if ( w.GetParent() == m_List )
			return w.GetName().ToInt();
		Widget p = w.GetParent();
		if ( p && p.GetParent() == m_List )
			return p.GetName().ToInt();
		return -1;
	}

	protected void RefreshRowColors()
	{
		if ( !m_List )
			return;
		Widget child = m_List.GetChildren();
		int i = 0;
		while ( child )
		{
			int color = COLOR_ROW_NORMAL;
			if ( i == m_HoveredRow )
				color = COLOR_ROW_HOVER;
			else if ( i == m_SelectedIndex )
				color = COLOR_ROW_SELECT;
			child.SetColor( color );
			child = child.GetSibling();
			i++;
		}
	}

	//! The whole control opens the list, not just the 24px chevron.
	//!
	//! On mouse-down rather than through OnClick: action_field is a PanelWidget
	//! and action_label a TextWidget, and neither raises a click at all - only
	//! the chevron, which is the one ButtonWidget here, ever did. Handling the
	//! press means every pixel of the row answers, and it is why OnClick no
	//! longer toggles: doing both would open on the way down and shut on the
	//! way up.
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( button != MouseState.LEFT || !IsHeaderWidget( w ) )
			return super.OnMouseButtonDown( w, x, y, button );

		// The disable overlay is a child of layoutRoot, so it passes the header
		// test. Widget.Enable(false) should already keep events away, but a
		// disabled control opening its list is not a failure worth risking.
		if ( !IsEnabled() )
			return true;

		SetOpen( !m_Open );

		return true;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( m_List && w.GetParent() == m_List )
		{
			int idx = w.GetName().ToInt();
			SetSelection( idx );
			return true;
		}

		// Click on child of a row (icon/text widget)
		if ( m_List )
		{
			Widget parent = w.GetParent();
			if ( parent && parent.GetParent() == m_List )
			{
				int rowIdx = parent.GetName().ToInt();
				SetSelection( rowIdx );
				return true;
			}
		}

		return false;
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( m_ListPanel && m_List && m_Field && m_ListAnchor )
		{
			float ax, ay, anchorW, anchorH;
			float fx, fy, fw, fh, gw, gh;
			m_ListAnchor.GetScreenPos( ax, ay );
			m_ListAnchor.GetScreenSize( anchorW, anchorH );
			m_Field.GetScreenPos( fx, fy );
			m_Field.GetScreenSize( fw, fh );
			m_List.GetScreenSize( gw, gh );

			// Track full height from grid content.
			if ( gh >= 1 )
				m_FullHeight = gh;

			// Drive animation target.
			float animTarget = 0.0;
			if ( m_Open )
				animTarget = 1.0;
			m_AnimT.SetTarget( animTarget, 12.0 );
			m_AnimT.Step( timeSlice );

			float t = JMUIAnim.EaseOut( m_AnimT.Value );

			if ( !m_Open && !m_AnimT.IsAnimating() )
				m_ListPanel.Show( false );

			if ( m_ListPanel.IsVisible() && m_FullHeight > 0 )
			{
				// A list can be taller than the window it opens in - the object
				// spawner's liquid list runs to sixteen entries - and nothing
				// renders outside the anchor, so cap it there first.
				float maxH = anchorH - LIST_EDGE_MARGIN * 2;
				float fullH = m_FullHeight;

				float listCapH = 360.0;
				if ( maxH > 0 && maxH < listCapH )
					listCapH = maxH;

				if ( fullH > listCapH )
					fullH = listCapH;

				//! No runtime scrollbar-visibility setter exists on ScrollWidget -
				//! the native C++ side shows it on its own once content actually
				//! overflows the capped height set above.
				float animH = fullH * t;
				m_ListPanel.SetFlags( WidgetFlags.HEXACTSIZE );
				m_ListPanel.SetFlags( WidgetFlags.VEXACTSIZE );
				m_ListPanel.SetSize( fw, animH );
				m_ListPanel.SetAlpha( t );

				// Position list below field; flip upward if it clips past anchor bottom.
				float posY;
				if ( fy + fh + fullH > ay + anchorH )
					posY = fy - ay - animH;
				else
					posY = fy - ay + fh;

				// Then clamp INSIDE the anchor. Flipping up is only right while
				// there is room above; a long list opening near the bottom used
				// to flip straight past the top edge and lose its first rows to
				// a region nothing draws in.
				float maxY = anchorH - animH - LIST_EDGE_MARGIN;

				if ( posY > maxY )
					posY = maxY;

				if ( posY < LIST_EDGE_MARGIN )
					posY = LIST_EDGE_MARGIN;

				m_ListPanel.SetPos( fx - ax, posY );
			}

			// Delay before allowing close-on-outside-click.
			if ( m_Open )
			{
				if ( m_OpenDelay > 0 )
				{
					m_OpenDelay -= timeSlice;
				}
				else if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) != 0 )
				{
					Widget under = GetWidgetUnderCursor();
					if ( !IsFocusWidget( under ) )
						SetOpen( false );
				}
			}
		}
	}

	override bool IsFocusWidget( Widget widget )
	{
		// The whole header, not just the field and the chevron: a press on the
		// label now toggles the list, so it must not also read as the outside
		// click that closes it.
		if ( IsHeaderWidget( widget ) ) return true;
		if ( IsListWidget( widget ) ) return true;
		return false;
	}
}
