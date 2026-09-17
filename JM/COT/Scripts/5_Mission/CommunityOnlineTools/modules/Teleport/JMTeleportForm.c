// =============================================================================
//  JMTeleportForm
//
//  Saved locations on the left; the map and the coordinates that go with it on
//  the right.
//
//  ---------------------------------------------------------------------------
//  WHAT REPLACED WHAT
//
//  The old form was a 400x510 column: a raw TextListboxWidget, a bare "X: / Z:"
//  row, one Teleport button, and an add/remove strip. It showed the module's
//  saved-location half and nothing else - teleporting to a typed coordinate had
//  no control anywhere in the UI, and picking a spot meant opening the separate
//  Map window and double-clicking it.
//
//  The map is now IN here, as UIActionMap - the shared component, not a private
//  MapWidget - so the list, the coordinate fields and the map are three views
//  of one position instead of three features in two windows.
//
//  There is no "selected location" panel any more. A location is acted on from
//  the list itself, by right-clicking it; a panel that existed to hold four
//  buttons for the row already highlighted was a second place to look at the
//  thing you were already looking at.
//
//  Selection is held by NAME, not by object. The module drops and rebuilds its
//  whole location array on every Reload, so a held JMTeleportLocation is a
//  pointer into the previous copy the moment anything is added or removed.
// =============================================================================

class JMTeleportForm: JMFormBase
{
	//! Menu row shown before any real category, meaning "do not filter". Held
	//! as an id rather than as a category name so a category could legally be
	//! called "ALL" without colliding with it.
	static const string CATEGORY_ALL = "*ALL*";

	//! The map layer this form owns. Anything else drawn on the same map - a
	//! player dot, a loot result - lives in its own layer and is not ours to
	//! clear.
	static const string MAP_LAYER = "tp_locations";

	//! Row ids for the per-location context menu.
	static const string MENU_TELEPORT = "teleport";
	static const string MENU_FOCUS    = "focus";
	static const string MENU_EDIT     = "edit";
	static const string MENU_DELETE   = "delete";

	//! Row ids for the menu the map itself opens, on bare ground.
	static const string MENU_MAP_GO   = "map_go";
	static const string MENU_MAP_SAVE = "map_save";

	//! Overlay layers this form owns on the shared map, beside its locations.
	static const string MAP_LAYER_PLAYERS    = "tp_players";
	static const string MAP_LAYER_VEHICLES   = "tp_vehicles";
	static const string MAP_LAYER_MARKERS    = "tp_markers";
	static const string MAP_LAYER_HELICRASH  = "tp_helicrash";
	static const string MAP_LAYER_TOXICZONES = "tp_toxiczones";

	//! What a row IN THE LIST is, not what layer it draws to. The list used to
	//! be locations only; it now mirrors every overlay the filter menu can turn
	//! on, so every row needs to say which kind it is before anything can act
	//! on it correctly (a vehicle has no Edit/Delete, teleporting to it uses a
	//! bare position rather than a saved JMTeleportLocation, ...).
	static const string ROWKIND_LOCATION  = "location";
	static const string ROWKIND_PLAYER    = "player";
	static const string ROWKIND_VEHICLE   = "vehicle";
	static const string ROWKIND_MARKER    = "marker";
	static const string ROWKIND_HELICRASH = "helicrash";
	static const string ROWKIND_TOXICZONE = "toxiczone";

	//! Top-level filter menu rows with sub-menus.
	static const string SUB_LOCATIONS        = "*SUB_LOCATIONS*";
	static const string SUB_VEHICLES         = "*SUB_VEHICLES*";
	static const string SUB_PLAYERS          = "*SUB_PLAYERS*";

	//! Quick toggles inside sub-menus.
	static const string TOGGLE_ALL_LOCATIONS = "*TOGGLE_ALL_LOCATIONS*";
	static const string TOGGLE_ALL_VEHICLES  = "*TOGGLE_ALL_VEHICLES*";
	static const string TOGGLE_ALL_PLAYERS   = "*TOGGLE_ALL_PLAYERS*";

	//! Main menu toggles.
	static const string FILTER_MAP_MARKERS   = "*FILTER_MAP_MARKERS*";
	static const string FILTER_HELI_CRASHES  = "*FILTER_HELI_CRASHES*";
	static const string FILTER_TOXIC_ZONES   = "*FILTER_TOXIC_ZONES*";

	//! Vehicle filter sub-entries.
	static const string FILTER_VEH_CAR       = "*VEH_CAR*";
	static const string FILTER_VEH_BOAT      = "*VEH_BOAT*";
	static const string FILTER_VEH_HELI      = "*VEH_HELI*";
	static const string FILTER_VEH_PLANE     = "*VEH_PLANE*";
	static const string FILTER_VEH_BIKE      = "*VEH_BIKE*";

	//! How often the live overlays are re-read. Players walk; a slower tick
	//! shows them where they were, a faster one buys nothing a map can show.
	static const int   OVERLAY_REFRESH_MS = 1000;

	//! Player dot colours, matching the standalone map so the two agree.
	static const int   COLOR_PLAYER          = 0xFF00FF00;
	static const int   COLOR_PLAYER_ADMIN    = 0xFFFF0000;
	static const int   COLOR_PLAYER_SELECTED = 0xFFFFFF00;

	//! Every icon button in a card header is 30x30, because the header strip is
	//! 30px tall and gives its children no width of their own. The premade
	//! Add/Copy/Paste helpers already size themselves; a raw CreateIconButton
	//! does not, and an unsized one comes out a full row wide.
	static const int   HEADER_BTN_PX  = 30;

	// ---- Left column geometry ----------------------------------------------
	//! Height of the search band. The list takes whatever is left, which is the
	//! one number on this form that has to be computed - see OnResize.
	static const int   SEARCH_H       = 34;

	//! Search box, filter popup button, refresh. Fractions rather than a fixed
	//! pixel size on the buttons: a WrapSpacer wraps the moment its children
	//! total more than the row, and mixing units makes that edge impossible to
	//! reason about. They sum to less than 1.
	static const float SEARCH_ROW_W   = 0.74;
	static const float FILTER_BTN_W   = 0.12;
	static const float REFRESH_BTN_W  = 0.12;

	//! Seed height of the map, in layout pixels. Only what it is built at -
	//! LayoutRightPane gives it the real height as soon as the form knows one.
	static const int   MAP_HEIGHT     = 460;

	//! What a card adds around its content: the header strip, the gap under
	//! it, and the matching margin at the bottom. Mirrors UIActionCard's own
	//! HEADER_HEIGHT + CONTENT_TOP_PADDING + BOTTOM_PADDING, and is what turns
	//! a band height into the map height that fills it.
	static const int   CARD_CHROME    = 44;

	//! Height the coordinates band opens at, before the card it holds has laid
	//! out and can be measured. See MeasureCoordsCard.
	static const int   COORDS_H_SEED  = 110;

	//! Floor under the map, so a very short window shrinks the coordinates
	//! band off the bottom rather than crushing the map to nothing.
	static const int   MAP_MIN_H      = 160;

	//! Where the right pane sits when the map is not expanded. Must match
	//! panel_right and panel_right_disable in the layout - expanding stretches
	//! them over the left column and has to be able to put them back.
	static const float RIGHT_X        = 0.35;
	static const float RIGHT_W        = 0.65;
	static const float DISABLE_X      = 0.345;
	static const float DISABLE_W      = 0.655;

	//! Picker row that turns the category control into a text box. Matched by
	//! index, not by text, so a category legitimately named after this row's
	//! (localized) label would still pick its own row.
	static const string CATEGORY_NEW_ROW = "#STR_COT_TELEPORT_MODULE_NEW_CATEGORY_ROW";

	// ---- Panels ------------------------------------------------------------
	//! m_LeftPanel, m_RightPanel, m_RightContent and m_RightPanelDisable are
	//! JMFormBase's - the base declares the structural panels every archetype
	//! shares. Redeclaring them here shadows the fields the base's own walks
	//! read.
	protected Widget m_SearchWrapper;
	protected Widget m_ListWrapper;

	//! The right pane is two bands rather than a scroller: the coordinates sit
	//! on the floor and the map takes everything above them, so the map grows
	//! with the window instead of being something to scroll past.
	protected Widget m_MapBand;
	protected Widget m_CoordsBand;
	protected Widget m_PanelSplit;

	// ---- Left column -------------------------------------------------------
	protected UIActionSearchBox   m_Filter;
	protected UIActionImageButton m_FilterButton;
	protected UIActionImageButton m_BtnRefresh;
	protected UIActionItemList    m_LocationList;
	protected UIActionContextMenu m_CategoryMenu;
	protected UIActionContextMenu m_RowMenu;
	protected UIActionContextMenu m_MapMenu;

	//! Where the map was right-clicked. Captured when the menu opens so that
	//! panning behind it cannot move the spot the menu is about.
	protected vector m_MapMenuWorldPos;

	//! Strongly-typed list items array replacing legacy parallel arrays.
	protected ref array< ref JMTeleportRowData > m_ListItems = new array< ref JMTeleportRowData >;

	// ---- Right pane --------------------------------------------------------
	protected UIActionMap            m_Map;
	protected UIActionCard           m_CoordsCard;
	protected UIActionImageButton    m_BtnExpandMap;
	protected UIActionEditableVector m_InputCoords;
	protected UIActionButton         m_BtnGoCoords;
	protected UIActionImageButton    m_BtnUseMyPos;
	protected UIActionImageButton    m_BtnCopyCoords;
	protected UIActionImageButton    m_BtnPasteCoords;
	protected UIActionImageButton    m_BtnSaveHere;

	// ---- Save / edit popup -------------------------------------------------
	protected Widget                 m_PopupShade;
	protected Widget                 m_PopupBody;
	protected UIActionCard           m_PopupCard;
	protected UIActionEditableText   m_PopupName;
	protected UIActionText           m_PopupPosition;
	protected UIActionButton         m_PopupConfirm;

	//! The category control is two controls, one showing at a time: a picker
	//! over the categories that exist, and a text box for a category that does
	//! not exist yet. m_PopupNewIndex is the picker row that swaps between
	//! them, and m_PopupNewCategory is which of the two is currently showing.
	protected UIActionDropdown       m_PopupCategory;
	protected UIActionEditableText   m_PopupCategoryNew;
	protected UIActionImageButton    m_PopupCategoryBack;
	protected int                    m_PopupNewIndex;
	protected bool                   m_PopupNewCategory;

	//! Position the popup will write. Captured when it opens so that panning
	//! the map behind it cannot move the thing being saved.
	protected vector m_PopupWorldPos;

	//! Name of the location the popup is editing, or "" when it is saving a new
	//! one. This is what makes it one popup rather than two.
	protected string m_PopupEditing;

	// ---- State -------------------------------------------------------------
	protected ref TStringArray m_Categories;

	//! Categories currently switched OFF, rather than the one that is on: a
	//! new category shows up the moment it exists, which is what an admin who
	//! has hidden two of eight expects.
	protected ref TStringArray m_HiddenCategories = new TStringArray;

	protected bool m_ShowPlayers = true;
	protected ref TStringArray m_HiddenPlayerRoles = new TStringArray;

	protected bool m_ShowMapMarkers = true;
	protected bool m_ShowHeliCrashes;
	protected bool m_ShowToxicZones;

	//! Locations start hidden - the first RefreshCategories() call fills
	//! m_HiddenCategories with every real category it finds, then leaves it
	//! alone so a later toggle isn't overwritten on the next refresh.
	protected bool m_CategoriesInitialized;

	protected UIActionContextMenu m_SubMenu;
	protected string m_CurrentSubMenuType;

	//! Bitmask of JMVT_* vehicle types currently shown on the overlay - 0
	//! means the whole vehicle layer is off, same as the old plain bool did,
	//! but broken down per type the way Vehicle Manager's own filter is.
	protected int m_VehicleTypeFilter = JMVT_ALL;

	protected JMVehiclesModule m_VehiclesModule;

	//! Marker id -> metadata for whatever the overlay last drew, so a map
	//! hover can look a vehicle up without re-walking the server list.
	protected ref map<string, ref JMVehicleMetaData> m_VehicleByMarkerId = new map<string, ref JMVehicleMetaData>;

	//! Same hover card the Vehicle Manager map uses - built lazily so a
	//! session that never hovers a vehicle here never pays for it.
	protected ref JMVehiclesHoverInfo m_VehicleHoverInfo;

	//! Last size the form was given, so the pane can be laid out again after
	//! something other than a resize changes it.
	protected float m_FormW;
	protected float m_FormH;

