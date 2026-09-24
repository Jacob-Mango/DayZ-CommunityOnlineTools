// =============================================================================
//  UIActionTabs
//
//  A row of tab buttons plus the content panels they switch between. Fires
//  UIEvent.CHANGE when the active tab changes; GetSelection() returns the new
//  tab's id.
//
//  Tabs and panels are added and removed one at a time, each with a stable id
//  that only ever grows and is never reused - keep the id AddTab() returns
//  instead of writing a number down (another mod may append tabs to the same
//  strip, and removing a tab never renumbers the others):
//
//      m_Tabs = UIActionManager.CreateTabStrip( parent, this, "OnChange_Tab" );
//      m_TabIdGeneral = m_Tabs.AddTab( "General", JMConstants.Lucide( "user" ), m_PanelGeneral );
//      m_TabIdStats   = m_Tabs.AddTab( "Stats",   JMConstants.Lucide( "chart-column" ), m_PanelStats );
//      m_Tabs.AddPanel( m_TabIdStats, m_ExtraStatsPanel );     // a tab may show several panels
//      m_Tabs.SetSelection( m_TabIdGeneral );
//
//      m_Tabs.RemoveTabById( m_TabIdStats );                   // or RemoveTab( m_PanelStats )
//      m_Tabs.RemovePanelById( id );                           // or RemovePanel( m_ExtraStatsPanel )
//
//  The strip only shows and hides panels; the widgets stay the caller's.
//
//  Tabs that do not fit on one row wrap onto more (see the Wrapping constants). The strip
//  fits itself to its width every WIDTH_POLL_MS, and a form that pins the strip's height gets
//  the extra rows from COT_FitTabs() and is told to pin again through OnTabStripFitted().
// =============================================================================
class UIActionTabs: UIActionBase
{
	protected Widget                          m_TabBar;      // the first row of tabs
	protected ref array<Widget>               m_ExtraBars;   // rows 2.. when the tabs wrap; each is its own single-row bar
	protected Widget                          m_TabBarParent;
	protected ref array<ref UIActionTabEntry> m_Entries;   // live tabs, in strip order
	protected ref array<ref UIActionTabPanel> m_Panels;
	protected int                             m_NextTabId;
	protected int                             m_NextPanelId;
	protected int                             m_Selected;   // tab id, -1 when none
	protected int                             m_Hovered;    // tab id, -1 when no tab hovered
	protected int                             m_Cols;       // the grid the bar is currently built as
	protected int                             m_Rows;
	protected float                           m_FittedWidth = -1;   // width, in pixels, the grid was last fitted to

	//! The selected tab is the mod's canonical "this one is ON" pill; every
	//! other on-state in the UI (filter chip, icon cell) pulls the same tokens.
	static const int COLOR_ACTIVE   = JMTheme.SELECTED_FILL;
	static const int COLOR_HOVER    = JMTheme.ACCENT_WASH;
	//! Inactive tabs carry no pill at all - only the icon and label show.
	static const int COLOR_INACTIVE = JMTheme.SELECTED_FILL_OFF;

	//! Only the selected tab draws its outline.
	static const int COLOR_OUTLINE  = JMTheme.SELECTED_OUTLINE;
	static const int ICON_ACTIVE    = JMTheme.BUTTON_TEXT;
	static const int ICON_INACTIVE  = JMTheme.BUTTON_TEXT_INACTIVE;

	// The tab label is centred across the WHOLE button, while the icon sits at a
	// fixed x=10 on the left (see UIActionTabButton.layout). A centred label
	// therefore grows leftwards straight into the icon as soon as the text is
	// wide enough - which is what made the tab bar's icons and text collide.
	//
	// Shifting the glyphs right by half the reserved strip re-centres the label
	// in the space LEFT OF nothing / RIGHT OF the icon instead:
	//   glyph centre moves from  W/2  to  W/2 + LABEL_OFFSET_ICON
	//   which is the centre of [ICON_RIGHT_EDGE + ICON_TEXT_GAP, W]
	// Tabs are equal-width grid columns, so a label that is wider than
	// W - (ICON_RIGHT_EDGE + ICON_TEXT_GAP) can still reach the icon; keep tab
	// captions short.
	static const int ICON_RIGHT_EDGE    = 26;   // 10 (position) + 16 (size)
	static const int ICON_TEXT_GAP      = 8;
	static const int LABEL_OFFSET_ICON  = ( ICON_RIGHT_EDGE + ICON_TEXT_GAP ) / 2;   // 17

