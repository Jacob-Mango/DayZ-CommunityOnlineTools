//! #define scope in Enforce is per-file, NOT per compiled module - see COTModule.c.
#define COT_DEBUGLOGS

class JMObjectSpawnerForm: JMFormBase
{
	//! Category chips for the few most recently used categories. The full set
	//! lives behind the filter button.
	//!
	//! Plain buttons in a wrap row, each a FRACTION of the row: the icon grid
	//! this replaced had to be told a pixel width that stayed in step with the
	//! chip count, and it rebuilt itself from inside its own click handler.
	protected ref JMItemCategoryPicker m_Categories;

	//! Categories moved out of a dropdown of their own and behind a filter
	//! button beside the search box. The dropdown cost a full row of the left
	//! column permanently for a control that is touched once per search.
	protected UIActionImageButton m_FilterButton;

	//! Class name for every row currently in the list.
	//!
	//! The visible text is NOT the class name once display names are switched
	//! on, so a row index resolves through this - which is what spawning, and
	//! every clipboard export, needs.
	protected ref TStringArray m_ListClasses;
	protected Widget m_SearchWrapper;
	protected Widget m_FilterWrapper;
	protected Widget m_RecentWrapper;
	protected Widget m_ListWrapper;
	protected UIActionScroller m_RightScroller;

	//! The property card - per-class rows and the values the admin picked.
	protected ref JMObjectSpawnerProperties m_Props;
	protected UIActionSearchBox m_SearchBox;

	//! Cycle selectors, not checkboxes: both of these choose between two named
	//! states rather than switching one on, and a checkbox labelled "display
	//! name" never says what the other state is.
	protected UIActionSelectBox m_NameModeSelect;
	protected UIActionSelectBox m_UnsafeSelect;
	protected UIActionDropdown m_SpawnMode;
	protected UIActionDropdown m_ObjSetupMode;

	//! Copying a list is not spawning anything, so it is no longer buried among
	//! the spawn destinations. Its own card, its own dropdown, its own button.
	protected UIActionDropdown m_ExportMode;

	//! COT_ObjectSpawnerMode for each entry of the two dropdowns, in the order
	//! they list them. The old single dropdown relied on its index BEING the
	//! enum value; split in two, neither can.
	protected ref TIntArray m_SpawnModeIds;
	protected ref TIntArray m_ExportModeIds;
	protected UIActionItemList m_ClassList;
	protected ItemPreviewWidget m_ItemPreview;

	//! Row context menu (right-click a class list row) - shares the same
	//! Filter/Object/Categories actions the row's own buttons offer.
	protected ref JMObjectSpawnerRowMenu m_RowMenu;

	//! The preview's wrapper panel. The preview itself is no longer the layout
	//! root: it now hangs inside a plainly-styled panel, the way vanilla's own
	//! previews do, so it never inherits whatever style the engine has
	//! registered as the default for a styleless widget - which is exactly what
	//! COT's own style table changes. Geometry is applied to the WRAPPER; the
	//! preview fills it fractionally.
	protected Widget m_ItemPreviewPanel;
	protected EntityAI m_PreviewItem;

	//! Setup mode the current preview entity was built with. The attachments a
	//! spawn produces depend on it, so a mode change has to rebuild the preview
	//! exactly like a class change does - an entity that already has its debug
	//! attachments cannot be walked back to a bare one.
	protected int m_PreviewSetupMode;
	protected vector m_Orientation;
	protected float m_Distance;
	protected int m_MouseX;
	protected int m_MouseY;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMObjectSpawnerModule m_Module;
	protected UIActionButton m_SpawnButton;
	protected UIActionConfirmInline m_DeleteCursorBtn;

	//! Object targeted by the quick-delete keybind while m_DeleteCursorBtn is
	//! armed. A second keybind press on the same target confirms; aiming at
	//! something else re-arms on the new target instead of deleting the old one.
	protected Object m_PendingDeleteCursorObj;
	protected static int s_ObjSpawnMode   = COT_ObjectSpawnerMode.CURSOR;
	protected static int s_ObjExportMode  = COT_ObjectSpawnerMode.COPYLISTRAW;

	//! How many recent categories are kept. Small on purpose - this is a
	//! shortcut back to what was just used, not a second category list.

	//! Height of the preview inside its card.
	static const int PREVIEW_H = 240;

	//! The left column's bands, top to bottom, in layout pixels.
	//!
	//! Every one of them is pinned from OnResize rather than left to the layout
	//! file. Two sources for the same four numbers is how the chip strip ended
	//! up overlapping the class list: the file said one thing, the single
	//! computed pin said another, and only the list moved.
	static const int SEARCH_H = 34;
	static const int FILTER_H = 36;
	static const int RECENT_H = 36;

	//! Row splits, as FRACTIONS of the row. Never a measured flex pass and
	//! never a fraction mixed with fixed pixels - see OnCreate.
	//!
	//! A fixed-pixel button beside a fractional field wraps the moment the row
	//! is narrower than the two of them together, and this window can be
	//! dragged down to RESIZE_MIN_WIDTH. Two fractions summing under 1 cannot.
	static const float MODE_ROW_W   = 0.5;
	static const float MODE_ROW_W2  = 0.49;
	static const float SEARCH_ROW_W = 0.86;
	static const float FILTER_BTN_W = 0.13;

