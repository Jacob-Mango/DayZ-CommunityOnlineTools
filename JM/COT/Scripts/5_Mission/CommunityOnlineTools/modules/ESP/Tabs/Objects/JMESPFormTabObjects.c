//! "Visible" (Objects) tab of JMESPForm - the tracked-object list, bulk
//! select/deselect/delete/move/export actions, and the row context menu.
//! Back-reference to the owning form, same shape as JMPlayerRowWidget.Menu.
class JMESPFormTabObjects: JMFormTab
{
	protected JMESPForm m_Form;

	//! The object list is sized to its CONTENT between these two bounds, not to
	//! whatever height is left over.
	static const int OBJECT_LIST_MIN_ROWS = 4;
	static const int OBJECT_LIST_MAX_ROWS = 12;

	//! Vertical space the rest of the tab needs, so a short window shrinks the
	//! list rather than hiding the card under it.
	static const int OBJECT_LIST_CHROME = 260;

	//! Square, and matching UIActionSearchBox's own authored height so the
	//! button reads as the same control as the box beside it. Fixed rather
	//! than a relative width, which made it as wide as the leftover space
	//! instead of a 1:1 button.
	static const float SEARCH_ROW_W = 0.93;
	static const float FILTER_MENU_W = 220;
	static const string PAGE_ROOT       = "root";
	static const string PAGE_CATEGORIES = "categories";
	static const string SUB_CATEGORIES  = "subcategories";
	static const string FILTER_SELECTED = "filterselected";
	protected UIActionScroller m_ESPSelectedObjects;
	protected UIActionItemList m_ObjectList;
	protected UIActionSearchBox m_ObjectSearch;
	protected UIActionImageButton m_FilterButton;

	//! Screen height the filter button was last squared to, so the measure
	//! only writes back when it has actually changed.
	protected float m_FilterButtonPx;
	protected ref UIActionFilterMenu m_FilterMenu;

	//! One bit per JMESPFormTabFilters GROUP_*, all set means nothing is
	//! filtered out. A mask rather than a set of hidden groups so "show
	//! everything" is the default a fresh tab starts in without having to
	//! populate anything.
	protected int m_CategoryMask = -1;
	protected bool m_SelectedOnly;
	protected UIActionCard m_ObjectsCard;
	protected UIActionImageButton m_ObjectListLockButton;

	//! Manual hold, set from the lock button. Independent of the automatic hold
	//! that focus applies, so releasing the mouse does not silently unpin a
	//! list the admin pinned on purpose.
	protected bool m_ObjectListPinned;

	//! What the lock button is currently drawing, so the icon is only swapped
	//! when the state actually changes rather than twice a second.
	protected bool m_ObjectListHeldShown;

	//! The same menu the world tags use, so a row in this list and the tag over
	//! the object it names offer the identical set of actions.
	protected ref JMESPActionMenu m_ObjectMenu;
	//! Parallel to the list's rows - the list itself only holds strings.
	protected ref array<JMESPMeta> m_ObjectRows = new array<JMESPMeta>;
	protected UIActionButton m_ExportButton;
	protected UIActionDropdown m_ExportTypeList;
	protected UIActionButton m_MoveToCursorButton;
	protected UIActionConfirmInline m_DeleteButton;

	//! Bulk selection. Feedback buttons because what they change is a list
	//! somewhere else on screen - see the note where they are built.
	protected UIActionFeedbackButton m_SelectAll;
	protected UIActionFeedbackButton m_DeselectAll;

	void JMESPFormTabObjects( JMESPForm form )
	{
		m_Form = form;
	}

	protected bool IsGroupShown( int group )
	{
		return ( m_CategoryMask & ( 1 << group ) ) != 0;
	}

	//! An object whose view type never resolved has no category to test,
	//! so it stays visible rather than disappearing the moment any
	//! category is switched off.
	protected bool IsMetaCategoryShown( JMESPMeta meta )
	{
		if ( !meta.type )
			return true;

		return IsGroupShown( JMESPFormTabFilters.GroupIndexFor( meta.type ) );
	}