	// Wrapping. Tabs stay equal-width columns; when the widest tab no longer fits its column the
	// strip gains a row instead of letting icons and captions collide. The tabs are spread evenly
	// over the rows, so nine tabs become 5 + 4, not 8 + 1. Each row is its own single-row bar
	// (TabBarN.layout), stacked TAB_ROW_HEIGHT apart - a multi-row grid spacer makes every row
	// taller than a single-row bar and the strip then clips it.
	static const float TAB_ROW_HEIGHT   = 30;   // must match UIActionTabButton.layout / TabBarN.layout
	static const float TAB_LABEL_PAD    = 20;   // breathing room around a caption, left and right together
	static const int   MAX_COLUMNS      = 9;    // the widest TabBarN layout there is
	static const int   WIDTH_POLL_MS    = 250;
	//! Fewer rows than now are only taken when they fit with this much to spare. Adding a row can make a
	//! scroller show its scrollbar, which narrows the strip by a few pixels - without the margin the
	//! strip would unwrap, get wider, wrap again, and flap on every poll.
	static const float UNWRAP_MARGIN    = 24;

	//! DEPRECATED - use GetPanel( tabId ).
	Widget GetContentPanel( int idx )
	{
		JMDeprecated.WarnOnce( this, "UIActionTabs.GetContentPanel() is deprecated. Please use GetPanel( tabId )." );

		return GetPanel( idx );
	}

	//! The first panel of tab `tabId`, or null.
	Widget GetPanel( int tabId )
	{
		foreach ( UIActionTabPanel entry : m_Panels )
		{
			if ( entry.TabId == tabId )
				return entry.PanelWidget;
		}

		return null;
	}

	//! Number of tabs currently in the strip.
	int GetTabCount()
	{
		return m_Entries.Count();
	}

	bool HasTab( int tabId )
	{
		return FindEntry( tabId ) != null;
	}

	bool IsTabEnabled( int tabId )
	{
		UIActionTabEntry entry = FindEntry( tabId );
		if ( !entry )
			return false;

		return entry.Enabled;
	}

	bool IsTabVisible( int tabId )
	{
		UIActionTabEntry entry = FindEntry( tabId );
		if ( !entry )
			return true;

		return entry.Visible;
	}

	// -------------------------------------------------------------------------
	//  Tab state - all by tab id
	// -------------------------------------------------------------------------

	void SetTabEnabled( int tabId, bool enabled )
	{
		UIActionTabEntry entry = FindEntry( tabId );
		if ( !entry )
			return;

		entry.Enabled = enabled;

		if ( entry.Button )
			entry.Button.Enable( enabled );

		UpdateTabColors();
	}

	//! Select tab `tabId` (state true), or - when it is the selected one - move to the first other
	//! visible tab (state false). Same grammar as SetTabEnabled / SetTabVisible.
	void SetTabFocused( int tabId, bool state )
	{
		if ( state )
		{
			SetSelection( tabId );
			return;
		}

		if ( GetSelection() != tabId )
			return;

		foreach ( UIActionTabEntry entry : m_Entries )
		{
			if ( entry.Id != tabId && entry.Visible && entry.Enabled )
			{
				SetSelection( entry.Id );
				return;
			}
		}
	}

	//! DEPRECATED - pass the icon to AddTab. One icon path per tab, in strip order.
	void SetTabIcons( notnull array<string> icons )
	{
		JMDeprecated.WarnOnce( this, "UIActionTabs.SetTabIcons() is deprecated. Please pass the icon to AddTab()." );

		for ( int i = 0; i < m_Entries.Count(); i++ )
		{
			if ( i < icons.Count() )
				m_Entries[i].IconPath = icons[i];
			else
				m_Entries[i].IconPath = "";

			ApplyEntryIcon( m_Entries[i] );
		}

		UpdateTabColors();
	}