	void JMObjectSpawnerForm()
	{
		m_PreviewSetupMode = -1;

		m_ListClasses    = new TStringArray;
		m_SpawnModeIds   = new TIntArray;
		m_ExportModeIds  = new TIntArray;

		m_Props   = new JMObjectSpawnerProperties( this );
		m_RowMenu = new JMObjectSpawnerRowMenu( this );
	}

	void ~JMObjectSpawnerForm()
	{
		if (m_PreviewItem)
			g_Game.ObjectDelete(m_PreviewItem);
	}

	string GetCurrentSelection()
	{
		int row = m_ClassList.GetSelectedIndex();

		if ( row < 0 || row >= m_ListClasses.Count() )
			return "";

		return m_ListClasses[row];
	}

	//! Sum of the three above. A function rather than a constant because a static const folded
	//! from other static consts evaluates to 0 in Enforce - and writing the total out by hand
	//! left it free to drift from the bands it adds up.
	static int GetLeftHeaderHeight()
	{
		return SEARCH_H + FILTER_H + RECENT_H;
	}

	//! The module this form drives. Public for the property card.
	JMObjectSpawnerModule GetModule()
	{
		return m_Module;
	}

	//! The entity the preview is showing, for the property card to push values onto.
	EntityAI GetPreviewItem()
	{
		return m_PreviewItem;
	}

	UIActionScroller GetRightScroller()
	{
		return m_RightScroller;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		//! The ONE thing that has to be computed: how far down the left column
		//! the class list reaches. The three bands above it are fixed height
		//! and the right column is a scroller that sizes itself, so nothing
		//! else here depends on a measurement.
		//!
		//! h is real content-height layout pixels handed over by JMWindowBase -
		//! not a widget's own GetScreenSize, which reads zero before the first
		//! render and is what made the old flex footer unreliable.
		//! All four bands, from one place. Only the last of them has anything to
		//! compute - the three above it are fixed - but they are pinned here too
		//! so the layout file cannot quietly disagree with this about where the
		//! list starts. When it did, the chip strip ran on underneath it.
		PinBand( m_SearchWrapper, 0, SEARCH_H );
		PinBand( m_FilterWrapper, SEARCH_H, FILTER_H );
		PinBand( m_RecentWrapper, SEARCH_H + FILTER_H, RECENT_H );

		if ( m_ListWrapper && h > GetLeftHeaderHeight() )
		{
			float listH = h - GetLeftHeaderHeight();

			PinBand( m_ListWrapper, GetLeftHeaderHeight(), listH );

			//! The list is told the height rather than measuring it. Same
			//! number, one source - and a widget cannot measure itself on the
			//! frame it is built.
			if ( m_ClassList )
				m_ClassList.SetViewportHeight( listH );
		}

		if ( m_RightScroller )
			m_RightScroller.UpdateScroller();
	}

	//! Place one band of the left column: exact y, exact height, full width.
	protected void PinBand( Widget band, float y, float height )
	{
		if ( !band )
			return;

		band.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		band.SetPos( 0, y );
		band.SetSize( 1, height );
	}

	override void OnShow()
	{
		super.OnShow();

		if ( m_ItemPreview )
			m_ItemPreview.Show( true );

		//! The deferred re-layout this used to schedule a tick after OnShow is
		//! gone with the flex row it existed for. Nothing in the form now needs
		//! to have been rendered once before it can lay itself out.
		if ( m_RightScroller )
			m_RightScroller.UpdateScroller();
	}

	override void OnHide()
	{
		super.OnHide();
		if ( m_ItemPreview )
			m_ItemPreview.Show( false );
	}

	override void OnCreate()
	{
		// ----------------------------------------------------------------------
		// Two columns, modelled on player_form.layout.
		//
		//   panel_left  - search, filters, recent chips, class list
		//   panel_right - preview, properties and spawn, as cards in a scroller
		//
		// The previous layout stacked four bands and computed all of their
		// geometry by hand on every resize, re-pinning the list and the preview
		// to measured pixel halves. Only ONE thing is computed now: how far the
		// class list reaches down the left column. Everything else is either a
		// fraction or sized by its own content.
		// ----------------------------------------------------------------------

		m_LeftPanel         = layoutRoot.FindAnyWidget( "panel_left" );
		m_RightPanel        = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );

		m_SearchWrapper = layoutRoot.FindAnyWidget( "spawner_search_wrapper" );
		m_FilterWrapper = layoutRoot.FindAnyWidget( "spawner_filter_wrapper" );
		m_RecentWrapper = layoutRoot.FindAnyWidget( "spawner_recent_wrapper" );
		m_ListWrapper   = layoutRoot.FindAnyWidget( "spawner_list_wrapper" );

		// --- Search -----------------------------------------------------------
		JMSearchRow toolbar = UIActionManager.CreateSearchRow( m_SearchWrapper, "#STR_COT_OBJECT_MODULE_SEARCH", this, "SearchInput_OnChange", "OnClick_CategoryFilter", "#STR_COT_OBJECT_MODULE_CATEGORY_DESC", "", SEARCH_ROW_W, FILTER_BTN_W );
		m_SearchBox = toolbar.Search;
		m_FilterButton = toolbar.Filter;