	//! True while the list must not be rebuilt under the admin.
	//!
	//! A 500ms rebuild re-sorts and re-indexes every row, which makes the list
	//! impossible to work: the entry under the cursor is gone by the time it is
	//! clicked. Touching the list holds it automatically, and the lock button
	//! holds it indefinitely.
	bool IsObjectListHeld()
	{
		if ( m_ObjectListPinned )
			return true;

		return m_ObjectList && m_ObjectList.IsFocused();
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_ESPSelectedObjects = UIActionManager.CreateScroller( panel );
		Widget container = m_ESPSelectedObjects.GetContentWidget();

		//! The list and the things you do to a selection are two different
		//! jobs, so they get a card each rather than a loose search row over a
		//! bare list. The count lives in the card's own title, which is where
		//! the eye already is and costs no row of its own.
		//!
		//! Selection first, tracked objects under it: the actions are a fixed
		//! three rows while the list grows to whatever the scan returned, so
		//! putting the list first pushes the buttons off the bottom of the tab.
		UIActionCard card = UIActionManager.CreateCard( container, "#STR_COT_ESP_MODULE_SELECTION_HEADER" );
		Widget cardContent = card.GetContent();

		//! Three even two-up rows, so the card reads as a block of paired
		//! actions instead of a ragged mix of full and half width buttons.
		Widget rowSelectors = UIActionManager.CreateGridSpacer( cardContent, 1, 2 );
		m_SelectAll = UIActionManager.CreateFeedbackButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_SELECT_ALL", "#STR_COT_SELECTED", JMConstants.ICON_CHECK_MARK, this, "" );
		if ( m_SelectAll ) m_SelectAll.SetOnClick( this, "OnClick_SelectAll" );
		m_SelectAll.SetIcon( JMConstants.ICON_SELECT_ALL );
		m_SelectAll.SetTooltip( "#STR_COT_ESP_MODULE_TT_OBJECT_SELECT_ALL" );
		m_DeselectAll = UIActionManager.CreateFeedbackButton( rowSelectors, "#STR_COT_ESP_MODULE_ACTION_DESELECT_ALL", "#STR_COT_CLEARED", JMConstants.ICON_CHECK_MARK, this, "" );
		if ( m_DeselectAll ) m_DeselectAll.SetOnClick( this, "OnClick_DeselectAll" );
		m_DeselectAll.SetIcon( JMConstants.ICON_DESELECT_ALL );
		m_DeselectAll.SetTooltip( "#STR_COT_ESP_MODULE_TT_OBJECT_DESELECT_ALL" );

		Widget rowMisc = UIActionManager.CreateGridSpacer( cardContent, 1, 2 );
		m_MoveToCursorButton = UIActionManager.CreateButton( rowMisc, "#STR_COT_ESP_MODULE_ACTION_MOVE_TO_CURSOR", this, "" );
		if ( m_MoveToCursorButton ) m_MoveToCursorButton.SetOnClick( this, "OnClick_MoveToCursor" );
		m_MoveToCursorButton.SetIcon( JMConstants.Lucide( "mouse-pointer-click" ) );
		m_MoveToCursorButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_MOVE_TO_CURSOR" );
		m_Form.BindPermission( m_MoveToCursorButton, JMConstants.PERM_ESP_OBJECT_MOVETOCURSOR );

		m_DeleteButton = UIActionManager.CreateConfirmInline( rowMisc, "#STR_COT_GENERIC_DELETE", this, "OnClick_DeleteSelected" );
		UIActionIconGrid.ApplyDeletePreset( m_DeleteButton );
		m_DeleteButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_DELETE_SELECTED" );
		m_Form.BindPermission( m_DeleteButton, JMConstants.PERM_ESP_OBJECT_DELETEALL );

		//! Format first, then the button that acts on it - the row reads in the
		//! order it is used.
		Widget rowExports = UIActionManager.CreateGridSpacer( cardContent, 1, 2 );

		TStringArray exportChoices = {
			"#STR_COT_ESP_MODULE_EXPORT_RAW",
			"#STR_COT_ESP_MODULE_EXPORT_SPAWNABLETYPES",
			"#STR_COT_ESP_MODULE_EXPORT_TYPES",
		#ifdef DZ_Expansion_Core
			"#STR_COT_ESP_MODULE_EXPORT_MAP",
		#endif
		#ifdef DZ_Expansion_Market
			"#STR_COT_ESP_MODULE_EXPORT_EXP_MARKET",
		#endif
		#ifdef DZ_Expansion_Core
			"#STR_COT_ESP_MODULE_EXPORT_EXP_LOADOUT",
		#endif
			"#STR_COT_LOADOUT_MODULE_NAME"
		};
		m_ExportTypeList = UIActionManager.CreateDropdown( rowExports, "", panel, this, "OnClick_ExportType", exportChoices );
		m_Form.AddOverlay( m_ExportTypeList );

		//! Without this the dropdown opens blank, which reads as "no format
		//! chosen" even though Raw is what the button would actually do.
		m_ExportTypeList.SetSelection( COT_ESPMode.COPYLISTRAW, false );

		m_ExportButton = UIActionManager.CreateButton( rowExports, "#STR_COT_TO_CLIPBOARD", this, "" );
		if ( m_ExportButton ) m_ExportButton.SetOnClick( this, "OnClick_CopyToClipboard" );
		m_ExportButton.SetIcon( JMConstants.ICON_STACK );
		m_ExportButton.SetTooltip( "#STR_COT_ESP_MODULE_TT_EXPORT_SELECTED" );

		m_ObjectsCard = UIActionManager.CreateCard( container, "#STR_COT_ESP_MODULE_OBJECTS_CARD" );
		Widget objectsContent = m_ObjectsCard.GetContent();

		m_ObjectsCard.AddRefreshButton( this, "OnClick_RefreshObjects" );

		m_ObjectListLockButton = m_ObjectsCard.AddCardHeaderAction( JMConstants.Lucide( "lock-open" ), this, "", "#STR_COT_ESP_MODULE_LIST_LOCK" );
		if ( m_ObjectListLockButton ) m_ObjectListLockButton.SetOnClick( this, "OnClick_ToggleListLock" );

		//! Search box with its filter button on the right, the same shape
		//! JMTeleportForm uses - the filters narrow the same list the box
		//! does, so they belong on its row rather than as controls of their
		//! own, and a menu holds the categories without costing a row per
		//! category.
		JMSearchRow toolbar = UIActionManager.CreateSearchRow( objectsContent, "#STR_COT_ESP_MODULE_OBJECT_FILTER", this, "OnChange_ObjectFilter", "OnClick_FilterButton", "#STR_COT_ESP_MODULE_TT_OBJECT_FILTER_BUTTON", "", SEARCH_ROW_W );
		m_ObjectSearch = toolbar.Search;
		m_ObjectSearch.SetTooltip( "#STR_COT_ESP_MODULE_TT_OBJECT_FILTER" );
		m_FilterButton = toolbar.Filter;

		//! Virtualized: only the visible rows exist as widgets, so a scan that
		//! comes back with a few thousand objects costs a few thousand strings
		//! rather than a few thousand layouts.
		m_ObjectList = UIActionManager.CreateItemList( objectsContent, this, "OnClick_ObjectRow" );
		ApplyObjectListHeight();

		RefreshList();

		m_ESPSelectedObjects.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		ApplyObjectListHeight();

		if ( m_ESPSelectedObjects )
			m_ESPSelectedObjects.UpdateScroller();
	}

