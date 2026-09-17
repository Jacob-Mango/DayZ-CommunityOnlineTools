// =============================================================================
//  JMLootAnalysisForm
//
//  Search / category filter / recent chips and the shared item picker on the
//  left, modelled directly on the object spawner's own left column. A tab
//  strip over Item Scan and Loot Distribution on the right, each tab a fixed
//  action row, a UIActionMap filling whatever space is left, and a stats
//  panel (Distribution also gets a marker-type legend) pinned under the map.
//
//  ---------------------------------------------------------------------------
//  WHAT REPLACED WHAT
//
//  "Show On Map" / "Find CE Spawn Locations" used to open JMMapModule and hand
//  it the results - a second window for what is fundamentally this module's
//  own output. Each tab now owns a UIActionMap, so the list, the actions and
//  the result markers stay in one window.
//
//  The item list was TextListboxWidget, which draws in the engine's debug
//  style and builds one widget per row - for the ~14000 spawnable classes this
//  scan walks, that is 14000 widgets rebuilt on every keystroke. UIActionItemList
//  is what the object spawner already moved to for the identical scan; a pool
//  of ~20 rows costs the same at 20 items as at 20000.
//
//  The category grid (two rows of eight buttons) is gone in favour of the
//  object spawner's own shape: a filter button beside the search box opens a
//  category menu, and the categories actually used land on a "recent" chip
//  row underneath - one row of chrome instead of two, and it does not run out
//  of room as usages/tiers get added below it.
// =============================================================================

class JMLootAnalysisForm: JMFormBase
{
	static const int TAB_ITEM_SCAN    = 0;
	static const int TAB_DISTRIBUTION = 1;

	//! Layers this form owns on its own two maps.
	static const string MAP_LAYER_SCAN = "loot_scan";
	static const string MAP_LAYER_DIST = "loot_distribution";

	//! Marker right-click menu rows.
	static const string MENU_MK_TELEPORT = "mk_teleport";
	static const string MENU_MK_COPYPOS  = "mk_copypos";
	static const string MENU_MK_FOCUS    = "mk_focus";
	static const string MENU_MK_COPYNAME = "mk_copyname";

	//! Category filter menu id for "".
	static const string MENU_ID_ALL = "__all";

	//! Left column bands, top to bottom, in layout pixels. Pinned from
	//! OnResize rather than left to the layout file - see JMObjectSpawnerForm
	//! for why two sources for the same number is how a chip strip ends up
	//! overlapping the list it sits above.
	static const int SEARCH_H = 34;
	static const int RECENT_H = 36;

	//! Search box and filter button, as fractions of the search row - never a
	//! fixed pixel width beside a fractional one. A WrapSpacer wraps the
	//! moment its children total more than the row, and mixing units (as this
	//! used to) makes that edge impossible to reason about - see
	//! JMTeleportForm's own SEARCH_ROW_W/FILTER_BTN_W for the same rule. Sum
	//! to less than 1.
	static const float SEARCH_ROW_W = 0.85;
	static const float FILTER_ROW_W = 0.14;

	//! Sum of the two above. Written out because a static const folded from
	//! other static consts evaluates to 0 in Enforce - keep the two and this
	//! in step by hand.
	static const int LEFT_HEADER_H = 70;

	//! Right column bands within each tab: the map card filling whatever is
	//! left, and an "Item Info" stats card (Distribution's also holds the
	//! clickable type legend) under it. The action row is gone - Show On Map
	//! / Clear / Delete All now live in the map card's own header.
	static const int STATS_H = 64;

	//! What UIActionCard adds around its content: header height plus its top
	//! and bottom padding. Duplicated here rather than read off UIActionCard
	//! itself - those constants describe ITS OWN layout, not a contract this
	//! form should reach into - the same way JMTeleportForm keeps its own
	//! CARD_CHROME beside the map card it lays out.
	static const int MAP_CARD_CHROME = 44;

	//! How many recent categories are kept - a shortcut back to what was just
	//! used, not a second category list.
	static const int RECENT_MAX = 4;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMLootAnalysisModule m_Module;

	// ---- Left column --------------------------------------------------------
	protected Widget m_SearchWrapper;
	protected Widget m_RecentWrapper;
	protected Widget m_ListWrapper;

	protected UIActionSearchBox m_SearchBox;
	protected UIActionImageButton m_FilterButton;
	protected UIActionContextMenu m_CategoryMenu;

	protected string m_SearchFilter = "";
	protected string m_CurrentCategory = "";

	protected ref TStringArray m_RecentIds = new TStringArray;
	protected ref array<ref UIActionButton> m_RecentButtons = new array<ref UIActionButton>;

	protected UIActionItemList m_ItemList;

	// ---- Right column --------------------------------------------------------
	protected UIActionTabs m_Tabs;
	protected Widget m_TabItemScan;
	protected Widget m_TabDistribution;

	//! Last content height OnResize computed for the right column - a tab
	//! built lazily after the last resize has missed every pass so far and
	//! needs laying out immediately, not on the next window resize that may
	//! never come.
	protected float m_LastRightContentH;

	// Item Scan tab
	protected Widget m_ScanMapWrapper;
	protected Widget m_ScanStatsWrapper;
	protected UIActionMap m_ItemScanMap;
	protected UIActionImageButton m_ShowOnMapButton;
	protected UIActionImageButton m_ClearMarkersButton;
	protected UIActionImageButton m_DeleteAllButton;
	protected UIActionCard m_ScanStatsCard;
	protected UIActionKeyValueList m_ScanStatsList;

	//! Real height of m_ScanStatsCard, corrected from STATS_H (a seed, not a
	//! measurement) once the card has something to measure - see
	//! MeasureScanStatsCard. A card sizes itself to its rows, and the row
	//! count here varies with whether CE data was found, so a fixed band
	//! either cut the Tiers row off or left dead space under a short one.
	protected float m_ScanStatsH = STATS_H;

	// Loot Distribution tab
	protected Widget m_DistMapWrapper;
	protected Widget m_DistStatsWrapper;

	//! Legend row, built directly under m_DistStatsCard.GetContent() - not a
	//! wrapper UIActionManager.CreatePanel() (default height -1, i.e. its own
	//! Size-To-Content-V - see UIPanel.layout), which nested inside a card's
	//! content stacks a SECOND auto-sizing widget on top of the card's own
	//! one. UIActionCard's header comment calls exactly-one-Size-To-Content-V
	//! a hard rule; the double gave m_DistStatsCard an unstable measured
	//! height, which is what starved the map band down to nothing under it.
	protected Widget m_DistLegendRow;
	protected UIActionMap m_DistributionMap;
	protected UIActionImageButton m_FindSpawnsButton;
	protected UIActionImageButton m_ClearDistMarkersButton;
	protected UIActionCard m_DistStatsCard;
	protected UIActionKeyValueList m_DistStatsList;
	protected float m_DistStatsH = STATS_H;