	//! Measured height of the coordinates card. See MeasureCoordsCard.
	protected float m_CoordsH = COORDS_H_SEED;

	protected bool m_MapExpanded;

	//! Key and kind of the selected row. See the class note on why the location
	//! case is not held as the location object; the two together are what
	//! survives a rebuild (search keystroke, filter toggle, live refresh) and
	//! are matched back to a row index by FindRow().
	protected string m_SelectedName;
	protected string m_SelectedKind = ROWKIND_LOCATION;

	protected string m_SearchFilter;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMTeleportModule m_Module;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	// -------------------------------------------------------------------------
	//  OnInit
	// -------------------------------------------------------------------------

	override void OnInit()
	{
		m_LeftPanel         = layoutRoot.FindAnyWidget( "panel_left" );
		m_RightPanel        = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightContent      = layoutRoot.FindAnyWidget( "panel_right_content" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );

		m_SearchWrapper     = layoutRoot.FindAnyWidget( "tp_search_wrapper" );
		m_ListWrapper       = layoutRoot.FindAnyWidget( "tp_list_wrapper" );

		m_PanelSplit        = layoutRoot.FindAnyWidget( "panel_split" );
		m_MapBand           = layoutRoot.FindAnyWidget( "tp_map_band" );
		m_CoordsBand        = layoutRoot.FindAnyWidget( "tp_coords_band" );

		m_PopupShade        = layoutRoot.FindAnyWidget( "tp_popup_shade" );
		m_PopupBody         = layoutRoot.FindAnyWidget( "tp_popup_body" );

		InitWidgetsLeft();
		InitWidgetsRight();
		InitPopup();
	}

	//! The list reaches as far down the left column as the window allows, which
	//! is the only measurement on this form. h is real content-height layout
	//! pixels handed over by JMWindowBase, not a widget's own GetScreenSize -
	//! that reads zero before the first render.
	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		m_FormW = w;
		m_FormH = h;

		PinBand( m_SearchWrapper, 0, SEARCH_H );

		if ( m_ListWrapper && h > SEARCH_H )
		{
			float listH = h - SEARCH_H;

			PinBand( m_ListWrapper, SEARCH_H, listH );

			//! The list is told the height rather than measuring it. Same
			//! number, one source - and a widget cannot measure itself on the
			//! frame it is built.
			if ( m_LocationList )
				m_LocationList.SetViewportHeight( listH );
		}

		MeasureCoordsCard();
		LayoutRightPane();