	//! Focused, or the form asked for a repaint (UpdateActiveTab): redraw the tracked list.
	override void OnUpdate()
	{
		RefreshList();
	}

	//! Built from the module's tracked list rather than from a walk of
	//! JMESPMeta.s_JM_All: the linked list also holds metas queued for creation
	//! and destruction, which are not on screen.
	void RefreshList( bool force = false )
	{
		if ( !m_ObjectList )
			return;

		RefreshListLockIndicator();
		SyncFilterButtonSize();

		if ( !force && IsObjectListHeld() )
		{
			//! Still re-derive the ticks while held. Selection is no longer
			//! something this list does itself - it happens in the row's
			//! context menu or on the object's world tag - so without this
			//! a held list (and any click holds it) would go on showing a
			//! stale selection until it was unlocked. Only flags on rows
			//! that already exist, so it does not rebuild anything.
			RefreshObjectListSelectionHighlight();
			return;
		}

		array< ref JMESPMeta > active = m_Form.m_Module.GetActiveObjects();

		string needle = "";
		if ( m_ObjectSearch )
			needle = m_ObjectSearch.GetText();
		needle.ToLower();

		m_ObjectRows.Clear();

		TStringArray names = new TStringArray;
		TStringArray subs  = new TStringArray;

		vector camera = g_Game.GetCurrentCameraPosition();

		foreach ( JMESPMeta meta : active )
		{
			if ( !meta || !meta.target )
				continue;

			if ( m_SelectedOnly && !( meta.widgetHandler && meta.widgetHandler.IsSelected() ) )
				continue;

			if ( !IsMetaCategoryShown( meta ) )
				continue;

			string label = meta.GetName();
			string classname = meta.GetType();

			if ( needle != "" )
			{
				string labelLower = label;
				labelLower.ToLower();

				string classLower = classname;
				classLower.ToLower();

				if ( labelLower.IndexOf( needle ) < 0 && classLower.IndexOf( needle ) < 0 )
					continue;
			}

			int distance = Math.Round( vector.Distance( camera, meta.target.GetPosition() ) );

			string category = classname;
			if ( meta.type )
				category = meta.type.Localisation;

			names.Insert( label );
			subs.Insert( string.Format( "%1  -  %2m", category, distance ) );

			m_ObjectRows.Insert( meta );
		}

		m_ObjectList.SetItems( names, subs );

		ApplyObjectListHeight();

		RefreshObjectCardTitle( names.Count() );

		RefreshObjectListSelectionHighlight();
	}

