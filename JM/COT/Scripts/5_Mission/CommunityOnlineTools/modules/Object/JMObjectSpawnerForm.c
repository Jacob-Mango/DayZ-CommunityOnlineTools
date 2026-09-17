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
	protected ref array<ref UIActionButton> m_RecentButtons;

	//! Categories moved out of a dropdown of their own and behind a filter
	//! button beside the search box. The dropdown cost a full row of the left
	//! column permanently for a control that is touched once per search.
	protected UIActionImageButton m_FilterButton;
	protected UIActionContextMenu m_CategoryMenu;
	protected UIActionContextMenu m_CategorySubMenu;

	//! Which group's submenu is up, "" when none.
	protected string m_CurrentGroup;

	protected ref TStringArray m_RecentIds;

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

	//! Property rows are rebuilt per selection rather than shown and hidden: a
	//! hidden child still owns its cell in a GridSpacer, which would leave a
	//! gap where an inapplicable slider used to be. m_PropsHost stays put so
	//! the rows keep their place in the card; m_PropsGrid is what gets
	//! replaced.
	protected Widget m_PropsHost;
	protected Widget m_PropsGrid;
	protected UIActionText m_PropsNone;

	//! Ranges, not single values: the low and high handles are the bounds the
	//! server rolls between, per spawned entity. Dragging them together is how
	//! you ask for one exact value.
	//!
	//! Both start COLLAPSED to one handle - see m_RangeToggle.
	protected UIActionSliderRange m_QuantityItem;
	protected UIActionSliderRange m_HealthItem;

	//! Switches the two spannable rows above between one value and a range.
	//!
	//! Off by default: rolling a spawn between two bounds is the advanced case,
	//! and the pair of handles reads as a range whether or not the admin meant
	//! one - a full magazine asked for by dragging both ends together is easy
	//! to leave a round short by accident. With it off the rows keep exactly
	//! the same look, minus the second handle.
	//!
	//! It lives in the card's TITLE BAR rather than in a row of its own: it is
	//! a mode for the whole card, not a property of the item being spawned, and
	//! a row would have put it in the same list as quantity and health where it
	//! reads as one more thing that gets spawned. It also outlives the rows -
	//! the grid under it is destroyed on every selection, the header is not.
	protected UIActionImageButton m_RangeToggle;
	protected bool m_RangesEnabled;

	//! Kept because the header action is created once and the ROWS it governs
	//! come and go: a class with neither a quantity nor a health bar has
	//! nothing to switch, and the button is hidden for it.
	protected UIActionCard m_PropsCard;

	//! Still a single value. Temperature reads back as a NAMED state next to the
	//! number, and there is no sensible name for a range.
	protected UIActionSlider m_TemperatureItem;

	//! Cooking state for food.
	//!
	//! A dropdown built into a card that is torn down on every selection is only
	//! safe because UIActionDropdown now unlinks its floating list panel with
	//! itself - the panel is parented to the FORM root, not to the control, so
	//! before that it would have been left behind once per rebuild.
	protected UIActionDropdown m_FoodStage;

	//! FoodStageType for each entry of m_FoodStage, in the same order. Only the
	//! stages the class actually declares are offered, so the indices are not
	//! the enum's.
	protected ref TIntArray m_FoodStageTypes;
	protected int m_FoodStageValue;

	//! What a liquid container is filled with. The values are liquid-type bits,
	//! except on a blood container where the module wants a 1-based index into
	//! the blood types instead - see CollectLiquids.
	protected UIActionDropdown m_LiquidType;
	protected ref TIntArray m_LiquidValues;
	protected int m_LiquidValue;

	//! Which cartridge a magazine is loaded with, as one of its own ammoItems
	//! classnames. "" leaves it on whatever it is configured to hold.
	protected UIActionDropdown m_AmmoType;
	protected ref TStringArray m_AmmoValues;
	protected string m_AmmoValue;

	//! Rags, bandages and sewing kits spawn either soiled or disinfected.
	protected UIActionToggleSwitch m_Disinfect;
	protected bool m_DisinfectValue;

	//! The sliders are destroyed on every rebuild, so what the admin chose has
	//! to outlive them. Health and quantity are kept as FRACTIONS of their own
	//! span - the span changes with the class, and 30 rounds of one calibre
	//! should come back as 30 rounds of the next, not as the number 30 against
	//! a different maximum. Temperature has a fixed range, so it is absolute.
	protected float m_HealthLowPct;
	protected float m_HealthHighPct;
	protected float m_QuantityLowPct;
	protected float m_QuantityHighPct;
	protected float m_TemperatureValue;
	protected bool  m_TemperatureSet;

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
	static const int RECENT_MAX = 4;

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

	//! Sum of the three above. Written out because a static const folded from
	//! other static consts evaluates to 0 in Enforce - keep the three and this
	//! in step by hand.
	static const int LEFT_HEADER_H = 106;

	//! Row splits, as FRACTIONS of the row. Never a measured flex pass and
	//! never a fraction mixed with fixed pixels - see OnInit.
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

		m_RecentIds      = new TStringArray;
		m_ListClasses    = new TStringArray;
		m_FoodStageTypes = new TIntArray;
		m_LiquidValues   = new TIntArray;
		m_AmmoValues     = new TStringArray;
		m_SpawnModeIds   = new TIntArray;
		m_ExportModeIds  = new TIntArray;
		m_RecentButtons  = new array<ref UIActionButton>;

		//! Both ranges open at their full span, so switching ranges ON asks for
		//! a spawn randomised across everything the class allows until the
		//! handles are pulled together. Until then only the high handle is
		//! shown, and it is the one value that spawns.
		m_HealthLowPct    = 0.0;
		m_HealthHighPct   = 1.0;
		m_QuantityLowPct  = 0.0;
		m_QuantityHighPct = 1.0;

		m_RangesEnabled = false;

		m_TemperatureValue = GameConstants.STATE_NEUTRAL_TEMP;
	}

	void ~JMObjectSpawnerForm()
	{
		if (m_PreviewItem)
			g_Game.ObjectDelete(m_PreviewItem);
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

		if ( m_ListWrapper && h > LEFT_HEADER_H )
		{
			float listH = h - LEFT_HEADER_H;

			PinBand( m_ListWrapper, LEFT_HEADER_H, listH );

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

	override void OnInit()
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
		Widget searchRow = UIActionManager.CreateWrapSpacerCompact( m_SearchWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_SearchBox = UIActionManager.CreateSearchBox( searchRow, this, "SearchInput_OnChange", "#STR_COT_OBJECT_MODULE_SEARCH" );
		m_SearchBox.SetWidth( SEARCH_ROW_W );

		m_FilterButton = UIActionManager.CreateIconButton( searchRow, JMConstants.Lucide( "list-filter" ), this, "OnClick_CategoryFilter" );
		m_FilterButton.SetWidth( FILTER_BTN_W );
		m_FilterButton.SetTooltip( "#STR_COT_OBJECT_MODULE_CATEGORY_DESC" );

		// --- Filters ----------------------------------------------------------
		Widget filterRow = UIActionManager.CreateWrapSpacerCompact( m_FilterWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		//! Entry text is printed verbatim by the selector, so the keys are
		//! resolved here rather than being handed over as keys.
		array<string> nameModes = new array<string>;
		nameModes.Insert( Widget.TranslateString( "#STR_COT_OBJECT_MODULE_NAMEMODE_CLASS" ) );
		nameModes.Insert( Widget.TranslateString( "#STR_COT_OBJECT_MODULE_NAMEMODE_DISPLAY" ) );

		m_NameModeSelect = UIActionManager.CreateSelectionBox( filterRow, "", nameModes, this, "Click_OnFilterDisplayName" );
		m_NameModeSelect.SetWidth( MODE_ROW_W );
		m_NameModeSelect.SetSelectorWidth( 1.0 );
		m_NameModeSelect.SetTooltip( "#STR_COT_OBJECT_MODULE_SPAWN_DISPLAYNAME_DESC" );

		if ( m_Module.m_FilterWithDisplayName )
			m_NameModeSelect.SetSelection( 1, false );

		array<string> unsafeModes = new array<string>;
		unsafeModes.Insert( Widget.TranslateString( "#STR_COT_OBJECT_MODULE_UNSAFE_HIDE" ) );
		unsafeModes.Insert( Widget.TranslateString( "#STR_COT_OBJECT_MODULE_UNSAFE_SHOW" ) );

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
		Widget previewHost = UIActionManager.CreatePanel( previewCard.GetContent(), 0x00000000, PREVIEW_H );

		m_ItemPreviewPanel = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/objectspawner_preview.layout", previewHost );

		if ( m_ItemPreviewPanel )
			Class.CastTo( m_ItemPreview, m_ItemPreviewPanel.FindAnyWidget( "object_preview" ) );

		m_PropsCard = UIActionManager.CreateCard( rightContent, "#STR_COT_OBJECT_MODULE_PROPERTIES" );
		m_PropsHost = UIActionManager.CreateGridSpacer( m_PropsCard.GetContent(), 1, 1 );

		//! An icon button, not a toggle switch: the header strip is 30px tall
		//! and gives its children no label room. The STATE is the tint - accent
		//! while ranges are on, the plain button fill while they are off - and
		//! the tooltip names both what it is doing and what a click would do.
		m_RangeToggle = m_PropsCard.AddCardHeaderAction( JMConstants.Lucide( "arrow-left-right" ), this, "Click_ToggleRanges" );

		PaintRangeToggle();

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
		RegisterOverlay( m_SpawnMode );

		array<string> setupOptions = new array<string>;
		setupOptions.Insert( Widget.TranslateString( "#STR_COT_OBJECT_MODULE_SPAWN_WITH_DEBUG" ) );
		setupOptions.Insert( Widget.TranslateString( "#STR_COT_OBJECT_MODULE_SPAWN_WITH_CE" ) );
		setupOptions.Insert( Widget.TranslateString( "#STR_COT_GENERIC_NONE" ) );
		m_ObjSetupMode = UIActionManager.CreateDropdown( modeRow, "", layoutRoot, this, "Click_ObjSetupMode", setupOptions );
		m_ObjSetupMode.SetWidth( MODE_ROW_W2 );
		m_ObjSetupMode.SetSelection( m_Module.m_ObjSetupMode, false );
		m_ObjSetupMode.SetTooltip( "#STR_COT_OBJECT_MODULE_SETUP_DESC" );
		RegisterOverlay( m_ObjSetupMode );

		//! Spawn and delete share a row and a width. Both layouts are 30 tall,
		//! and neither carries a pixel width, so the pair scales together and
		//! the row cannot wrap however narrow the window gets.
		Widget spawnRow = UIActionManager.CreateWrapSpacerCompact( spawnBody, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_SpawnButton = UIActionManager.CreateButton( spawnRow, "#STR_COT_OBJECT_MODULE_SPAWN_ON", this, "Click_SpawnObject" );
		m_SpawnButton.SetWidth( MODE_ROW_W );
		m_SpawnButton.SetColor( JMTheme.SUCCESS_FILL );
		m_SpawnButton.SetTooltip( "#STR_COT_OBJECT_MODULE_SPAWN_DESC" );

		m_DeleteCursorBtn = UIActionManager.CreateConfirmInline( spawnRow, "#STR_COT_OBJECT_MODULE_DELETE_CURSOR", this, "DeleteCursor" );
		UIActionIconGrid.ApplyDeletePreset( m_DeleteCursorBtn );
		m_DeleteCursorBtn.SetWidth( MODE_ROW_W2 );
		m_DeleteCursorBtn.SetTooltip( "#STR_COT_OBJECT_MODULE_DELETE_DESC" );

		RegisterPermission( m_DeleteCursorBtn, "Entity.Delete" );

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
		RegisterOverlay( m_ExportMode );

		UIActionButton copyBtn = UIActionManager.CreateButton( exportRow, "#STR_COT_GENERIC_COPY", this, "Click_CopyList" );
		copyBtn.SetWidth( MODE_ROW_W2 );
		copyBtn.SetTooltip( "#STR_COT_OBJECT_MODULE_EXPORT_COPY_DESC" );

		// The Spawn button names the destination the dropdown is set to.
		SyncSpawnButtonLabel();

		RebuildRecentCategories();

		m_SearchBox.SetText(m_Module.m_SearchText);
		UpdateList();

		UpdateItemPreview();
	}

	// -------------------------------------------------------------------------
	//  Categories
	// -------------------------------------------------------------------------

	//! Id / icon / label for every category, in dropdown order. The id is the
	//! config base class the list filters on; "" is everything.
	//!
	//! One table, read by both the dropdown and the recent chips, so the two
	//! can never drift apart.
	static void CategoryTable( out TStringArray ids, out TStringArray icons, out TStringArray labels )
	{
		ids = new TStringArray;
		icons = new TStringArray;
		labels = new TStringArray;

		AddCategory( ids, icons, labels, "",               JMConstants.Lucide( "layers" ),    "#STR_COT_OBJECT_CATEGORY_ALL" );
		AddCategory( ids, icons, labels, "edible_base",    JMConstants.ICON_MEAT,             "#STR_COT_OBJECT_CATEGORY_FOOD" );
		AddCategory( ids, icons, labels, "bottle_base",    JMConstants.ICON_BEER_BOTTLE,      "#STR_COT_OBJECT_CATEGORY_DRINKS" );
		AddCategory( ids, icons, labels, "transport",      JMConstants.ICON_JEEP,             "#STR_COT_OBJECT_CATEGORY_VEHICLES" );
		AddCategory( ids, icons, labels, "weapon_base",    JMConstants.ICON_FAMAS,            "#STR_COT_OBJECT_CATEGORY_FIREARMS" );
		AddCategory( ids, icons, labels, "meleeweapon",    JMConstants.ICON_GLADIUS,          "#STR_COT_OBJECT_CATEGORY_MELEE" );
		AddCategory( ids, icons, labels, "magazine_base",  JMConstants.ICON_MACHINE_GUN_MAG,  "#STR_COT_OBJECT_CATEGORY_AMMO" );
		AddCategory( ids, icons, labels, "clothing_base",  JMConstants.ICON_CLOTHES,          "#STR_COT_OBJECT_CATEGORY_CLOTHING" );
		AddCategory( ids, icons, labels, "headgear_base",  JMConstants.ICON_STAHLHELM,        "#STR_COT_OBJECT_CATEGORY_HEADGEAR" );
		AddCategory( ids, icons, labels, "mask_base",      JMConstants.ICON_BALACLAVA,        "#STR_COT_OBJECT_CATEGORY_MASKS" );
		AddCategory( ids, icons, labels, "glasses_base",   JMConstants.ICON_SUNGLASSES,       "#STR_COT_OBJECT_CATEGORY_GLASSES" );
		AddCategory( ids, icons, labels, "top_base",       JMConstants.ICON_T_SHIRT,          "#STR_COT_OBJECT_CATEGORY_TOPS" );
		AddCategory( ids, icons, labels, "pants_base",     JMConstants.ICON_TROUSERS,         "#STR_COT_OBJECT_CATEGORY_PANTS" );
		AddCategory( ids, icons, labels, "vest_base",      JMConstants.ICON_BELT,             "#STR_COT_OBJECT_CATEGORY_VESTS" );
		AddCategory( ids, icons, labels, "gloves_base",    JMConstants.ICON_GLOVES,           "#STR_COT_OBJECT_CATEGORY_GLOVES" );
		AddCategory( ids, icons, labels, "shoes_base",     JMConstants.ICON_TROUSERS,         "#STR_COT_OBJECT_CATEGORY_SHOES" );
		AddCategory( ids, icons, labels, "backpack_base",  JMConstants.ICON_LIGHT_BACKPACK,   "#STR_COT_OBJECT_CATEGORY_BACKPACKS" );
		AddCategory( ids, icons, labels, "container_base", JMConstants.ICON_KNAPSACK,         "#STR_COT_OBJECT_CATEGORY_CONTAINERS" );
		AddCategory( ids, icons, labels, "inventory_base", JMConstants.ICON_FULL_FOLDER,      "#STR_COT_OBJECT_CATEGORY_ITEMS" );
		AddCategory( ids, icons, labels, "itemmedical",    JMConstants.ICON_MEDICINES,        "#STR_COT_OBJECT_CATEGORY_MEDICAL" );
		AddCategory( ids, icons, labels, "tool_base",      JMConstants.ICON_SHARP_AXE,        "#STR_COT_OBJECT_CATEGORY_TOOLS" );
		AddCategory( ids, icons, labels, "trapbase",       JMConstants.ICON_TINKER,           "#STR_COT_OBJECT_CATEGORY_TRAPS" );
		AddCategory( ids, icons, labels, "electricdevice", JMConstants.ICON_ELECTRIC,         "#STR_COT_OBJECT_CATEGORY_ELECTRONICS" );
		AddCategory( ids, icons, labels, "tentbase",       JMConstants.ICON_CAMPING_TENT,     "#STR_COT_OBJECT_CATEGORY_TENTS" );
		AddCategory( ids, icons, labels, "grenade_base",   JMConstants.ICON_UNLIT_BOMB,       "#STR_COT_OBJECT_CATEGORY_EXPLOSIVES" );
		AddCategory( ids, icons, labels, "house",          JMConstants.ICON_HOME_GARAGE,      "#STR_COT_OBJECT_CATEGORY_BUILDINGS" );
		AddCategory( ids, icons, labels, "dz_lightai",     JMConstants.ICON_SHAMBLING_ZOMBIE, "#STR_COT_OBJECT_CATEGORY_AI" );
	}

	//! Categories are presented in groups, each opening a submenu, the way the
	//! teleport filter is organised. Flat, the 27 rows needed two columns to fit
	//! the window and still read as a wall of icons.
	static const string GROUP_GEAR     = "grp_gear";
	static const string GROUP_WEAPONS  = "grp_weapons";
	static const string GROUP_SURVIVAL = "grp_survival";
	static const string GROUP_WORLD    = "grp_world";

	static void CategoryGroupTable( out TStringArray ids, out TStringArray icons, out TStringArray labels )
	{
		ids = new TStringArray;
		icons = new TStringArray;
		labels = new TStringArray;

		AddCategory( ids, icons, labels, GROUP_GEAR,     JMConstants.ICON_CLOTHES, "#STR_COT_OBJECT_CATEGORY_GROUP_GEAR" );
		AddCategory( ids, icons, labels, GROUP_WEAPONS,  JMConstants.ICON_FAMAS,   "#STR_COT_OBJECT_CATEGORY_GROUP_WEAPONS" );
		AddCategory( ids, icons, labels, GROUP_SURVIVAL, JMConstants.ICON_MEAT,    "#STR_COT_OBJECT_CATEGORY_GROUP_SURVIVAL" );
		AddCategory( ids, icons, labels, GROUP_WORLD,    JMConstants.ICON_JEEP,    "#STR_COT_OBJECT_CATEGORY_GROUP_WORLD" );
	}

	//! Every id here must also appear in CategoryTable, and between them the four
	//! groups must cover it entirely - a category in neither place can no longer
	//! be reached from the filter button at all.
	static TStringArray CategoryGroupMembers( string groupId )
	{
		TStringArray members = new TStringArray;

		if ( groupId == GROUP_GEAR )
		{
			members.Insert( "clothing_base" );
			members.Insert( "headgear_base" );
			members.Insert( "mask_base" );
			members.Insert( "glasses_base" );
			members.Insert( "top_base" );
			members.Insert( "pants_base" );
			members.Insert( "vest_base" );
			members.Insert( "gloves_base" );
			members.Insert( "shoes_base" );
			members.Insert( "backpack_base" );
			return members;
		}

		if ( groupId == GROUP_WEAPONS )
		{
			members.Insert( "weapon_base" );
			members.Insert( "meleeweapon" );
			members.Insert( "magazine_base" );
			members.Insert( "grenade_base" );
			members.Insert( "trapbase" );
			return members;
		}

		if ( groupId == GROUP_SURVIVAL )
		{
			members.Insert( "edible_base" );
			members.Insert( "bottle_base" );
			members.Insert( "itemmedical" );
			members.Insert( "tool_base" );
			members.Insert( "tentbase" );
			members.Insert( "container_base" );
			return members;
		}

		if ( groupId == GROUP_WORLD )
		{
			members.Insert( "transport" );
			members.Insert( "house" );
			members.Insert( "dz_lightai" );
			members.Insert( "electricdevice" );
			members.Insert( "inventory_base" );
			return members;
		}

		return members;
	}

	protected string GroupOfCategory( string categoryId )
	{
		TStringArray gids, gicons, glabels;
		CategoryGroupTable( gids, gicons, glabels );

		for ( int i = 0; i < gids.Count(); i++ )
		{
			TStringArray members = CategoryGroupMembers( gids[i] );

			if ( members.Find( categoryId ) >= 0 )
				return gids[i];
		}

		return "";
	}

	static void AddCategory( TStringArray ids, TStringArray icons, TStringArray labels, string id, string icon, string label )
	{
		ids.Insert( id );
		icons.Insert( icon );
		labels.Insert( label );
	}

	protected string CategoryLabel( string id )
	{
		TStringArray ids, icons, labels;
		CategoryTable( ids, icons, labels );

		for ( int i = 0; i < ids.Count(); i++ )
		{
			if ( ids[i] == id )
				return labels[i];
		}

		return "";
	}

	protected string CategoryIcon( string id )
	{
		TStringArray ids, icons, labels;
		CategoryTable( ids, icons, labels );

		for ( int i = 0; i < ids.Count(); i++ )
		{
			if ( ids[i] == id )
				return icons[i];
		}

		return JMConstants.Lucide( "layers" );
	}

	//! The "everything" category is the empty string, which the menu cannot use
	//! as an id - an empty GetLastClickedId() is also what a menu that has never
	//! been clicked reports.
	static const string MENU_ID_ALL = "__all";

	protected string MenuIdFor( string categoryId )
	{
		if ( categoryId == "" )
			return MENU_ID_ALL;

		return categoryId;
	}

	protected string CategoryIdFor( string menuId )
	{
		if ( menuId == MENU_ID_ALL )
			return "";

		return menuId;
	}

	//! Open the category list under the filter button, the way the vehicle form
	//! opens its type filter. A popup rather than a control parked on the form:
	//! it is needed for one click per search and costs nothing in between.
	void OnClick_CategoryFilter( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_FilterButton )
			return;

		if ( !m_CategoryMenu )
		{
			if ( !m_Window )
				return;

			m_CategoryMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_CategoryMenu" );

			if ( !m_CategoryMenu )
				return;

			RegisterOverlay( m_CategoryMenu );
			m_CategoryMenu.SetOwnerWidget( m_FilterButton.GetLayoutRoot() );

			//! A press on a group row opens its submenu; closing the parent out
			//! from under it would take the submenu's anchor with it.
			m_CategoryMenu.SetCloseOnClick( false );
		}

		//! A second press on the button puts the menu away instead of reopening
		//! it in place, which is what a dropdown is expected to do.
		if ( m_CategoryMenu.IsOpen() )
		{
			CloseCategoryMenus();
			return;
		}

		RebuildCategoryMenu();

		float fx;
		float fy;
		float fw;
		float fh;

		m_FilterButton.GetLayoutRoot().GetScreenPos( fx, fy );
		m_FilterButton.GetLayoutRoot().GetScreenSize( fw, fh );

		m_CategoryMenu.ShowAt( fx, fy + fh );
	}

	//! Rows are built ONCE. AddItem rebuilds every row it already holds, so
	//! filling 27 entries on each open would create them 378 times over; the
	//! selected one is marked by recolouring instead.
	protected void RebuildCategoryMenu()
	{
		if ( !m_CategoryMenu )
			return;

		if ( m_CategoryMenu.GetItemCount() == 0 )
		{
			m_CategoryMenu.AddItem( MENU_ID_ALL, CategoryLabel( "" ), CategoryIcon( "" ) );

			TStringArray gids, gicons, glabels;
			CategoryGroupTable( gids, gicons, glabels );

			for ( int i = 0; i < gids.Count(); i++ )
				m_CategoryMenu.AddItem( gids[i], glabels[i], gicons[i], 0, true );
		}

		RefreshCategoryMenuColors();
	}

	protected void RefreshCategoryMenuColors()
	{
		if ( !m_CategoryMenu )
			return;

		//! 0 means "use the menu default", so only the active row names a colour
		//! of its own. A group is marked when the active category is one of its
		//! members, which is the only cue while its submenu is shut.
		int allColor = 0;

		if ( m_Module.m_CurrentType == "" )
			allColor = JMTheme.ACCENT;

		m_CategoryMenu.SetItemTextColor( MENU_ID_ALL, allColor );

		string activeGroup = GroupOfCategory( m_Module.m_CurrentType );

		TStringArray gids, gicons, glabels;
		CategoryGroupTable( gids, gicons, glabels );

		for ( int i = 0; i < gids.Count(); i++ )
		{
			int color = 0;

			if ( gids[i] == activeGroup )
				color = JMTheme.ACCENT;

			m_CategoryMenu.SetItemTextColor( gids[i], color );
		}

		RefreshCategorySubMenuColors();
	}

	protected void OpenCategorySubMenu( string groupId, float x, float y )
	{
		if ( !m_Window )
			return;

		if ( !m_CategorySubMenu )
		{
			m_CategorySubMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_CategorySubMenu" );

			if ( !m_CategorySubMenu )
				return;

			RegisterOverlay( m_CategorySubMenu );
		}

		if ( m_CategorySubMenu.IsOpen() && m_CurrentGroup == groupId )
		{
			m_CategorySubMenu.Close();
			m_CurrentGroup = "";
			return;
		}

		m_CurrentGroup = groupId;

		RebuildCategorySubMenu();

		m_CategorySubMenu.ShowAt( x, y );
	}

	protected void RebuildCategorySubMenu()
	{
		if ( !m_CategorySubMenu )
			return;

		m_CategorySubMenu.ClearItems();

		TStringArray members = CategoryGroupMembers( m_CurrentGroup );

		for ( int i = 0; i < members.Count(); i++ )
			m_CategorySubMenu.AddItem( MenuIdFor( members[i] ), CategoryLabel( members[i] ), CategoryIcon( members[i] ) );

		RefreshCategorySubMenuColors();
	}

	protected void RefreshCategorySubMenuColors()
	{
		if ( !m_CategorySubMenu || m_CurrentGroup == "" )
			return;

		TStringArray members = CategoryGroupMembers( m_CurrentGroup );

		for ( int i = 0; i < members.Count(); i++ )
		{
			int color = 0;

			if ( members[i] == m_Module.m_CurrentType )
				color = JMTheme.ACCENT;

			m_CategorySubMenu.SetItemTextColor( MenuIdFor( members[i] ), color );
		}
	}

	protected void CloseCategoryMenus()
	{
		if ( m_CategorySubMenu )
			m_CategorySubMenu.Close();

		if ( m_CategoryMenu )
			m_CategoryMenu.Close();

		m_CurrentGroup = "";
	}

	void OnClick_CategoryMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_CategoryMenu )
			return;

		string id = m_CategoryMenu.GetLastClickedId();

		if ( id == MENU_ID_ALL )
		{
			SelectCategory( "" );
			CloseCategoryMenus();
			return;
		}

		int mx, my;
		GetMousePos( mx, my );

		OpenCategorySubMenu( id, mx, my );
	}

	void OnClick_CategorySubMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_CategorySubMenu )
			return;

		SelectCategory( CategoryIdFor( m_CategorySubMenu.GetLastClickedId() ) );
		CloseCategoryMenus();
	}

	//! Which chip was pressed, from the action that fired. The buttons carry no
	//! id of their own - their position in m_RecentButtons IS their position in
	//! m_RecentIds, because the two are built in one pass.
	void OnClick_RecentChip( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		UIActionButton chip;

		if ( !Class.CastTo( chip, action ) )
			return;

		int index = m_RecentButtons.Find( chip );

		if ( index < 0 || index >= m_RecentIds.Count() )
			return;

		SelectCategory( m_RecentIds[index] );
	}

	protected void SelectCategory( string id )
	{
		m_Module.m_CurrentType = id;

		//! A chip press must never destroy the strip it landed on. Adding a
		//! category the strip does not hold yet is the only thing that changes
		//! its shape, and even then the rebuild is deferred a tick so it cannot
		//! happen inside the click that caused it - the engine answers a
		//! vanished press target by recentring the cursor.
		if ( PushRecentCategory( id ) )
			g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Call( RebuildRecentCategories );
		else
			PaintRecentCategories();

		//! The menu marks what is active, so it follows a chip click as well as
		//! its own.
		RefreshCategoryMenuColors();

		UpdateList();
	}

	//! Newest first, capped. Returns whether the strip's contents changed.
	//!
	//! Re-picking a category the strip ALREADY holds leaves the order alone.
	//! Promoting it to the front would slide every other chip sideways under
	//! the cursor that just pressed one, which is the last thing a shortcut
	//! bar should do. "All" gets no chip - it is the first row of the menu.
	protected bool PushRecentCategory( string id )
	{
		if ( id == "" )
			return false;

		if ( m_RecentIds.Find( id ) >= 0 )
			return false;

		m_RecentIds.InsertAt( id, 0 );

		while ( m_RecentIds.Count() > RECENT_MAX )
			m_RecentIds.Remove( m_RecentIds.Count() - 1 );

		return true;
	}

	//! Rebuild the chip strip. Only ever called when the SET of recent
	//! categories changed - a plain re-selection repaints instead.
	protected void RebuildRecentCategories()
	{
		if ( !m_RecentWrapper )
			return;

		//! Drop the script references BEFORE the widgets that own them go, so a
		//! chip is never left alive with a layoutRoot that has been unlinked.
		m_RecentButtons.Clear();

		Widget child = m_RecentWrapper.GetChildren();

		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		if ( m_RecentIds.Count() == 0 )
			return;

		Widget strip = UIActionManager.CreateWrapSpacerCompact( m_RecentWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		//! Each chip takes an equal FRACTION of the row. The icon grid this
		//! replaced was given an exact pixel width computed from the chip count
		//! and an authored cell size, which had to stay in step with both and
		//! spilled out of its band when it did not.
		float chipW = 0.99 / m_RecentIds.Count();

		for ( int i = 0; i < m_RecentIds.Count(); i++ )
		{
			UIActionButton chip = UIActionManager.CreateButton( strip, CategoryLabel( m_RecentIds[i] ), this, "OnClick_RecentChip" );

			if ( !chip )
				continue;

			chip.SetWidth( chipW );
			chip.SetIcon( CategoryIcon( m_RecentIds[i] ) );

			m_RecentButtons.Insert( chip );
		}

		PaintRecentCategories();
	}

	//! Mark whichever chip matches the active category.
	protected void PaintRecentCategories()
	{
		if ( !m_RecentButtons )
			return;

		for ( int i = 0; i < m_RecentButtons.Count(); i++ )
		{
			if ( i >= m_RecentIds.Count() )
				return;

			//! A WASH rather than the solid accent: UIActionButton has no way to
			//! recolour its own label, and the default one is unreadable on a
			//! saturated fill.
			if ( m_RecentIds[i] == m_Module.m_CurrentType )
				m_RecentButtons[i].SetColor( JMTheme.ACCENT_WASH_STRONG );
			else
				m_RecentButtons[i].SetColor( JMTheme.INK_700 );
		}
	}

	//! Dragging a slider must NOT run the full preview refresh: that rebuilds
	//! the property rows, which destroys the very slider being dragged. Only
	//! the health the preview entity is wearing is re-applied.
	void Click_SetHealth( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
		UpdateHealthItemColor();
		RefreshPreviewHealth();
	}

	void Click_SetTemperature( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
		UpdateTemperatureItemColor();
	}

	//! Push the health slider onto the entity already being previewed.
	//!
	//! Same pass the spawn runs: the value lands on the item, on its damage
	//! zones, and on every attachment at the same PERCENTAGE of that
	//! attachment's own max health. Without the cascade a ruined rifle previews
	//! with a pristine optic and magazine hanging off it.
	protected void RefreshPreviewHealth()
	{
		if ( !m_PreviewItem || !m_HealthItem )
			return;

		if ( m_PreviewItem.IsTransport() )
			return;

		m_Module.SetupEntityHealth( m_PreviewItem, m_HealthItem.GetRangeHigh(), -1 );
	}

	void UpdateHealthItemColor()
	{
		//! The row is absent for a class with no health at all, rather than
		//! present and disabled - see RebuildProperties.
		if ( !m_HealthItem )
			return;

		//! The bar is SHADED across itself rather than given one colour: a range
		//! that runs from ruined to pristine is not any single condition, and
		//! painting it one colour could only ever describe one end of it.
		//!
		//! The ramp is sampled densely rather than handed over as the five
		//! condition colours, because the vanilla bands are not evenly spaced -
		//! they break at 30%, 50% and 70% - and evenly spaced stops would put
		//! the colour changes in the wrong places.
		TIntArray stops = new TIntArray;

		for ( int i = 0; i < HEALTH_GRADIENT_STOPS; i++ )
			stops.Insert( HealthColor( ( 1.0 * i ) / ( HEALTH_GRADIENT_STOPS - 1 ) ) );

		m_HealthItem.SetGradient( stops );
		m_HealthItem.SetAlpha( 1.0 );
	}


	//! Samples taken across the health range to build its gradient. 32 puts
	//! every band edge within about 3% of where vanilla draws it.
	static const int HEALTH_GRADIENT_STOPS = 32;

	//! Force a vanilla colour constant opaque.
	//!
	//! The condition and temperature constants in Colors are written with an
	//! alpha byte of ZERO - COLOR_PRISTINE is 0x0040FF00. They are meant for
	//! text colouring, where the alpha is supplied separately. Handed to
	//! Widget.SetColor, which reads plain ARGB, every one of them paints a
	//! fully transparent widget: the colour is applied exactly as asked and
	//! nothing appears.
	//!
	//! The single-value slider hid this by calling SetAlpha(1.0) straight after
	//! SetColor - but that only reaches the one widget SetAlpha touches, so
	//! anything else painted from these constants stays invisible.
	static int Opaque( int color )
	{
		return color | 0xFF000000;
	}

	//! The vanilla condition bands, as a colour.
	protected int HealthColor( float health01 )
	{
		if ( health01 >= 0.7 )
			return Opaque( Colors.COLOR_PRISTINE );

		if ( health01 >= 0.5 )
			return Opaque( Colors.COLOR_WORN );

		if ( health01 >= 0.3 )
			return Opaque( Colors.COLOR_DAMAGED );

		if ( health01 > 0 )
			return Opaque( Colors.COLOR_BADLY_DAMAGED );

		return Opaque( Colors.COLOR_RUINED );
	}

	void UpdateTemperatureItemColor()
	{
		if ( !m_TemperatureItem )
			return;

		int value = m_TemperatureItem.GetCurrent();

		//! Same zero-alpha constants as the condition colours - see Opaque.
		m_TemperatureItem.SetColor( Opaque( ObjectTemperatureState.GetStateData(value).m_Color ) );
		if (ObjectTemperatureState.GetStateData(value).m_State != GameConstants.STATE_NEUTRAL_TEMP)
			m_TemperatureItem.SetFormat( "#STR_COT_FORMAT_DEGREE " + ObjectTemperatureState.GetStateData(value).m_LocalizedName );
		else
			m_TemperatureItem.SetFormat("#STR_COT_FORMAT_DEGREE");

		m_TemperatureItem.SetAlpha( 1.0 );
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

	void Click_SpawnObject( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

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

		RebuildProperties( strSelection );

		if ( m_PreviewItem )
		{
			// Same pass the spawn runs: the value lands on the item, on its
			// damage zones, and on every attachment at the same PERCENTAGE of
			// that attachment's own max health. Without the cascade a ruined
			// rifle previews with a pristine optic and magazine hanging off it.
			// temp stays -1 - the temperature slider is a spawn-time value and
			// has nothing to show in the preview.
			if (m_HealthItem && !m_PreviewItem.IsTransport())
				m_Module.SetupEntityHealth( m_PreviewItem, m_HealthItem.GetRangeHigh(), -1 );

			RefreshPreviewFoodStage();
		}
		else
		{
			m_ItemPreview.Show( false );
		}

		#ifdef COT_DEBUGLOGS
		Print( "-JMObjectSpawnerForm::UpdateItemPreview" );
		#endif
	}

	// -------------------------------------------------------------------------
	//  Properties
	// -------------------------------------------------------------------------

	//! Build only the sliders that mean something for this class.
	//!
	//! The old form kept all three on screen and called Disable() on whichever
	//! did not apply - but a disabled UIAction is painted under a 90%-opaque
	//! panel, so "not applicable" and "unreadable" looked the same. The rows
	//! are built instead, and a row that does not apply is simply absent.
	//!
	//! They are REBUILT rather than shown and hidden because a hidden child
	//! still owns its cell in a GridSpacer, which would leave a gap exactly
	//! where the missing slider used to be.
	protected void RebuildProperties( string classname )
	{
		if ( !m_PropsHost )
			return;

		CaptureProperties();

		//! The food dropdown floats a list panel over the form, and the form is
		//! what dismisses those. Take it off the overlay register before the
		//! widget it belongs to stops existing.
		if ( m_FoodStage )
		{
			m_FoodStage.Close();
			UnregisterOverlay( m_FoodStage );
		}

		if ( m_LiquidType )
		{
			m_LiquidType.Close();
			UnregisterOverlay( m_LiquidType );
		}

		if ( m_AmmoType )
		{
			m_AmmoType.Close();
			UnregisterOverlay( m_AmmoType );
		}

		if ( m_PropsGrid )
		{
			m_PropsGrid.Unlink();
			m_PropsGrid = NULL;
		}

		m_QuantityItem    = NULL;
		m_TemperatureItem = NULL;
		m_HealthItem      = NULL;
		m_FoodStage       = NULL;
		m_LiquidType      = NULL;
		m_AmmoType        = NULL;
		m_Disinfect       = NULL;
		m_PropsNone       = NULL;

		float maxHealth = 0;

		if ( classname != "" )
			maxHealth = MiscGameplayFunctions.GetTypeMaxGlobalHealth( classname );

		bool wantsHealth = ( maxHealth > 0 );
		bool wantsQuantity = false;
		bool wantsTemperature = false;

		float quantityMin = 0;
		float quantityMax = 0;
		float quantityStep = 1;

		array<string> foodStages = new array<string>;
		array<string> liquids    = new array<string>;
		array<string> ammoTypes  = new array<string>;

		bool wantsFoodStage = false;
		bool wantsDisinfect = false;
		bool wantsLiquid    = false;
		bool wantsAmmo      = false;

		ItemBase item = ItemBase.Cast( m_PreviewItem );

		if ( item )
		{
			if ( item.IsLiquidContainer() || ( item.HasFoodStage() && item.CanBeCooked() ) )
				wantsTemperature = true;

			//! Only the stages the class actually declares - offering "Boiled"
			//! for something with no boiled stage would spawn it unchanged and
			//! look like the control did nothing.
			if ( item.HasFoodStage() && item.IsInherited( Edible_Base ) )
			{
				CollectFoodStages( classname, foodStages );
				wantsFoodStage = ( foodStages.Count() > 0 );
			}

			//! Rags, bandages and sewing kits. The engine already answers which
			//! ones, so there is no class list to keep in step.
			wantsDisinfect = item.CanBeDisinfected();

			if ( item.IsLiquidContainer() )
			{
				CollectLiquids( classname, item, liquids );

				//! One entry is the "leave it as configured" row on its own,
				//! which is not worth a control.
				wantsLiquid = ( liquids.Count() > 1 );
			}

			//! What a magazine calls quantity is its ammo count, and it has its
			//! own floor: an ammo pile cannot hold zero rounds and still be a
			//! pile.
			Magazine mag;

			if ( Class.CastTo( mag, item ) )
			{
				if ( mag.GetAmmoMax() > 0 )
				{
					wantsQuantity = true;
					quantityMax   = mag.GetAmmoMax();
					quantityStep  = 1;

					if ( mag.IsAmmoPile() && mag.GetAmmoMax() > 1 )
						quantityMin = 1.0;
				}
			}
			else if ( item.GetQuantityMax() - item.GetQuantityMin() > 0 )
			{
				wantsQuantity = true;
				quantityMin   = item.GetQuantityMin();
				quantityMax   = item.GetQuantityMax();

				if ( item.IsSplitable() )
					quantityStep = 1;
				else
					quantityStep = 0.1;
			}
		}

		//! Which cartridge, for a magazine that lists more than the one.
		//!
		//! Read from the class rather than from the preview entity: ammoItems is
		//! declared on each magazine directly, so the config answers even when
		//! there is no entity to ask, and a class that is not a magazine simply
		//! has none.
		CollectAmmoTypes( classname, ammoTypes );
		wantsAmmo = ( ammoTypes.Count() > 1 );

		//! The switch is only shown where there is something for it to switch.
		//! A class with neither a quantity nor a health bar has no spannable
		//! property at all.
		bool wantsRanges = ( wantsQuantity || wantsHealth );

		ShowRangeToggle( wantsRanges );

		int rows = 0;

		if ( wantsQuantity )    rows++;
		if ( wantsHealth )      rows++;
		if ( wantsTemperature ) rows++;
		if ( wantsFoodStage )   rows++;
		if ( wantsLiquid )      rows++;
		if ( wantsAmmo )        rows++;
		if ( wantsDisinfect )   rows++;

		if ( rows == 0 )
		{
			m_PropsGrid = UIActionManager.CreateGridSpacer( m_PropsHost, 1, 1 );
			m_PropsNone = UIActionManager.CreateText( m_PropsGrid, "#STR_COT_OBJECT_MODULE_NO_PROPERTIES", "" );
			return;
		}

		m_PropsGrid = UIActionManager.CreateGridSpacer( m_PropsHost, rows, 1 );

		if ( wantsQuantity )
		{
			m_QuantityItem = UIActionManager.CreateSliderRange( m_PropsGrid, "#STR_COT_OBJECT_MODULE_QUANTITY", quantityMin, quantityMax, this, "Click_SetQuantity" );
			m_QuantityItem.SetFormat( "#STR_COT_FORMAT_NUMBER" );
			m_QuantityItem.SetStep( quantityStep );
			m_QuantityItem.SetRange( quantityMin + m_QuantityLowPct * ( quantityMax - quantityMin ), quantityMin + m_QuantityHighPct * ( quantityMax - quantityMin ) );

			//! After SetRange, never before: collapsing pins the low bound onto
			//! whatever the high one is at the time.
			m_QuantityItem.SetSingle( !m_RangesEnabled );
		}

		if ( wantsHealth )
		{
			m_HealthItem = UIActionManager.CreateSliderRange( m_PropsGrid, "#STR_COT_OBJECT_MODULE_HEALTH", 0, maxHealth, this, "Click_SetHealth" );
			m_HealthItem.SetFormat( "#STR_COT_FORMAT_NUMBER" );
			m_HealthItem.SetStep( 1 );
			m_HealthItem.SetRange( m_HealthLowPct * maxHealth, m_HealthHighPct * maxHealth );
			m_HealthItem.SetSingle( !m_RangesEnabled );

			UpdateHealthItemColor();
		}

		if ( wantsTemperature )
		{
			m_TemperatureItem = UIActionManager.CreateSlider( m_PropsGrid, "#STR_COT_OBJECT_MODULE_TEMPERATURE", GameConstants.STATE_COLD_LVL_FOUR, GameConstants.STATE_HOT_LVL_FOUR, this, "Click_SetTemperature" );
			m_TemperatureItem.SetSliderWidth( 0.6 );
			m_TemperatureItem.SetStepValue( 1 );
			m_TemperatureItem.SetFormat( "#STR_COT_FORMAT_DEGREE" );
			m_TemperatureItem.SetCurrent( m_TemperatureValue );

			UpdateTemperatureItemColor();
		}

		if ( wantsFoodStage )
		{
			m_FoodStage = UIActionManager.CreateDropdown( m_PropsGrid, "#STR_COT_OBJECT_MODULE_FOODSTAGE", layoutRoot, this, "Click_SetFoodStage", foodStages );
			m_FoodStage.SetWidth( 1.0 );
			m_FoodStage.SetTooltip( "#STR_COT_OBJECT_MODULE_FOODSTAGE_DESC" );

			RegisterOverlay( m_FoodStage );

			//! Carry the chosen stage across the rebuild where the next class
			//! also offers it; fall back to its first stage where it does not.
			int stageIndex = m_FoodStageTypes.Find( m_FoodStageValue );

			if ( stageIndex < 0 )
				stageIndex = 0;

			m_FoodStage.SetSelection( stageIndex, false );
			m_FoodStageValue = m_FoodStageTypes[stageIndex];
		}

		if ( wantsLiquid )
		{
			m_LiquidType = UIActionManager.CreateDropdown( m_PropsGrid, "#STR_COT_OBJECT_MODULE_LIQUID", layoutRoot, this, "Click_SetLiquid", liquids );
			m_LiquidType.SetWidth( 1.0 );
			m_LiquidType.SetTooltip( "#STR_COT_OBJECT_MODULE_LIQUID_DESC" );

			RegisterOverlay( m_LiquidType );

			int liquidIndex = m_LiquidValues.Find( m_LiquidValue );

			if ( liquidIndex < 0 )
				liquidIndex = 0;

			m_LiquidType.SetSelection( liquidIndex, false );
			m_LiquidValue = m_LiquidValues[liquidIndex];
		}

		if ( wantsAmmo )
		{
			m_AmmoType = UIActionManager.CreateDropdown( m_PropsGrid, "#STR_COT_OBJECT_MODULE_AMMOTYPE", layoutRoot, this, "Click_SetAmmoType", ammoTypes );
			m_AmmoType.SetWidth( 1.0 );
			m_AmmoType.SetTooltip( "#STR_COT_OBJECT_MODULE_AMMOTYPE_DESC" );

			RegisterOverlay( m_AmmoType );

			int ammoIndex = m_AmmoValues.Find( m_AmmoValue );

			if ( ammoIndex < 0 )
				ammoIndex = 0;

			m_AmmoType.SetSelection( ammoIndex, false );
			m_AmmoValue = m_AmmoValues[ammoIndex];
		}

		if ( wantsDisinfect )
		{
			m_Disinfect = UIActionManager.CreateToggleSwitch( m_PropsGrid, "#STR_COT_OBJECT_MODULE_DISINFECTED", this, "Click_SetDisinfect", m_DisinfectValue );
			m_Disinfect.SetTooltip( "#STR_COT_OBJECT_MODULE_DISINFECTED_DESC" );
		}

		if ( m_RightScroller )
			m_RightScroller.UpdateScroller();
	}

	//! Fill m_FoodStageTypes and `labels` with the cooking stages `classname`
	//! declares, in enum order.
	//!
	//! A food class lists its stages under Food FoodStages, one subclass per
	//! stage, and there is no script-side query for them - so the config is the
	//! only place that knows which of the six a given item has.
	protected void CollectFoodStages( string classname, out array<string> labels )
	{
		m_FoodStageTypes.Clear();
		labels.Clear();

		if ( classname == "" )
			return;

		TStringArray configNames = { "Raw", "Baked", "Boiled", "Dried", "Burned", "Rotten" };
		TIntArray    stageTypes  = { FoodStageType.RAW, FoodStageType.BAKED, FoodStageType.BOILED, FoodStageType.DRIED, FoodStageType.BURNED, FoodStageType.ROTTEN };
		TStringArray stageLabels = { "#STR_COT_OBJECT_FOODSTAGE_RAW", "#STR_COT_OBJECT_FOODSTAGE_BAKED", "#STR_COT_OBJECT_FOODSTAGE_BOILED", "#STR_COT_OBJECT_FOODSTAGE_DRIED", "#STR_COT_OBJECT_FOODSTAGE_BURNED", "#STR_COT_OBJECT_FOODSTAGE_ROTTEN" };

		for ( int i = 0; i < configNames.Count(); i++ )
		{
			if ( !g_Game.ConfigIsExisting( "CfgVehicles " + classname + " Food FoodStages " + configNames[i] ) )
				continue;

			m_FoodStageTypes.Insert( stageTypes[i] );

			//! The selector prints entry text verbatim, so the key is resolved
			//! here rather than handed over as a key.
			labels.Insert( Widget.TranslateString( stageLabels[i] ) );
		}
	}

	//! Every liquid the game defines, as bit and name. One table, so the list
	//! of what a container accepts and the name of what it already holds can
	//! never disagree.
	static void LiquidTable( out TIntArray bits, out TStringArray keys )
	{
		bits = { LIQUID_WATER, LIQUID_CLEANWATER, LIQUID_FRESHWATER, LIQUID_RIVERWATER, LIQUID_STILLWATER, LIQUID_HOTWATER, LIQUID_SALTWATER, LIQUID_SNOW, LIQUID_VODKA, LIQUID_BEER, LIQUID_GASOLINE, LIQUID_DIESEL, LIQUID_DISINFECTANT, LIQUID_SOLUTION, LIQUID_SALINE, LIQUID_BLOOD_0_P, LIQUID_BLOOD_0_N, LIQUID_BLOOD_A_P, LIQUID_BLOOD_A_N, LIQUID_BLOOD_B_P, LIQUID_BLOOD_B_N, LIQUID_BLOOD_AB_P, LIQUID_BLOOD_AB_N };
		keys = { "#STR_COT_OBJECT_LIQUID_WATER", "#STR_COT_OBJECT_LIQUID_CLEANWATER", "#STR_COT_OBJECT_LIQUID_FRESHWATER", "#STR_COT_OBJECT_LIQUID_RIVERWATER", "#STR_COT_OBJECT_LIQUID_STILLWATER", "#STR_COT_OBJECT_LIQUID_HOTWATER", "#STR_COT_OBJECT_LIQUID_SALTWATER", "#STR_COT_OBJECT_LIQUID_SNOW", "#STR_COT_OBJECT_LIQUID_VODKA", "#STR_COT_OBJECT_LIQUID_BEER", "#STR_COT_OBJECT_LIQUID_GASOLINE", "#STR_COT_OBJECT_LIQUID_DIESEL", "#STR_COT_OBJECT_LIQUID_DISINFECTANT", "#STR_COT_OBJECT_LIQUID_SOLUTION", "#STR_COT_OBJECT_LIQUID_SALINE", "#STR_COT_OBJECT_LIQUID_BLOOD_0_P", "#STR_COT_OBJECT_LIQUID_BLOOD_0_N", "#STR_COT_OBJECT_LIQUID_BLOOD_A_P", "#STR_COT_OBJECT_LIQUID_BLOOD_A_N", "#STR_COT_OBJECT_LIQUID_BLOOD_B_P", "#STR_COT_OBJECT_LIQUID_BLOOD_B_N", "#STR_COT_OBJECT_LIQUID_BLOOD_AB_P", "#STR_COT_OBJECT_LIQUID_BLOOD_AB_N" };
	}

	//! Name for one liquid bit; "Empty" for a container that starts with none.
	protected string LiquidName( int bit )
	{
		TIntArray bits;
		TStringArray keys;
		LiquidTable( bits, keys );

		for ( int i = 0; i < bits.Count(); i++ )
		{
			if ( bits[i] == bit )
				return Widget.TranslateString( keys[i] );
		}

		return Widget.TranslateString( "#STR_COT_OBJECT_LIQUID_NONE" );
	}

	//! "<name> (default)" - the row that changes nothing, named after what the
	//! class actually spawns with. "As configured" told the admin nothing they
	//! could act on; the answer to "what do I get if I leave this alone" is the
	//! whole reason the row is there.
	protected string DefaultLabel( string name )
	{
		return string.Format( Widget.TranslateString( "#STR_COT_OBJECT_DEFAULT_FORMAT" ), name );
	}

	//! Fill m_LiquidValues and `labels` with what this container can hold.
	//!
	//! A container declares its liquids as a BITMASK in liquidContainerType, so
	//! a canteen is not offered petrol and a jerrycan is not offered water.
	protected void CollectLiquids( string classname, ItemBase item, out array<string> labels )
	{
		m_LiquidValues.Clear();
		labels.Clear();

		//! 0 is the module's "leave the item state alone", so the first row is
		//! whatever the class is configured to hold, named. Every list below
		//! then SKIPS that liquid: picking it explicitly and leaving it alone
		//! are the same spawn, and one of them is already row zero.
		int init = item.GetLiquidTypeInit();

		m_LiquidValues.Insert( 0 );
		labels.Insert( DefaultLabel( LiquidName( init ) ) );

		if ( classname == "" )
			return;

		//! A blood container is the one case where what travels is NOT the
		//! liquid bit: the module raises LIQUID_BLOOD_0_P to the power of the
		//! value it is given, so a blood bag wants a 1-based index instead.
		if ( item.IsBloodContainer() )
		{
			TStringArray bloodKeys = { "#STR_COT_OBJECT_LIQUID_BLOOD_0_P", "#STR_COT_OBJECT_LIQUID_BLOOD_0_N", "#STR_COT_OBJECT_LIQUID_BLOOD_A_P", "#STR_COT_OBJECT_LIQUID_BLOOD_A_N", "#STR_COT_OBJECT_LIQUID_BLOOD_B_P", "#STR_COT_OBJECT_LIQUID_BLOOD_B_N", "#STR_COT_OBJECT_LIQUID_BLOOD_AB_P", "#STR_COT_OBJECT_LIQUID_BLOOD_AB_N", "#STR_COT_OBJECT_LIQUID_SALINE" };

			for ( int b = 0; b < bloodKeys.Count(); b++ )
			{
				//! The module reads a blood value as an exponent, so index b
				//! stands for the bit LIQUID_BLOOD_0_P << b.
				if ( Math.Pow( 2, b ) == init )
					continue;

				m_LiquidValues.Insert( b + 1 );
				labels.Insert( Widget.TranslateString( bloodKeys[b] ) );
			}

			return;
		}

		//! From the ITEM, never from "CfgVehicles <class> liquidContainerType".
		//!
		//! That absolute path answers 0 for every container in the game -
		//! measured, not assumed - because liquidContainerType is declared on a
		//! base class and g_Game.ConfigGetInt does not walk the inheritance
		//! chain. The entity's own relative lookup does, and GetLiquidContainerMask
		//! is the result of it.
		int mask = item.GetLiquidContainerMask();

		if ( mask == 0 )
			return;

		TIntArray bits;
		TStringArray keys;
		LiquidTable( bits, keys );

		for ( int i = 0; i < bits.Count(); i++ )
		{
			if ( ( mask & bits[i] ) == 0 )
				continue;

			if ( bits[i] == init )
				continue;

			m_LiquidValues.Insert( bits[i] );
			labels.Insert( Widget.TranslateString( keys[i] ) );
		}
	}

	//! Display name for one ammo pile class, falling back to the classname.
	protected string AmmoPileName( string pile )
	{
		string display;

		if ( g_Game.ConfigGetText( CFG_MAGAZINESPATH + " " + pile + " displayName", display ) && display != "" )
			return Widget.TranslateString( display );

		return pile;
	}

	//! Fill m_AmmoValues and `labels` with the ammo piles this magazine accepts.
	//!
	//! ammoItems is a flat config array, so it has to be read with
	//! ConfigGetTextArray - ConfigGetChildrenCount answers 0 for one of those.
	protected void CollectAmmoTypes( string classname, out array<string> labels )
	{
		m_AmmoValues.Clear();
		labels.Clear();

		TStringArray ammoItems = new TStringArray;

		if ( classname != "" )
			g_Game.ConfigGetTextArray( CFG_MAGAZINESPATH + " " + classname + " ammoItems", ammoItems );

		//! What the magazine loads itself with is a CfgAmmo cartridge, while
		//! everything the admin can pick is an ammo PILE. Match the two up by
		//! the cartridge each pile names, so the first row can say which of the
		//! offered rounds is already the default rather than "as configured".
		string defaultCartridge;

		if ( classname != "" )
			g_Game.ConfigGetText( CFG_MAGAZINESPATH + " " + classname + " ammo", defaultCartridge );

		string defaultName = "";
		string defaultPile = "";

		for ( int d = 0; d < ammoItems.Count(); d++ )
		{
			string pileCartridge;

			if ( !g_Game.ConfigGetText( CFG_MAGAZINESPATH + " " + ammoItems[d] + " ammo", pileCartridge ) )
				continue;

			if ( pileCartridge != defaultCartridge || pileCartridge == "" )
				continue;

			defaultPile = ammoItems[d];
			defaultName = AmmoPileName( defaultPile );
			break;
		}

		//! No pile claims the configured cartridge - name the first one it
		//! accepts, which is what it will be loaded with anyway.
		if ( defaultName == "" && ammoItems.Count() > 0 )
		{
			defaultPile = ammoItems[0];
			defaultName = AmmoPileName( defaultPile );
		}

		if ( defaultName == "" )
			defaultName = Widget.TranslateString( "#STR_COT_OBJECT_AMMO_NONE" );

		m_AmmoValues.Insert( "" );
		labels.Insert( DefaultLabel( defaultName ) );

		for ( int i = 0; i < ammoItems.Count(); i++ )
		{
			string pile = ammoItems[i];

			if ( pile == "" )
				continue;

			//! Already row zero, under its own name.
			if ( pile == defaultPile )
				continue;

			m_AmmoValues.Insert( pile );
			labels.Insert( AmmoPileName( pile ) );
		}
	}

	void Click_SetLiquid( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
	}

	void Click_SetAmmoType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
	}

	void Click_SetFoodStage( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
		RefreshPreviewFoodStage();
	}

	void Click_SetDisinfect( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		//! Nothing to preview - cleanness has no model of its own.
		m_DisinfectValue = action.IsChecked();
	}

	//! Cook the preview entity to the chosen stage so the model in the card is
	//! the model that will be spawned - a burned steak does not look like a raw
	//! one, and that is most of the reason to pick a stage at all.
	protected void RefreshPreviewFoodStage()
	{
		if ( !m_PreviewItem || m_FoodStageValue == 0 )
			return;

		Edible_Base food;

		if ( !Class.CastTo( food, m_PreviewItem ) )
			return;

		if ( !food.GetFoodStage() )
			return;

		food.ChangeFoodStage( m_FoodStageValue );
		food.GetFoodStage().UpdateVisualsEx( true );
	}

	//! Remember what the sliders were set to before they are destroyed.
	//!
	//! Health and quantity are kept as a fraction of their own range: the range
	//! is a property of the class, so a full magazine stays full when the next
	//! class holds a different number of rounds.
	protected void CaptureProperties()
	{
		//! While a slider is collapsed its low bound is PINNED to the value, so
		//! capturing it would overwrite where the range's low handle was last
		//! left - and switching ranges back on could then only ever reopen the
		//! span onto the value itself.
		if ( m_HealthItem && m_HealthItem.GetMax() > 0 )
		{
			if ( !m_HealthItem.IsSingle() )
				m_HealthLowPct = m_HealthItem.GetRangeLow() / m_HealthItem.GetMax();

			m_HealthHighPct = m_HealthItem.GetRangeHigh() / m_HealthItem.GetMax();
		}

		if ( m_QuantityItem )
		{
			float range = m_QuantityItem.GetMax() - m_QuantityItem.GetMin();

			if ( range > 0 )
			{
				if ( !m_QuantityItem.IsSingle() )
					m_QuantityLowPct = ( m_QuantityItem.GetRangeLow() - m_QuantityItem.GetMin() ) / range;

				m_QuantityHighPct = ( m_QuantityItem.GetRangeHigh() - m_QuantityItem.GetMin() ) / range;
			}
		}

		if ( m_TemperatureItem )
		{
			m_TemperatureValue = m_TemperatureItem.GetCurrent();
			m_TemperatureSet   = true;
		}

		if ( m_FoodStage )
		{
			int stageIndex = m_FoodStage.GetSelection();

			if ( stageIndex >= 0 && stageIndex < m_FoodStageTypes.Count() )
				m_FoodStageValue = m_FoodStageTypes[stageIndex];
		}

		if ( m_LiquidType )
		{
			int liquidIndex = m_LiquidType.GetSelection();

			if ( liquidIndex >= 0 && liquidIndex < m_LiquidValues.Count() )
				m_LiquidValue = m_LiquidValues[liquidIndex];
		}

		if ( m_AmmoType )
		{
			int ammoIndex = m_AmmoType.GetSelection();

			if ( ammoIndex >= 0 && ammoIndex < m_AmmoValues.Count() )
				m_AmmoValue = m_AmmoValues[ammoIndex];
		}

		if ( m_Disinfect )
			m_DisinfectValue = m_Disinfect.IsChecked();
	}

	void Click_SetQuantity( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		CaptureProperties();
	}

	//! Switch the two spannable rows between one value and a range.
	//!
	//! An icon button carries no checked state of its own, so the mode is held
	//! here and the button is repainted from it.
	void Click_ToggleRanges( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_RangesEnabled = !m_RangesEnabled;

		PaintRangeToggle();
		ApplyRangeMode();
	}

	//! Tint and label the header switch for the mode it is in.
	protected void PaintRangeToggle()
	{
		if ( !m_RangeToggle )
			return;

		if ( m_RangesEnabled )
		{
			m_RangeToggle.SetColor( JMTheme.ACCENT );
			m_RangeToggle.SetTooltip( "#STR_COT_OBJECT_MODULE_USE_RANGE_ON" );
		}
		else
		{
			m_RangeToggle.SetColor( JMTheme.BUTTON_FILL );
			m_RangeToggle.SetTooltip( "#STR_COT_OBJECT_MODULE_USE_RANGE_OFF" );
		}
	}

	//! Hide the header switch for a class with nothing to switch.
	//!
	//! Hidden rather than disabled: a disabled UIAction is painted under a
	//! 90%-opaque panel, which in a 30px title bar is a grey smudge that reads
	//! as a broken icon rather than as an inapplicable one.
	protected void ShowRangeToggle( bool show )
	{
		if ( !m_RangeToggle )
			return;

		m_RangeToggle.GetLayoutRoot().Show( show );
	}

	//! Put the quantity and health rows into whichever mode the switch is in.
	//!
	//! The rows are changed IN PLACE rather than rebuilt: RebuildProperties
	//! destroys every row in the card, and one of them is the switch that was
	//! just clicked - the engine answers a vanished press target by recentring
	//! the cursor, which is the same reason dragging a slider does not refresh
	//! the preview.
	//!
	//! Reopening a span has to put the low handle BACK: collapsing pinned it
	//! onto the value, and the fraction it was left at is the only record of
	//! where the admin had it.
	protected void ApplyRangeMode()
	{
		if ( m_QuantityItem )
		{
			m_QuantityItem.SetSingle( !m_RangesEnabled );

			if ( m_RangesEnabled )
			{
				float quantitySpan = m_QuantityItem.GetMax() - m_QuantityItem.GetMin();
				float quantityLow  = m_QuantityItem.GetMin() + m_QuantityLowPct * quantitySpan;

				m_QuantityItem.SetRange( quantityLow, m_QuantityItem.GetRangeHigh() );
			}
		}

		if ( m_HealthItem )
		{
			m_HealthItem.SetSingle( !m_RangesEnabled );

			if ( m_RangesEnabled )
			{
				float healthLow = m_HealthLowPct * m_HealthItem.GetMax();

				m_HealthItem.SetRange( healthLow, m_HealthItem.GetRangeHigh() );
			}
		}

		CaptureProperties();
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
		labels.Insert( Widget.TranslateString( label ) );
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

	void Click_CopyList( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

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

				g_Game.CopyToClipboard( clipboardOutput );
				break;

			case COT_ObjectSpawnerMode.COPYLISTTYPES:
				clipboardOutput = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
				clipboardOutput += "<types>\n";

				for ( int j = 0; j < m_ListClasses.Count(); j++ )
				{
					clipboardOutput += "	<type name=\"" + m_ListClasses[j] + "\">\n";
					clipboardOutput += "		<nominal>0</nominal>\n";
					clipboardOutput += "		<lifetime>3888000</lifetime>\n";
					clipboardOutput += "		<restock>0</restock>\n";
					clipboardOutput += "		<min>0</min>\n";
					clipboardOutput += "		<quantmin>-1</quantmin>\n";
					clipboardOutput += "		<quantmax>-1</quantmax>\n";
					clipboardOutput += "		<cost>100</cost>\n";
					clipboardOutput += "		<flags count_in_cargo=\"0\" count_in_hoarder=\"0\" count_in_map=\"1\" count_in_player=\"0\" crafted=\"0\" deloot=\"0\"/>\n";
					clipboardOutput += "	</type>\n";
				}

				clipboardOutput += "</types>";
				g_Game.CopyToClipboard( clipboardOutput );
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
					g_Game.CopyToClipboard( categoryJSON );
				else
					COTCreateLocalAdminNotification( new StringLocaliser( errorMsg ) );
				break;
		#endif
		}
	}

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
		float health   = RangeLowOr( m_HealthItem, -1 );
		float temp     = SliderValueOr( m_TemperatureItem, -1 );
		float quantity = RangeLowOr( m_QuantityItem, -1 );

		m_Module.m_SpawnHealthMax   = RangeHighOr( m_HealthItem, -1 );
		m_Module.m_SpawnQuantityMax = RangeHighOr( m_QuantityItem, -1 );

		//! 0 is the module's "leave the item state alone". The three things it
		//! can mean - liquid type, cooking stage, cleanness - are mutually
		//! exclusive per class, and only one control for it is ever built.
		int itemState = 0;

		if ( m_FoodStage )
			itemState = m_FoodStageValue;
		else if ( m_LiquidType )
			itemState = m_LiquidValue;
		else if ( m_Disinfect && m_Disinfect.IsChecked() )
			itemState = 1;

		//! Module state rather than another parameter, the same way the setup
		//! mode travels. Cleared when the class has no ammo to choose, or a
		//! magazine picked earlier would follow every later spawn.
		if ( m_AmmoType )
			m_Module.m_SpawnAmmoType = m_AmmoValue;
		else
			m_Module.m_SpawnAmmoType = "";

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

		string closestMatch;

		TStringArray configs = new TStringArray;
		configs.Insert( CFG_VEHICLESPATH );
		configs.Insert( CFG_WEAPONSPATH );
		configs.Insert( CFG_MAGAZINESPATH );

		COT_String strSearch = m_Module.m_SearchText;
		bool requireAllKeywords;
		TStringArray keywords = strSearch.KeywordSearch_Prepare(requireAllKeywords);

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

					if ( strSearch != "" && !strNameSearch.KeywordSearchImplEx(strSearch, keywords, requireAllKeywords, closestMatch) )
						continue;

					// Display-name mode changes what the row SHOWS, not what it
					// means. The class name goes into m_ListClasses either way -
					// spawning and the clipboard exports read from there.
					strRowText = strName;

					if (m_Module.m_FilterWithDisplayName)
					{
						if (g_Game.ConfigGetText(strConfigPath + " " + strName + " displayName", strDisplay) && strDisplay != "")
							strRowText = Widget.TranslateString(strDisplay);
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
			ReportExactClassRejection( strSearch, configs );

	#ifdef DIAG
		float elapsed = TickCount(ticks) * 0.0001;
		PrintFormat("UpdateList %1 %2 ms", m_Module.m_SearchText, elapsed);
	#endif

		m_SearchBox.SetTextPreview(closestMatch);
	}

	protected float SliderValueOr( UIActionSlider slider, float fallback )
	{
		if ( !slider )
			return fallback;

		return slider.GetCurrent();
	}

	protected float RangeLowOr( UIActionSliderRange slider, float fallback )
	{
		if ( !slider )
			return fallback;

		return slider.GetRangeLow();
	}

	protected float RangeHighOr( UIActionSliderRange slider, float fallback )
	{
		if ( !slider )
			return fallback;

		return slider.GetRangeHigh();
	}

	string GetCurrentSelection()
	{
		int row = m_ClassList.GetSelectedIndex();

		if ( row < 0 || row >= m_ListClasses.Count() )
			return "";

		return m_ListClasses[row];
	}
}