		// --- Filters ----------------------------------------------------------
		Widget filterRow = UIActionManager.CreateWrapSpacerCompact( m_FilterWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		//! Entry text is printed verbatim by the selector, so the keys are
		//! resolved here rather than being handed over as keys.
		array<string> nameModes = new array<string>;
		nameModes.Insert( "#STR_COT_OBJECT_MODULE_NAMEMODE_CLASS" );
		nameModes.Insert( "#STR_COT_OBJECT_MODULE_NAMEMODE_DISPLAY" );

		m_NameModeSelect = UIActionManager.CreateSelectionBox( filterRow, "", nameModes, this, "Click_OnFilterDisplayName" );
		m_NameModeSelect.SetWidth( MODE_ROW_W );
		m_NameModeSelect.SetSelectorWidth( 1.0 );
		m_NameModeSelect.SetTooltip( "#STR_COT_OBJECT_MODULE_SPAWN_DISPLAYNAME_DESC" );

		if ( m_Module.m_FilterWithDisplayName )
			m_NameModeSelect.SetSelection( 1, false );

		array<string> unsafeModes = new array<string>;
		unsafeModes.Insert( "#STR_COT_OBJECT_MODULE_UNSAFE_HIDE" );
		unsafeModes.Insert( "#STR_COT_OBJECT_MODULE_UNSAFE_SHOW" );

		m_UnsafeSelect = UIActionManager.CreateSelectionBox( filterRow, "", unsafeModes, this, "Click_OnSafetyToogle" );
		m_UnsafeSelect.SetWidth( MODE_ROW_W2 );
		m_UnsafeSelect.SetSelectorWidth( 1.0 );
		m_UnsafeSelect.SetTooltip( "#STR_COT_OBJECT_MODULE_SHOWUNSAFE_DESC" );

		if ( m_Module.m_AllowRestrictedClassNames )
			m_UnsafeSelect.SetSelection( 1, false );

		// --- Class list -------------------------------------------------------
		//! COT's own list, not TextListboxWidget. That widget draws in the
		//! engine's debug style and builds one entry per item - roughly 14000
		//! widgets on every keystroke here. This one keeps a pool of the rows
		//! that fit and moves the data through it.
		m_ClassList = UIActionManager.CreateItemList( m_ListWrapper, this, "OnClick_ClassList" );
		m_ClassList.SetEmptyText( "#STR_COT_OBJECT_MODULE_NO_RESULTS" );

		// --- Right column -----------------------------------------------------
		m_RightScroller = UIActionManager.CreateScroller( m_RightPanel );
		Widget rightContent = m_RightScroller.GetContentWidget();

		UIActionCard previewCard = UIActionManager.CreateCard( rightContent, "#STR_COT_OBJECT_MODULE_PREVIEW" );

		//! A fixed-height panel to hang the preview in. The preview layout is
		//! fractional, and a fractional height inside a size-to-content card
		//! resolves against the whole column.
		Widget previewHost = UIActionManager.CreateRow( previewCard.GetContent(), PREVIEW_H );

		m_ItemPreviewPanel = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/objectspawner_preview.layout", previewHost );

		if ( m_ItemPreviewPanel )
			Class.CastTo( m_ItemPreview, m_ItemPreviewPanel.FindAnyWidget( "object_preview" ) );

		//! ItemPreviewWidget is an RTT surface - it renders nothing outside the
		//! narrow sort band the Preview Lab's live sweep found (2026-09-24; see
		//! JMUILayout.SORT_PREVIEW's note). The earlier read - that it only
		//! renders inside a UIScriptedMenu - was a red herring from only ever
		//! having tried sort 0 or 32000, never this band; a plain window child
		//! at the right sort works fine, no render-layer menu needed.
		if ( m_ItemPreview )
			m_ItemPreview.SetSort( JMUILayout.SORT_PREVIEW );

		m_Props.Build( rightContent );

		// --- Spawn ------------------------------------------------------------
		array<string> spawnModes = new array<string>;
		m_SpawnModeIds.Clear();

		AddMode( m_SpawnModeIds, spawnModes, COT_ObjectSpawnerMode.CURSOR,           "#STR_COT_OBJECT_MODULE_CROSSHAIR" );
		AddMode( m_SpawnModeIds, spawnModes, COT_ObjectSpawnerMode.PLAYER_POSITION,  "#STR_COT_OBJECT_MODULE_SELF" );
		AddMode( m_SpawnModeIds, spawnModes, COT_ObjectSpawnerMode.TARGET_INVENTORY, "#STR_COT_OBJECT_MODULE_TARGET" );

		if ( !g_Game.IsMultiplayer() )
			AddMode( m_SpawnModeIds, spawnModes, COT_ObjectSpawnerMode.PLAYER_INVENTORY, "#STR_COT_OBJECT_MODULE_INVENTORY" );
		else
			AddMode( m_SpawnModeIds, spawnModes, COT_ObjectSpawnerMode.PLAYER_INVENTORY, "#STR_COT_OBJECT_MODULE_SELECTED_PLAYERS" );

		AddMode( m_SpawnModeIds, spawnModes, COT_ObjectSpawnerMode.OBJECT_INVENTORY, "#STR_COT_OBJECT_MODULE_SELECTED_OBJECTS" );

		UIActionCard spawnCard = UIActionManager.CreateCard( rightContent, "#STR_COT_OBJECT_MODULE_SPAWN" );
		Widget spawnBody = UIActionManager.CreateGridSpacer( spawnCard.GetContent(), 2, 1 );

		//! Every width in this card is a FRACTION of its row, never a measured
		//! flex split. UIActionFlexRow can only divide a row it has already
		//! measured, and it cannot measure one that has not been drawn - which
		//! is the state every row is in on the frame it is built. This form used
		//! to work around that with a CallLater one tick after OnShow; a
		//! fraction is right on the first frame and needs no workaround.
		Widget modeRow = UIActionManager.CreateWrapSpacerCompact( spawnBody, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_SpawnMode = UIActionManager.CreateDropdown( modeRow, "", layoutRoot, this, "ChangeSpawnMode", spawnModes );
		m_SpawnMode.SetWidth( MODE_ROW_W );
		m_SpawnMode.SetSelection( IndexOfMode( m_SpawnModeIds, s_ObjSpawnMode ), false );
		m_SpawnMode.SetTooltip( "#STR_COT_OBJECT_MODULE_MODE_DESC" );
		AddOverlay( m_SpawnMode );

		array<string> setupOptions = new array<string>;
		setupOptions.Insert( "#STR_COT_OBJECT_MODULE_SPAWN_WITH_DEBUG" );
		setupOptions.Insert( "#STR_COT_OBJECT_MODULE_SPAWN_WITH_CE" );
		setupOptions.Insert( "#STR_COT_GENERIC_NONE" );
		m_ObjSetupMode = UIActionManager.CreateDropdown( modeRow, "", layoutRoot, this, "Click_ObjSetupMode", setupOptions );
		m_ObjSetupMode.SetWidth( MODE_ROW_W2 );
		m_ObjSetupMode.SetSelection( m_Module.m_ObjSetupMode, false );
		m_ObjSetupMode.SetTooltip( "#STR_COT_OBJECT_MODULE_SETUP_DESC" );
		AddOverlay( m_ObjSetupMode );

		//! Spawn and delete share a row and a width. Both layouts are 30 tall,
		//! and neither carries a pixel width, so the pair scales together and
		//! the row cannot wrap however narrow the window gets.
		Widget spawnRow = UIActionManager.CreateWrapSpacerCompact( spawnBody, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_SpawnButton = UIActionManager.CreateButton( spawnRow, "#STR_COT_OBJECT_MODULE_SPAWN_ON", this, "" );
		if ( m_SpawnButton ) m_SpawnButton.SetOnClick( this, "Click_SpawnObject" );
		m_SpawnButton.SetWidth( MODE_ROW_W );
		m_SpawnButton.SetColor( JMTheme.SUCCESS_FILL );
		m_SpawnButton.SetTooltip( "#STR_COT_OBJECT_MODULE_SPAWN_DESC" );

		m_DeleteCursorBtn = UIActionManager.CreateConfirmInline( spawnRow, "#STR_COT_OBJECT_MODULE_DELETE_CURSOR", this, "DeleteCursor" );
		UIActionIconGrid.ApplyDeletePreset( m_DeleteCursorBtn );
		m_DeleteCursorBtn.SetWidth( MODE_ROW_W2 );
		m_DeleteCursorBtn.SetTooltip( "#STR_COT_OBJECT_MODULE_DELETE_DESC" );

		BindPermission( m_DeleteCursorBtn, JMConstants.PERM_ENTITY_DELETE );

		// --- Export -----------------------------------------------------------
		//! Its own card under Spawn. Copying a class list to the clipboard is
		//! not a kind of spawning, and while it lived in the spawn dropdown the
		//! Spawn button had to rename itself and disable the Setup control to
		//! stop meaning what it said.
		array<string> exportModes = new array<string>;
		m_ExportModeIds.Clear();

		AddMode( m_ExportModeIds, exportModes, COT_ObjectSpawnerMode.COPYLISTRAW,   "#STR_COT_OBJECT_MODULE_EXPORT_RAW" );
		AddMode( m_ExportModeIds, exportModes, COT_ObjectSpawnerMode.COPYLISTTYPES, "#STR_COT_OBJECT_MODULE_EXPORT_TYPES" );

	#ifdef DZ_Expansion_Market
		AddMode( m_ExportModeIds, exportModes, COT_ObjectSpawnerMode.COPYLISTEXPMARKET, "#STR_COT_OBJECT_MODULE_EXPORT_MARKET" );
	#endif

		UIActionCard exportCard = UIActionManager.CreateCard( rightContent, "#STR_COT_OBJECT_MODULE_EXPORT" );
		Widget exportRow = UIActionManager.CreateWrapSpacerCompact( exportCard.GetContent(), WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_ExportMode = UIActionManager.CreateDropdown( exportRow, "", layoutRoot, this, "ChangeExportMode", exportModes );
		m_ExportMode.SetWidth( MODE_ROW_W );
		m_ExportMode.SetSelection( IndexOfMode( m_ExportModeIds, s_ObjExportMode ), false );
		m_ExportMode.SetTooltip( "#STR_COT_OBJECT_MODULE_EXPORT_DESC" );
		AddOverlay( m_ExportMode );

		UIActionButton copyBtn = UIActionManager.CreateButton( exportRow, "#STR_COT_GENERIC_COPY", this, "" );
		if ( copyBtn ) copyBtn.SetOnClick( this, "Click_CopyList" );
		copyBtn.SetWidth( MODE_ROW_W2 );
		copyBtn.SetTooltip( "#STR_COT_OBJECT_MODULE_EXPORT_COPY_DESC" );

		// The Spawn button names the destination the dropdown is set to.
		SyncSpawnButtonLabel();

		m_Categories = new JMItemCategoryPicker( this, m_RecentWrapper, JMFilterRegistry.OBJECTS, this, "OnCategoryChanged", m_Module.m_CurrentType );
		m_Categories.RebuildRecent();

		m_SearchBox.SetText(m_Module.m_SearchText);
		UpdateList();

		UpdateItemPreview();
	}

	// -------------------------------------------------------------------------
	//  Categories - the filter button, its menu and the recent chips are a
	//  JMItemCategoryPicker, shared with Loot Analysis.
	// -------------------------------------------------------------------------

	void OnClick_CategoryFilter( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK && m_Categories && m_FilterButton )
			m_Categories.Toggle( m_FilterButton.GetLayoutRoot() );
	}

	//! The picker's change callback: "" is everything, otherwise the config base class to filter on.
	void OnCategoryChanged( string categoryId )
	{
		m_Module.m_CurrentType = categoryId;

		UpdateList();
	}

	//! DEPRECATED - use JMItemCategoryPicker.CategoryTable().
	static void CategoryTable( out TStringArray ids, out TStringArray icons, out TStringArray labels )
	{
		JMDeprecated.WarnOnce( null, "JMObjectSpawnerForm.CategoryTable() is deprecated. Please use JMItemCategoryPicker.CategoryTable()." );
		JMItemCategoryPicker.CategoryTable( ids, icons, labels );
	}

	//! DEPRECATED - use JMItemCategoryPicker.CategoryGroupTable().
	static void CategoryGroupTable( out TStringArray ids, out TStringArray icons, out TStringArray labels )
	{
		JMDeprecated.WarnOnce( null, "JMObjectSpawnerForm.CategoryGroupTable() is deprecated. Please use JMItemCategoryPicker.CategoryGroupTable()." );
		JMItemCategoryPicker.CategoryGroupTable( ids, icons, labels );
	}

	//! DEPRECATED - use JMItemCategoryPicker.CategoryGroupMembers().
	static TStringArray CategoryGroupMembers( string groupId )
	{
		JMDeprecated.WarnOnce( null, "JMObjectSpawnerForm.CategoryGroupMembers() is deprecated. Please use JMItemCategoryPicker.CategoryGroupMembers()." );
		return JMItemCategoryPicker.CategoryGroupMembers( groupId );
	}

	void Click_OnSafetyToogle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Module.m_AllowRestrictedClassNames = ( action.GetSelection() == 1 );
		UpdateList();
	}

	void Click_OnFilterDisplayName( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Module.m_FilterWithDisplayName = ( action.GetSelection() == 1 );
		UpdateList();
	}

	void Click_ObjSetupMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_Module.m_ObjSetupMode = action.GetSelection();

		UpdateItemPreview();
	}