	//! Re-derives the list's per-row highlight from each row's underlying
	//! selection state. Needed after every SetItems (which wipes the list's
	//! own highlight array) and after any click that toggles a row.
	protected void RefreshObjectListSelectionHighlight()
	{
		if ( !m_ObjectList )
			return;

		for ( int i = 0; i < m_ObjectRows.Count(); i++ )
		{
			JMESPMeta meta = m_ObjectRows[i];

			bool selected = meta && meta.widgetHandler && meta.widgetHandler.IsSelected();

			m_ObjectList.SetHighlighted( i, selected );
		}
	}

	//! Card title carries the count, and says so when the list is being held.
	protected void RefreshObjectCardTitle( int count )
	{
		if ( !m_ObjectsCard )
			return;

		string title = string.Format( "%1 (%2)", "#STR_COT_ESP_MODULE_OBJECTS_CARD", count );

		if ( m_ObjectListHeldShown )
			title = string.Format( "%1  -  %2", title, "#STR_COT_ESP_MODULE_LIST_HELD" );

		m_ObjectsCard.SetLabel( title );
	}

	void OnClick_RefreshObjects( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		//! An explicit refresh beats the hold - that is what it is for.
		RefreshList( true );
	}

	void OnClick_ToggleListLock( UIActionBase action )
	{
		m_ObjectListPinned = !m_ObjectListPinned;

		RefreshListLockIndicator();

		//! Unpinning is a request to see current data now, not in half a
		//! second.
		if ( !m_ObjectListPinned )
			RefreshList( true );
	}

	//! Icon and title marker follow the EFFECTIVE hold, so a list held by the
	//! cursor says so just as plainly as one held by the button.
	protected void RefreshListLockIndicator()
	{
		bool held = IsObjectListHeld();

		if ( held == m_ObjectListHeldShown )
			return;

		m_ObjectListHeldShown = held;

		if ( !m_ObjectListLockButton )
			return;

		if ( held )
		{
			m_ObjectListLockButton.SetImage( JMConstants.Lucide( "lock" ) );
			m_ObjectListLockButton.SetColor( JMTheme.WARNING );
		}
		else
		{
			m_ObjectListLockButton.SetImage( JMConstants.Lucide( "lock-open" ) );
			m_ObjectListLockButton.SetColor( JMTheme.TEXT_SECONDARY );
		}

		RefreshObjectCardTitle( m_ObjectRows.Count() );
	}