	//! Distinct matched-usage strings in the current Distribution result set,
	//! in first-seen order - what RebuildDistributionLegend() renders. Parallel
	//! to m_LegendBadges, the same way m_RecentIds is parallel to
	//! m_RecentButtons: one build pass, so the two can never drift apart.
	protected ref TStringArray m_DistTypesSeen = new TStringArray;
	protected ref array<ref UIActionBadge> m_LegendBadges = new array<ref UIActionBadge>;

	//! Usages the legend has toggled off - RepaintDistributionMarkers skips
	//! any marker whose usage is in here. Not "deleted", just hidden: toggling
	//! the badge again brings them straight back from m_DistMarker*All below,
	//! no re-request to the server needed.
	protected ref TStringArray m_HiddenUsages = new TStringArray;

	//! Marker id -> building display name. Distribution markers draw as bare
	//! coloured dots (a name label per marker was unreadable clutter at CE
	//! density), so the name is kept here instead for the right-click menu's
	//! Copy Name, rather than lost with it.
	protected ref map<string, string> m_DistMarkerNames = new map<string, string>;

	//! Every Distribution marker from the last result set, regardless of
	//! whether its usage is currently hidden - the source of truth
	//! RepaintDistributionMarkers repaints from. Parallel arrays, same shape
	//! as the RPC's own names/positions/types.
	protected ref TStringArray m_DistMarkerIdsAll = new TStringArray;
	protected ref array<vector> m_DistMarkerPositionsAll = new array<vector>;
	protected ref TStringArray m_DistMarkerUsagesAll = new TStringArray;

	//! CE tuning values for the last classname either tab asked about. Shared
	//! across both tabs' stats panels - nominal/tiers are a property of the
	//! item, not of which button was pressed.
	protected ref JMLootItemTypeInfo m_LastTypeInfo;

	//! Nominal/Min/Lifetime/Restock editors, one set per tab's Item Info
	//! card. Read on Save (see SaveCEData) and written back to the
	//! classname's source types.xml - see JMLootAnalysisModule.SaveItemTypeInfo.
	protected ref JMLootCEEditRow m_ScanCEEdit;
	protected ref JMLootCEEditRow m_DistCEEdit;
	protected UIActionImageButton m_ScanSaveCEButton;
	protected UIActionImageButton m_DistSaveCEButton;

	//! Item Info header Reload - re-fetches CE data for whatever classname is
	//! currently shown, without re-running a scan/distribution search (see
	//! JMLootAnalysisModule.RequestItemTypeInfo). Not permission-gated, same
	//! as every other AddRefreshButton in this codebase - it is a read-only
	//! re-fetch, not an admin action.
	protected UIActionImageButton m_ScanReloadButton;
	protected UIActionImageButton m_DistReloadButton;

	//! Values captured at Save click, re-applied from SaveCEData_Yes once the
	//! confirmation dialog answers - the dialog callback gets no arguments of
	//! its own to carry them through.
	protected string m_PendingSaveCEClassName;
	protected int    m_PendingSaveCENominal;
	protected int    m_PendingSaveCEMin;
	protected int    m_PendingSaveCELifetime;
	protected int    m_PendingSaveCERestock;

	//! Item Scan marker id -> per-entity health/quantity/lifetime/attachments,
	//! for the hover preview - see OnMarker_HoverEnter. Distribution markers
	//! have no equivalent: they are potential spawn points, not entities.
	protected ref map<string, ref JMLootScanItemDetail> m_ScanItemDetails = new map<string, ref JMLootScanItemDetail>;

	//! Floating hover-preview panel over the Item Scan map, built once and
	//! reused - mirrors JMVehiclesForm's own m_HoverInfo.
	protected ref JMLootAnalysisHoverInfo m_HoverInfo;

	//! Right-click marker menu - shared by both maps, since the action set is
	//! identical either way.
	protected UIActionContextMenu m_MarkerMenu;
	protected UIActionMap m_MarkerMenuMap;
	protected vector m_MarkerMenuPos;
	protected string m_MarkerMenuLabel;

	//! Stored classname for Delete All confirmation (prevents stale selection)
	protected string m_PendingDeleteClassName;

	protected override bool SetModule(JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}

	override void OnInit()
	{
		m_LeftPanel         = layoutRoot.FindAnyWidget("panel_left");
		m_RightPanel        = layoutRoot.FindAnyWidget("panel_right");
		m_RightTabStrip     = layoutRoot.FindAnyWidget("panel_right_tabs");
		m_RightContent      = layoutRoot.FindAnyWidget("panel_right_content");
		m_RightPanelDisable = layoutRoot.FindAnyWidget("panel_right_disable");

		m_SearchWrapper = layoutRoot.FindAnyWidget("la_search_wrapper");
		m_RecentWrapper = layoutRoot.FindAnyWidget("la_recent_wrapper");
		m_ListWrapper   = layoutRoot.FindAnyWidget("la_list_wrapper");

		InitWidgetsLeft();
		InitWidgetsRight();
	}

	//! The list reaches as far down the left column as the window allows,
	//! which is the only measurement on the left. On the right, each tab's
	//! map takes whatever is left after its own fixed bands.
	override void OnResize(float w, float h)
	{
		super.OnResize(w, h);

		PinBand(m_SearchWrapper, 0, SEARCH_H);
		PinBand(m_RecentWrapper, SEARCH_H, RECENT_H);

		if (m_ListWrapper && h > LEFT_HEADER_H)
		{
			float listH = h - LEFT_HEADER_H;

			PinBand(m_ListWrapper, LEFT_HEADER_H, listH);

			//! The list is told the height rather than measuring it - same
			//! number, one source.
			if (m_ItemList)
				m_ItemList.SetViewportHeight(listH);
		}

		PinRightPanelGeometry(h);

		m_LastRightContentH = h - TAB_STRIP_HEIGHT;

		LayoutItemScanTab(m_LastRightContentH);
		LayoutDistributionTab(m_LastRightContentH);
	}

	//! Place one band: exact y, exact height, full width.
	protected void PinBand(Widget band, float y, float height)
	{
		if (!band)
			return;

		band.SetFlags(WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true);
		band.SetPos(0, y);
		band.SetSize(1, height);
	}

	protected void LayoutItemScanTab(float contentH)
	{
		if (!m_ScanMapWrapper)
			return;

		float mapCardH = contentH - m_ScanStatsH;
		if (mapCardH < 0)
			mapCardH = 0;

		PinBand(m_ScanMapWrapper, 0, mapCardH);
		PinBand(m_ScanStatsWrapper, mapCardH, m_ScanStatsH);

		if (m_ItemScanMap)
		{
			float mapH = mapCardH - MAP_CARD_CHROME;
			if (mapH < 0)
				mapH = 0;
			m_ItemScanMap.SetHeight(mapH);
		}
	}

	//! m_ScanStatsH starts at the STATS_H seed and is corrected here once the
	//! card has laid out its rows for real - it cannot know that on the frame
	//! it is built, and the row count itself only settles once a scan result
	//! actually arrives. Re-checked every Update() tick the tab is active;
	//! the comparison is a cheap float read, and it only ever relayouts when
	//! the measured height actually moved.
	protected void MeasureScanStatsCard()
	{
		if (!m_ScanStatsCard || m_LastRightContentH <= 0)
			return;

		float cw, ch;
		m_ScanStatsCard.GetLayoutRoot().GetScreenSize(cw, ch);

		if (ch < 1 || Math.AbsFloat(ch - m_ScanStatsH) < 1)
			return;

		m_ScanStatsH = ch;
		LayoutItemScanTab(m_LastRightContentH);
	}