	void Click_SpawnObject( UIActionBase action )
	{
		SpawnObject(s_ObjSpawnMode);
	}

	void UpdateRotation( int mouse_x, int mouse_y, bool is_dragging )
	{
		m_Orientation[0] = m_Orientation[0] + ( ( m_MouseY - mouse_y ) * 0.01 );
		m_Orientation[1] = m_Orientation[1] - ( ( m_MouseX - mouse_x ) * 0.01 );

		m_ItemPreview.SetModelOrientation( m_Orientation );
	}

	void UpdateDistance( float wheel )
	{
		vector minMax[2];
		float radius = m_PreviewItem.ClippingInfo(minMax);

		m_Distance = m_Distance - (wheel * radius / 10.0);

		m_ItemPreview.SetModelPosition( Vector( m_Distance, 0, 0.5 + m_Distance ) );
	}

	void UpdateItemPreview()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+JMObjectSpawnerForm::UpdateItemPreview" );
		#endif

		string strSelection = GetCurrentSelection();

		bool modeChanged = m_PreviewSetupMode != m_Module.m_ObjSetupMode;

		if (m_PreviewItem && (m_PreviewItem.GetType() != strSelection || modeChanged))
		{
			g_Game.ObjectDelete( m_PreviewItem );
			m_PreviewItem = null;
		}