	//! Height of the object list: as many rows as it actually holds, clamped to
	//! [MIN_ROWS, MAX_ROWS] and then to whatever the window can spare. An empty
	//! list is four rows tall, not half the form.
	//! Squares the filter button against the search box's RENDERED height.
	//!
	//! Not SetFixedSize with the box's authored 30: that writes widget-local
	//! units, which the workspace then scales by the player's UI scale, so
	//! the button came out a different size on screen than the box it sits
	//! beside. Screen pixels are the one space where the number that sizes
	//! one widget means the same for the other (the same rule as
	//! JMPlayerForm.MeasureInventoryCell). Re-checked rather than set once,
	//! because at Build time nothing has been measured yet.
	protected void SyncFilterButtonSize()
	{
		if ( !m_FilterButton || !m_ObjectSearch )
			return;

		float sw, sh;
		m_ObjectSearch.GetLayoutRoot().GetScreenSize( sw, sh );

		if ( sh <= 0 || Math.AbsFloat( sh - m_FilterButtonPx ) < 1 )
			return;

		m_FilterButtonPx = sh;

		Widget btn = m_FilterButton.GetLayoutRoot();
		btn.SetFlags( WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE, true );
		btn.SetScreenSize( sh, sh );
	}

	protected void ApplyObjectListHeight()
	{
		SyncFilterButtonSize();

		if ( !m_ObjectList )
			return;

		int rows = Math.Clamp( m_ObjectList.GetItemCount(), OBJECT_LIST_MIN_ROWS, OBJECT_LIST_MAX_ROWS );

		float listHeight = rows * UIActionItemList.ROW_H;

		float tabStripHeight = m_Form.GetPinnedStripHeight( m_Form.GetBottomTabStrip() );
		float available = m_Form.m_ContentHeight - JMESPForm.TOOLBAR_HEIGHT - tabStripHeight - OBJECT_LIST_CHROME;
		float floorPx   = OBJECT_LIST_MIN_ROWS * UIActionItemList.ROW_H;

		if ( available < floorPx )
			available = floorPx;

		if ( listHeight > available )
			listHeight = available;

		m_ObjectList.SetViewportHeight( listHeight );
	}