	void SetTabPermission( int tabId, string permission )
	{
		UIActionTabEntry entry = FindEntry( tabId );
		if ( !entry )
			return;

		entry.Permission = permission;
		UpdateTabPermissions();
	}

	// -------------------------------------------------------------------------
	//  DEPRECATED - the pre-id spellings. Use AddTab / AddPanel.
	// -------------------------------------------------------------------------

	//! DEPRECATED - use AddTab( label, icon, panel ) for each tab. Replaces the whole set of tabs.
	void SetTabs( notnull array<string> labels )
	{
		JMDeprecated.WarnOnce( this, "UIActionTabs.SetTabs() is deprecated. Please use AddTab( label, icon, panel ) for each tab." );

		for ( int i = m_Entries.Count() - 1; i >= 0; i-- )
			RemoveTabById( m_Entries[i].Id );

		foreach ( string label : labels )
			AddTab( label );
	}

	void SetTabVisible( int tabId, bool visible )
	{
		UIActionTabEntry entry = FindEntry( tabId );
		if ( !entry )
			return;

		entry.Visible = visible;
		ApplyEntryVisibility( entry );

		if ( !visible && m_Selected == tabId )
			SelectFirstVisibleTab();

		UpdateTabColors();
		UpdateContentVisibility();
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_TabBarParent, layoutRoot.FindAnyWidget( "action_wrapper" ) );

		m_Entries = new array<ref UIActionTabEntry>;
		m_Panels  = new array<ref UIActionTabPanel>;
		m_Selected = -1;
		m_Hovered  = -1;