	//! The legend used to be a third band of its own; it now lives inside the
	//! stats card (see InitWidgetsDistribution), so its height is already
	//! folded into m_DistStatsH by MeasureDistStatsCard - nothing extra to
	//! account for here.
	protected void LayoutDistributionTab(float contentH)
	{
		if (!m_DistMapWrapper)
			return;

		float mapCardH = contentH - m_DistStatsH;
		if (mapCardH < 0)
			mapCardH = 0;

		PinBand(m_DistMapWrapper, 0, mapCardH);
		PinBand(m_DistStatsWrapper, mapCardH, m_DistStatsH);

		if (m_DistributionMap)
		{
			float mapH = mapCardH - MAP_CARD_CHROME;
			if (mapH < 0)
				mapH = 0;
			m_DistributionMap.SetHeight(mapH);
		}
	}

	//! See MeasureScanStatsCard.
	protected void MeasureDistStatsCard()
	{
		if (!m_DistStatsCard || m_LastRightContentH <= 0)
			return;

		float cw, ch;
		m_DistStatsCard.GetLayoutRoot().GetScreenSize(cw, ch);

		if (ch < 1 || Math.AbsFloat(ch - m_DistStatsH) < 1)
			return;

		m_DistStatsH = ch;
		LayoutDistributionTab(m_LastRightContentH);
	}

	//! Markers are real widgets repositioned every tick (see UIActionMap), so
	//! only the map on the visible tab is ticked - the other has nothing to
	//! place itself against. Same for the stats card measurement below it.
	override void Update()
	{
		super.Update();

		if (IsTabActive(TAB_ITEM_SCAN))
		{
			MeasureScanStatsCard();
			if (m_ItemScanMap)
				m_ItemScanMap.TickMarkers();
		}

		if (IsTabActive(TAB_DISTRIBUTION))
		{
			MeasureDistStatsCard();
			if (m_DistributionMap)
				m_DistributionMap.TickMarkers();
		}
	}

	// =========================================================================
	//  Left column
	// =========================================================================

	protected void InitWidgetsLeft()
	{
		if (m_SearchWrapper)
		{
			//! Search box and filter button only - the search box already has
			//! its own built-in clear "x" (see UIActionSearchBox), so a second
			//! one here was a redundant fixed-size sibling that fought the
			//! fractional search box for room on a narrow left column.
			//!
			//! Compact, not the normal spacer: the normal variant adds its own
			//! per-child padding/margin on top of the two widths below, and
			//! that was enough extra room for the filter button to overflow
			//! onto a second line the fixed-height search band then clipped
			//! off - exactly what ObjectSpawner and JMTeleportForm avoid by
			//! using the compact spacer for this same row.
			Widget searchSpacer = UIActionManager.CreateWrapSpacerCompact(m_SearchWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);
			m_SearchBox = UIActionManager.CreateSearchBox(searchSpacer, this, "OnChange_Search", "Search:");
			m_SearchBox.SetWidth(SEARCH_ROW_W);

			m_FilterButton = UIActionManager.CreateIconButton(searchSpacer, JMConstants.Lucide("list-filter"), this, "OnClick_CategoryFilter");
			m_FilterButton.SetWidth(FILTER_ROW_W);
			m_FilterButton.SetTooltip("Filter by item category");
		}

		// Shared item list - pooled rows, not one widget per class. See the
		// class header comment for why this replaced TextListboxWidget.
		if (m_ListWrapper)
		{
			m_ItemList = UIActionManager.CreateItemList(m_ListWrapper, this, "OnClick_ItemList");
			m_ItemList.SetEmptyText("No items found");
		}

		UpdateItemList();
	}

	// -------------------------------------------------------------------------
	//  Categories - id / icon / label, one table read by both the filter menu
	//  and the recent chips so the two can never drift apart.
	// -------------------------------------------------------------------------

	static void CategoryTable(out TStringArray ids, out TStringArray icons, out TStringArray labels)
	{
		ids = new TStringArray;
		icons = new TStringArray;
		labels = new TStringArray;

		AddCategoryRow(ids, icons, labels, "",               JMConstants.Lucide("layers"),     "All");
		AddCategoryRow(ids, icons, labels, "edible_base",     JMConstants.ICON_MEAT,            "Food");
		AddCategoryRow(ids, icons, labels, "transport",       JMConstants.ICON_JEEP,            "Vehicles");
		AddCategoryRow(ids, icons, labels, "weapon_base",     JMConstants.ICON_FAMAS,           "Firearms");
		AddCategoryRow(ids, icons, labels, "magazine_base",   JMConstants.ICON_MACHINE_GUN_MAG, "Ammo/Mags");
		AddCategoryRow(ids, icons, labels, "clothing_base",   JMConstants.ICON_CLOTHES,         "Clothing");
		AddCategoryRow(ids, icons, labels, "container_base",  JMConstants.ICON_KNAPSACK,        "Containers");
		AddCategoryRow(ids, icons, labels, "inventory_base",  JMConstants.ICON_FULL_FOLDER,     "Items");
	}

	static void AddCategoryRow(TStringArray ids, TStringArray icons, TStringArray labels, string id, string icon, string label)
	{
		ids.Insert(id);
		icons.Insert(icon);
		labels.Insert(label);
	}

	protected string MenuIdFor(string categoryId)
	{
		if (categoryId == "")
			return MENU_ID_ALL;

		return categoryId;
	}

	protected string CategoryIdFor(string menuId)
	{
		if (menuId == MENU_ID_ALL)
			return "";

		return menuId;
	}

	protected string CategoryLabel(string id)
	{
		TStringArray ids, icons, labels;
		CategoryTable(ids, icons, labels);

		for (int i = 0; i < ids.Count(); i++)
			if (ids[i] == id)
				return labels[i];

		return "";
	}

	protected string CategoryIcon(string id)
	{
		TStringArray ids, icons, labels;
		CategoryTable(ids, icons, labels);

		for (int i = 0; i < ids.Count(); i++)
			if (ids[i] == id)
				return icons[i];

		return JMConstants.Lucide("layers");
	}

	//! Open the category list under the filter button, the way the object
	//! spawner opens its own.
	void OnClick_CategoryFilter(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_FilterButton)
			return;

		if (!m_CategoryMenu)
		{
			if (!m_Window)
				return;

			m_CategoryMenu = UIActionManager.CreateContextMenu(layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_CategoryMenu");

			if (!m_CategoryMenu)
				return;

			RegisterOverlay(m_CategoryMenu);
			m_CategoryMenu.SetOwnerWidget(m_FilterButton.GetLayoutRoot());
		}

		if (m_CategoryMenu.IsOpen())
		{
			m_CategoryMenu.Close();
			return;
		}

		RebuildCategoryMenu();

		float fx, fy, fw, fh;
		m_FilterButton.GetLayoutRoot().GetScreenPos(fx, fy);
		m_FilterButton.GetLayoutRoot().GetScreenSize(fw, fh);

		m_CategoryMenu.ShowAt(fx, fy + fh);
	}