	void OnChange_ObjectFilter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		RefreshList( true );
	}

	protected bool AreAllGroupsShown()
	{
		for ( int g = 0; g < JMESPFormTabFilters.GROUP_COUNT; g++ )
		{
			if ( !IsGroupShown( g ) )
				return false;
		}

		return true;
	}

	//! What a context menu's panel hangs off. The ESP container is created
	//! at the workspace root rather than inside a form, which is why the
	//! world tags' own menu anchors to it (JMESPActionMenu) - a menu given
	//! the window root instead is a child of the window and gets cut off at
	//! its edge, which is what was slicing the labels off this one. Falls
	//! back to the window when ESP is not up, where a clipped menu still
	//! beats no menu.
	protected Widget FloatingMenuAnchor()
	{
		if ( JMStatics.ESP_CONTAINER )
			return JMStatics.ESP_CONTAINER;

		CF_Window window = m_Form.GetWindow();

		if ( !window )
			return null;

		return window.GetWidgetRoot();
	}

	//! Opens under the filter button, same as JMTeleportForm's and the
	//! object spawner's - all three now share UIActionFilterMenu (see its
	//! class header) instead of each hand-rolling a menu-plus-submenu pair,
	//! which is what used to leave the drill-down page dead to clicks and
	//! hover until the whole COT window was closed and reopened.
	void OnClick_FilterButton( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_FilterButton )
			return;

		if ( !m_FilterMenu )
		{
			m_FilterMenu = UIActionManager.CreateOverlayFilterMenu( m_Form, m_FilterButton.GetLayoutRoot(), JMFilterRegistry.ESP, FloatingMenuAnchor() );

			if ( !m_FilterMenu )
				return;

			//! Pinned rather than measured. The automatic width is what
			//! clipped the labels to "Liv"/"Ve", and the menu's own
			//! off-screen flip is decided from that same width - too
			//! narrow a measurement and it does not realise it needs to
			//! flip, so it ran off the edge instead.
			m_FilterMenu.SetMenuWidth( FILTER_MENU_W );

			m_FilterMenu.AddPage( PAGE_ROOT, this, "BuildFilterRootPage", "", false, "", this, "OnFilterRootChange" );
			m_FilterMenu.AddPage( PAGE_CATEGORIES, this, "BuildFilterCategoriesPage", PAGE_ROOT, true, "#STR_COT_TELEPORT_MODULE_TOGGLE_ALL", this, "OnFilterCategoriesChange" );
		}

		m_FilterMenu.ToggleAt( m_FilterButton.GetLayoutRoot() );
	}

	void BuildFilterRootPage( UIActionFilterMenu menu )
	{
		bool allShown = AreAllGroupsShown();

		menu.AddRow( SUB_CATEGORIES, "#STR_COT_ESP_MODULE_MENU_CATEGORIES", JMConstants.Lucide( "shapes" ), UIActionFilterMenu.ToggleTextColor( allShown ), true, PAGE_CATEGORIES );
		menu.AddRow( FILTER_SELECTED, "#STR_COT_ESP_MODULE_SELECTED_ONLY", UIActionFilterMenu.CheckIcon( m_SelectedOnly ), UIActionFilterMenu.ToggleTextColor( m_SelectedOnly ) );
	}

	void OnFilterRootChange( string id )
	{
		if ( id != FILTER_SELECTED )
			return;

		m_SelectedOnly = !m_SelectedOnly;
		RefreshList( true );
	}

	//! Each row carries its own category's icon rather than a checkbox - the
	//! same icon that category wears on its group header in the filters
	//! tab, so the two lists name the same thing the same way. On/off is
	//! left to the label colour, which was already saying it.
	void BuildFilterCategoriesPage( UIActionFilterMenu menu )
	{
		TStringArray groupLabels = JMESPFormTabFilters.GroupLabels();
		TStringArray groupIcons = JMESPFormTabFilters.GroupIcons();

		for ( int g = 0; g < JMESPFormTabFilters.GROUP_COUNT; g++ )
		{
			bool shown = IsGroupShown( g );
			menu.AddRow( "" + g, groupLabels[g], groupIcons[g], UIActionFilterMenu.ToggleTextColor( shown ) );
		}
	}

	void OnFilterCategoriesChange( string id )
	{
		if ( id == UIActionFilterMenu.ID_TOGGLE_ALL )
		{
			//! All on goes to all off, anything else goes to all on - the
			//! same "one click gets you to a clean state either way" the
			//! teleport filter's toggle-all has.
			if ( AreAllGroupsShown() )
				m_CategoryMask = 0;
			else
				m_CategoryMask = -1;
		}
		else
		{
			m_CategoryMask = m_CategoryMask ^ ( 1 << id.ToInt() );
		}

		RefreshList( true );
	}

	void OnClick_ObjectRow( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.DOUBLE_CLICK )
		{
			OnDoubleClick_ObjectRow();
			return;
		}

		if ( eid != UIEvent.CLICK )
			return;

		int idx = m_ObjectList.GetSelectedIndex();

		if ( idx < 0 || idx >= m_ObjectRows.Count() )
			return;

		JMESPMeta meta = m_ObjectRows[idx];

		if ( !meta || !meta.widgetHandler )
			return;

		//! Any click on a real row is a deliberate interaction with the list -
		//! lock it the same way the button does, so the 1500ms refresh can't
		//! yank the row out from under a context menu or a second click.
		m_ObjectListPinned = true;
		RefreshListLockIndicator();

		if ( m_ObjectList.GetLastClickButton() == MouseState.RIGHT )
			ShowObjectMenu( meta );

		//! A SINGLE left click deliberately does NOT toggle selection. Rows
		//! shift under the cursor as the list refreshes and scrolls, so a
		//! plain click - or one that lands while scrolling - was selecting
		//! whatever happened to be under the pointer. Selecting is an
		//! explicit act: a double click below, the right click menu here,
		//! or clicking the object's own world tag.
	}

	//! Deliberate enough to mean selection, where a single click is not.
	//! The list raises CLICK before DOUBLE_CLICK, so the row is already
	//! current and the list already held by the time this runs.
	protected void OnDoubleClick_ObjectRow()
	{
		int idx = m_ObjectList.GetSelectedIndex();

		if ( idx < 0 || idx >= m_ObjectRows.Count() )
			return;

		JMESPMeta meta = m_ObjectRows[idx];

		if ( !meta || !meta.widgetHandler )
			return;

		if ( meta.widgetHandler.IsSelected() )
			meta.widgetHandler.Deselect();
		else
			meta.widgetHandler.Select();

		//! Deselecting under "selected only" has to drop the row, not just
		//! untick it, so rebuild rather than only repainting the ticks.
		if ( m_SelectedOnly )
			RefreshList( true );
		else
			RefreshObjectListSelectionHighlight();
	}

	protected void ShowObjectMenu( JMESPMeta meta )
	{
		if ( !m_ObjectMenu )
		{
			CF_Window window = m_Form.GetWindow();
			if ( !window )
				return;

			m_ObjectMenu = new JMESPActionMenu();

			//! Anchored to the window root, not to the list: a menu parented to
			//! the list would be clipped by it on the bottom rows.
			UIActionContextMenu menu = m_ObjectMenu.Init( m_Form.GetLayoutRoot(), window.GetWidgetRoot() );

			if ( !menu )
			{
				m_ObjectMenu = NULL;
				return;
			}

			m_Form.AddOverlay( menu );
		}

		int mx;
		int my;
		GetMousePos( mx, my );

		m_ObjectMenu.Open( meta, mx, my );
	}

	void OnClick_ExportType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( m_ExportTypeList.GetSelection() == COT_ESPMode.CREATELOADOUT )
			m_ExportButton.SetButton( "#STR_COT_SAVE_AS" );
		else
			m_ExportButton.SetButton( "#STR_COT_TO_CLIPBOARD" );
	}

	void OnClick_DeleteSelected( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_ESP_OBJECT_DELETEALL ) )
			return;

		m_Form.CreateAdvancedObjectConfirm( "DeleteSelected", "DeleteSelected" );
	}

	void OnClick_SelectAll( UIActionBase action )
	{
		auto node = JMESPMeta.s_JM_All.m_Head;
		while ( node )
		{
			if ( node.m_Value.widgetRoot.IsVisible() )
				node.m_Value.widgetHandler.Select();

			node = node.m_Next;
		}
	}

	void OnClick_DeselectAll( UIActionBase action )
	{
		auto node = JMESPMeta.s_JM_All.m_Head;
		while ( node )
		{
			node.m_Value.widgetHandler.Deselect();
			node = node.m_Next;
		}

		JM_GetSelected().ClearObjects();
	}

	void OnClick_MoveToCursor( UIActionBase action )
	{
		if ( !JMPermissions.Has( JMConstants.PERM_ESP_OBJECT_MOVETOCURSOR ) )
			return;

		m_Form.CreateAdvancedObjectConfirm( "MoveToCursor", "MoveToCursor" );
	}

	void OnClick_CopyToClipboard( UIActionBase action )
	{
		switch ( m_ExportTypeList.GetSelection() )
		{
			case COT_ESPMode.COPYLISTRAW:
				m_Form.m_Module.CopyToClipboardRaw();
			break;
			case COT_ESPMode.COPYLISTSPAWNABLETYPES:
				m_Form.m_Module.CopyToClipboardSpawnableTypes();
			break;
			case COT_ESPMode.COPYLISTTYPES:
				m_Form.m_Module.CopyToClipboardTypes();
			break;
		#ifdef DZ_Expansion_Core
			case COT_ESPMode.COPYLISTMAP:
				m_Form.m_Module.CopyToClipboardMap();
			break;
		#endif
		#ifdef DZ_Expansion_Market
			case COT_ESPMode.COPYLISTEXPMARKET:
				m_Form.m_Module.CopyToClipboardMarket();
			break;
		#endif
		#ifdef DZ_Expansion_Core
			case COT_ESPMode.COPYLISTEXPLOADOUT:
				m_Form.m_Module.CopyToClipboardExpLoadout( JMSelectedObject );
			break;
		#endif
			case COT_ESPMode.CREATELOADOUT:
				m_Form.PromptInput( "#STR_COT_ESP_MODULE_LOADOUT_MESSAGE_HEADER", "#STR_COT_ESP_MODULE_LOADOUT_MESSAGE_BODY", "CreateLoadout_Confirm", "#STR_COT_GENERIC_CREATE" );
			break;
		}
	}
}