		//! A window is resized by the user with nothing telling a strip about it, so the strip
		//! looks at its own width now and then. A form that pins the strip also fits it directly
		//! from its own OnResize (see COT_FitTabs), which is what keeps that resize flicker-free.
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( CheckWidth, WIDTH_POLL_MS, true );
	}

	void ~UIActionTabs()
	{
		if ( !g_Game )
			return;

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( CheckWidth );
	}

	//! Height of the strip as it is built now: one TAB_ROW_HEIGHT per row.
	float GetRequiredHeight()
	{
		int rows = m_Rows;
		if ( rows < 1 )
			rows = 1;

		return rows * TAB_ROW_HEIGHT;
	}

	//! Lay the tabs out for a strip `width` pixels wide: as many equal columns as the widest tab
	//! allows, as many rows as the tabs then need. Rebuilds the bar only when that changes.
	//! Returns the height gained over a single row (0 when everything fits on one).
	float FitToWidth( float width )
	{
		if ( !m_Entries || m_Entries.Count() == 0 || width <= 0 )
			return 0;

		m_FittedWidth = width;

		int cols;
		int rows;
		ChooseGrid( width, cols, rows );

		if ( m_Rows > 0 && rows < m_Rows )
		{
			int roomyCols;
			int roomyRows;
			ChooseGrid( width - UNWRAP_MARGIN, roomyCols, roomyRows );

			if ( roomyRows >= m_Rows )
			{
				cols = m_Cols;
				rows = m_Rows;
			}
		}

		if ( cols != m_Cols || rows != m_Rows )
		{
			m_Cols = cols;
			m_Rows = rows;
			RebuildBar();
		}

		return GetRequiredHeight() - TAB_ROW_HEIGHT;
	}

	override float COT_FitTabs( float width )
	{
		return FitToWidth( width );
	}

	protected void CheckWidth()
	{
		if ( !layoutRoot || !layoutRoot.IsVisibleHierarchy() )
			return;

		float width;
		float height;
		layoutRoot.GetScreenSize( width, height );

		if ( Math.AbsFloat( width - m_FittedWidth ) < 1 )
			return;

		int rowsBefore = m_Rows;

		FitToWidth( width );

		//! A form that pins this strip measured the window before its layout settled, so it pinned the
		//! old height. Now that the row count is right, tell it to pin again.
		if ( m_Rows != rowsBefore )
			NotifyForm();
	}

	protected void NotifyForm()
	{
		Widget node = layoutRoot;
		while ( node )
		{
			JMFormBase form = null;
			node.GetScript( form );

			if ( form )
			{
				form.OnTabStripFitted();
				return;
			}

			node = node.GetParent();
		}
	}

	//! Width a tab needs so its icon and caption do not collide: the icon strip, the caption and
	//! padding. The caption is measured once its label exists; before that it is estimated.
	protected float MinTabWidth( UIActionTabEntry entry )
	{
		float textWidth;

		if ( entry.LabelWidget )
		{
			int measuredW;
			int measuredH;
			entry.LabelWidget.GetTextSize( measuredW, measuredH );
			textWidth = measuredW;
		}

		if ( textWidth <= 0 )
			textWidth = entry.Caption.Length() * 8;

		float needed = textWidth + TAB_LABEL_PAD;
		if ( entry.IconPath != "" )
			needed += ICON_RIGHT_EDGE + ICON_TEXT_GAP - 10;

		return needed;
	}

	//! Columns and rows for `width`. Rows come first, then the columns are the fewest that hold the
	//! tabs in that many rows, so the last row is never a lone straggler.
	protected void ChooseGrid( float width, out int cols, out int rows )
	{
		int count = m_Entries.Count();

		float widest;
		foreach ( UIActionTabEntry entry : m_Entries )
			widest = Math.Max( widest, MinTabWidth( entry ) );

		cols = count;
		if ( widest > 0 )
			cols = (int)( width / widest );

		cols = Math.Clamp( cols, 1, Math.Min( count, MAX_COLUMNS ) );

		rows = ( count + cols - 1 ) / cols;
		cols = ( count + rows - 1 ) / rows;
	}

	// -------------------------------------------------------------------------
	//  Tabs
	// -------------------------------------------------------------------------

	//! Append a tab and return its id (-1 if the strip has no bar to put it in). `panel` is optional
	//! sugar for AddPanel( id, panel ): it is shown while the tab is selected.
	int AddTab( string label, string icon = "", Widget panel = null )
	{
		if ( !m_TabBarParent )
			return -1;

		UIActionTabEntry entry = new UIActionTabEntry();
		entry.Id    = m_NextTabId;
		entry.Caption = label;
		entry.IconPath  = icon;
		m_NextTabId++;

		m_Entries.Insert( entry );
		RebuildBar();

		if ( panel )
			AddPanel( entry.Id, panel );

		return entry.Id;
	}

	//! Remove a tab and every panel attached to it. The other tabs keep their ids. If it was the
	//! selected one the first other visible tab is selected (CHANGE fires).
	void RemoveTabById( int tabId )
	{
		UIActionTabEntry entry = FindEntry( tabId );
		if ( !entry )
			return;

		for ( int p = m_Panels.Count() - 1; p >= 0; p-- )
		{
			if ( m_Panels[p].TabId == tabId )
				DetachPanel( p );
		}

		bool wasSelected = ( m_Selected == tabId );

		m_Entries.RemoveItem( entry );

		if ( m_Hovered == tabId )
			m_Hovered = -1;

		RebuildBar();

		if ( wasSelected )
		{
			m_Selected = -1;
			SelectFirstVisibleTab();
		}
	}

	//! Remove the tab that owns `panel` (the widget passed to AddTab / AddPanel).
	void RemoveTab( Widget panel )
	{
		UIActionTabPanel panelEntry = FindPanelEntry( panel );
		if ( panelEntry )
			RemoveTabById( panelEntry.TabId );
	}

	// -------------------------------------------------------------------------
	//  Panels
	// -------------------------------------------------------------------------

	//! Attach `panel` to tab `tabId` and return the panel's id (-1 if there is no such tab). It is
	//! shown while that tab is selected; a tab may have several panels.
	int AddPanel( int tabId, Widget panel )
	{
		if ( !panel || !FindEntry( tabId ) )
			return -1;

		UIActionTabPanel entry = new UIActionTabPanel();
		entry.Id    = m_NextPanelId;
		entry.TabId = tabId;
		entry.PanelWidget = panel;
		m_NextPanelId++;

		m_Panels.Insert( entry );
		UpdateContentVisibility();

		return entry.Id;
	}

	//! Detach a panel from its tab. The widget is hidden but not destroyed - it stays the caller's.
	void RemovePanelById( int panelId )
	{
		for ( int i = 0; i < m_Panels.Count(); i++ )
		{
			if ( m_Panels[i].Id == panelId )
			{
				DetachPanel( i );
				return;
			}
		}
	}

	void RemovePanel( Widget panel )
	{
		for ( int i = 0; i < m_Panels.Count(); i++ )
		{
			if ( m_Panels[i].PanelWidget == panel )
			{
				DetachPanel( i );
				return;
			}
		}
	}

	protected void DetachPanel( int index )
	{
		Widget detached = m_Panels[index].PanelWidget;
		m_Panels.Remove( index );

		if ( detached )
			detached.Show( false );
	}

	void SelectFirstVisibleTab()
	{
		foreach ( UIActionTabEntry entry : m_Entries )
		{
			if ( entry.Visible && entry.Enabled )
			{
				SetSelection( entry.Id );
				return;
			}
		}

		m_Selected = -1;
		UpdateTabColors();
		UpdateContentVisibility();
	}

	void UpdateTabPermissions()
	{
		int visibleCount = 0;
		bool anyPermission = false;

		foreach ( UIActionTabEntry entry : m_Entries )
		{
			bool allowed = true;
			if ( entry.Permission != "" )
			{
				anyPermission = true;
				allowed = JMPermissions.Has( entry.Permission );
			}

			SetTabVisible( entry.Id, allowed );
			SetTabEnabled( entry.Id, allowed );

			if ( allowed )
				visibleCount++;
		}

		if ( anyPermission )
			SetVisible( visibleCount > 0 );
	}

	override void UpdatePermission( string permission )
	{
		super.UpdatePermission( permission );
		UpdateTabPermissions();
	}

	override int GetSelection()
	{
		return m_Selected;
	}

	//! (Parameter named `i` to match the UIActionBase prototype - overrides must reuse its names.)
	override void SetSelection( int i, bool sendEvent = true )
	{
		UIActionTabEntry entry = FindEntry( i );
		if ( !entry || !entry.Enabled || !entry.Visible )
			return;

		m_Selected = i;
		UpdateTabColors();
		UpdateContentVisibility();

		if ( sendEvent )
			CallEvent( UIEvent.CHANGE );
	}

	// -------------------------------------------------------------------------
	//  COT_ScriptedWidgetEventHandler tab-strip contract - what JMFormBase
	//  (4_World, cannot name this class) calls to add, remove and query tabs.
	// -------------------------------------------------------------------------

	override int COT_TabSelection()
	{
		return GetSelection();
	}

	override int COT_AddTab( string label, string icon, Widget host )
	{
		if ( !host )
			return -1;

		return AddTab( label, icon, UIActionManager.CreateGridSpacer( host, 8, 1 ) );
	}

	override int COT_AddPanel( int tab, Widget panel )
	{
		return AddPanel( tab, panel );
	}

	override void COT_RemoveTabById( int tab )
	{
		RemoveTabById( tab );
	}

	override void COT_RemovePanelById( int panelId )
	{
		RemovePanelById( panelId );
	}

	override void COT_RemovePanel( Widget panel )
	{
		RemovePanel( panel );
	}

	override Widget COT_TabContent( int idx )
	{
		return GetPanel( idx );
	}

	override void COT_SetTabEnabled( int idx, bool enabled )
	{
		SetTabEnabled( idx, enabled );
	}

	override bool COT_IsTabEnabled( int idx )
	{
		return IsTabEnabled( idx );
	}

	override void COT_SetTabVisible( int idx, bool visible )
	{
		SetTabVisible( idx, visible );
	}

	override void COT_SetTabFocused( int idx, bool focused )
	{
		SetTabFocused( idx, focused );
	}

	// -------------------------------------------------------------------------
	//  Input
	// -------------------------------------------------------------------------

	override bool OnClick( Widget w, int x, int y, int button )
	{
		int tabId = TabIdOf( w );
		if ( tabId >= 0 && IsTabEnabled( tabId ) && IsTabVisible( tabId ) )
		{
			SetSelection( tabId );
			return true;
		}

		return false;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		super.OnMouseEnter( w, x, y );

		int tabId = TabIdOf( w );
		if ( tabId >= 0 && IsTabEnabled( tabId ) && IsTabVisible( tabId ) )
		{
			m_Hovered = tabId;
			UpdateTabColors();
		}

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		super.OnMouseLeave( w, enterW, x, y );

		int tabId = TabIdOf( w );
		if ( tabId >= 0 && tabId == m_Hovered )
		{
			m_Hovered = -1;
			UpdateTabColors();
		}

		return false;
	}

	// -------------------------------------------------------------------------
	//  Internals
	// -------------------------------------------------------------------------

	protected UIActionTabEntry FindEntry( int tabId )
	{
		foreach ( UIActionTabEntry entry : m_Entries )
		{
			if ( entry.Id == tabId )
				return entry;
		}

		return null;
	}

	protected UIActionTabPanel FindPanelEntry( Widget panel )
	{
		foreach ( UIActionTabPanel entry : m_Panels )
		{
			if ( entry.PanelWidget == panel )
				return entry;
		}

		return null;
	}

	protected int TabIdOf( Widget w )
	{
		foreach ( UIActionTabEntry entry : m_Entries )
		{
			if ( entry.Button && w == entry.Button )
				return entry.Id;
		}

		return -1;
	}

	//! (Re)create the bar and one button per live tab. The bar's layout is picked from the tab count
	//! (equal-width columns), so it is rebuilt whenever a tab is added or removed.
	protected void RebuildBar()
	{
		if ( m_TabBar )
		{
			m_TabBar.Unlink();
			m_TabBar = null;
		}

		if ( m_ExtraBars )
		{
			foreach ( Widget oldBar : m_ExtraBars )
				oldBar.Unlink();

			m_ExtraBars.Clear();
		}

		int count = m_Entries.Count();
		if ( count == 0 || !m_TabBarParent )
			return;

		//! Until the strip has been fitted to a real width the tabs share one row.
		if ( m_Cols < 1 || m_Rows < 1 || m_Cols * m_Rows < count )
		{
			m_Cols = Math.Clamp( count, 1, MAX_COLUMNS );
			m_Rows = ( count + m_Cols - 1 ) / m_Cols;
			m_FittedWidth = -1;
		}

		//! One single-row bar per row, stacked TAB_ROW_HEIGHT apart. A multi-row grid spacer was tried
		//! first and made every row taller than a single-row bar (44 px instead of 30), which the
		//! strip then clipped; separate bars keep each row exactly the height it always had. Every
		//! bar has the same column count, so a short last row keeps the same tab width.
		string gridLayout = string.Format( "JM/COT/GUI/layouts/uiactions/Wrappers/TabBar/TabBar%1.layout", m_Cols );

		if ( !m_ExtraBars )
			m_ExtraBars = new array<Widget>;

		m_TabBar = g_Game.GetWorkspace().CreateWidgets( gridLayout, m_TabBarParent );
		if ( m_TabBar )
			m_TabBar.SetHandler( this );

		for ( int row = 1; row < m_Rows; row++ )
		{
			Widget rowBar = g_Game.GetWorkspace().CreateWidgets( gridLayout, m_TabBarParent );
			if ( !rowBar )
				continue;

			rowBar.SetFlags( WidgetFlags.VEXACTPOS, true );
			rowBar.SetPos( 0, row * TAB_ROW_HEIGHT );
			rowBar.SetHandler( this );

			m_ExtraBars.Insert( rowBar );
		}

		ApplyStripHeight();

		if ( !m_TabBar )
			return;

		for ( int index = 0; index < m_Entries.Count(); index++ )
			BuildButton( m_Entries[index], BarForRow( index / m_Cols ) );

		UpdateTabColors();
	}

	//! The strip is TAB_ROW_HEIGHT pixels per row; the parent (a flow container) is asked to lay out again.
	protected void ApplyStripHeight()
	{
		float width;
		float height;
		layoutRoot.GetSize( width, height );
		layoutRoot.SetSize( width, GetRequiredHeight() );
		layoutRoot.Update();

		Widget host = layoutRoot.GetParent();
		if ( host )
			host.Update();
	}

	//! The bar that holds row `row` (0 = the first).
	protected Widget BarForRow( int row )
	{
		if ( row <= 0 || !m_ExtraBars || row > m_ExtraBars.Count() )
			return m_TabBar;

		return m_ExtraBars[row - 1];
	}

	protected bool IsBar( Widget w )
	{
		if ( w == m_TabBar )
			return true;

		return m_ExtraBars && m_ExtraBars.Find( w ) != -1;
	}

	protected void BuildButton( UIActionTabEntry entry, Widget bar )
	{
		entry.Button = null;
		entry.Fill = null;
		entry.Outline = null;
		entry.IconWidget = null;
		entry.LabelWidget = null;

		Widget tabWidget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionTabButton.layout", bar );
		if ( !tabWidget )
			return;

		Class.CastTo( entry.Button, tabWidget );
		Class.CastTo( entry.LabelWidget, tabWidget.FindAnyWidget( "label" ) );
		Class.CastTo( entry.IconWidget, tabWidget.FindAnyWidget( "icon" ) );

		entry.Fill    = tabWidget.FindAnyWidget( "fill" );
		entry.Outline = tabWidget.FindAnyWidget( "outline" );

		tabWidget.SetHandler( this );

		if ( entry.LabelWidget )
			entry.LabelWidget.SetText( entry.Caption );

		ApplyEntryIcon( entry );

		if ( entry.Button )
			entry.Button.Enable( entry.Enabled );

		ApplyEntryVisibility( entry );
	}

	protected void ApplyEntryIcon( UIActionTabEntry entry )
	{
		if ( entry.IconWidget )
		{
			if ( entry.IconPath != "" )
			{
				entry.IconWidget.LoadImageFile( 0, entry.IconPath );
				entry.IconWidget.Show( true );
			}
			else
			{
				entry.IconWidget.Show( false );
			}
		}

		if ( entry.LabelWidget )
		{
			//! Glyphs move right by half the icon strip so the label stays centred beside it.
			if ( entry.IconPath != "" )
				entry.LabelWidget.SetTextOffset( LABEL_OFFSET_ICON, 0 );
			else
				entry.LabelWidget.SetTextOffset( 0, 0 );
		}
	}

	protected void ApplyEntryVisibility( UIActionTabEntry entry )
	{
		if ( !entry.Button )
			return;

		Widget w = entry.Button;
		if ( w.GetParent() && !IsBar( w.GetParent() ) )
			w = w.GetParent();

		w.Show( entry.Visible );
	}

	protected void UpdateTabColors()
	{
		foreach ( UIActionTabEntry entry : m_Entries )
		{
			int color = COLOR_INACTIVE;
			if ( !entry.Enabled )
				color = 0x00000000;
			else if ( entry.Id == m_Selected )
				color = COLOR_ACTIVE;
			else if ( entry.Id == m_Hovered )
				color = COLOR_HOVER;

			if ( entry.Fill )
				entry.Fill.SetColor( color );

			if ( entry.Outline )
				entry.Outline.Show( entry.Enabled && entry.Id == m_Selected );

			int glyphColor = ICON_INACTIVE;
			if ( !entry.Enabled )
				glyphColor = ARGB( 50, 255, 255, 255 );
			else if ( entry.Id == m_Selected )
				glyphColor = ICON_ACTIVE;

			if ( entry.IconWidget )
				entry.IconWidget.SetColor( glyphColor );

			if ( entry.LabelWidget )
				entry.LabelWidget.SetColor( glyphColor );
		}
	}

	protected void UpdateContentVisibility()
	{
		foreach ( UIActionTabPanel entry : m_Panels )
		{
			if ( entry.PanelWidget )
				entry.PanelWidget.Show( entry.TabId == m_Selected && IsTabVisible( entry.TabId ) );
		}
	}

	//! DEPRECATED - use AddTab( label, icon, panel ) or AddPanel( tabId, panel ). Attaches `panel` to
	//! the first tab that has none yet and returns that tab's id.
	int AddContent( Widget panel )
	{
		JMDeprecated.WarnOnce( this, "UIActionTabs.AddContent() is deprecated. Please use AddTab( label, icon, panel ) or AddPanel( tabId, panel )." );

		foreach ( UIActionTabEntry entry : m_Entries )
		{
			if ( !GetPanel( entry.Id ) )
			{
				AddPanel( entry.Id, panel );
				return entry.Id;
			}
		}

		return -1;
	}
}