	protected void RebuildCategoryMenu()
	{
		if (!m_CategoryMenu)
			return;

		m_CategoryMenu.ClearItems();

		TStringArray ids, icons, labels;
		CategoryTable(ids, icons, labels);

		for (int i = 0; i < ids.Count(); i++)
		{
			int color = 0;
			if (ids[i] == m_CurrentCategory)
				color = JMTheme.ACCENT;

			m_CategoryMenu.AddItem(MenuIdFor(ids[i]), labels[i], icons[i], color);
		}
	}

	void OnClick_CategoryMenu(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_CategoryMenu)
			return;

		SelectCategory(CategoryIdFor(m_CategoryMenu.GetLastClickedId()));
	}

	//! Which chip was pressed, from the action that fired. The buttons carry
	//! no id of their own - their position in m_RecentButtons IS their
	//! position in m_RecentIds, because the two are built in one pass.
	void OnClick_RecentChip(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		UIActionButton chip;
		if (!Class.CastTo(chip, action))
			return;

		int index = m_RecentButtons.Find(chip);
		if (index < 0 || index >= m_RecentIds.Count())
			return;

		SelectCategory(m_RecentIds[index]);
	}

	protected void SelectCategory(string id)
	{
		m_CurrentCategory = id;

		//! A chip press must never destroy the strip it landed on - the
		//! rebuild is deferred a tick so it cannot happen inside the click
		//! that caused it.
		if (PushRecentCategory(id))
			g_Game.GetCallQueue(CALL_CATEGORY_GUI).Call(RebuildRecentCategories);

		UpdateItemList();
	}

	//! Newest first, capped. "All" gets no chip - it is the first row of the
	//! menu.
	protected bool PushRecentCategory(string id)
	{
		if (id == "")
			return false;

		if (m_RecentIds.Find(id) >= 0)
			return false;

		m_RecentIds.InsertAt(id, 0);

		while (m_RecentIds.Count() > RECENT_MAX)
			m_RecentIds.Remove(m_RecentIds.Count() - 1);

		return true;
	}

	protected void RebuildRecentCategories()
	{
		if (!m_RecentWrapper)
			return;

		//! Drop the script references BEFORE the widgets that own them go, so
		//! a chip is never left alive with a layoutRoot that has been
		//! unlinked.
		m_RecentButtons.Clear();

		Widget child = m_RecentWrapper.GetChildren();
		while (child)
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		if (m_RecentIds.Count() == 0)
			return;

		Widget strip = UIActionManager.CreateWrapSpacerCompact(m_RecentWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);

		float chipW = 0.99 / m_RecentIds.Count();

		for (int i = 0; i < m_RecentIds.Count(); i++)
		{
			UIActionButton chip = UIActionManager.CreateButton(strip, CategoryLabel(m_RecentIds[i]), this, "OnClick_RecentChip");
			if (!chip)
				continue;

			chip.SetWidth(chipW);
			chip.SetIcon(CategoryIcon(m_RecentIds[i]));

			m_RecentButtons.Insert(chip);
		}
	}

	void OnChange_Search(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CHANGE)
			return;

		m_SearchFilter = m_SearchBox.GetText();

		//! Debounced: this scans every CfgVehicles/CfgWeapons/CfgMagazines
		//! entry, so running it on every keystroke lags typing once the
		//! server has a large modded item set.
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateItemList);
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateItemList, 300, false);
	}

	// -------------------------------------------------------------------------
	//  UpdateItemList - iterate CfgVehicles / CfgWeapons / CfgMagazines, apply
	//  category + keyword search, set autocomplete preview on the search box.
	//  Note: JMEntityTracker is server-side only and always returns 0 on the
	//  client, so we scan config directly here. The server scan (ShowItemsOnMap)
	//  filters to actually-spawned entities when the user requests map markers.
	// -------------------------------------------------------------------------
	void UpdateItemList()
	{
		if (!m_ItemList)
			return;

		COT_String strSearch = m_SearchFilter;
		bool requireAllKeywords;
		TStringArray keywords = strSearch.KeywordSearch_Prepare(requireAllKeywords);
		string closestMatch;

		TStringArray configs = new TStringArray;
		configs.Insert(CFG_VEHICLESPATH);
		configs.Insert(CFG_WEAPONSPATH);
		configs.Insert(CFG_MAGAZINESPATH);

		array<string> names = new array<string>;

		COT_String strNameLower;

		for (int nConfig = 0; nConfig < configs.Count(); nConfig++)
		{
			string strConfigPath = configs.Get(nConfig);
			int nClasses = g_Game.ConfigGetChildrenCount(strConfigPath);

			for (int nClass = 0; nClass < nClasses; nClass++)
			{
				string strName;
				g_Game.ConfigGetChildName(strConfigPath, nClass, strName);

				int scope = g_Game.ConfigGetInt(strConfigPath + " " + strName + " scope");
				if (scope == 0)
					continue;

				strNameLower = strName;
				strNameLower.ToLower();

				// Apply category filter
				if (m_CurrentCategory != "" && !g_Game.IsKindOf(strNameLower, m_CurrentCategory))
					continue;

				// Apply keyword search filter
				if (strSearch != "" && !strNameLower.KeywordSearchImplEx(strSearch, keywords, requireAllKeywords, closestMatch))
					continue;

				names.Insert(strName);
			}
		}

		m_ItemList.SetItems(names);

		if (m_SearchBox)
			m_SearchBox.SetTextPreview(closestMatch);
	}

	void OnClick_ItemList(UIEvent eid, UIActionBase action)
	{
		if (eid == UIEvent.DOUBLE_CLICK)
			ActivateSelection();
	}

	//! Show On Map / Find CE Spawns, whichever the active tab means - shared
	//! by the list's double-click.
	protected void ActivateSelection()
	{
		string className = GetSelectedItem();
		if (className == "" || !m_Module)
			return;

		if (GetActiveTabIndex() == TAB_DISTRIBUTION)
			m_Module.ShowCESpawnLocations(className);
		else
			m_Module.ShowItemsOnMap(className);
	}

	// Returns the selected list row, or the search bar text if nothing is
	// selected - used for Show On Map so a typed classname works without
	// picking a row.
	string GetSelectedItem()
	{
		if (m_ItemList)
		{
			string sel = m_ItemList.GetSelectedItem();
			if (sel != "")
				return sel;
		}

		if (m_SearchBox)
		{
			string searchText = m_SearchBox.GetText();
			searchText.Trim();
			return searchText;
		}

		return "";
	}

	// Strict version - only returns a value when something is explicitly
	// selected in the list (used for Delete All to avoid accidental mass
	// deletions).
	string GetSelectedItemStrict()
	{
		if (!m_ItemList || m_ItemList.GetSelectedIndex() == -1)
			return "";

		return m_ItemList.GetSelectedItem();
	}

	// =========================================================================
	//  Right column - tabs
	// =========================================================================

	protected void InitWidgetsRight()
	{
		m_TabItemScan     = UIActionManager.CreatePanel(m_RightContent, 0x00000000);
		m_TabDistribution = UIActionManager.CreatePanel(m_RightContent, 0x00000000);

		ref array<string> tabLabels = { "#STR_COT_LOOT_TAB_ITEM_SCAN", "#STR_COT_LOOT_TAB_DISTRIBUTION" };
		ref array<string> tabIcons  = { JMConstants.Lucide("scan-search"), JMConstants.Lucide("map-pin") };

		m_Tabs = UIActionManager.CreateTabs(m_RightTabStrip, tabLabels, tabIcons, this, "OnChange_Tab");

		m_Tabs.AddContent(m_TabItemScan);
		m_Tabs.AddContent(m_TabDistribution);

		InitTabState(2);

		m_Tabs.SetSelection(TAB_ITEM_SCAN, false);

		BuildTabIfNeeded(TAB_ITEM_SCAN);
	}

	private void BuildTabIfNeeded(int tabIdx)
	{
		//! ShouldBuildTab is true exactly once per index, and marks it built.
		if (!ShouldBuildTab(tabIdx))
			return;

		switch (tabIdx)
		{
			case TAB_ITEM_SCAN:    InitWidgetsItemScan(m_TabItemScan);        break;
			case TAB_DISTRIBUTION: InitWidgetsDistribution(m_TabDistribution); break;
		}

		//! A tab built after the form's first resize has missed every layout
		//! pass so far - apply the cached height now instead of waiting on a
		//! window resize that may never come.
		if (m_LastRightContentH > 0)
		{
			LayoutItemScanTab(m_LastRightContentH);
			LayoutDistributionTab(m_LastRightContentH);
		}
	}

	override int GetActiveTabIndex()
	{
		if (!m_Tabs)
			return -1;

		return m_Tabs.GetSelection();
	}

	void OnChange_Tab(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CHANGE)
			return;

		CloseAllOverlays();
		HideHoverInfo();

		BuildTabIfNeeded(GetActiveTabIndex());
	}

	//! The map is a real UIActionCard, not a bare panel: its title bar is
	//! where "Show On Map" / "Clear Markers" / "Delete All" now live, next to
	//! the map's own name, instead of a separate button row above it that had
	//! its own background, its own wrap behaviour and its own way to clip a
	//! button that did not fit.
	protected void InitWidgetsItemScan(Widget parent)
	{
		m_ScanMapWrapper   = UIActionManager.CreatePanel(parent, 0x00000000);
		m_ScanStatsWrapper = UIActionManager.CreatePanel(parent, 0x00000000, STATS_H);

		UIActionCard mapCard = UIActionManager.CreateCard(m_ScanMapWrapper, "Item Scan Map");

		m_ShowOnMapButton = mapCard.AddCardHeaderAction(JMConstants.Lucide("crosshair"), this, "OnClick_ShowOnMap", "Place a marker on the map for every selected item");
		m_ClearMarkersButton = mapCard.AddCardHeaderAction(JMConstants.Lucide("eraser"), this, "OnClick_ClearMarkers", "Remove every map marker added by this module");

		//! A plain icon button, the same premade JMPlayerForm/JMTeleportForm use
		//! for every other destructive header action (AddDeleteButton -> the
		//! same CreateIconButton path as the two buttons beside it) - not the
		//! UIActionConfirmInline morph this used to be. The morph's own
		//! confirm/cancel swap was ALSO redundant: OnClick_DeleteAll already
		//! opens a full popup confirmation, so the inline one was a second,
		//! unnecessary "are you sure" in front of it, and its icon never
		//! centred as cleanly as a plain header action does.
		m_DeleteAllButton = mapCard.AddDeleteButton(this, "OnClick_DeleteAll", "Delete every world item matching the current filter");

		RegisterPermission(m_ShowOnMapButton, "Admin.LootAnalysis.ItemScan");
		RegisterPermission(m_ClearMarkersButton, "Admin.Map.View");
		RegisterPermission(m_DeleteAllButton, "Admin.LootAnalysis.Delete");

		//! A fixed height, not CreateMapFill - the card sizes ITSELF to its
		//! content (see UIActionCard's Size-To-Content-V), so the map is told
		//! its height explicitly and LayoutItemScanTab keeps it in step with
		//! the band on every resize, the same way JMTeleportForm's map card
		//! does.
		m_ItemScanMap = UIActionManager.CreateMap(mapCard.GetContent(), this, "OnClick_Map");
		RegisterPermission(m_ItemScanMap, "Admin.Map.View");

		//! A real card, like the map above it, with its own title and a Copy
		//! header action - and a key/value LIST inside, one row per stat,
		//! instead of one long line built with "+". The long line is what ran
		//! "Tiers:" off the edge of the panel with no wrap; a row each has
		//! nowhere to run off to, and the card just grows to fit however many
		//! there are (see MeasureScanStatsCard).
		m_ScanStatsCard = UIActionManager.CreateCard(m_ScanStatsWrapper, "Item Info");
		m_ScanStatsCard.AddCopyButton(this, "OnClick_CopyScanStats");
		m_ScanReloadButton = m_ScanStatsCard.AddRefreshButton(this, "OnClick_ReloadScanTypeInfo", "Reload CE data for this item");
		m_ScanSaveCEButton = m_ScanStatsCard.AddSaveButton(this, "OnClick_SaveScanCEData", "Save Nominal/Min/Lifetime/Restock to the source types.xml");
		RegisterPermission(m_ScanSaveCEButton, "Admin.LootAnalysis.Edit");

		m_ScanStatsList = UIActionManager.CreateKeyValueList(m_ScanStatsCard.GetContent());
		m_ScanCEEdit = new JMLootCEEditRow(m_ScanStatsCard.GetContent());
		RefreshScanStats();
	}

	protected void InitWidgetsDistribution(Widget parent)
	{
		m_DistMapWrapper    = UIActionManager.CreatePanel(parent, 0x00000000);
		m_DistStatsWrapper  = UIActionManager.CreatePanel(parent, 0x00000000, STATS_H);

		UIActionCard mapCard = UIActionManager.CreateCard(m_DistMapWrapper, "Distribution Map");

		m_FindSpawnsButton = mapCard.AddCardHeaderAction(JMConstants.Lucide("search"), this, "OnClick_FindCESpawns", "Show every CE area that can spawn the selected item");
		m_ClearDistMarkersButton = mapCard.AddCardHeaderAction(JMConstants.Lucide("eraser"), this, "OnClick_ClearDistMarkers", "Remove distribution markers from the map");

		RegisterPermission(m_FindSpawnsButton, "Admin.LootAnalysis.Distribution");
		RegisterPermission(m_ClearDistMarkersButton, "Admin.Map.View");

		m_DistributionMap = UIActionManager.CreateMap(mapCard.GetContent(), this, "OnClick_Map");
		RegisterPermission(m_DistributionMap, "Admin.Map.View");

		m_DistStatsCard = UIActionManager.CreateCard(m_DistStatsWrapper, "Item Info");
		m_DistStatsCard.AddCopyButton(this, "OnClick_CopyDistStats");
		m_DistReloadButton = m_DistStatsCard.AddRefreshButton(this, "OnClick_ReloadDistTypeInfo", "Reload CE data for this item");
		m_DistSaveCEButton = m_DistStatsCard.AddSaveButton(this, "OnClick_SaveDistCEData", "Save Nominal/Min/Lifetime/Restock to the source types.xml");
		RegisterPermission(m_DistSaveCEButton, "Admin.LootAnalysis.Edit");

		m_DistStatsList = UIActionManager.CreateKeyValueList(m_DistStatsCard.GetContent());
		m_DistCEEdit = new JMLootCEEditRow(m_DistStatsCard.GetContent());

		RefreshDistStats();

		RebuildDistributionLegend();
	}

	// =========================================================================
	//  Item Scan / Distribution actions
	// =========================================================================

	void OnClick_ShowOnMap(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		string className = GetSelectedItem();
		if (className == "")
			return;

		action.AnimateFeedback();

		if (m_Module)
			m_Module.ShowItemsOnMap(className);
	}

	void OnClick_ClearMarkers(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();

		if (m_ItemScanMap)
			m_ItemScanMap.ClearLayer(MAP_LAYER_SCAN);

		m_ScanItemDetails.Clear();
		HideHoverInfo();

		RefreshScanStats();
	}

	void OnClick_DeleteAll(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		string className = GetSelectedItemStrict();
		if (className == "")
			return;

		// Store now - the list selection may be cleared by the time the dialog is confirmed
		m_PendingDeleteClassName = className;

		CreateConfirmation_Two(JMConfirmationType.INFO, "Delete All Items", "Are you sure you want to delete ALL instances of: " + className + "?", "Cancel", "", "Confirm", "DeleteAll_Yes");
	}

	void DeleteAll_Yes(JMConfirmation confirmation)
	{
		if (!m_Module)
			return;

		if (m_PendingDeleteClassName != "")
		{
			m_Module.DeleteAllItems(m_PendingDeleteClassName);
			m_PendingDeleteClassName = "";
		}
	}

	void OnClick_ReloadScanTypeInfo(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();
		RequestTypeInfoReload();
	}

	void OnClick_ReloadDistTypeInfo(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();
		RequestTypeInfoReload();
	}

	//! Whichever classname Item Info is currently showing - the last lookup's
	//! own answer if there is one, falling back to the list/search selection
	//! so Reload still works before any lookup has happened yet.
	protected void RequestTypeInfoReload()
	{
		if (!m_Module)
			return;

		string className = "";
		if (m_LastTypeInfo)
			className = m_LastTypeInfo.m_ClassName;

		if (className == "")
			className = GetSelectedItem();

		if (className == "")
			return;

		m_Module.RequestItemTypeInfo(className);
	}

	void OnClick_SaveScanCEData(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		SaveCEData(m_ScanCEEdit);
	}

	void OnClick_SaveDistCEData(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		SaveCEData(m_DistCEEdit);
	}

	//! Reads the four fields now, before the confirmation dialog even opens -
	//! GetSelectedItem()/m_LastTypeInfo could both have moved on by the time
	//! the admin answers it, the same reason OnClick_DeleteAll captures
	//! m_PendingDeleteClassName up front instead of re-reading the list.
	protected void SaveCEData(JMLootCEEditRow editRow)
	{
		if (!editRow || !m_LastTypeInfo || !m_LastTypeInfo.m_Found || !m_Module)
			return;

		string className = m_LastTypeInfo.m_ClassName;
		if (className == "")
			return;

		editRow.ReadValues(m_LastTypeInfo, m_PendingSaveCENominal, m_PendingSaveCEMin, m_PendingSaveCELifetime, m_PendingSaveCERestock);
		m_PendingSaveCEClassName = className;

		CreateConfirmation_Two(JMConfirmationType.EDIT, "Save CE Data", "Overwrite " + className + "'s Nominal/Min/Lifetime/Restock in its source types.xml? A server restart or economy reload is needed for the change to affect spawns.", "Cancel", "", "Save", "SaveCEData_Yes");
	}

	void SaveCEData_Yes(JMConfirmation confirmation)
	{
		if (!m_Module || m_PendingSaveCEClassName == "")
			return;

		m_Module.SaveItemTypeInfo(m_PendingSaveCEClassName, m_PendingSaveCENominal, m_PendingSaveCEMin, m_PendingSaveCELifetime, m_PendingSaveCERestock);

		m_PendingSaveCEClassName = "";
	}

	void OnClick_FindCESpawns(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		string className = GetSelectedItem();
		if (className == "")
			return;

		action.AnimateFeedback();

		if (m_Module)
			m_Module.ShowCESpawnLocations(className);
	}

	void OnClick_ClearDistMarkers(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();

		if (m_DistributionMap)
			m_DistributionMap.ClearLayer(MAP_LAYER_DIST);

		m_DistTypesSeen.Clear();
		m_DistMarkerNames.Clear();
		m_DistMarkerIdsAll.Clear();
		m_DistMarkerPositionsAll.Clear();
		m_DistMarkerUsagesAll.Clear();
		m_HiddenUsages.Clear();

		RebuildDistributionLegend();
		RefreshDistStats();
	}

	// =========================================================================
	//  Module -> form marker / stats hand-off
	// =========================================================================

	//! Called by the module once the server answers a RPC_RequestItemScan.
	//! `details` is parallel to positions - one entry per entity, keyed into
	//! m_ScanItemDetails by the same marker id AddMarker gets, for the hover
	//! preview (see OnMarker_HoverEnter).
	void AddItemScanMarkers(array<string> names, array<vector> positions, array<ref JMLootScanItemDetail> details)
	{
		if (!m_ItemScanMap)
			return;

		m_ItemScanMap.ClearLayer(MAP_LAYER_SCAN);
		m_ScanItemDetails.Clear();

		for (int i = 0; i < positions.Count(); i++)
		{
			string label = "";
			if (i < names.Count())
				label = names[i];

			string markerId = "scan_" + i;

			m_ItemScanMap.AddMarker(markerId, positions[i], label, JMTheme.ACCENT, "", MAP_LAYER_SCAN);

			if (details && i < details.Count())
				m_ScanItemDetails.Set(markerId, details[i]);
		}

		if (positions.Count() > 0)
			m_ItemScanMap.CenterOnDeferred(positions[0]);

		RefreshScanStats();
	}

	//! Called by the module once the server answers a RPC_RequestLootDistribution.
	//! `types` is the usage tag that matched each position - what markers are
	//! coloured by and what the legend (in Item Info) groups on.
	//!
	//! A fresh result set replaces the last one outright, including which
	//! usages are hidden - a new search has nothing to do with what the last
	//! one's legend had toggled off.
	void AddDistributionMarkers(array<string> names, array<vector> positions, array<string> types)
	{
		if (!m_DistributionMap)
			return;

		m_DistTypesSeen.Clear();
		m_DistMarkerNames.Clear();
		m_DistMarkerIdsAll.Clear();
		m_DistMarkerPositionsAll.Clear();
		m_DistMarkerUsagesAll.Clear();
		m_HiddenUsages.Clear();

		for (int i = 0; i < positions.Count(); i++)
		{
			string label = "";
			if (i < names.Count())
				label = names[i];

			string usage = "";
			if (i < types.Count())
				usage = types[i];

			if (usage != "" && m_DistTypesSeen.Find(usage) < 0)
				m_DistTypesSeen.Insert(usage);

			string markerId = "dist_" + i;
			m_DistMarkerNames.Set(markerId, label);

			m_DistMarkerIdsAll.Insert(markerId);
			m_DistMarkerPositionsAll.Insert(positions[i]);
			m_DistMarkerUsagesAll.Insert(usage);
		}

		RepaintDistributionMarkers();

		if (positions.Count() > 0)
			m_DistributionMap.CenterOnDeferred(positions[0]);

		RebuildDistributionLegend();
	}

	//! Redraws every Distribution marker from m_DistMarker*All, skipping
	//! whichever usages are currently in m_HiddenUsages - the legend badges'
	//! only effect. Re-derives from the stored result set rather than hiding
	//! individual marker widgets, so toggling a usage back on needs nothing
	//! more than re-adding it from data already in hand.
	protected void RepaintDistributionMarkers()
	{
		if (!m_DistributionMap)
			return;

		m_DistributionMap.ClearLayer(MAP_LAYER_DIST);

		for (int i = 0; i < m_DistMarkerIdsAll.Count(); i++)
		{
			string usage = m_DistMarkerUsagesAll[i];

			if (usage != "" && m_HiddenUsages.Find(usage) >= 0)
				continue;

			//! Icon "" and label "" - AddMarker falls back to a small dot, and
			//! at a few hundred spawn points a full icon plus an always-on name
			//! label per marker is unreadable clutter. Colour alone (matched to
			//! the legend) carries the type; the name is kept in
			//! m_DistMarkerNames for the right-click menu's Copy Name.
			m_DistributionMap.AddMarker(m_DistMarkerIdsAll[i], m_DistMarkerPositionsAll[i], "", UsageColor(usage), "", MAP_LAYER_DIST);
		}

		RefreshDistStats();
	}

	//! Colour for a CE usage tag. Usages are data-driven (mod types.xml can
	//! define arbitrary ones), so this covers the common vanilla set and falls
	//! back to a neutral colour rather than failing on anything unknown.
	static int UsageColor(string usage)
	{
		string key = usage;
		key.ToLower();

		if (key == "military")    return JMTheme.DANGER;
		if (key == "police")      return JMTheme.INFO;
		if (key == "medic")       return JMTheme.SUCCESS;
		if (key == "farm")        return JMTheme.WARNING;
		if (key == "firefighter") return JMTheme.DANGER_BRIGHT;
		if (key == "industrial")  return JMTheme.ACCENT_TEAL;
		if (key == "village")     return JMTheme.ACCENT;
		if (key == "town")        return JMTheme.ACCENT;
		if (key == "office")      return JMTheme.INFO;
		if (key == "hunting")     return JMTheme.WARNING;
		if (key == "prison")      return JMTheme.DANGER;
		if (key == "coast")       return JMTheme.INFO;
		if (key == "school")      return JMTheme.ACCENT;

		return JMTheme.SUCCESS_FILL;
	}

	//! One clickable badge per distinct usage, tracked in m_LegendBadges
	//! parallel to m_DistTypesSeen - clicking one hides or shows every marker
	//! of that type (see OnClick_LegendBadge). Lives inside the Item Info
	//! card's own content grid, not a strip under the map.
	protected void RebuildDistributionLegend()
	{
		if (!m_DistStatsCard)
			return;

		m_LegendBadges.Clear();

		if (m_DistLegendRow)
		{
			m_DistLegendRow.Unlink();
			m_DistLegendRow = NULL;
		}

		if (m_DistTypesSeen.Count() == 0)
			return;

		m_DistLegendRow = UIActionManager.CreateWrapSpacerCompact(m_DistStatsCard.GetContent(), WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);

		foreach (string usage : m_DistTypesSeen)
		{
			UIActionBadge badge = UIActionManager.CreateBadge(m_DistLegendRow, "", usage, UsageColor(usage));
			badge.SetClickable(true);
			badge.SetCallback(this, "OnClick_LegendBadge");
			badge.SetTooltip("Click to show or hide " + usage + " markers");

			m_LegendBadges.Insert(badge);
		}

		RepaintLegendBadgeStates();
	}

	//! Show/hide a whole usage from the map by clicking its legend badge.
	void OnClick_LegendBadge(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		UIActionBadge badge;
		if (!Class.CastTo(badge, action))
			return;

		int index = m_LegendBadges.Find(badge);
		if (index < 0 || index >= m_DistTypesSeen.Count())
			return;

		string usage = m_DistTypesSeen[index];

		int hiddenIndex = m_HiddenUsages.Find(usage);
		if (hiddenIndex >= 0)
			m_HiddenUsages.Remove(hiddenIndex);
		else
			m_HiddenUsages.Insert(usage);

		RepaintDistributionMarkers();
		RepaintLegendBadgeStates();
	}

	//! Dim a hidden usage's badge so the legend itself shows what is and is
	//! not on the map, without a second row of on/off text to keep in sync.
	protected void RepaintLegendBadgeStates()
	{
		for (int i = 0; i < m_LegendBadges.Count() && i < m_DistTypesSeen.Count(); i++)
		{
			string usage = m_DistTypesSeen[i];
			int color = UsageColor(usage);

			if (m_HiddenUsages.Find(usage) >= 0)
				color = JMTheme.Fade(color, 0.35);

			m_LegendBadges[i].SetBadgeColor(color);
		}
	}

	//! Called by the module once the server answers either request with the
	//! classname's CE tuning values (or m_Found = false if it has none).
	void SetItemTypeInfo(JMLootItemTypeInfo info)
	{
		m_LastTypeInfo = info;

		RefreshScanStats();
		RefreshDistStats();
	}

	protected void RefreshScanStats()
	{
		if (!m_ScanStatsList)
			return;

		int count = 0;
		if (m_ItemScanMap)
			count = m_ItemScanMap.GetMarkerCount();

		m_ScanStatsList.SetValue("Markers", count.ToString());
		ApplyTypeInfoToList(m_ScanStatsList, m_LastTypeInfo);

		if (m_ScanCEEdit)
			m_ScanCEEdit.SetValues(m_LastTypeInfo);
	}

	protected void RefreshDistStats()
	{
		if (!m_DistStatsList)
			return;

		int count = 0;
		if (m_DistributionMap)
			count = m_DistributionMap.GetMarkerCount();

		m_DistStatsList.SetValue("Spawn Locations", count.ToString());
		ApplyTypeInfoToList(m_DistStatsList, m_LastTypeInfo);

		if (m_DistCEEdit)
			m_DistCEEdit.SetValues(m_LastTypeInfo);
	}

	//! Tiers only - Nominal/Min/Lifetime/Restock moved out to JMLootCEEditRow
	//! so they can be edited, not just read. This still runs into the same
	//! wrap problem a single "+"-built line had if it ever grew a second
	//! multi-value field, but Tiers alone is short enough to fit one row.
	protected void ApplyTypeInfoToList(UIActionKeyValueList list, JMLootItemTypeInfo info)
	{
		if (!list)
			return;

		if (!info || !info.m_Found)
		{
			list.SetValue("CE Data", "Not found");
			list.RemoveKey("Tiers");
			list.RemoveKey("Values");
			return;
		}

		list.RemoveKey("CE Data");

		list.SetValue("Tiers", JoinOrDash(info.m_Tiers));

		//! The type's own <usage name="Military"/> etc - what the user calls
		//! "values" (military/village/...). Independent of the clickable
		//! legend badges in Distribution, which only reflect a completed
		//! spawn-location search - this shows immediately, on either tab, as
		//! soon as CE data is found for the classname.
		list.SetValue("Values", JoinOrDash(info.m_Usages));
	}

	protected string JoinOrDash(array<string> values)
	{
		if (!values || values.Count() == 0)
			return "-";

		string joined = "";
		for (int i = 0; i < values.Count(); i++)
		{
			if (i > 0)
				joined += ", ";
			joined += values[i];
		}

		return joined;
	}

	void OnClick_CopyScanStats(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();

		if (m_ScanStatsList)
			g_Game.CopyToClipboard(StatsListToText(m_ScanStatsList));
	}

	void OnClick_CopyDistStats(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();

		if (m_DistStatsList)
			g_Game.CopyToClipboard(StatsListToText(m_DistStatsList));
	}

	protected string StatsListToText(UIActionKeyValueList list)
	{
		string text = "";

		foreach (string key : list.GetKeys())
		{
			if (text != "")
				text += "\n";
			text += key + ": " + list.GetValue(key);
		}

		return text;
	}

	// =========================================================================
	//  Map interaction - right-click marker menu (shared by both maps)
	// =========================================================================

	//! Ground click on either map. Left clicks are informational only; a
	//! right press on a marker opens the action menu. Also the callback for
	//! marker hover (MOUSE_ENTER/MOUSE_LEAVE) - shared with the click
	//! handling because both maps were already wired to one callback and a
	//! UIActionBase callback is one function per action, not one per event.
	void OnClick_Map(UIEvent eid, UIActionBase action)
	{
		if (eid == UIEvent.MOUSE_ENTER)
		{
			OnMarker_HoverEnter(action);
			return;
		}

		if (eid == UIEvent.MOUSE_LEAVE)
		{
			HideHoverInfo();
			return;
		}

		if (eid != UIEvent.CLICK)
			return;

		UIActionMap clickedMap;
		if (!Class.CastTo(clickedMap, action))
			return;

		if (clickedMap.GetLastClickButton() != MouseState.RIGHT)
			return;

		string markerId = clickedMap.GetLastClickedMarkerId();
		if (markerId == "")
			return;

		ShowMarkerMenu(clickedMap, markerId);
	}

	//! Item Scan markers only - Distribution markers are potential spawn
	//! points, not entities, and have nothing in m_ScanItemDetails to show.
	protected void OnMarker_HoverEnter(UIActionBase action)
	{
		UIActionMap hoveredMap;
		if (!Class.CastTo(hoveredMap, action) || hoveredMap != m_ItemScanMap)
			return;

		string markerId = hoveredMap.GetHoveredMarkerId();
		if (markerId == "" || !m_ScanItemDetails.Contains(markerId))
			return;

		if (!m_HoverInfo)
		{
			if (!m_Window)
				return;

			m_HoverInfo = new JMLootAnalysisHoverInfo(m_Window.GetWidgetRoot());
		}

		int mx, my;
		GetMousePos(mx, my);

		m_HoverInfo.ShowAt(markerId, m_ScanItemDetails.Get(markerId), mx, my);
	}

	protected void HideHoverInfo()
	{
		if (m_HoverInfo)
			m_HoverInfo.Hide();
	}

	override void OnHide()
	{
		super.OnHide();

		HideHoverInfo();
	}

	protected void ShowMarkerMenu(UIActionMap clickedMap, string markerId)
	{
		JMMapMarker marker = clickedMap.GetMarker(markerId);
		if (!marker)
			return;

		if (!m_MarkerMenu)
		{
			if (!m_Window)
				return;

			m_MarkerMenu = UIActionManager.CreateContextMenu(layoutRoot, m_Window.GetWidgetRoot(), this, "OnClick_MarkerMenu");

			if (!m_MarkerMenu)
				return;

			RegisterOverlay(m_MarkerMenu);
		}

		m_MarkerMenuMap   = clickedMap;
		m_MarkerMenuPos   = marker.Position;
		m_MarkerMenuLabel = marker.Label;

		//! Distribution markers carry no on-map label (see AddDistributionMarkers) -
		//! their name lives in the side table instead.
		if (m_MarkerMenuLabel == "" && clickedMap == m_DistributionMap && m_DistMarkerNames.Contains(markerId))
			m_MarkerMenuLabel = m_DistMarkerNames.Get(markerId);

		m_MarkerMenu.ClearItems();
		m_MarkerMenu.AddItem(MENU_MK_TELEPORT, "Teleport To", JMConstants.Lucide("footprints"));
		m_MarkerMenu.AddItem(MENU_MK_COPYPOS, "Copy Position", JMConstants.Lucide("copy"));
		m_MarkerMenu.AddItem(MENU_MK_FOCUS, "Center Map Here", JMConstants.Lucide("locate-fixed"));

		if (m_MarkerMenuLabel != "")
			m_MarkerMenu.AddItem(MENU_MK_COPYNAME, "Copy Name", JMConstants.Lucide("copy"));

		m_MarkerMenu.SetItemEnabled(MENU_MK_TELEPORT, GetPermissionsManager().HasPermission("Admin.Player.Teleport.Position"));

		int mx, my;
		GetMousePos(mx, my);

		m_MarkerMenu.ShowAt(mx, my);
	}

	void OnClick_MarkerMenu(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_MarkerMenu)
			return;

		string id = m_MarkerMenu.GetLastClickedId();

		if (id == MENU_MK_TELEPORT)
		{
			if (!GetPermissionsManager().HasPermission("Admin.Player.Teleport.Position"))
				return;

			JMTeleportModule teleportModule = CF_Modules<JMTeleportModule>.Get();
			JMPlayerInstance self = GetPermissionsManager().GetClientPlayer();

			if (teleportModule && self)
				teleportModule.Position(m_MarkerMenuPos, { self.GetGUID() });

			return;
		}

		if (id == MENU_MK_COPYPOS)
		{
			g_Game.CopyToClipboard("<" + m_MarkerMenuPos[0] + ", " + m_MarkerMenuPos[1] + ", " + m_MarkerMenuPos[2] + ">");
			return;
		}

		if (id == MENU_MK_FOCUS)
		{
			if (m_MarkerMenuMap)
				m_MarkerMenuMap.CenterOn(m_MarkerMenuPos);

			return;
		}

		if (id == MENU_MK_COPYNAME)
		{
			g_Game.CopyToClipboard(m_MarkerMenuLabel);
			return;
		}
	}
}