		if (!m_PreviewItem)
		{
			m_PreviewItem = EntityAI.Cast( g_Game.CreateObject( strSelection, vector.Zero, true, false, false ) );
			m_PreviewSetupMode = m_Module.m_ObjSetupMode;

			if (m_PreviewItem)
			{
				dBodyActive(m_PreviewItem, ActiveState.INACTIVE);
				dBodyDynamic(m_PreviewItem, false);
				m_PreviewItem.DisableSimulation(true);

				// Give the preview entity the same attachment pass the spawn
				// itself would run, so a weapon shows its optic and magazine and
				// a vehicle its parts instead of a stripped model that never
				// matches what actually lands in the world.
				PlayerBase previewOwner = PlayerBase.Cast( g_Game.GetPlayer() );
				if ( previewOwner )
					m_Module.SetupEntityForMode( m_PreviewItem, previewOwner, m_Module.m_ObjSetupMode );

				m_ItemPreview.SetItem( m_PreviewItem );

				m_Distance = 0;
				m_Orientation = vector.Zero;
				m_ItemPreview.SetModelPosition( Vector( m_Distance, 0, 0.5 + m_Distance ) );
				m_ItemPreview.SetModelOrientation( vector.Zero );
				m_ItemPreview.SetView( m_ItemPreview.GetItem().GetViewIndex() );
				m_ItemPreview.Show( true );
			}
		}

		m_Props.Rebuild( strSelection );

		if ( m_PreviewItem )
		{
			// Same pass the spawn runs - see JMObjectSpawnerProperties.RefreshPreviewHealth.
			// temp stays -1 - the temperature slider is a spawn-time value and
			// has nothing to show in the preview.
			m_Props.RefreshPreviewHealth();
			m_Props.RefreshPreviewFoodStage();
		}
		else
		{
			m_ItemPreview.Show( false );
		}