		//! The coordinates card isn't rendered yet on the OnResize that opens
		//! the form, so MeasureCoordsCard bails on a zero GetScreenSize and
		//! LayoutRightPane sizes the map off the COORDS_H_SEED guess instead
		//! of the card's real height - which is why the map used to stay
		//! wrong-sized until something else forced a second OnResize. Queue
		//! one remeasure a frame later, once the card has actually rendered.
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( DeferredRemeasureCoords );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( DeferredRemeasureCoords, 50, false );
	}

	protected void DeferredRemeasureCoords()
	{
		MeasureCoordsCard();
		LayoutRightPane();
	}

	//! Place one band: exact y, exact height, full width.
	protected void PinBand( Widget band, float y, float height )
	{
		if ( !band )
			return;

		band.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		band.SetPos( 0, y );
		band.SetSize( 1, height );
	}

	protected void InitWidgetsLeft()
	{
		if ( m_SearchWrapper )
		{
			//! A WrapSpacer packs left to right in creation order, so this IS
			//! the order on screen: refresh, then the box, then the filter.
			Widget searchRow = UIActionManager.CreateWrapSpacerCompact( m_SearchWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			m_BtnRefresh = UIActionManager.CreateIconButton( searchRow, JMConstants.Lucide( "refresh-cw" ), this, "Click_Refresh" );
			m_BtnRefresh.SetWidth( REFRESH_BTN_W );
			m_BtnRefresh.SetTooltip( "#STR_COT_GENERIC_REFRESH" );

			m_Filter = UIActionManager.CreateSearchBox( searchRow, this, "Type_UpdateList", "#STR_COT_TELEPORT_MODULE_FILTER" );
			m_Filter.SetWidth( SEARCH_ROW_W );

			m_FilterButton = UIActionManager.CreateIconButton( searchRow, JMConstants.Lucide( "list-filter" ), this, "OnClick_CategoryFilter" );
			m_FilterButton.SetWidth( FILTER_BTN_W );
			m_FilterButton.SetTooltip( "#STR_COT_TELEPORT_MODULE_FILTER_BY_CATEGORY_TOOLTIP" );
		}

		//! COT's own list, not TextListboxWidget. That widget draws in the
		//! engine's debug style and builds one widget per row; this one keeps a
		//! pool of the rows that fit and moves the data through it.
		m_LocationList = UIActionManager.CreateItemList( m_ListWrapper, this, "OnClick_LocationList" );
		m_LocationList.SetEmptyText( "#STR_COT_TELEPORT_MODULE_NO_LOCATIONS_MATCH" );
	}

	protected void InitWidgetsRight()
	{
		if ( !m_MapBand || !m_CoordsBand )
			return;

		BuildMapCard( m_MapBand );
		BuildCoordinatesCard( m_CoordsBand );

		RegisterRightPermissions();
	}

	protected void BuildMapCard( Widget parent )
	{
		UIActionCard card = UIActionManager.CreateCard( parent, "#STR_COT_TELEPORT_MODULE_MAP_CARD_TITLE" );

		m_BtnExpandMap = UIActionManager.CreateIconButton( card.GetHeaderActions(), JMConstants.Lucide( "maximize-2" ), this, "Click_ExpandMap" );
		m_BtnExpandMap.SetFixedSize( HEADER_BTN_PX, HEADER_BTN_PX );
		m_BtnExpandMap.SetTooltip( "#STR_COT_TELEPORT_MODULE_EXPAND_MAP_TOOLTIP" );

		UIActionImageButton centerBtn = UIActionManager.CreateIconButton( card.GetHeaderActions(), JMConstants.Lucide( "locate-fixed" ), this, "Click_CenterOnMe" );
		centerBtn.SetFixedSize( HEADER_BTN_PX, HEADER_BTN_PX );
		centerBtn.SetTooltip( "#STR_COT_TELEPORT_MODULE_CENTER_ON_ME_TOOLTIP" );

		m_Map = UIActionManager.CreateMap( card.GetContent(), this, "OnClick_Map", MAP_HEIGHT );
	}

	// =========================================================================
	//  Right pane layout
	// =========================================================================

	//! Coordinates on the floor, map above them.
	//!
	//! Both bands are placed by hand rather than stacked in a spacer, because
	//! the map has to be told a height - it has no content height of its own -
	//! and that height is whatever the coordinates card does not need.
	protected void LayoutRightPane()
	{
		if ( m_FormH <= 0 )
			return;

		float coordsH = m_CoordsH;

		//! Expanded hides the coordinates outright: the point of expanding is
		//! the map, and a band pinned to the floor of a full-window map is the
		//! one place it would be in the way.
		if ( m_MapExpanded )
			coordsH = 0;

		float mapH = m_FormH - coordsH;

		if ( mapH < MAP_MIN_H )
			mapH = MAP_MIN_H;

		PinBand( m_MapBand, 0, mapH );

		if ( m_CoordsBand )
		{
			m_CoordsBand.Show( !m_MapExpanded );
			PinBand( m_CoordsBand, mapH, coordsH );
		}

		if ( m_Map )
			m_Map.SetHeight( mapH - CARD_CHROME );
	}

	//! Read the height the coordinates card settled on.
	//!
	//! The card sizes itself to its rows, and no widget can measure itself on
	//! the frame it is built, so the band opens at a seed height and is
	//! corrected from the real one. Measured rather than hard-coded because a
	//! row added to that card would otherwise silently overlap the map.
	protected void MeasureCoordsCard()
	{
		if ( !m_CoordsCard )
			return;

		float cw;
		float ch;
		m_CoordsCard.GetLayoutRoot().GetScreenSize( cw, ch );

		if ( ch < 1 || Math.AbsFloat( ch - m_CoordsH ) < 1 )
			return;

		m_CoordsH = ch;
	}

	void Click_ExpandMap( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		m_MapExpanded = !m_MapExpanded;

		ApplyMapExpanded();
	}

	//! Expanded means the map takes the whole window, not a bigger slice of the
	//! right pane: the left column is hidden and the right pane is stretched
	//! over it. Nothing is reparented, so the map keeps the widget tree it was
	//! built in - a MapWidget moved between parents comes back blank.
	protected void ApplyMapExpanded()
	{
		if ( m_LeftPanel )
			m_LeftPanel.Show( !m_MapExpanded );

		if ( m_PanelSplit )
			m_PanelSplit.Show( !m_MapExpanded );

		if ( m_RightPanel )
		{
			if ( m_MapExpanded )
			{
				m_RightPanel.SetPos( 0, 0 );
				m_RightPanel.SetSize( 1, 1 );
			}
			else
			{
				m_RightPanel.SetPos( RIGHT_X, 0 );
				m_RightPanel.SetSize( RIGHT_W, 1 );
			}
		}

		//! The permission shade covers the right pane, so it has to move with
		//! it or an admin without the permission would see an uncovered map.
		if ( m_RightPanelDisable )
		{
			if ( m_MapExpanded )
			{
				m_RightPanelDisable.SetPos( 0, 0 );
				m_RightPanelDisable.SetSize( 1, 1 );
			}
			else
			{
				m_RightPanelDisable.SetPos( DISABLE_X, 0 );
				m_RightPanelDisable.SetSize( DISABLE_W, 1 );
			}
		}

		if ( m_BtnExpandMap )
		{
			if ( m_MapExpanded )
			{
				m_BtnExpandMap.SetImage( JMConstants.Lucide( "minimize-2" ) );
				m_BtnExpandMap.SetTooltip( "#STR_COT_TELEPORT_MODULE_SHRINK_MAP_TOOLTIP" );
			}
			else
			{
				m_BtnExpandMap.SetImage( JMConstants.Lucide( "maximize-2" ) );
				m_BtnExpandMap.SetTooltip( "#STR_COT_TELEPORT_MODULE_EXPAND_MAP_TOOLTIP" );
			}
		}

		LayoutRightPane();
	}

	//! Teleport to a typed, clicked or pasted coordinate - the module capability
	//! the old form never exposed at all.
	protected void BuildCoordinatesCard( Widget parent )
	{
		UIActionCard card = UIActionManager.CreateCard( parent, "#STR_COT_TELEPORT_MODULE_COORDINATES_CARD_TITLE" );

		m_CoordsCard = card;

		//! Header order is the order these are reached for: fill from me, save
		//! what is in the fields, then the clipboard pair.
		m_BtnUseMyPos = UIActionManager.CreateIconButton( card.GetHeaderActions(), JMConstants.Lucide( "map-pin" ), this, "Click_UseMyPosition" );
		m_BtnUseMyPos.SetFixedSize( HEADER_BTN_PX, HEADER_BTN_PX );
		m_BtnUseMyPos.SetTooltip( "#STR_COT_TELEPORT_MODULE_USE_MY_POSITION_TOOLTIP" );

		m_BtnSaveHere = card.AddAddButton( this, "Click_OpenSavePopup", "#STR_COT_TELEPORT_MODULE_SAVE_HERE_TOOLTIP" );

		//! Clipboard in and out, in the same "<x, y, z>" form the player form
		//! writes, so a position can be carried between the two.
		m_BtnCopyCoords  = card.AddCopyButton( this, "Click_CopyCoordinates", "#STR_COT_TELEPORT_MODULE_COPY_COORDS_TOOLTIP" );
		m_BtnPasteCoords = card.AddPasteButton( this, "Click_PasteCoordinates", "#STR_COT_TELEPORT_MODULE_PASTE_COORDS_TOOLTIP" );

		Widget body = UIActionManager.CreateGridSpacer( card.GetContent(), 2, 1 );

		m_InputCoords = UIActionManager.CreateEditableVector( body, "#STR_COT_GENERIC_POSITION", this );
		m_InputCoords.SetTooltip( "#STR_COT_TELEPORT_MODULE_INPUT_COORDS_TOOLTIP" );

		m_BtnGoCoords = UIActionManager.CreateButton( body, "#STR_COT_TELEPORT_MODULE_GO", this, "Click_GoToCoordinates" );
		m_BtnGoCoords.SetColor( JMTheme.SUCCESS_FILL );
		m_BtnGoCoords.SetTooltip( "#STR_COT_TELEPORT_MODULE_GO_COORDS_TOOLTIP" );
	}

	// =========================================================================
	//  Save / edit popup
	//
	//  One popup for both jobs. Saving and editing ask for the same two things,
	//  and the only difference is whether there is an existing location to write
	//  back to - which is what m_PopupEditing records.
	// =========================================================================

	protected void InitPopup()
	{
		if ( !m_PopupBody )
			return;

		m_PopupCard = UIActionManager.CreateCard( m_PopupBody, "#STR_COT_TELEPORT_MODULE_SAVE_LOCATION" );

		Widget body = UIActionManager.CreateGridSpacer( m_PopupCard.GetContent(), 4, 1 );

		m_PopupPosition = UIActionManager.CreateLabeledValue( body, "#STR_COT_GENERIC_POSITION", "" );

		//! Both rows are built the same way on purpose. CreateLabeledInput
		//! splits its row 50/50, which puts one box at the far right and the
		//! one below it a quarter in.
		Widget nameRow = UIActionManager.CreateWrapSpacer( body, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionText nameLabel = UIActionManager.CreateText( nameRow, "#STR_COT_GENERIC_NAME" );
		nameLabel.SetWidth( 0.24 );
		nameLabel.SetLabelVAlign( UIActionVAlign.CENTER );

		m_PopupName = UIActionManager.CreateEditableText( nameRow, "", this );
		m_PopupName.SetWidth( 0.74 );
		m_PopupName.SetWidgetWidth( m_PopupName.GetLabelWidget(), 0.0 );
		m_PopupName.SetWidgetWidth( m_PopupName.GetEditBoxWidget(), 1.0 );

		Widget catRow = UIActionManager.CreateWrapSpacer( body, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionText catLabel = UIActionManager.CreateText( catRow, "#STR_COT_GENERIC_CATEGORY" );
		catLabel.SetWidth( 0.24 );
		catLabel.SetLabelVAlign( UIActionVAlign.CENTER );

		//! The list is anchored to the shade, not to the window root. The shade
		//! is the popup's own layer and draws above the form; a list anchored
		//! any lower would open behind the card it belongs to.
		m_PopupCategory = UIActionManager.CreateDropdown( catRow, "", m_PopupShade, this, "PopupCategory_OnChange" );
		m_PopupCategory.SetWidth( 0.74 );

		//! The dropdown layout keeps the left 30% for its own label, which this
		//! one does not use - the row already has a label of its own. Pull the
		//! field over that gap so the control fills what it was given.
		Widget catField = m_PopupCategory.GetLayoutRoot().FindAnyWidget( "action_field" );
		if ( catField )
		{
			m_PopupCategory.SetWidgetPosition( catField, 0.0 );
			m_PopupCategory.SetWidgetWidth( catField, 1.0 );
		}

		RegisterOverlay( m_PopupCategory );

		m_PopupCategoryNew = UIActionManager.CreateEditableText( catRow, "", this );
		m_PopupCategoryNew.SetWidth( 0.60 );
		m_PopupCategoryNew.SetWidgetWidth( m_PopupCategoryNew.GetLabelWidget(), 0.0 );
		m_PopupCategoryNew.SetWidgetWidth( m_PopupCategoryNew.GetEditBoxWidget(), 1.0 );
		m_PopupCategoryNew.SetTooltip( "#STR_COT_TELEPORT_MODULE_NEW_CATEGORY_NAME_TOOLTIP" );

		m_PopupCategoryBack = UIActionManager.CreateIconButton( catRow, JMConstants.Lucide( "list" ), this, "Click_PopupCategoryBack" );
		m_PopupCategoryBack.SetFixedSize( HEADER_BTN_PX, HEADER_BTN_PX );
		m_PopupCategoryBack.SetTooltip( "#STR_COT_TELEPORT_MODULE_PICK_EXISTING_CATEGORY_TOOLTIP" );

		SetCategoryMode( false );

		UIActionButton cancel;
		UIActionManager.CreateButtonPair( body, "#STR_COT_GENERIC_CANCEL", this, "Click_PopupCancel", cancel, "#STR_COT_GENERIC_SAVE", this, "Click_PopupConfirm", m_PopupConfirm );

		m_PopupConfirm.SetColor( JMTheme.SUCCESS_FILL );

		HidePopup();
	}

	//! Open the popup over whatever the coordinate fields hold.
	void Click_OpenSavePopup( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION + ".Add" ) )
			return;

		ShowPopup( "", m_InputCoords.GetValue(), "", "" );
	}

	protected void ShowPopup( string editing, vector position, string name, string category )
	{
		if ( !m_PopupShade )
			return;

		m_PopupEditing  = editing;
		m_PopupWorldPos = position;

		if ( m_PopupCard )
		{
			//! SetLabel, not SetTitle - the card's header text is its label.
			if ( editing == "" )
				m_PopupCard.SetLabel( "#STR_COT_TELEPORT_MODULE_SAVE_LOCATION" );
			else
				m_PopupCard.SetLabel( Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_EDIT_LOCATION_PREFIX" ) + editing );
		}

		//! The position row is read-only text, and greyed while editing: the
		//! edit path renames a location, it does not move it.
		if ( m_PopupPosition )
			m_PopupPosition.SetText( FormatVector( position ) );

		m_PopupName.SetText( name );

		RebuildPopupCategories( category );

		m_PopupShade.Show( true );
	}

	protected void HidePopup()
	{
		if ( m_PopupShade )
			m_PopupShade.Show( false );

		m_PopupEditing = "";
	}

	void Click_PopupCancel( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		HidePopup();
	}

	//! Show one of the two category controls and hide the other.
	protected void SetCategoryMode( bool newCategory )
	{
		m_PopupNewCategory = newCategory;

		if ( m_PopupCategory )
			m_PopupCategory.SetVisible( !newCategory );

		if ( m_PopupCategoryNew )
			m_PopupCategoryNew.SetVisible( newCategory );

		if ( m_PopupCategoryBack )
			m_PopupCategoryBack.SetVisible( newCategory );

		if ( m_PopupBody )
			m_PopupBody.Update();
	}

	//! Fill the picker with the categories that exist plus the row that swaps
	//! it for a text box, and select `select`.
	//!
	//! Rebuilt on every open rather than once: a category this popup creates
	//! has to be pickable the next time it opens.
	protected void RebuildPopupCategories( string select )
	{
		int i;
		int added       = 0;
		int selectIndex = -1;

		if ( !m_PopupCategory )
			return;

		m_PopupCategory.ClearEntries();

		if ( m_Categories )
		{
			for ( i = 0; i < m_Categories.Count(); i++ )
			{
				if ( m_Categories[i] == CATEGORY_ALL )
					continue;

				if ( m_Categories[i] == select )
					selectIndex = added;

				m_PopupCategory.AddEntry( m_Categories[i] );

				added++;
			}
		}

		m_PopupNewIndex = added;
		m_PopupCategory.AddEntry( Widget.TranslateString( CATEGORY_NEW_ROW ), JMConstants.Lucide( "plus" ), JMTheme.ACCENT );

		//! Open straight in the text box when there is nothing to pick, or when
		//! the category being edited is not one of the rows - landing on a row
		//! that says the wrong thing would silently recategorise the location
		//! on save.
		if ( added == 0 || ( select != "" && selectIndex < 0 ) )
		{
			SetCategoryMode( true );
			m_PopupCategoryNew.SetText( select );
			return;
		}

		SetCategoryMode( false );
		m_PopupCategoryNew.SetText( "" );

		if ( selectIndex < 0 )
			selectIndex = 0;

		m_PopupCategory.SetSelection( selectIndex, false );
	}

	//! Whichever of the two category controls is showing.
	protected string PopupCategoryText()
	{
		if ( m_PopupNewCategory )
			return m_PopupCategoryNew.GetText();

		if ( !m_PopupCategory )
			return "";

		if ( m_PopupCategory.GetSelection() == m_PopupNewIndex )
			return "";

		return m_PopupCategory.GetSelectedText();
	}

	void Click_PopupCategoryBack( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		SetCategoryMode( false );

		//! Step off the "new" row on the way back. SetSelection ignores a
		//! selection that is already current, so leaving it there would make
		//! the second visit to the text box impossible.
		if ( m_PopupCategory && m_PopupNewIndex > 0 )
			m_PopupCategory.SetSelection( 0, false );
	}

	void Click_PopupConfirm( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		string name = m_PopupName.GetText();

		if ( name == "" )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_TELEPORT_MODULE_TELEPORT_ERROR_NONAME" ) );
			return;
		}

		string category = PopupCategoryText();

		if ( category == "" )
		{
			COTCreateLocalAdminNotification( new StringLocaliser( "#STR_COT_TELEPORT_MODULE_PICK_OR_NAME_CATEGORY" ) );
			return;
		}

		if ( m_PopupEditing == "" )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION + ".Add" ) )
				return;

			m_Module.AddLocation( name, category, m_PopupWorldPos );

			COTCreateLocalAdminNotification( new StringLocaliser( Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_ADDED_LOCATION_PREFIX" ) + name + Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_TO_CATEGORY_PREFIX" ) + category ) );
		}
		else
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_EDIT ) )
				return;

			JMTeleportLocation target = FindLocation( m_PopupEditing );

			if ( !target )
			{
				HidePopup();
				return;
			}

			m_Module.EditLocation( target, name, category );

			COTCreateLocalAdminNotification( new StringLocaliser( Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_EDITED_LOCATION_PREFIX" ) + m_PopupEditing ) );

			//! Follow the rename, so the row that was selected is still the
			//! selected row once the reload lands.
			m_SelectedName = name;
		}

		HidePopup();

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION + ".Refresh" ) )
			return;

		m_Module.Reload();
	}

	//! The picker only has to notice one row: the one that is not a category.
	void PopupCategory_OnChange( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || !m_PopupCategory )
			return;

		if ( m_PopupCategory.GetSelection() != m_PopupNewIndex )
			return;

		SetCategoryMode( true );

		m_PopupCategoryNew.SetText( "" );
	}

	// =========================================================================
	//  Left pane - the location list
	// =========================================================================

	protected void RebuildList()
	{
		if ( !m_LocationList )
			return;

		m_ListItems.Clear();

		TStringArray rowLabels = new TStringArray;
		TStringArray rowSubs   = new TStringArray;

		if ( !m_Module || !m_Module.IsLoaded() )
		{
			m_LocationList.SetEmptyText( "#STR_COT_TELEPORT_MODULE_LOADING" );
			m_LocationList.SetItems( rowLabels, rowSubs );
			return;
		}

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION ) )
		{
			m_LocationList.SetEmptyText( "#STR_COT_TELEPORT_MODULE_NO_VIEW_PERMISSION" );
			m_LocationList.SetItems( rowLabels, rowSubs );
			return;
		}

		array< ref JMTeleportLocation > locations = m_Module.GetLocations();
		if ( !locations )
		{
			m_LocationList.SetItems( rowLabels, rowSubs );
			return;
		}

		COT_String filter = m_SearchFilter;
		bool requireAllKeywords;
		TStringArray keywords = filter.KeywordSearch_Prepare( requireAllKeywords );
		string closestMatch;

		if ( m_Map )
			m_Map.ClearLayer( MAP_LAYER );

		for ( int i = 0; i < locations.Count(); i++ )
		{
			JMTeleportLocation location = locations[i];
			if ( !location )
				continue;

			if ( IsCategoryHidden( location.Type ) )
				continue;

			if ( m_SearchFilter != "" )
			{
				COT_String name = location.Name;
				name.ToLower();

				if ( !name.KeywordSearchImplEx( m_SearchFilter, keywords, requireAllKeywords, closestMatch ) )
					continue;
			}

			rowLabels.Insert( location.Name );

			//! The category rides along as the row's subtitle, which is what
			//! the old category column was for.
			rowSubs.Insert( location.Type );

			AddRow( location.Name, ROWKIND_LOCATION, location.Position, MarkerIdFor( location.Name ) );

			//! The map shows exactly what the list shows. Filtering the list and
			//! leaving the map covered in the rest would make the two disagree
			//! about what "the locations" are.
			if ( m_Map )
				m_Map.AddMarker( MarkerIdFor( location.Name ), location.Position, location.Name, MarkerColorFor( location.Type ), MarkerIconFor( location.Type ), MAP_LAYER );
		}

		//! Every overlay the filter menu can turn on gets folded into the same
		//! list, under the same search - it used to be map-only, which meant
		//! the one place an admin actually reads names had nothing to show for
		//! "show players" beyond dots nobody could tell apart without hovering.
		if ( m_ShowPlayers )
			AppendPlayerRows( rowLabels, rowSubs, keywords, requireAllKeywords );

		if ( m_VehicleTypeFilter != 0 )
			AppendVehicleRows( rowLabels, rowSubs, keywords, requireAllKeywords );

		if ( m_ShowMapMarkers )
			AppendMapMarkerRows( rowLabels, rowSubs, keywords, requireAllKeywords );

		if ( m_ShowHeliCrashes )
			AppendHeliCrashRows( rowLabels, rowSubs, keywords, requireAllKeywords );

		if ( m_ShowToxicZones )
			AppendToxicZoneRows( rowLabels, rowSubs, keywords, requireAllKeywords );

		m_LocationList.SetEmptyText( "#STR_COT_TELEPORT_MODULE_NO_LOCATIONS_MATCH" );
		m_LocationList.SetItems( rowLabels, rowSubs );

		//! The list is rebuilt on every keystroke, on every filter toggle and on
		//! the live overlay tick, so the selection has to be found again by key
		//! + kind rather than kept as an index any one of those has just
		//! invalidated.
		int selected = FindRow( m_SelectedName, m_SelectedKind );
		if ( selected >= 0 )
			m_LocationList.SetSelectedIndex( selected, false );

		ApplyMapSelection();

		if ( m_Filter )
			m_Filter.SetTextPreview( closestMatch );
	}

	//! Push one row's data into the four parallel arrays. Call this at the same
	//! point a caller inserts into rowLabels/rowSubs, so index i keeps meaning
	//! the same row in all of them.
	protected void AddRow( string key, string kind, vector position, string markerId )
	{
		m_ListItems.Insert( new JMTeleportRowData( key, kind, position, markerId ) );
	}

	//! True if `label` matches the current search, or there is no search to
	//! match. Shared by every overlay kind so "type to filter" behaves the same
	//! whether it is hunting through locations or through the player list.
	protected bool PassesSearch( string label, TStringArray keywords, bool requireAllKeywords )
	{
		if ( m_SearchFilter == "" )
			return true;

		COT_String s = label;
		s.ToLower();

		string unused;
		return s.KeywordSearchImplEx( m_SearchFilter, keywords, requireAllKeywords, unused );
	}

	//! Shared by AppendPlayerRows and RefreshPlayerMarkers, so a role hidden on
	//! the map is hidden here too - the two would otherwise be free to disagree
	//! about who "the visible players" are.
	protected bool IsPlayerVisibleByRole( JMPlayerInstance player )
	{
		if ( !m_HiddenPlayerRoles || m_HiddenPlayerRoles.Count() == 0 )
			return true;

		array< string > playerRoles = player.GetRoles();

		if ( playerRoles && playerRoles.Count() > 0 )
		{
			for ( int r = 0; r < playerRoles.Count(); r++ )
			{
				if ( !IsRoleHidden( playerRoles[r] ) )
					return true;
			}

			return false;
		}

		return !IsRoleHidden( "Default" ) && !IsRoleHidden( "Player" ) && !IsRoleHidden( "Everyone" );
	}

	protected void AppendPlayerRows( TStringArray rowLabels, TStringArray rowSubs, TStringArray keywords, bool requireAllKeywords )
	{
		if ( !JMPermissions.Has( JMConstants.PERM_MAP_PLAYERS ) )
			return;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();
		if ( !players )
			return;

		string kindLabel = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_ROW_PLAYER" );

		for ( int i = 0; i < players.Count(); i++ )
		{
			JMPlayerInstance player = players[i];
			if ( !player )
				continue;

			if ( !IsPlayerVisibleByRole( player ) )
				continue;

			string name = player.GetName();

			if ( !PassesSearch( name, keywords, requireAllKeywords ) )
				continue;

			rowLabels.Insert( name );
			rowSubs.Insert( kindLabel );

			AddRow( player.GetGUID(), ROWKIND_PLAYER, player.GetPosition(), "tp_player_" + player.GetGUID() );
		}
	}

	//! Mirrors RefreshVehicleMarkers()'s type filter and, critically, its index
	//! scheme: "tp_veh_" + the FULL-list index i, not a running counter, so the
	//! marker id built here always matches the one the map already drew for the
	//! same vehicle.
	protected void AppendVehicleRows( TStringArray rowLabels, TStringArray rowSubs, TStringArray keywords, bool requireAllKeywords )
	{
		if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW ) )
			return;

		JMVehiclesModule vehicles = VehiclesModule();
		if ( !vehicles )
			return;

		array< ref JMVehicleMetaData > list = vehicles.GetServerVehicles();
		if ( !list )
			return;

		string kindLabel = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_ROW_VEHICLE" );

		for ( int i = 0; i < list.Count(); i++ )
		{
			JMVehicleMetaData meta = list[i];
			if ( !meta )
				continue;

			if ( meta.m_VehicleType != 0 && ( meta.m_VehicleType & m_VehicleTypeFilter ) == 0 )
				continue;

			if ( !PassesSearch( meta.m_DisplayName, keywords, requireAllKeywords ) )
				continue;

			rowLabels.Insert( meta.m_DisplayName );
			rowSubs.Insert( kindLabel );

			AddRow( "veh_" + i, ROWKIND_VEHICLE, meta.m_Position, "tp_veh_" + i );
		}
	}

	//! Walks the Expansion server-marker list once; shared by AppendMapMarkerRows
	//! and RefreshMapMarkers so the list and the map can never disagree about
	//! what a marker is called or where it sits.
	protected void CollectMapMarkerEntries( out array< ref JMTeleportMapEntry > entries )
	{
		entries = new array< ref JMTeleportMapEntry >();

#ifdef EXPANSIONMODNAVIGATION
		ExpansionMapSettings mapSettings = GetExpansionSettings().GetMap();
		if ( !mapSettings || !mapSettings.ServerMarkers )
			return;

		for ( int i = 0; i < mapSettings.ServerMarkers.Count(); i++ )
		{
			ExpansionMarkerData marker = mapSettings.ServerMarkers[i];
			if ( !marker )
				continue;

			string markerName = marker.GetName();
			if ( markerName == "" )
				markerName = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_MARKER_DEFAULT_NAME" );

			//! The marker's own glyph, so a trader reads as a trader here the
			//! same as it does on Expansion's map. GetIcon() resolves the
			//! stored icon NAME through ExpansionIcons to a texture path or
			//! an imageset reference, both of which LoadImageFile takes; a
			//! marker that named no icon falls back to the plain dot.
			string markerIcon = marker.GetIcon();
			if ( markerIcon == "" )
				markerIcon = UIActionMap.ICON_DOT;

			entries.Insert( new JMTeleportMapEntry( "marker_" + i, marker.GetPosition(), markerName, "", markerIcon, marker.GetColor() ) );
		}
#endif
	}

	protected void AppendMapMarkerRows( TStringArray rowLabels, TStringArray rowSubs, TStringArray keywords, bool requireAllKeywords )
	{
		string kindLabel = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_ROW_MAP_MARKER" );

		array< ref JMTeleportMapEntry > entries;
		CollectMapMarkerEntries( entries );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			JMTeleportMapEntry entry = entries[i];

			if ( !PassesSearch( entry.m_Label, keywords, requireAllKeywords ) )
				continue;

			rowLabels.Insert( entry.m_Label );
			rowSubs.Insert( kindLabel );

			AddRow( entry.m_Id, ROWKIND_MARKER, entry.m_Position, "tp_" + entry.m_Id );
		}
	}

	//! Walks whichever heli-crash list is live once; shared by AppendHeliCrashRows
	//! and RefreshHeliCrashMarkers.
	protected void CollectHeliCrashEntries( out array< ref JMTeleportMapEntry > entries )
	{
		entries = new array< ref JMTeleportMapEntry >();

		//! Every crash carries the same localized name; the ordinal is the
		//! only thing that tells two of them apart in a list.
		string crashName = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_HELI_CRASH_MARKER" );
		int crashIndex = 0;

#ifdef EXPANSIONMODAI
		if ( CrashBase.s_Expansion_HeliCrashes )
		{
			CF_DoublyLinkedNode_WeakRef<CrashBase> expNode = CrashBase.s_Expansion_HeliCrashes.m_Head;
			while ( expNode )
			{
				CrashBase expCrash = expNode.m_Value;
				if ( expCrash )
				{
					entries.Insert( new JMTeleportMapEntry( "heli_" + crashIndex, expCrash.GetPosition(), crashName, "#" + ( crashIndex + 1 ), JMConstants.Lucide( "helicopter" ), ARGB( 255, 243, 18, 156 ) ) );
					crashIndex++;
				}
				expNode = expNode.m_Next;
			}
		}
#else
		if ( CrashBase.s_JM_COT_AllCrashes )
		{
			CF_DoublyLinkedNode_WeakRef<CrashBase> cotNode = CrashBase.s_JM_COT_AllCrashes.m_Head;
			while ( cotNode )
			{
				CrashBase cotCrash = cotNode.m_Value;
				if ( cotCrash )
				{
					entries.Insert( new JMTeleportMapEntry( "heli_" + crashIndex, cotCrash.GetPosition(), crashName, "#" + ( crashIndex + 1 ), JMConstants.Lucide( "helicopter" ), ARGB( 255, 243, 18, 156 ) ) );
					crashIndex++;
				}
				cotNode = cotNode.m_Next;
			}
		}
#endif
	}

	protected void AppendHeliCrashRows( TStringArray rowLabels, TStringArray rowSubs, TStringArray keywords, bool requireAllKeywords )
	{
		string crashName = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_HELI_CRASH_MARKER" );

		if ( !PassesSearch( crashName, keywords, requireAllKeywords ) )
			return;

		array< ref JMTeleportMapEntry > entries;
		CollectHeliCrashEntries( entries );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			JMTeleportMapEntry entry = entries[i];
			rowLabels.Insert( entry.m_Label );
			rowSubs.Insert( entry.m_SubLabel );
			AddRow( entry.m_Id, ROWKIND_HELICRASH, entry.m_Position, "tp_" + entry.m_Id );
		}
	}

	//! Walks whichever toxic-zone list is live once; shared by AppendToxicZoneRows
	//! and RefreshToxicZoneMarkers.
	protected void CollectToxicZoneEntries( out array< ref JMTeleportMapEntry > entries )
	{
		entries = new array< ref JMTeleportMapEntry >();

		string zoneName = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_TOXIC_ZONE_MARKER" );
		int zoneIndex = 0;

#ifdef EXPANSIONMODMISSIONS
		if ( ExpansionContaminatedArea.s_JM_COT_AllAreas )
		{
			for ( int ei = 0; ei < ExpansionContaminatedArea.s_JM_COT_AllAreas.Count(); ei++ )
			{
				ExpansionContaminatedArea expArea = ExpansionContaminatedArea.s_JM_COT_AllAreas[ei];
				if ( !expArea )
					continue;

				entries.Insert( new JMTeleportMapEntry( "toxic_" + zoneIndex, expArea.GetPosition(), zoneName, "#" + ( zoneIndex + 1 ), JMConstants.Lucide( "biohazard" ), ARGB( 255, 80, 180, 40 ) ) );
				zoneIndex++;
			}
		}
#else
		if ( EffectArea.s_JM_COT_AllAreas )
		{
			for ( int vi = 0; vi < EffectArea.s_JM_COT_AllAreas.Count(); vi++ )
			{
				EffectArea effArea = EffectArea.s_JM_COT_AllAreas[vi];
				if ( !effArea )
					continue;

				entries.Insert( new JMTeleportMapEntry( "toxic_" + zoneIndex, effArea.GetPosition(), zoneName, "#" + ( zoneIndex + 1 ), JMConstants.Lucide( "biohazard" ), ARGB( 255, 80, 180, 40 ) ) );
				zoneIndex++;
			}
		}
#endif
	}

	protected void AppendToxicZoneRows( TStringArray rowLabels, TStringArray rowSubs, TStringArray keywords, bool requireAllKeywords )
	{
		string zoneName = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_TOXIC_ZONE_MARKER" );

		if ( !PassesSearch( zoneName, keywords, requireAllKeywords ) )
			return;

		array< ref JMTeleportMapEntry > entries;
		CollectToxicZoneEntries( entries );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			JMTeleportMapEntry entry = entries[i];
			rowLabels.Insert( entry.m_Label );
			rowSubs.Insert( entry.m_SubLabel );
			AddRow( entry.m_Id, ROWKIND_TOXICZONE, entry.m_Position, "tp_" + entry.m_Id );
		}
	}

	//! Find the row currently holding (key, kind), after a rebuild has thrown
	//! every previous index away.
	protected int FindRow( string key, string kind )
	{
		if ( key == "" )
			return -1;

		for ( int i = 0; i < m_ListItems.Count(); i++ )
		{
			JMTeleportRowData item = m_ListItems[i];
			if ( item && item.m_Name == key && item.m_Kind == kind )
				return i;
		}

		return -1;
	}

	//! Same lookup as FindRow(), but by the id a map marker was created with
	//! rather than by name + kind - what a click on the map has on hand.
	//! Location markers are namespaced by list key/kind instead, so this is
	//! only needed for the overlay rows (players, vehicles, ...).
	protected int FindRowByMarkerId( string markerId )
	{
		if ( markerId == "" )
			return -1;

		for ( int i = 0; i < m_ListItems.Count(); i++ )
		{
			JMTeleportRowData item = m_ListItems[i];
			if ( item && item.m_MarkerId == markerId )
				return i;
		}

		return -1;
	}

	//! The row currently selected in the list, or -1. Every action that reads
	//! "the current row" - teleport, focus, the row menu - goes through this
	//! rather than re-deriving it from m_SelectedName, so it always agrees with
	//! what is actually highlighted right now.
	protected int CurrentRow()
	{
		if ( !m_LocationList )
			return -1;

		return m_LocationList.GetSelectedIndex();
	}

	protected string MarkerIdFor( string locationName )
	{
		return MAP_LAYER + "_" + locationName;
	}

	//! A glyph per kind of place, so the map is readable without reading every
	//! label. The category names are the ones DayZ puts in CfgWorlds Names, and
	//! anything a map pack invents beyond them falls through to a pin.
	//! Not the lucide set the rest of this form uses: a map mark is drawn by
	//! the map, not by an ImageWidget, and that renderer reads .paa only - see
	//! the note on UIActionMap.ICON_DOT. These are DayZ's own navigation
	//! glyphs, which are the only marker-shaped .paa textures that are
	//! guaranteed to be present.
	protected string MarkerIconFor( string category )
	{
		if ( category == "City" || category == "CityCapital" || category == "Capital" )
			return UIActionMap.ICON_GOVOFFICE;

		if ( category == "Village" || category == "VillageCapital" || category == "Settlement" )
			return UIActionMap.ICON_STORE;

		if ( category == "Camp" )
			return UIActionMap.ICON_CAMP;

		if ( category == "Ruin" )
			return UIActionMap.ICON_MONUMENT;

		if ( category == "Hill" || category == "Mount" )
			return UIActionMap.ICON_HILL;

		if ( category == "Marine" )
			return UIActionMap.ICON_TOURISM;

		if ( category == "RailroadStation" )
			return UIActionMap.ICON_STATION;

		if ( category == "ViewPoint" )
			return UIActionMap.ICON_VIEWPOINT;

		if ( category == "Local" || category == "Area" )
			return UIActionMap.ICON_TSIGN;

		return UIActionMap.ICON_DOT;
	}

	//! Colour carries the same split as the glyph, so a category is still
	//! tellable at a zoom where the glyph is a few pixels across.
	protected int MarkerColorFor( string category )
	{
		if ( category == "City" || category == "CityCapital" )
			return JMTheme.DANGER;

		if ( category == "Village" || category == "VillageCapital" || category == "Settlement" )
			return JMTheme.WARNING;

		if ( category == "Marine" )
			return JMTheme.INFO;

		if ( category == "Hill" || category == "Mount" )
			return JMTheme.SUCCESS;

		return JMTheme.ACCENT;
	}

	protected void ApplyMapSelection()
	{
		if ( !m_Map )
			return;

		int row = CurrentRow();

		if ( row < 0 || row >= m_ListItems.Count() )
		{
			m_Map.SetSelectedMarker( "" );
			return;
		}

		m_Map.SetSelectedMarker( m_ListItems[row].m_MarkerId );
	}

	void OnClick_LocationList( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK && eid != UIEvent.DOUBLE_CLICK )
			return;

		int row = m_LocationList.GetSelectedIndex();

		if ( row < 0 || row >= m_ListItems.Count() )
			return;

		if ( eid == UIEvent.DOUBLE_CLICK )
		{
			//! CLICK is raised first, so the selection is already the one being
			//! jumped to by the time a double click gets here.
			TeleportSelf();
			return;
		}

		JMTeleportRowData item = m_ListItems[row];
		if ( !item )
			return;

		m_SelectedName = item.m_Name;
		m_SelectedKind = item.m_Kind;

		vector pos = item.m_Position;

		m_Module.OnSelectLocation( pos );

		//! Selecting a row aims the coordinate fields at it, so Go and Save act
		//! on the thing that was just picked - true of a saved location the
		//! same as a player, a vehicle or any other overlay row.
		if ( m_InputCoords )
			m_InputCoords.SetValue( pos );

		//! And the map follows the list. Picking a name off a list of a
		//! thousand and then hunting for its dot is not a thing anyone wants to
		//! do twice. Zoom is left alone - the admin's, not ours.
		if ( m_Map )
			m_Map.CenterOn( pos );

		ApplyMapSelection();

		if ( m_LocationList.GetLastClickButton() == MouseState.RIGHT )
			ShowRowMenu();
	}

	// =========================================================================
	//  Per-row context menu
	// =========================================================================

	protected void ShowRowMenu()
	{
		if ( !m_Window )
			return;

		int row = CurrentRow();
		if ( row < 0 || row >= m_ListItems.Count() )
			return;

		JMTeleportRowData item = m_ListItems[row];
		if ( !item )
			return;

		string kind = item.m_Kind;

		if ( !m_RowMenu )
		{
			m_RowMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_RowMenu" );

			if ( !m_RowMenu )
				return;

			RegisterOverlay( m_RowMenu );
		}

		//! Rebuilt on every open rather than once: Edit and Delete only exist
		//! for a saved location, and a player or vehicle row needs the menu to
		//! change SHAPE, not just grey out the two rows that do not apply to it.
		m_RowMenu.ClearItems();
		m_RowMenu.AddItem( MENU_TELEPORT, "#STR_COT_TELEPORT_MODULE_MENU_TELEPORT_TO", JMConstants.Lucide( "move-3d"   ) );
		m_RowMenu.AddItem( MENU_FOCUS,    "#STR_COT_TELEPORT_MODULE_MENU_SHOW_ON_MAP", JMConstants.Lucide( "crosshair" ) );

		//! A location teleport is gated by the location permission; every other
		//! kind goes through Position(), which is gated by the plain
		//! teleport-to-position one.
		string teleportPerm = JMConstants.PERM_PLAYER_TELEPORT_POSITION;
		if ( kind == ROWKIND_LOCATION )
			teleportPerm = JMConstants.PERM_PLAYER_TELEPORT_LOCATION;

		m_RowMenu.SetItemEnabled( MENU_TELEPORT, JMPermissions.Has( teleportPerm ) );

		if ( kind == ROWKIND_LOCATION )
		{
			m_RowMenu.AddItem( MENU_EDIT,   "#STR_COT_GENERIC_EDIT",   JMConstants.Lucide( "pencil"  ) );
			m_RowMenu.AddItem( MENU_DELETE, "#STR_COT_GENERIC_DELETE", JMConstants.Lucide( "trash-2" ) );

			//! A row the admin cannot act on still opens the menu; the row is
			//! greyed instead of missing, so the menu does not change shape from
			//! one server to the next.
			m_RowMenu.SetItemEnabled( MENU_EDIT,   JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_EDIT ) );
			m_RowMenu.SetItemEnabled( MENU_DELETE, JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_REMOVE ) );
		}

		int mx, my;
		GetMousePos( mx, my );

		m_RowMenu.ShowAt( mx, my );
	}

	void OnClick_RowMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_RowMenu )
			return;

		string id = m_RowMenu.GetLastClickedId();

		int row = CurrentRow();
		if ( row < 0 || row >= m_ListItems.Count() )
			return;

		JMTeleportRowData item = m_ListItems[row];
		if ( !item )
			return;

		string kind = item.m_Kind;
		vector pos  = item.m_Position;

		if ( id == MENU_TELEPORT )
		{
			Click_Teleport( UIEvent.CLICK, NULL );
			return;
		}

		if ( id == MENU_FOCUS )
		{
			//! Centre without changing zoom: someone who has already zoomed in
			//! asked where this is, not to have their view reset.
			if ( m_Map )
				m_Map.CenterOn( pos );

			if ( m_InputCoords )
				m_InputCoords.SetValue( pos );

			return;
		}

		//! Edit and Delete only ever appear on a location row - ShowRowMenu()
		//! does not even add them for anything else - but the menu is a single
		//! shared control, so guard here too against a stale click landing after
		//! the selection has moved on.
		if ( kind != ROWKIND_LOCATION )
			return;

		JMTeleportLocation location = GetCurrentLocation();
		if ( !location )
			return;

		if ( id == MENU_EDIT )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_EDIT ) )
				return;

			ShowPopup( location.Name, location.Position, location.Name, location.Type );
			return;
		}

		if ( id == MENU_DELETE )
			OnClick_DeleteLocation( UIEvent.CLICK, NULL );
	}

	// =========================================================================
	//  Category filter
	//
	//  A popup hung off the filter button, the way the object spawner and the
	//  vehicle form do it: needed for one click per search and costing nothing
	//  in between, where a selector parked on the form costs a band of the left
	//  column forever.
	// =========================================================================

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
			m_CategoryMenu.SetCloseOnClick( false );
		}

		if ( m_CategoryMenu.IsOpen() )
		{
			m_CategoryMenu.Close();
			if ( m_SubMenu )
				m_SubMenu.Close();
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

	protected void RebuildCategoryMenu()
	{
		if ( !m_CategoryMenu )
			return;

		m_CategoryMenu.ClearItems();

		bool allCategoriesShown = ( !m_HiddenCategories || m_HiddenCategories.Count() == 0 );
		bool anyVehicleTypeShown = ( m_VehicleTypeFilter != 0 );
		bool allPlayersShown = m_ShowPlayers && ( !m_HiddenPlayerRoles || m_HiddenPlayerRoles.Count() == 0 );

		m_CategoryMenu.AddItem( SUB_LOCATIONS, "#STR_COT_TELEPORT_MODULE_MENU_LOCATIONS", JMConstants.Lucide( "map-pin" ), ToggleTextColor( allCategoriesShown ), true );
		m_CategoryMenu.AddItem( SUB_VEHICLES,  "#STR_COT_TELEPORT_MODULE_MENU_VEHICLES",  JMConstants.Lucide( "car"     ), ToggleTextColor( anyVehicleTypeShown ), true );
		m_CategoryMenu.AddItem( SUB_PLAYERS,   "#STR_COT_TELEPORT_MODULE_MENU_PLAYERS",   JMConstants.Lucide( "user"    ), ToggleTextColor( allPlayersShown ), true );

		m_CategoryMenu.AddItem( FILTER_MAP_MARKERS,  "#STR_COT_TELEPORT_MODULE_MENU_MAP_MARKERS",  CheckIcon( m_ShowMapMarkers  ), ToggleTextColor( m_ShowMapMarkers  ) );
		m_CategoryMenu.AddItem( FILTER_HELI_CRASHES, "#STR_COT_TELEPORT_MODULE_MENU_HELI_CRASHES", CheckIcon( m_ShowHeliCrashes ), ToggleTextColor( m_ShowHeliCrashes ) );
		m_CategoryMenu.AddItem( FILTER_TOXIC_ZONES,  "#STR_COT_TELEPORT_MODULE_MENU_TOXIC_ZONES",  CheckIcon( m_ShowToxicZones  ), ToggleTextColor( m_ShowToxicZones  ) );

		m_CategoryMenu.SetItemEnabled( SUB_PLAYERS,  JMPermissions.Has( JMConstants.PERM_MAP_PLAYERS ) );
		m_CategoryMenu.SetItemEnabled( SUB_VEHICLES, JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW ) );
	}

	protected void OpenSubMenu( string subMenuType, float x, float y )
	{
		if ( !m_SubMenu )
		{
			if ( !m_Window )
				return;

			m_SubMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_SubMenu" );

			if ( !m_SubMenu )
				return;

			RegisterOverlay( m_SubMenu );
			m_SubMenu.SetCloseOnClick( false );
		}

		if ( m_SubMenu.IsOpen() && m_CurrentSubMenuType == subMenuType )
		{
			m_SubMenu.Close();
			m_CurrentSubMenuType = "";
			return;
		}

		m_CurrentSubMenuType = subMenuType;

		if ( subMenuType == SUB_LOCATIONS )
			RebuildSubMenuLocations();
		else if ( subMenuType == SUB_VEHICLES )
			RebuildSubMenuVehicles();
		else if ( subMenuType == SUB_PLAYERS )
			RebuildSubMenuPlayers();

		m_SubMenu.ShowAt( x, y );
	}

	protected void RebuildSubMenuLocations()
	{
		if ( !m_SubMenu )
			return;

		m_SubMenu.ClearItems();

		bool allCategoriesShown = ( !m_HiddenCategories || m_HiddenCategories.Count() == 0 );
		m_SubMenu.AddItem( TOGGLE_ALL_LOCATIONS, "#STR_COT_TELEPORT_MODULE_TOGGLE_ALL", CheckIcon( allCategoriesShown ), ToggleTextColor( allCategoriesShown ) );

		if ( m_Categories )
		{
			for ( int i = 0; i < m_Categories.Count(); i++ )
			{
				if ( m_Categories[i] == CATEGORY_ALL )
					continue;

				bool shown = !IsCategoryHidden( m_Categories[i] );
				m_SubMenu.AddItem( m_Categories[i], m_Categories[i], CheckIcon( shown ), ToggleTextColor( shown ) );
			}
		}
	}

	protected void RebuildSubMenuVehicles()
	{
		if ( !m_SubMenu )
			return;

		m_SubMenu.ClearItems();

		bool anyVehicleTypeShown = ( m_VehicleTypeFilter != 0 );
		m_SubMenu.AddItem( TOGGLE_ALL_VEHICLES, "#STR_COT_TELEPORT_MODULE_TOGGLE_ALL", CheckIcon( anyVehicleTypeShown ), ToggleTextColor( anyVehicleTypeShown ) );

		bool canSeeVehicles = JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW );

		m_SubMenu.AddItem( FILTER_VEH_CAR,   "#STR_COT_TELEPORT_MODULE_VEH_CARS",        CheckIcon( IsVehicleTypeShown( JMVT_CAR ) ),        ToggleTextColor( IsVehicleTypeShown( JMVT_CAR ) ) );
		m_SubMenu.AddItem( FILTER_VEH_BOAT,  "#STR_COT_TELEPORT_MODULE_VEH_BOATS",       CheckIcon( IsVehicleTypeShown( JMVT_BOAT ) ),       ToggleTextColor( IsVehicleTypeShown( JMVT_BOAT ) ) );
		m_SubMenu.AddItem( FILTER_VEH_HELI,  "#STR_COT_TELEPORT_MODULE_VEH_HELICOPTERS", CheckIcon( IsVehicleTypeShown( JMVT_HELICOPTER ) ), ToggleTextColor( IsVehicleTypeShown( JMVT_HELICOPTER ) ) );
		m_SubMenu.AddItem( FILTER_VEH_PLANE, "#STR_COT_TELEPORT_MODULE_VEH_PLANES",      CheckIcon( IsVehicleTypeShown( JMVT_PLANE ) ),      ToggleTextColor( IsVehicleTypeShown( JMVT_PLANE ) ) );
		m_SubMenu.AddItem( FILTER_VEH_BIKE,  "#STR_COT_TELEPORT_MODULE_VEH_BIKES",       CheckIcon( IsVehicleTypeShown( JMVT_BIKE ) ),       ToggleTextColor( IsVehicleTypeShown( JMVT_BIKE ) ) );

		m_SubMenu.SetItemEnabled( FILTER_VEH_CAR,   canSeeVehicles );
		m_SubMenu.SetItemEnabled( FILTER_VEH_BOAT,  canSeeVehicles );
		m_SubMenu.SetItemEnabled( FILTER_VEH_HELI,  canSeeVehicles );
		m_SubMenu.SetItemEnabled( FILTER_VEH_PLANE, canSeeVehicles );
		m_SubMenu.SetItemEnabled( FILTER_VEH_BIKE,  canSeeVehicles );
	}

	protected void RebuildSubMenuPlayers()
	{
		if ( !m_SubMenu )
			return;

		m_SubMenu.ClearItems();

		bool allPlayersShown = m_ShowPlayers && ( !m_HiddenPlayerRoles || m_HiddenPlayerRoles.Count() == 0 );
		m_SubMenu.AddItem( TOGGLE_ALL_PLAYERS, "#STR_COT_TELEPORT_MODULE_TOGGLE_ALL", CheckIcon( allPlayersShown ), ToggleTextColor( allPlayersShown ) );

		bool canSeePlayers = JMPermissions.Has( JMConstants.PERM_MAP_PLAYERS );

		array< JMRole > roles = new array< JMRole >;
		GetPermissionsManager().GetRolesAsList( roles );

		for ( int i = 0; i < roles.Count(); i++ )
		{
			JMRole role = roles[i];
			if ( !role )
				continue;

			string roleName = role.Name;
			bool roleShown = m_ShowPlayers && !IsRoleHidden( roleName );

			m_SubMenu.AddItem( roleName, roleName, CheckIcon( roleShown ), ToggleTextColor( roleShown ) );
			m_SubMenu.SetItemEnabled( roleName, canSeePlayers );
		}
	}

	void OnClick_SubMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_SubMenu )
			return;

		string id = m_SubMenu.GetLastClickedId();

		if ( m_CurrentSubMenuType == SUB_LOCATIONS )
		{
			if ( id == TOGGLE_ALL_LOCATIONS )
			{
				bool allShown = ( !m_HiddenCategories || m_HiddenCategories.Count() == 0 );
				if ( allShown )
					HideAllCategories();
				else
					ShowAllCategories();
			}
			else
			{
				ToggleCategory( id );
			}

			RebuildSubMenuLocations();
			RebuildCategoryMenu();
			UpdateFilterTooltip();
			RebuildList();
			return;
		}

		if ( m_CurrentSubMenuType == SUB_VEHICLES )
		{
			if ( id == TOGGLE_ALL_VEHICLES )
			{
				if ( m_VehicleTypeFilter == 0 )
					m_VehicleTypeFilter = JMVT_ALL;
				else
					m_VehicleTypeFilter = 0;
			}
			else
			{
				ToggleVehicleType( id );
			}

			RebuildSubMenuVehicles();
			RebuildCategoryMenu();
			ApplyOverlays();
			return;
		}

		if ( m_CurrentSubMenuType == SUB_PLAYERS )
		{
			if ( id == TOGGLE_ALL_PLAYERS )
			{
				bool allPlayersOn = m_ShowPlayers && ( !m_HiddenPlayerRoles || m_HiddenPlayerRoles.Count() == 0 );

				if ( allPlayersOn )
				{
					m_ShowPlayers = false;
					HideAllPlayerRoles();
				}
				else
				{
					m_ShowPlayers = true;
					m_HiddenPlayerRoles.Clear();
				}
			}
			else
			{
				m_ShowPlayers = true;
				TogglePlayerRole( id );
			}

			RebuildSubMenuPlayers();
			RebuildCategoryMenu();
			ApplyOverlays();
			return;
		}
	}

	protected bool IsRoleHidden( string roleName )
	{
		if ( !m_HiddenPlayerRoles )
			return false;

		return m_HiddenPlayerRoles.Find( roleName ) >= 0;
	}

	protected void TogglePlayerRole( string roleName )
	{
		if ( !m_HiddenPlayerRoles )
			m_HiddenPlayerRoles = new TStringArray;

		int idx = m_HiddenPlayerRoles.Find( roleName );
		if ( idx >= 0 )
		{
			m_HiddenPlayerRoles.Remove( idx );
			return;
		}

		m_HiddenPlayerRoles.Insert( roleName );
	}

	protected void HideAllPlayerRoles()
	{
		m_HiddenPlayerRoles = new TStringArray;

		array< JMRole > roles = new array< JMRole >;
		GetPermissionsManager().GetRolesAsList( roles );

		for ( int i = 0; i < roles.Count(); i++ )
		{
			if ( roles[i] )
				m_HiddenPlayerRoles.Insert( roles[i].Name );
		}
	}

	protected bool IsVehicleTypeShown( int typeBit )
	{
		return ( m_VehicleTypeFilter & typeBit ) != 0;
	}

	protected int VehicleTypeBitFor( string id )
	{
		if ( id == FILTER_VEH_CAR )
			return JMVT_CAR;

		if ( id == FILTER_VEH_BOAT )
			return JMVT_BOAT;

		if ( id == FILTER_VEH_HELI )
			return JMVT_HELICOPTER;

		if ( id == FILTER_VEH_PLANE )
			return JMVT_PLANE;

		if ( id == FILTER_VEH_BIKE )
			return JMVT_BIKE;

		return 0;
	}

	//! Flip one vehicle type's bit. False (and no change) if `id` is not one
	//! of the per-type rows.
	protected bool ToggleVehicleType( string id )
	{
		int bit = VehicleTypeBitFor( id );

		if ( bit == 0 )
			return false;

		m_VehicleTypeFilter = m_VehicleTypeFilter ^ bit;
		return true;
	}

	protected void HideAllCategories()
	{
		m_HiddenCategories = new TStringArray;

		if ( !m_Categories )
			return;

		for ( int i = 0; i < m_Categories.Count(); i++ )
		{
			if ( m_Categories[i] == CATEGORY_ALL )
				continue;

			m_HiddenCategories.Insert( m_Categories[i] );
		}
	}

	protected void ShowAllCategories()
	{
		m_HiddenCategories = new TStringArray;
	}

	protected string CheckIcon( bool on )
	{
		if ( on )
			return JMConstants.Lucide( "square-check" );

		return JMConstants.Lucide( "square" );
	}

	protected int ToggleTextColor( bool on )
	{
		if ( on )
			return JMTheme.TEXT_PRIMARY;

		return JMTheme.TEXT_DISABLED;
	}

	void OnClick_CategoryMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_CategoryMenu )
			return;

		string id = m_CategoryMenu.GetLastClickedId();

		if ( id == SUB_LOCATIONS || id == SUB_VEHICLES || id == SUB_PLAYERS )
		{
			int mx, my;
			GetMousePos( mx, my );
			OpenSubMenu( id, mx, my );
			return;
		}

		if ( id == FILTER_MAP_MARKERS )
		{
			m_ShowMapMarkers = !m_ShowMapMarkers;
			MarkToggleRow( id, m_ShowMapMarkers );
			ApplyOverlays();
			return;
		}

		if ( id == FILTER_HELI_CRASHES )
		{
			m_ShowHeliCrashes = !m_ShowHeliCrashes;
			MarkToggleRow( id, m_ShowHeliCrashes );
			ApplyOverlays();
			return;
		}

		if ( id == FILTER_TOXIC_ZONES )
		{
			m_ShowToxicZones = !m_ShowToxicZones;
			MarkToggleRow( id, m_ShowToxicZones );
			ApplyOverlays();
			return;
		}
	}

	//! Repaint one row in place. Rebuilding the menu here would destroy the row
	//! the press landed on, which the engine answers by recentring the cursor.
	protected void MarkToggleRow( string id, bool on )
	{
		if ( !m_CategoryMenu )
			return;

		m_CategoryMenu.SetItemIcon( id, CheckIcon( on ) );
		m_CategoryMenu.SetItemTextColor( id, ToggleTextColor( on ) );
	}

	protected bool IsCategoryHidden( string category )
	{
		if ( !m_HiddenCategories )
			return false;

		return m_HiddenCategories.Find( category ) >= 0;
	}

	protected void ToggleCategory( string category )
	{
		if ( !m_HiddenCategories )
			m_HiddenCategories = new TStringArray;

		int at = m_HiddenCategories.Find( category );

		if ( at >= 0 )
		{
			m_HiddenCategories.Remove( at );
			return;
		}

		m_HiddenCategories.Insert( category );
	}

	//! The button is the only thing showing that a filter is on at all, so it
	//! has to say how many rows are being held back.
	protected void UpdateFilterTooltip()
	{
		int hidden = 0;

		if ( !m_FilterButton )
			return;

		if ( m_HiddenCategories )
			hidden = m_HiddenCategories.Count();

		if ( hidden == 0 )
			m_FilterButton.SetTooltip( "#STR_COT_TELEPORT_MODULE_FILTERS_TOOLTIP" );
		else
			m_FilterButton.SetTooltip( Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_FILTERS_HIDDEN_PREFIX" ) + hidden + Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_FILTERS_HIDDEN_SUFFIX" ) );
	}

	// =========================================================================
	//  Live overlays - players and vehicles
	//
	//  Neither belongs to this module. They are drawn here because the map is
	//  here, each in its own layer, so switching one off cannot disturb the
	//  locations or the other one.
	// =========================================================================

	//! Start or stop the poll, and take down whatever was just switched off.
	protected void ApplyOverlays()
	{
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( RefreshOverlays );

		if ( m_Map )
		{
			if ( !m_ShowPlayers )
				m_Map.ClearLayer( MAP_LAYER_PLAYERS );

			if ( m_VehicleTypeFilter == 0 )
				m_Map.ClearLayer( MAP_LAYER_VEHICLES );

			if ( !m_ShowMapMarkers )
				m_Map.ClearLayer( MAP_LAYER_MARKERS );

			if ( !m_ShowHeliCrashes )
				m_Map.ClearLayer( MAP_LAYER_HELICRASH );

			if ( !m_ShowToxicZones )
				m_Map.ClearLayer( MAP_LAYER_TOXICZONES );
		}

		if ( !m_ShowPlayers && m_VehicleTypeFilter == 0 && !m_ShowMapMarkers && !m_ShowHeliCrashes && !m_ShowToxicZones )
			return;

		//! The vehicle list is a server round trip, not a local read, so it is
		//! asked for once when the overlay is switched on rather than on every
		//! tick. The module keeps it current from its own deltas after that.
		if ( m_VehicleTypeFilter != 0 )
			RequestVehicles();

		RefreshOverlays();

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( RefreshOverlays, OVERLAY_REFRESH_MS, true );
	}

	protected JMVehiclesModule VehiclesModule()
	{
		if ( !m_VehiclesModule )
			Class.CastTo( m_VehiclesModule, GetModuleManager().GetModule( JMVehiclesModule ) );

		return m_VehiclesModule;
	}

	protected void RequestVehicles()
	{
		if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW ) )
			return;

		JMVehiclesModule vehicles = VehiclesModule();

		if ( !vehicles )
			return;

		vehicles.RequestServerVehicles();
	}

	void RefreshOverlays()
	{
		if ( !m_Map )
			return;

		if ( m_ShowPlayers )
			RefreshPlayerMarkers();

		if ( m_VehicleTypeFilter != 0 )
			RefreshVehicleMarkers();

		if ( m_ShowMapMarkers )
			RefreshMapMarkers();

		if ( m_ShowHeliCrashes )
			RefreshHeliCrashMarkers();

		if ( m_ShowToxicZones )
			RefreshToxicZoneMarkers();

		//! The list mirrors these same overlays now, so a join, a leave or a
		//! vehicle spawning has to reach it on the same tick that moves its dot
		//! on the map - not only on the next search keystroke or filter toggle.
		RebuildList();
	}

	protected void RefreshPlayerMarkers()
	{
		if ( !JMPermissions.Has( JMConstants.PERM_MAP_PLAYERS ) )
			return;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

		if ( !players )
			return;

		//! One redraw for the whole layer instead of one per player.
		m_Map.BeginBatch();
		m_Map.ClearLayer( MAP_LAYER_PLAYERS );

		for ( int i = 0; i < players.Count(); i++ )
		{
			JMPlayerInstance player = players[i];

			if ( !player )
				continue;

			if ( !IsPlayerVisibleByRole( player ) )
				continue;

			int color = COLOR_PLAYER;

			if ( JM_GetSelected().IsSelected( player.GetGUID() ) )
				color = COLOR_PLAYER_SELECTED;
			else if ( player.HasPermission( "COT" ) )
				color = COLOR_PLAYER_ADMIN;

			m_Map.AddMarker( "tp_player_" + player.GetGUID(), player.GetPosition(), player.GetName(), color, UIActionMap.ICON_DOT, MAP_LAYER_PLAYERS );
		}

		m_Map.EndBatch();

		//! Positions only arrive because something asked for them.
		GetCommunityOnlineTools().RefreshClientPositions();
	}

	protected void RefreshMapMarkers()
	{
		if ( !m_Map )
			return;

		m_Map.BeginBatch();
		m_Map.ClearLayer( MAP_LAYER_MARKERS );

		array< ref JMTeleportMapEntry > entries;
		CollectMapMarkerEntries( entries );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			JMTeleportMapEntry entry = entries[i];
			m_Map.AddMarker( "tp_" + entry.m_Id, entry.m_Position, entry.m_Label, entry.m_Color, entry.m_Icon, MAP_LAYER_MARKERS );
		}

		m_Map.EndBatch();
	}

	protected void RefreshHeliCrashMarkers()
	{
		if ( !m_Map )
			return;

		m_Map.BeginBatch();
		m_Map.ClearLayer( MAP_LAYER_HELICRASH );

		array< ref JMTeleportMapEntry > entries;
		CollectHeliCrashEntries( entries );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			JMTeleportMapEntry entry = entries[i];
			m_Map.AddMarker( "tp_" + entry.m_Id, entry.m_Position, entry.m_Label, entry.m_Color, entry.m_Icon, MAP_LAYER_HELICRASH );
		}

		m_Map.EndBatch();
	}

	protected void RefreshToxicZoneMarkers()
	{
		if ( !m_Map )
			return;

		m_Map.BeginBatch();
		m_Map.ClearLayer( MAP_LAYER_TOXICZONES );

		array< ref JMTeleportMapEntry > entries;
		CollectToxicZoneEntries( entries );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			JMTeleportMapEntry entry = entries[i];
			m_Map.AddMarker( "tp_" + entry.m_Id, entry.m_Position, entry.m_Label, entry.m_Color, entry.m_Icon, MAP_LAYER_TOXICZONES );
		}

		m_Map.EndBatch();
	}

	//! Raised via UIActionMap's UIEvent.MOUSE_ENTER (see OnClick_Map). Static
	//! snapshot only, unlike the Vehicle Manager's own hover which polls the
	//! server for live figures - this overlay is a teleport aid, not a status
	//! panel, so the metadata already on hand is enough.
	protected void ShowVehicleHoverInfo( JMVehicleMetaData vehicle )
	{
		if ( !vehicle || !m_Window )
			return;

		if ( !m_VehicleHoverInfo )
			m_VehicleHoverInfo = new JMVehiclesHoverInfo( m_Window.GetWidgetRoot() );

		int x, y;
		GetMousePos( x, y );

		m_VehicleHoverInfo.ShowAt( vehicle, x, y );
	}

	protected void HideVehicleHoverInfo()
	{
		if ( m_VehicleHoverInfo )
			m_VehicleHoverInfo.Hide();
	}

	protected void RefreshVehicleMarkers()
	{
		if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW ) )
			return;

		JMVehiclesModule vehicles = VehiclesModule();

		if ( !vehicles )
			return;

		array< ref JMVehicleMetaData > list = vehicles.GetServerVehicles();

		if ( !list )
			return;

		m_Map.BeginBatch();
		m_Map.ClearLayer( MAP_LAYER_VEHICLES );
		m_VehicleByMarkerId.Clear();

		for ( int i = 0; i < list.Count(); i++ )
		{
			JMVehicleMetaData meta = list[i];

			if ( !meta )
				continue;

			//! A recognized type answers to its own checkbox; a vehicle
			//! carrying none of the known bits (modded, unclassified) shows
			//! whenever the vehicle layer is on at all - same as before this
			//! had per-type rows.
			if ( meta.m_VehicleType != 0 && ( meta.m_VehicleType & m_VehicleTypeFilter ) == 0 )
				continue;

			//! Indexed rather than keyed on the network id: the ids are a pair
			//! of ints, and the layer is replaced whole on every refresh, so
			//! nothing outside this loop ever asks for one of these by name.
			string vehMarkerId = "tp_veh_" + i;
			m_Map.AddMarker( vehMarkerId, meta.m_Position, meta.m_DisplayName, VehicleColorFor( meta.m_VehicleType ), VehicleIconFor( meta.m_VehicleType ), MAP_LAYER_VEHICLES );
			m_VehicleByMarkerId.Insert( vehMarkerId, meta );
		}

		m_Map.EndBatch();
	}

	//! Same colours the vehicle form's own map uses, so a helicopter is the
	//! same blue wherever it is drawn.
	protected int VehicleColorFor( int vehicleType )
	{
		if ( vehicleType == JMVT_BOAT )
			return ARGB( 255, 243, 18, 156 );

		if ( vehicleType == JMVT_HELICOPTER )
			return ARGB( 255, 18, 156, 243 );

		if ( vehicleType == JMVT_PLANE )
			return ARGB( 255, 18, 243, 156 );

		if ( vehicleType == JMVT_BIKE )
			return ARGB( 255, 243, 156, 100 );

		return ARGB( 255, 243, 156, 18 );
	}

	//! Same glyphs the vehicle form's map uses, so a boat is a boat on both maps.
	protected string VehicleIconFor( int vehicleType )
	{
		if ( vehicleType == JMVT_BOAT )
			return JMConstants.Lucide( "sailboat" );

		if ( vehicleType == JMVT_HELICOPTER )
			return JMConstants.Lucide( "helicopter" );

		if ( vehicleType == JMVT_PLANE )
			return JMConstants.Lucide( "plane" );

		if ( vehicleType == JMVT_BIKE )
			return JMConstants.Lucide( "bike" );

		return UIActionMap.ICON_CAR;
	}

	// =========================================================================
	//  The map
	// =========================================================================

	//! A press on the map aims the coordinate fields; a double press teleports.
	//! Landing on a marker selects that location instead of picking bare ground,
	//! which is how the map talks back to the list.
	void OnClick_Map( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.MOUSE_ENTER )
		{
			string hoveredId = m_Map.GetHoveredMarkerId();
			if ( m_VehicleByMarkerId.Contains( hoveredId ) )
				ShowVehicleHoverInfo( m_VehicleByMarkerId.Get( hoveredId ) );
			return;
		}

		if ( eid == UIEvent.MOUSE_LEAVE )
		{
			HideVehicleHoverInfo();
			return;
		}

		if ( eid != UIEvent.CLICK && eid != UIEvent.DOUBLE_CLICK )
			return;

		string markerId = m_Map.GetLastClickedMarkerId();
		bool   onMarker = false;

		//! This form's own location marks are named MAP_LAYER + the location
		//! name, so a hit there can be resolved straight to that location.
		//! Every other kind (player, vehicle, ...) is keyed by its own marker
		//! id, matched below via the same id AddMarker() was given for that row.
		if ( markerId.IndexOf( MAP_LAYER + "_" ) == 0 )
		{
			string name = markerId.Substring( MAP_LAYER.Length() + 1, markerId.Length() - MAP_LAYER.Length() - 1 );

			int row = FindRow( name, ROWKIND_LOCATION );

			if ( row >= 0 )
			{
				onMarker       = true;
				m_SelectedName = name;
				m_SelectedKind = ROWKIND_LOCATION;

				m_LocationList.SetSelectedIndex( row, true );
				ApplyMapSelection();
			}
		}
		else
		{
			int overlayRow = FindRowByMarkerId( markerId );

			if ( overlayRow >= 0 && overlayRow < m_ListItems.Count() )
			{
				onMarker       = true;
				m_SelectedName = m_ListItems[overlayRow].m_Name;
				m_SelectedKind = m_ListItems[overlayRow].m_Kind;

				m_LocationList.SetSelectedIndex( overlayRow, true );
				ApplyMapSelection();
			}
		}

		vector world = m_Map.GetLastClickWorldPos();

		//! A press on a marker means the marker, not the patch of ground it
		//! happens to sit on, so the coordinate fields take the saved position
		//! rather than wherever the pointer landed within the hit radius.
		if ( onMarker )
		{
			JMTeleportLocation picked = GetCurrentLocation();

			if ( picked )
			{
				world = picked.Position;
			}
			else
			{
				int posRow = FindRowByMarkerId( markerId );
				if ( posRow >= 0 && posRow < m_ListItems.Count() )
					world = m_ListItems[posRow].m_Position;
			}
		}

		if ( m_InputCoords )
			m_InputCoords.SetValue( world );

		if ( eid == UIEvent.DOUBLE_CLICK )
		{
			Click_GoToCoordinates( UIEvent.CLICK, NULL );
			return;
		}

		if ( m_Map.GetLastClickButton() != MouseState.RIGHT )
			return;

		//! Right on a marker is the same menu the list offers - it is the same
		//! location, reached a different way. Right on bare ground is a
		//! different question, so it gets its own two answers.
		if ( onMarker )
		{
			ShowRowMenu();
			return;
		}

		m_MapMenuWorldPos = world;
		ShowMapMenu();
	}

	// =========================================================================
	//  Map context menu - bare ground
	// =========================================================================

	protected void ShowMapMenu()
	{
		if ( !m_Window )
			return;

		if ( !m_MapMenu )
		{
			m_MapMenu = UIActionManager.CreateContextMenu( layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_MapMenu" );

			if ( !m_MapMenu )
				return;

			RegisterOverlay( m_MapMenu );

			m_MapMenu.AddItem( MENU_MAP_GO,   "#STR_COT_TELEPORT_MODULE_MENU_TELEPORT_HERE",   JMConstants.Lucide( "move-3d"  ) );
			m_MapMenu.AddItem( MENU_MAP_SAVE, "#STR_COT_TELEPORT_MODULE_MENU_SAVE_AS_LOCATION", JMConstants.Lucide( "map-pin-plus" ) );
		}

		m_MapMenu.SetItemEnabled( MENU_MAP_GO,   JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) );
		m_MapMenu.SetItemEnabled( MENU_MAP_SAVE, JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION + ".Add" ) );

		int mx, my;
		GetMousePos( mx, my );

		m_MapMenu.ShowAt( mx, my );
	}

	void OnClick_MapMenu( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_MapMenu )
			return;

		string id = m_MapMenu.GetLastClickedId();

		if ( id == MENU_MAP_GO )
		{
			if ( m_InputCoords )
				m_InputCoords.SetValue( m_MapMenuWorldPos );

			Click_GoToCoordinates( UIEvent.CLICK, NULL );
			return;
		}

		if ( id != MENU_MAP_SAVE )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION + ".Add" ) )
			return;

		ShowPopup( "", m_MapMenuWorldPos, "", "" );
	}

	void Click_CenterOnMe( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Map || !g_Game.GetPlayer() )
			return;

		m_Map.CenterOn( g_Game.GetPlayer().GetPosition() );
	}

	// =========================================================================
	//  Form lifecycle
	// =========================================================================

	override void OnShow()
	{
		super.OnShow();

		if ( !m_Module )
			return;

		//! The map opens where the admin is standing, and the coordinate fields
		//! start there too, so every control on the right agrees before
		//! anything has been clicked.
		if ( g_Game.GetPlayer() )
		{
			vector self = g_Game.GetPlayer().GetPosition();

			if ( m_Map )
				m_Map.CenterOnDeferred( self );

			if ( m_InputCoords )
				m_InputCoords.SetValue( self );
		}

		if ( m_Module.IsLoaded() )
			UpdateList();
		else
			m_Module.Load();

		//! Overlays survive a close and reopen, so the poll has to be started
		//! again - it is stopped on hide rather than left running over a form
		//! nobody is looking at.
		ApplyOverlays();
	}

	override void OnHide()
	{
		HidePopup();
		HideVehicleHoverInfo();

		if ( m_CategoryMenu )
			m_CategoryMenu.Close();

		if ( m_SubMenu )
			m_SubMenu.Close();

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( RefreshOverlays );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( DeferredRemeasureCoords );

		super.OnHide();
	}

	//! UIActionMap's marker widgets need reprojecting onto the map's current
	//! pan/zoom every frame the form is up - see UIActionMap.TickMarkers().
	override void Update()
	{
		super.Update();

		if ( m_Map )
			m_Map.TickMarkers();
	}

	override void OnSettingsUpdated()
	{
		UpdateList();
	}

	//! Rebuilds the category set and the list from the module's current
	//! settings. Named for the call the module already makes.
	void UpdateList()
	{
		if ( !m_Module || !m_Module.IsLoaded() )
			return;

		RefreshCategories();

		//! A location can be deleted from under the selection, by this admin or
		//! by another one. Only meaningful for a location row - GetCurrentLocation()
		//! looks locations up by name, and would wrongly clear the selection on
		//! every call for a player or vehicle row, whose key is never a location
		//! name to begin with.
		if ( m_SelectedKind == ROWKIND_LOCATION && m_SelectedName != "" && !GetCurrentLocation() )
		{
			m_SelectedName = "";
			m_SelectedKind = ROWKIND_LOCATION;
		}

		RebuildList();
	}

	protected void RefreshCategories()
	{
		TStringArray types = m_Module.GetLocationTypes();
		if ( !types )
			return;

		m_Categories = new TStringArray;
		m_Categories.Insert( CATEGORY_ALL );

		for ( int i = 0; i < types.Count(); i++ )
		{
			//! JMTeleportSerialize puts a literal "ALL" at the head of Types -
			//! it is the old form's filter row, stored with the real
			//! categories. This menu already has an "everything" row of its
			//! own, so that one would show up twice.
			if ( types[i] == "ALL" )
				continue;

			m_Categories.Insert( types[i] );
		}

		if ( !m_CategoriesInitialized )
		{
			m_CategoriesInitialized = true;

			m_HiddenCategories = new TStringArray;
			for ( int j = 0; j < m_Categories.Count(); j++ )
			{
				if ( m_Categories[j] == CATEGORY_ALL )
					continue;

				m_HiddenCategories.Insert( m_Categories[j] );
			}
		}

		//! The hidden set is deliberately not pruned against the categories
		//! that still exist. A category emptied by a delete and recreated by
		//! the next save is the same category to the admin who hid it.
		UpdateFilterTooltip();
	}

	// =========================================================================
	//  Left pane callbacks
	// =========================================================================

	void Click_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION + ".Refresh" ) )
			return;

		m_BtnRefresh.TriggerSpin( 2 );
		m_Module.Reload();
	}

	void Type_UpdateList( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_SearchFilter = m_Filter.GetText();

		RebuildList();
	}

	// =========================================================================
	//  Teleporting
	// =========================================================================

	void Click_Teleport( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( CurrentRow() < 0 )
			return;

		CreateAdvancedPlayerConfirm( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_ME_TO", "TeleportMulti", "TeleportSingle", "TeleportSelf", false );
	}

	void TeleportMulti( JMConfirmation confirmation = NULL )
	{
		TeleportToCurrentRow( JM_GetSelected().GetPlayers() );
	}

	void TeleportSingle( JMConfirmation confirmation = NULL )
	{
		TeleportToCurrentRow( {JM_GetSelected().GetPlayers()[0]} );
	}

	void TeleportSelf( JMConfirmation confirmation = NULL )
	{
		TeleportToCurrentRow( {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	//! A saved location goes through Location(), which the module resolves to
	//! the location's CURRENT position by name server-side - the one thing that
	//! still matters once the click that started this confirm is long past.
	//! Every other row kind has no such object to resolve through, so it goes
	//! through Position() with the position captured when the row was built.
	protected void TeleportToCurrentRow( array< string > guids )
	{
		int row = CurrentRow();
		if ( row < 0 || row >= m_ListItems.Count() )
			return;

		JMTeleportRowData item = m_ListItems[row];
		if ( !item )
			return;

		if ( item.m_Kind == ROWKIND_LOCATION )
		{
			JMTeleportLocation location = GetCurrentLocation();
			if ( location )
				m_Module.Location( location, guids );

			return;
		}

		m_Module.Position( item.m_Position, guids );
	}

	// =========================================================================
	//  Coordinates
	// =========================================================================

	void Click_UseMyPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_InputCoords || !g_Game.GetPlayer() )
			return;

		m_InputCoords.SetValue( g_Game.GetPlayer().GetPosition() );
	}

	void Click_CopyCoordinates( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_InputCoords )
			return;

		g_Game.CopyToClipboard( FormatVector( m_InputCoords.GetValue() ) );

		m_BtnCopyCoords.ShowFeedback();
	}

	void Click_PasteCoordinates( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_InputCoords )
			return;

		string clipboard;
		g_Game.CopyFromClipboard( clipboard );

		vector pos = clipboard.BeautifiedToVector();

		//! The copy answers a click with a check mark, so the paste has to
		//! answer one too - otherwise the only button in the strip that can
		//! fail is also the only one that says nothing when it works. A
		//! clipboard holding anything that is not a vector flashes a red cross
		//! instead and the fields are left alone.
		if ( pos == vector.Zero )
		{
			m_BtnPasteCoords.SetFeedbackIcon( JMConstants.Lucide( "x" ) );
			m_BtnPasteCoords.SetFeedbackColor( JMTheme.DANGER_FILL );
			m_BtnPasteCoords.ShowFeedback();
			return;
		}

		m_InputCoords.SetValue( pos );

		m_BtnPasteCoords.SetFeedbackIcon( JMConstants.ICON_CHECK_MARK );
		m_BtnPasteCoords.SetFeedbackColor( JMTheme.SUCCESS_DIM );
		m_BtnPasteCoords.ShowFeedback();
	}

	void Click_GoToCoordinates( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) )
			return;

		CreateAdvancedPlayerConfirm( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_ME_TO", "CoordinatesMulti", "CoordinatesSingle", "CoordinatesSelf", false );
	}

	void CoordinatesMulti( JMConfirmation confirmation = NULL )
	{
		m_Module.Position( m_InputCoords.GetValue(), JM_GetSelected().GetPlayers() );
	}

	void CoordinatesSingle( JMConfirmation confirmation = NULL )
	{
		m_Module.Position( m_InputCoords.GetValue(), {JM_GetSelected().GetPlayers()[0]} );
	}

	void CoordinatesSelf( JMConfirmation confirmation = NULL )
	{
		m_Module.Position( m_InputCoords.GetValue(), {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	//! The "<x, y, z>" shape the player form writes to the clipboard, so a
	//! position copied in one place pastes in the other.
	protected string FormatVector( vector pos )
	{
		return "<" + FormatCoord( pos[0] ) + ", " + FormatCoord( pos[1] ) + ", " + FormatCoord( pos[2] ) + ">";
	}

	//! Whole metres, through an int.
	//!
	//! The old form printed the raw float, which spills a coordinate like
	//! 3712.440185546875 across the column. Rounding to a float and printing
	//! that just moves the noise, so the round lands in an int and the string
	//! is built from that.
	protected string FormatCoord( float value )
	{
		int rounded = Math.Round( value );
		return "" + rounded;
	}

	// =========================================================================
	//  Removing
	// =========================================================================

	void OnClick_DeleteLocation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_REMOVE ) )
			return;

		JMTeleportLocation location = GetCurrentLocation();
		if ( !location )
			return;

		string delMsg = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_DELETE_LOCATION_CONFIRM_PREFIX" ) + location.Name + Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_DELETE_LOCATION_CONFIRM_SUFFIX" );

		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_GENERIC_CONFIRM", delMsg, "#STR_COT_GENERIC_NO", "", "#STR_COT_GENERIC_YES", "OnDeleteLocation_Confirm" );
	}

	void OnDeleteLocation_Confirm( JMConfirmation confirmation )
	{
		JMTeleportLocation location = GetCurrentLocation();
		if ( !location )
			return;

		COTCreateLocalAdminNotification( new StringLocaliser( Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_REMOVED_LOCATION_PREFIX" ) + location.Name ) );

		m_Module.RemoveLocation( location );

		m_SelectedName = "";

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION + ".Refresh" ) )
		{
			RebuildList();
			return;
		}

		m_Module.Reload();
	}

	// =========================================================================
	//  Permission gating
	//
	//  Bound rather than evaluated once: a permission granted while the form is
	//  open has to reach the controls without a close and reopen.
	// =========================================================================

	protected void RegisterRightPermissions()
	{
		RegisterPermission( m_InputCoords,    "Admin.Player.Teleport.Position" );
		RegisterPermission( m_BtnUseMyPos,    "Admin.Player.Teleport.Position" );
		RegisterPermission( m_BtnGoCoords,    "Admin.Player.Teleport.Position" );
		RegisterPermission( m_BtnPasteCoords, "Admin.Player.Teleport.Position" );

		RegisterPermission( m_BtnSaveHere,    "Admin.Player.Teleport.Location.Add" );
	}

	// =========================================================================
	//  Selection
	// =========================================================================

	JMTeleportLocation GetCurrentLocation()
	{
		return FindLocation( m_SelectedName );
	}

	protected JMTeleportLocation FindLocation( string name )
	{
		if ( name == "" || !m_Module || !m_Module.IsLoaded() )
			return NULL;

		array< ref JMTeleportLocation > locations = m_Module.GetLocations();
		if ( !locations )
			return NULL;

		for ( int i = 0; i < locations.Count(); i++ )
		{
			if ( locations[i] && locations[i].Name == name )
				return locations[i];
		}

		return NULL;
	}

	string GetCurrentPositionName()
	{
		return m_SelectedName;
	}
}
