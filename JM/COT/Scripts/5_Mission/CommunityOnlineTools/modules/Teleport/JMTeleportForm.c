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
	protected UIActionImageButton m_BtnRefresh;
	protected UIActionItemList    m_LocationList;
	protected UIActionContextMenu m_RowMenu;
	protected UIActionContextMenu m_MapMenu;

	//! Where the map was right-clicked. Captured when the menu opens so that
	//! panning behind it cannot move the spot the menu is about.
	protected vector m_MapMenuWorldPos;

	//! Strongly-typed list items array replacing legacy parallel arrays.
	protected ref array< ref JMTeleportRowData > m_ListItems = new array< ref JMTeleportRowData >;

	// ---- Right pane --------------------------------------------------------
	protected UIActionMap            m_Map;
	protected ref UIActionMapController m_MapController;
	protected UIActionCard           m_CoordsCard;
	protected UIActionImageButton    m_BtnExpandMap;
	protected UIActionEditableVector m_InputCoords;
	protected UIActionButton         m_BtnGoCoords;
	protected UIActionImageButton    m_BtnUseMyPos;
	protected UIActionImageButton    m_BtnCopyCoords;
	protected UIActionImageButton    m_BtnPasteCoords;
	protected UIActionImageButton    m_BtnSaveHere;

	// ---- State -------------------------------------------------------------
	protected ref TStringArray m_Categories;

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

	//! The filter dropdown and the state behind it. See JMTeleportFilter.
	protected ref JMTeleportFilter m_Filters;

	//! The save / edit location popup. See JMTeleportLocationPopup.
	protected ref JMTeleportLocationPopup m_Popup;

	//! The live map layers (players, vehicles, markers, crashes, toxic zones). See JMTeleportMapOverlays.
	protected ref JMTeleportMapOverlays m_Overlays;

	void JMTeleportForm()
	{
		m_Filters = new JMTeleportFilter( this );
		m_Popup   = new JMTeleportLocationPopup( this );
		m_Overlays = new JMTeleportMapOverlays( this );
	}

	//! The location categories, "everything" row first. Read by JMTeleportFilter.
	TStringArray GetCategories()
	{
		return m_Categories;
	}

	// =========================================================================
	//  Selection
	// =========================================================================

	JMTeleportLocation GetCurrentLocation()
	{
		return FindLocation( m_SelectedName );
	}

	string GetCurrentPositionName()
	{
		return m_SelectedName;
	}

	JMTeleportFilter GetFilters()
	{
		return m_Filters;
	}

	//! The coordinates input; read by the location popup.
	UIActionEditableVector GetInputCoords()
	{
		return m_InputCoords;
	}

	//! The map widget; read by JMTeleportMapOverlays.
	UIActionMap GetMap()
	{
		return m_Map;
	}

	JMTeleportModule GetModule()
	{
		return m_Module;
	}

	//! Name of the row selected in the list; read by the location popup.
	string GetSelectedName()
	{
		return m_SelectedName;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	// -------------------------------------------------------------------------
	//  OnCreate
	// -------------------------------------------------------------------------

	override void OnCreate()
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

		InitWidgetsLeft();
		InitWidgetsRight();
		m_Popup.Build();
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
		DeferCall( "DeferredRemeasureCoords", 50 );
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
			JMSearchRow toolbar = UIActionManager.CreateSearchRow( m_SearchWrapper, "#STR_COT_TELEPORT_MODULE_FILTER", this, "Type_UpdateList", "OnClick_CategoryFilter", "#STR_COT_TELEPORT_MODULE_FILTER_BY_CATEGORY_TOOLTIP", "Click_Refresh", SEARCH_ROW_W, FILTER_BTN_W, REFRESH_BTN_W );
			m_BtnRefresh = toolbar.Refresh;
			m_Filter = toolbar.Search;
			m_Filters.SetButton( toolbar.Filter );
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

		BindRightPermissions();
	}

	protected void BuildMapCard( Widget parent )
	{
		UIActionCard card = UIActionManager.CreateCard( parent, "#STR_COT_TELEPORT_MODULE_MAP_CARD_TITLE" );

		m_BtnExpandMap = UIActionManager.CreateIconButton( card.GetHeaderActions(), JMConstants.Lucide( "maximize-2" ), this, "" );
		if ( m_BtnExpandMap ) m_BtnExpandMap.SetOnClick( this, "Click_ExpandMap" );
		m_BtnExpandMap.SetFixedSize( HEADER_BTN_PX, HEADER_BTN_PX );
		m_BtnExpandMap.SetTooltip( "#STR_COT_TELEPORT_MODULE_EXPAND_MAP_TOOLTIP" );

		UIActionImageButton centerBtn = UIActionManager.CreateIconButton( card.GetHeaderActions(), JMConstants.Lucide( "locate-fixed" ), this, "Click_CenterOnMe" );
		centerBtn.SetFixedSize( HEADER_BTN_PX, HEADER_BTN_PX );
		centerBtn.SetTooltip( "#STR_COT_TELEPORT_MODULE_CENTER_ON_ME_TOOLTIP" );

		m_Map = UIActionManager.CreateMap( card.GetContent(), this, "OnClick_Map", MAP_HEIGHT );
		m_MapController = UIActionManager.CreateMapController( m_Map, this, "OnClick_MapController" );
	}

	void OnClick_MapController( UIActionMapController controller )
	{
		if ( !controller )
			return;

		vector picked = controller.GetPickedWorldPosition();
		if ( m_InputCoords )
			m_InputCoords.SetValue( picked );
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

	void Click_ExpandMap( UIActionBase action )
	{
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
		m_BtnUseMyPos = UIActionManager.CreateIconButton( card.GetHeaderActions(), JMConstants.Lucide( "map-pin" ), this, "" );
		if ( m_BtnUseMyPos ) m_BtnUseMyPos.SetOnClick( this, "Click_UseMyPosition" );
		m_BtnUseMyPos.SetFixedSize( HEADER_BTN_PX, HEADER_BTN_PX );
		m_BtnUseMyPos.SetTooltip( "#STR_COT_TELEPORT_MODULE_USE_MY_POSITION_TOOLTIP" );

		m_BtnSaveHere = card.AddAddButton( this, "", "#STR_COT_TELEPORT_MODULE_SAVE_HERE_TOOLTIP" );
		if ( m_BtnSaveHere ) m_BtnSaveHere.SetOnClick( this, "Click_OpenSavePopup" );

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
	//  Left pane - the location list
	// =========================================================================

	void RebuildList()
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

		JMSearchMatcher matcher = new JMSearchMatcher( m_SearchFilter );

		if ( m_Map )
			m_Map.ClearLayer( MAP_LAYER );

		for ( int i = 0; i < locations.Count(); i++ )
		{
			JMTeleportLocation location = locations[i];
			if ( !location )
				continue;

			if ( m_Filters.IsCategoryHidden( location.Type ) )
				continue;

			if ( !matcher.Matches( location.Name ) )
				continue;

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
		if ( m_Filters.ShowsPlayers() )
			AppendPlayerRows( rowLabels, rowSubs, matcher );

		if ( m_Filters.GetVehicleTypeFilter() != 0 )
			AppendVehicleRows( rowLabels, rowSubs, matcher );

		if ( m_Filters.ShowsMapMarkers() )
			AppendMapMarkerRows( rowLabels, rowSubs, matcher );

		if ( m_Filters.ShowsHeliCrashes() )
			AppendHeliCrashRows( rowLabels, rowSubs, matcher );

		if ( m_Filters.ShowsToxicZones() )
			AppendToxicZoneRows( rowLabels, rowSubs, matcher );

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
			m_Filter.SetTextPreview( matcher.GetClosestMatch() );
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
	protected bool PassesSearch( string label, JMSearchMatcher matcher )
	{
		return matcher.Matches( label );
	}

	protected void AppendPlayerRows( TStringArray rowLabels, TStringArray rowSubs, JMSearchMatcher matcher )
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

			if ( !m_Filters.IsPlayerVisibleByRole( player ) )
				continue;

			string name = player.GetName();

			if ( !PassesSearch( name, matcher ) )
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
	protected void AppendVehicleRows( TStringArray rowLabels, TStringArray rowSubs, JMSearchMatcher matcher )
	{
		if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW ) )
			return;

		JMVehiclesModule vehicles = m_Overlays.VehiclesModule();
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

			if ( meta.m_VehicleType != 0 && ( meta.m_VehicleType & m_Filters.GetVehicleTypeFilter() ) == 0 )
				continue;

			if ( !PassesSearch( meta.m_DisplayName, matcher ) )
				continue;

			rowLabels.Insert( meta.m_DisplayName );
			rowSubs.Insert( kindLabel );

			AddRow( "veh_" + i, ROWKIND_VEHICLE, meta.m_Position, "tp_veh_" + i );
		}
	}

	//! Walks the Expansion server-marker list once; shared by AppendMapMarkerRows
	//! and RefreshMapMarkers so the list and the map can never disagree about
	//! what a marker is called or where it sits.
	void CollectMapMarkerEntries( out array< ref JMTeleportMapEntry > entries )
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

	protected void AppendMapMarkerRows( TStringArray rowLabels, TStringArray rowSubs, JMSearchMatcher matcher )
	{
		string kindLabel = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_ROW_MAP_MARKER" );

		array< ref JMTeleportMapEntry > entries;
		CollectMapMarkerEntries( entries );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			JMTeleportMapEntry entry = entries[i];

			if ( !PassesSearch( entry.m_Label, matcher ) )
				continue;

			rowLabels.Insert( entry.m_Label );
			rowSubs.Insert( kindLabel );

			AddRow( entry.m_Id, ROWKIND_MARKER, entry.m_Position, "tp_" + entry.m_Id );
		}
	}

	//! Walks whichever heli-crash list is live once; shared by AppendHeliCrashRows
	//! and RefreshHeliCrashMarkers.
	void CollectHeliCrashEntries( out array< ref JMTeleportMapEntry > entries )
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

	protected void AppendHeliCrashRows( TStringArray rowLabels, TStringArray rowSubs, JMSearchMatcher matcher )
	{
		string crashName = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_HELI_CRASH_MARKER" );

		if ( !PassesSearch( crashName, matcher ) )
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
	void CollectToxicZoneEntries( out array< ref JMTeleportMapEntry > entries )
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

	protected void AppendToxicZoneRows( TStringArray rowLabels, TStringArray rowSubs, JMSearchMatcher matcher )
	{
		string zoneName = Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_TOXIC_ZONE_MARKER" );

		if ( !PassesSearch( zoneName, matcher ) )
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
			m_RowMenu = UIActionManager.CreateOverlayMenu( this, this, "OnClick_RowMenu" );

			if ( !m_RowMenu )
				return;

			AddOverlay( m_RowMenu );
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

		m_RowMenu.OpenAt( mx, my );
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

			m_Popup.ShowPopup( location.Name, location.Position, location.Name, location.Type );
			return;
		}

		if ( id == MENU_DELETE )
			OnClick_DeleteLocation( UIEvent.CLICK, NULL );
	}

	// =========================================================================
	//  Live overlays - players and vehicles
	//
	//  Neither belongs to this module. They are drawn here because the map is
	//  here, each in its own layer, so switching one off cannot disturb the
	//  locations or the other one.
	// =========================================================================

	//! Start or stop the poll, and take down whatever was just switched off.
	void ApplyOverlays()
	{
		CancelDeferredCall( "RefreshOverlays" );

		if ( m_Map )
		{
			if ( !m_Filters.ShowsPlayers() )
				m_Map.ClearLayer( MAP_LAYER_PLAYERS );

			if ( m_Filters.GetVehicleTypeFilter() == 0 )
				m_Map.ClearLayer( MAP_LAYER_VEHICLES );

			if ( !m_Filters.ShowsMapMarkers() )
				m_Map.ClearLayer( MAP_LAYER_MARKERS );

			if ( !m_Filters.ShowsHeliCrashes() )
				m_Map.ClearLayer( MAP_LAYER_HELICRASH );

			if ( !m_Filters.ShowsToxicZones() )
				m_Map.ClearLayer( MAP_LAYER_TOXICZONES );
		}

		if ( !m_Filters.ShowsPlayers() && m_Filters.GetVehicleTypeFilter() == 0 && !m_Filters.ShowsMapMarkers() && !m_Filters.ShowsHeliCrashes() && !m_Filters.ShowsToxicZones() )
			return;

		//! The vehicle list is a server round trip, not a local read, so it is
		//! asked for once when the overlay is switched on rather than on every
		//! tick. The module keeps it current from its own deltas after that.
		if ( m_Filters.GetVehicleTypeFilter() != 0 )
			m_Overlays.RequestVehicles();

		RefreshOverlays();

		DeferCall( "RefreshOverlays", OVERLAY_REFRESH_MS, true );
	}

	void RefreshOverlays()
	{
		if ( !m_Map )
			return;

		if ( m_Filters.ShowsPlayers() )
			m_Overlays.RefreshPlayerMarkers();

		if ( m_Filters.GetVehicleTypeFilter() != 0 )
			m_Overlays.RefreshVehicleMarkers();

		if ( m_Filters.ShowsMapMarkers() )
			RefreshMapMarkers();

		if ( m_Filters.ShowsHeliCrashes() )
			m_Overlays.RefreshHeliCrashMarkers();

		if ( m_Filters.ShowsToxicZones() )
			m_Overlays.RefreshToxicZoneMarkers();

		//! The list mirrors these same overlays now, so a join, a leave or a
		//! vehicle spawning has to reach it on the same tick that moves its dot
		//! on the map - not only on the next search keystroke or filter toggle.
		RebuildList();
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
			m_Overlays.ShowHoverFor( hoveredId );
			return;
		}

		if ( eid == UIEvent.MOUSE_LEAVE )
		{
			m_Overlays.HideVehicleHoverInfo();
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
			m_MapMenu = UIActionManager.CreateOverlayMenu( this, this, "OnClick_MapMenu" );

			if ( !m_MapMenu )
				return;

			AddOverlay( m_MapMenu );

			m_MapMenu.AddItem( MENU_MAP_GO,   "#STR_COT_TELEPORT_MODULE_MENU_TELEPORT_HERE",   JMConstants.Lucide( "move-3d"  ) );
			m_MapMenu.AddItem( MENU_MAP_SAVE, "#STR_COT_TELEPORT_MODULE_MENU_SAVE_AS_LOCATION", JMConstants.Lucide( "map-pin-plus" ) );
		}

		m_MapMenu.SetItemEnabled( MENU_MAP_GO,   JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) );
		m_MapMenu.SetItemEnabled( MENU_MAP_SAVE, JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_CREATE ) );

		int mx, my;
		GetMousePos( mx, my );

		m_MapMenu.OpenAt( mx, my );
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

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION_CREATE ) )
			return;

		m_Popup.ShowPopup( "", m_MapMenuWorldPos, "", "" );
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
		m_Popup.HidePopup();
		m_Overlays.HideVehicleHoverInfo();

		m_Filters.CloseMenu();

		CancelDeferredCall( "RefreshOverlays" );
		CancelDeferredCall( "DeferredRemeasureCoords" );

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

		m_Filters.OnCategoriesRefreshed();
	}

	// =========================================================================
	//  Left pane callbacks
	// =========================================================================

	void Click_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION ) )
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

		ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_ME_TO", this, "TeleportTargets", false );
	}

	void TeleportTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		TeleportToCurrentRow( targets );
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

	void Click_UseMyPosition( UIActionBase action )
	{
		if ( !m_InputCoords || !g_Game.GetPlayer() )
			return;

		m_InputCoords.SetValue( g_Game.GetPlayer().GetPosition() );
	}

	void Click_CopyCoordinates( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_InputCoords )
			return;

		COTFeedback.Copy( FormatVector( m_InputCoords.GetValue() ) );

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

		ConfirmPlayerAction( "#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_ME_TO", this, "CoordinatesTargets", false );
	}

	void CoordinatesTargets( array<string> targets, JMConfirmation confirmation = NULL )
	{
		m_Module.Position( m_InputCoords.GetValue(), targets );
	}

	//! The "<x, y, z>" shape the player form writes to the clipboard, so a
	//! position copied in one place pastes in the other.
	string FormatVector( vector pos )
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

		ConfirmAction( "#STR_COT_GENERIC_CONFIRM", delMsg, "OnDeleteLocation_Confirm", "#STR_COT_GENERIC_YES", "#STR_COT_GENERIC_NO" );
	}

	void OnDeleteLocation_Confirm( JMConfirmation confirmation )
	{
		JMTeleportLocation location = GetCurrentLocation();
		if ( !location )
			return;

		COTCreateLocalAdminNotification( new StringLocaliser( Widget.TranslateString( "#STR_COT_TELEPORT_MODULE_REMOVED_LOCATION_PREFIX" ) + location.Name ) );

		m_Module.RemoveLocation( location );

		m_SelectedName = "";

		if ( !JMPermissions.Has( JMConstants.PERM_PLAYER_TELEPORT_LOCATION ) )
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

	protected void BindRightPermissions()
	{
		BindPermission( m_InputCoords,    JMConstants.PERM_PLAYER_TELEPORT_POSITION );
		BindPermission( m_BtnUseMyPos,    JMConstants.PERM_PLAYER_TELEPORT_POSITION );
		BindPermission( m_BtnGoCoords,    JMConstants.PERM_PLAYER_TELEPORT_POSITION );
		BindPermission( m_BtnPasteCoords, JMConstants.PERM_PLAYER_TELEPORT_POSITION );

		BindPermission( m_BtnSaveHere,    JMConstants.PERM_PLAYER_TELEPORT_LOCATION_CREATE );
	}

	JMTeleportLocation FindLocation( string name )
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

	//! The search row resolves its callbacks on the form, so the filter button's click lands here.
	void OnClick_CategoryFilter( UIEvent eid, UIActionBase action )
	{
		m_Filters.OnClick_CategoryFilter( eid, action );
	}

	//! The coordinates card resolves its button callbacks on the form.
	void Click_OpenSavePopup( UIActionBase action )
	{
		m_Popup.Click_OpenSavePopup( action );
	}

	//! Extension point, unchanged: override, call super, then add your own markers to m_Map
	//! (see Ex_FilterTeleportMenu.c). The built-in marker layer is drawn by JMTeleportMapOverlays.
	protected void RefreshMapMarkers()
	{
		m_Overlays.RefreshCustomMarkers();
	}
}