		#ifdef COT_DEBUGLOGS
		Print( "-JMObjectSpawnerForm::UpdateItemPreview" );
		#endif
	}

	override void OnFocus()
	{
		super.OnFocus();

		m_ItemPreview.Show(true);
	}

	override void OnUnfocus()
	{
		super.OnUnfocus();

		m_ItemPreview.Show(false);
	}

	void OnClick_ClassList( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CLICK )
		{
			UpdateItemPreview();

			if ( m_ClassList.GetLastClickButton() == MouseState.RIGHT && m_RowMenu )
				m_RowMenu.Open( GetCurrentSelection() );

			return;
		}

		//! A double click spawns what was just selected. CLICK is raised first,
		//! so the preview and the property rows are already the ones being
		//! spawned by the time this runs.
		if ( eid == UIEvent.DOUBLE_CLICK )
			SpawnObject( s_ObjSpawnMode );
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		super.OnMouseButtonDown( w, x, y, button );

		if ( w == m_ItemPreview && button == MouseState.LEFT )
		{
			g_Game.GetDragQueue().Call( this, "UpdateRotation" );
			GetMousePos( m_MouseX, m_MouseY );

			return true;
		}

		return false;
	}

	override bool OnMouseWheel(Widget  w, int  x, int  y, int wheel)
	{
		if ( w == m_ItemPreview && m_PreviewItem )
		{
			UpdateDistance(wheel);

			return true;
		}

		return super.OnMouseWheel( w, x, y, wheel );
	}

	// Reflect the current s_ObjSpawnMode on the Spawn button label + enable/disable
	// the Setup dropdown (clipboard modes don't use setup options).
	//! Add one entry to a dropdown and record which COT_ObjectSpawnerMode it
	//! stands for. Entry text is printed verbatim by the dropdown - unlike its
	//! label, it does not translate - so the key is resolved here.
	protected void AddMode( TIntArray ids, array<string> labels, int mode, string label )
	{
		ids.Insert( mode );
		labels.Insert( label );
	}

	protected int IndexOfMode( TIntArray ids, int mode )
	{
		int index = ids.Find( mode );

		if ( index < 0 )
			return 0;

		return index;
	}

	protected int ModeAt( TIntArray ids, int index )
	{
		if ( index < 0 || index >= ids.Count() )
			return -1;

		return ids[index];
	}

	//! Name the destination on the button. The clipboard modes have their own
	//! card and their own button now, so this no longer has to rename Spawn
	//! into something that does not spawn, or disable the Setup dropdown to
	//! stop it from claiming to apply.
	void SyncSpawnButtonLabel()
	{
		string label = "#STR_COT_OBJECT_MODULE_SPAWN";

		switch ( s_ObjSpawnMode )
		{
			case COT_ObjectSpawnerMode.CURSOR:
				label = "#STR_COT_OBJECT_MODULE_SPAWN_AT_CROSSHAIR";
				break;

			case COT_ObjectSpawnerMode.PLAYER_POSITION:
				label = "#STR_COT_OBJECT_MODULE_SPAWN_AT_PLAYER";
				break;

			case COT_ObjectSpawnerMode.TARGET_INVENTORY:
				label = "#STR_COT_OBJECT_MODULE_SPAWN_AT_TARGET";
				break;

			case COT_ObjectSpawnerMode.PLAYER_INVENTORY:
				if ( g_Game.IsMultiplayer() )
					label = "#STR_COT_OBJECT_MODULE_SPAWN_IN_PLAYERS";
				else
					label = "#STR_COT_OBJECT_MODULE_SPAWN_IN_OWN";
				break;

			case COT_ObjectSpawnerMode.OBJECT_INVENTORY:
				label = "#STR_COT_OBJECT_MODULE_SPAWN_IN_OBJECTS";
				break;
		}

		m_SpawnButton.SetButton( label );
	}

	void ChangeSpawnMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		int mode = ModeAt( m_SpawnModeIds, action.GetSelection() );

		if ( mode < 0 )
			return;

		s_ObjSpawnMode = mode;
		SyncSpawnButtonLabel();
	}

	void ChangeExportMode( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		int mode = ModeAt( m_ExportModeIds, action.GetSelection() );

		if ( mode < 0 )
			return;

		s_ObjExportMode = mode;
	}

	void Click_CopyList( UIActionBase action )
	{
		ExportList( s_ObjExportMode );
	}

	//! Put the CURRENT LIST on the clipboard in one of the export formats.
	//!
	//! Always from m_ListClasses, never from the row text: the rows carry
	//! display names while that filter is on, and none of these formats is any
	//! use full of display names.
	void ExportList( int mode )
	{
		string clipboardOutput = "";

		if ( m_ListClasses.Count() == 0 )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_OBJECT_MODULE_EXPORT_EMPTY" ) );
			return;
		}

		switch ( mode )
		{
			default:
			case COT_ObjectSpawnerMode.COPYLISTRAW:
				for ( int i = 0; i < m_ListClasses.Count(); i++ )
					clipboardOutput += m_ListClasses[i] + "\n";

				COTFeedback.Copy( clipboardOutput );
				break;

			case COT_ObjectSpawnerMode.COPYLISTTYPES:
				clipboardOutput = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
				clipboardOutput += "<types>\n";

				for ( int j = 0; j < m_ListClasses.Count(); j++ )
					clipboardOutput += BuildTypesXmlEntry( m_ListClasses[j] );

				clipboardOutput += "</types>";
				COTFeedback.Copy( clipboardOutput );
				break;

		#ifdef DZ_Expansion_Market
			case COT_ObjectSpawnerMode.COPYLISTEXPMARKET:
				string categoryJSON;

				auto category = new ExpansionMarketCategory();
				category.Defaults();
				category.DisplayName = m_SearchBox.GetText();

				for ( int k = 0; k < m_ListClasses.Count(); k++ )
				{
					auto item = new ExpansionMarketItem( -1, m_ListClasses[k], 100, 100, 1, 1 );
					category.Items.Insert( item );
				}

				string errorMsg;

				if ( JsonFileLoader<ExpansionMarketCategory>.MakeData( category, categoryJSON, errorMsg ) )
					COTFeedback.Copy( categoryJSON );
				else
					COTCreateLocalAdminNotification( new StringLocaliser( errorMsg ) );
				break;
		#endif
		}
	}

	//! One <type> block for `className` - the same shape ExportList's TYPES
	//! mode writes for every row, shared so the row menu's single-item copy
	//! matches the list export exactly.
	string BuildTypesXmlEntry( string className )
	{
		string entry = "	<type name=\"" + className + "\">\n";
		entry += "		<nominal>0</nominal>\n";
		entry += "		<lifetime>3888000</lifetime>\n";
		entry += "		<restock>0</restock>\n";
		entry += "		<min>0</min>\n";
		entry += "		<quantmin>-1</quantmin>\n";
		entry += "		<quantmax>-1</quantmax>\n";
		entry += "		<cost>100</cost>\n";
		entry += "		<flags count_in_cargo=\"0\" count_in_hoarder=\"0\" count_in_map=\"1\" count_in_player=\"0\" crafted=\"0\" deloot=\"0\"/>\n";
		entry += "	</type>\n";
		return entry;
	}

#ifdef DZ_Expansion_Market
	//! A one-item Expansion market category, same shape ExportList's
	//! COPYLISTEXPMARKET mode writes for the whole list - the row menu's
	//! single-item equivalent.
	void CopyMarketEntry( string className )
	{
		auto category = new ExpansionMarketCategory();
		category.Defaults();
		category.DisplayName = className;

		auto item = new ExpansionMarketItem( -1, className, 100, 100, 1, 1 );
		category.Items.Insert( item );

		string categoryJSON;
		string errorMsg;

		if ( JsonFileLoader<ExpansionMarketCategory>.MakeData( category, categoryJSON, errorMsg ) )
			COTFeedback.Copy( categoryJSON );
		else
			COTCreateLocalAdminNotification( new StringLocaliser( errorMsg ) );
	}
#endif

	void SpawnObject(int mode = COT_ObjectSpawnerMode.CURSOR)
	{
		string selection = GetCurrentSelection();

		if ( selection == "" )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_OBJECT_MODULE_NO_SELECTION" ) );
			return;
		}

		//! -1 is the module's "not specified" for all three, so a property the
		//! selected class does not have simply is not sent.
		//!
		//! Health and quantity travel as a PAIR: the low end as the value, the
		//! high end as module state alongside the setup mode. The server rolls
		//! between them once per entity, so one call that fills five inventories
		//! produces five different results.
		float health;
		float temp;
		float quantity;
		int itemState;
		m_Props.ReadSpawnValues( health, temp, quantity, itemState );

		switch (mode)
		{
			default:
			case COT_ObjectSpawnerMode.PLAYER_POSITION:
				m_Module.SpawnEntity_Position(selection, g_Game.GetPlayer().GetPosition(), quantity, health, temp, itemState);
				break;

			case COT_ObjectSpawnerMode.CURSOR:
				m_Module.SpawnEntity_Position(selection, GetCursorPos(), quantity, health, temp, itemState);
				break;

			case COT_ObjectSpawnerMode.TARGET_INVENTORY:
				m_Module.SpawnEntity_Position(selection, GetCursorPos(), quantity, health, temp, itemState, true);
				break;

			case COT_ObjectSpawnerMode.PLAYER_INVENTORY:
				m_Module.SpawnEntity_Inventory(selection, JM_GetSelected().GetPlayers(), quantity, health, temp, itemState);
				break;

			case COT_ObjectSpawnerMode.OBJECT_INVENTORY:
				m_Module.SpawnEntity_Inventory(selection, JM_GetSelected().GetObjects(), quantity, health, temp, itemState);
				break;
		}
	}

	//! Fired by m_DeleteCursorBtn on CHANGE, i.e. once the confirm/cancel pair
	//! has actually been confirmed - by a mouse click on Confirm, or by a
	//! second keybind press via DeleteCursor(Object) below.
	void DeleteCursor( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		Object obj = m_PendingDeleteCursorObj;
		m_PendingDeleteCursorObj = null;

		if ( !obj )
			obj = m_Module.GetObjectAtCursor();

		if ( !obj )
			return;

		m_Module.DeleteEntity( obj );

		if ( m_Module.m_AutoShow )
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( m_Module.Hide );
	}

	//! Keybind entry point. First press on a target arms m_DeleteCursorBtn
	//! (same confirm/cancel morph + timeout as clicking it) instead of
	//! deleting outright. A second press while still armed on the SAME
	//! target confirms; aiming elsewhere re-arms on the new target.
	void DeleteCursor( Object obj )
	{
		if ( !obj || !m_DeleteCursorBtn )
			return;

		if ( m_DeleteCursorBtn.IsPending() && m_PendingDeleteCursorObj == obj )
		{
			m_DeleteCursorBtn.ConfirmPending();
			return;
		}

		m_PendingDeleteCursorObj = obj;
		m_DeleteCursorBtn.Arm();
	}

	void SearchInput_OnChange( UIEvent eid, UIActionBase action )
	{
		m_Module.m_SearchText = m_SearchBox.GetText();
		UpdateList();
	}

	protected void ReportExactClassRejection( string searchText, TStringArray configs )
	{
		string searchLower = searchText;
		searchLower.ToLower();

		foreach ( string cfgPath: configs )
		{
			int count = g_Game.ConfigGetChildrenCount( cfgPath );
			for ( int i = 0; i < count; i++ )
			{
				string name;
				g_Game.ConfigGetChildName( cfgPath, i, name );

				string nameLower = name;
				nameLower.ToLower();

				if ( nameLower != searchLower )
					continue;

				int scope = g_Game.ConfigGetInt( cfgPath + " " + name + " scope" );
				if ( scope == 0 || (scope == 1 && !m_Module.m_AllowRestrictedClassNames) )
				{
					COTCreateLocalAdminNotification( new StringLocaliser( name + " " + Widget.TranslateString( "#STR_COT_OBJECT_MODULE_REJECT_SCOPE" ) ) );
					return;
				}

				string model;
				if ( !g_Game.ConfigGetText( cfgPath + " " + name + " model", model ) || model == string.Empty || model == "bmp" )
				{
					COTCreateLocalAdminNotification( new StringLocaliser( name + " " + Widget.TranslateString( "#STR_COT_OBJECT_MODULE_REJECT_NO_MODEL" ) ) );
					return;
				}

				if ( m_Module.IsExcludedClassName( nameLower ) )
				{
					COTCreateLocalAdminNotification( new StringLocaliser( name + " " + Widget.TranslateString( "#STR_COT_OBJECT_MODULE_REJECT_BLACKLISTED" ) ) );
					return;
				}

				// Class exists and passed all filters - no rejection to report.
				return;
			}
		}
	}

	void UpdateList()
	{
	#ifdef DIAG
		int ticks = TickCount(0);
	#endif

		m_ListClasses.Clear();

		//! What the rows SHOW, and what they show beside it. In display-name
		//! mode the class name becomes the secondary column rather than being
		//! hidden - it is still the thing an admin has to type anywhere else.
		array<string> rowLabels = new array<string>;
		array<string> rowSubs   = new array<string>;

		TStringArray configs = new TStringArray;
		configs.Insert( CFG_VEHICLESPATH );
		configs.Insert( CFG_WEAPONSPATH );
		configs.Insert( CFG_MAGAZINESPATH );

		JMSearchMatcher matcher = new JMSearchMatcher(m_Module.m_SearchText);

		for ( int nConfig; nConfig < configs.Count(); nConfig++ )
		{
			string strConfigPath = configs.Get( nConfig );

			int nClasses = g_Game.ConfigGetChildrenCount( strConfigPath );

			for ( int nClass = 0; nClass < nClasses; nClass++ )
			{
				string strName;
				string strDisplay;
				string strRowText;

				g_Game.ConfigGetChildName( strConfigPath, nClass, strName );

				string strNameLower = strName;
				strNameLower.ToLower();

				int scope = g_Game.ConfigGetInt( strConfigPath + " " + strName + " scope" );

				if ( scope == 0 || (scope == 1 && !m_Module.m_AllowRestrictedClassNames) )
					continue;

				string model;
				if (!g_Game.ConfigGetText(strConfigPath + " " + strName + " model", model) || model == string.Empty || model == "bmp")
					continue;

				if (m_Module.m_CurrentType == "" || g_Game.IsKindOf( strNameLower, m_Module.m_CurrentType ) )
				{
					if ( m_Module.IsExcludedClassName( strNameLower ) )
						continue;

					COT_String strNameSearch = strNameLower;
					if (m_Module.m_FilterWithDisplayName)
					{
						if (!g_Game.ConfigGetText(strConfigPath + " " + strName + " displayName", strNameSearch))
							continue;

						strNameSearch.ToLower();
					}

					if ( !matcher.Matches(strNameSearch) )
						continue;

					// Display-name mode changes what the row SHOWS, not what it
					// means. The class name goes into m_ListClasses either way -
					// spawning and the clipboard exports read from there.
					strRowText = strName;

					if (m_Module.m_FilterWithDisplayName)
					{
						if (g_Game.ConfigGetText(strConfigPath + " " + strName + " displayName", strDisplay) && strDisplay != "")
							strRowText = strDisplay;
					}

					rowLabels.Insert( strRowText );

					if ( strRowText == strName )
						rowSubs.Insert( "" );
					else
						rowSubs.Insert( strName );

					m_ListClasses.Insert( strName );
				}
			}
		}

		m_ClassList.SetItems( rowLabels, rowSubs );

		//if ( strSearch != "" && m_ListClasses.Count() == 0 )
			ReportExactClassRejection( matcher.GetSearch(), configs );

	#ifdef DIAG
		float elapsed = TickCount(ticks) * 0.0001;
		PrintFormat("UpdateList %1 %2 ms", m_Module.m_SearchText, elapsed);
	#endif

		m_SearchBox.SetTextPreview(matcher.GetClosestMatch());
	}
}
