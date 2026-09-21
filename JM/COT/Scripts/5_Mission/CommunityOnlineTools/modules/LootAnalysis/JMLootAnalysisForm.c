// =============================================================================
//  JMLootAnalysisForm
//
//  Search / category filter / recent chips and the shared item picker on the
//  left, modelled directly on the object spawner's own left column. A tab
//  strip over Item Scan and Loot Distribution on the right - each tab's own
//  map/stats/CE-edit widgets live in JMLootAnalysisFormTabItemScan.c /
//  JMLootAnalysisFormTabDistribution.c. What stays here: the left column
//  (shared by both tabs), the CE-save confirmation chain (SaveCEData_Yes is
//  a JMConfirmation named callback - it dispatches against whatever object
//  its window was Init()'d with, not against whichever tab raised the
//  popup, so it and its DeleteAll_Yes sibling have to stay on the form), and
//  the right-click marker menu shared by both tabs' maps.
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
	//! Tab indices - what the strip's AddTab() returned for each tab, never written as numbers.
	protected int m_TabIdItemScan;
	protected int m_TabIdDistribution;

	//! Left column bands, top to bottom, in layout pixels. Pinned from
	//! OnResize rather than left to the layout file - see JMObjectSpawnerForm
	//! for why two sources for the same number is how a chip strip ends up
	//! overlapping the list it sits above.
	static const int SEARCH_H = 34;
	static const int RECENT_H = 36;

	//! Search box and filter button, as fractions of the search row - never a
	//! fixed pixel width beside a fractional one.
	static const float SEARCH_ROW_W = 0.85;
	static const float FILTER_ROW_W = 0.14;
	static const int LEFT_HEADER_H = 70;

	//! Right column bands within each tab: the map card filling whatever is
	//! left, and an "Item Info" stats card under it. Shared by both tab
	//! classes' own layout math.
	static const int STATS_H = 64;
	static const int MAP_CARD_CHROME = 44;

	//! protected, not private: sub-mods reach for the module through the form.
	//! Also public enough (no modifier) for the two tab classes to reach it
	//! through their back-reference.
	JMLootAnalysisModule m_Module;

	// ---- Left column --------------------------------------------------------
	protected Widget m_SearchWrapper;
	protected Widget m_RecentWrapper;
	protected Widget m_ListWrapper;
	protected UIActionSearchBox m_SearchBox;
	protected UIActionImageButton m_FilterButton;
	protected ref JMItemCategoryPicker m_Categories;
	protected string m_SearchFilter = "";
	protected string m_CurrentCategory = "";
	protected UIActionItemList m_ItemList;

	// ---- Right column --------------------------------------------------------
	protected UIActionTabs m_Tabs;
	protected Widget m_TabItemScan;
	protected Widget m_TabDistribution;

	//! One class per tab, in its own file - only this form constructs/
	//! dispatches to them.
	protected ref JMLootAnalysisFormTabItemScan    m_TabItemScanCtrl;
	protected ref JMLootAnalysisFormTabDistribution m_TabDistributionCtrl;

	//! Right-click marker menu, shared by both maps.
	protected ref JMLootMarkerMenu m_MarkerMenu;

	//! Last content height OnResize computed for the right column - a tab
	//! built lazily after the last resize has missed every pass so far and
	//! needs laying out immediately, not on the next window resize that may
	//! never come. Public: both tab classes' Measure() re-layout against it.
	float m_LastRightContentH;

	//! The size OnResize last received, for laying the right column out again when the tab strip wraps.
	protected float m_LastResizeW;
	protected float m_LastResizeH;

	//! CE tuning values for the last classname either tab asked about. Shared
	//! across both tabs' stats panels - nominal/tiers are a property of the
	//! item, not of which button was pressed. Public: both tab classes read it.
	ref JMLootItemTypeInfo m_LastTypeInfo;

	//! Values captured at Save click, re-applied from SaveCEData_Yes once the
	//! confirmation dialog answers - the dialog callback gets no arguments of
	//! its own to carry them through. Public: SaveCEData() is called by
	//! either tab class, through their own save button.
	string m_PendingSaveCEClassName;
	int    m_PendingSaveCENominal;
	int    m_PendingSaveCEMin;
	int    m_PendingSaveCELifetime;
	int    m_PendingSaveCERestock;

	//! Stored classname for Delete All confirmation (prevents stale selection).
	//! Public: JMLootAnalysisFormTabItemScan.OnClick_DeleteAll sets this.
	string m_PendingDeleteClassName;

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

	//! Called by the module once the server answers either request with the
	//! classname's CE tuning values (or m_Found = false if it has none).
	void SetItemTypeInfo(JMLootItemTypeInfo info)
	{
		m_LastTypeInfo = info;

		if (m_TabItemScanCtrl)
			m_TabItemScanCtrl.RefreshStats();

		if (m_TabDistributionCtrl)
			m_TabDistributionCtrl.RefreshStats();
	}

	protected override bool SetModule(JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}

	override void OnCreate()
	{
		m_LeftPanel         = layoutRoot.FindAnyWidget("panel_left");
		m_RightPanel        = layoutRoot.FindAnyWidget("panel_right");
		m_RightTabStrip     = layoutRoot.FindAnyWidget("panel_right_tabs");
		m_RightContent      = layoutRoot.FindAnyWidget("panel_right_content");
		m_RightPanelDisable = layoutRoot.FindAnyWidget("panel_right_disable");

		m_SearchWrapper = layoutRoot.FindAnyWidget("la_search_wrapper");
		m_RecentWrapper = layoutRoot.FindAnyWidget("la_recent_wrapper");
		m_ListWrapper   = layoutRoot.FindAnyWidget("la_list_wrapper");

		//! Same category filter (menu, groups, recent chips) as the Object Spawner.
		m_Categories = new JMItemCategoryPicker(this, m_RecentWrapper, JMFilterRegistry.ITEMS, this, "OnCategoryChanged");

		m_MarkerMenu = new JMLootMarkerMenu(this);

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

		m_LastResizeW = w;
		m_LastResizeH = h;
		m_LastRightContentH = h - GetPinnedStripHeight(m_RightTabStrip);

		ResizeTabs(w, m_LastRightContentH);
	}

	//! The tab strip wrapped or unwrapped: the right column has a row more or less than OnResize assumed.
	override void OnTabStripFitted()
	{
		super.OnTabStripFitted();

		if (m_LastResizeH <= 0)
			return;

		m_LastRightContentH = m_LastResizeH - GetPinnedStripHeight(m_RightTabStrip);

		ResizeTabs(m_LastResizeW, m_LastRightContentH);
	}

	//! Place one band: exact y, exact height, full width. Public: both tab
	//! classes' OnResize() call this through their back-reference.
	void PinBand(Widget band, float y, float height)
	{
		if (!band)
			return;

		band.SetFlags(WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true);
		band.SetPos(0, y);
		band.SetSize(1, height);
	}

	//! Markers are real widgets repositioned every tick (see UIActionMap), so
	//! only the map on the focused tab is ticked - the other has nothing to
	//! place itself against. Same for the stats card measurement below it.
	override void Update()
	{
		super.Update();

		UpdateActiveTab();
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
			Widget searchSpacer = UIActionManager.CreateWrapSpacerCompact(m_SearchWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);
			m_SearchBox = UIActionManager.CreateSearchBox(searchSpacer, this, "OnChange_Search", "Search:");
			m_SearchBox.SetWidth(SEARCH_ROW_W);

			m_FilterButton = UIActionManager.CreateIconButton(searchSpacer, JMConstants.Lucide("list-filter"), this, "OnClick_CategoryFilter");
			m_FilterButton.SetWidth(FILTER_ROW_W);
			m_FilterButton.SetTooltip("#STR_COT_LOOTANALYSIS_FILTER_BY_ITEM_CATEGORY");
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
	//  Categories - the filter button, its menu and the recent chips are a
	//  JMItemCategoryPicker, shared with the Object Spawner.
	// -------------------------------------------------------------------------

	void OnClick_CategoryFilter(UIEvent eid, UIActionBase action)
	{
		if (eid == UIEvent.CLICK && m_Categories && m_FilterButton)
			m_Categories.Toggle(m_FilterButton.GetLayoutRoot());
	}

	//! The picker's change callback: "" is everything, otherwise the config base class to filter on.
	void OnCategoryChanged(string categoryId)
	{
		m_CurrentCategory = categoryId;

		UpdateItemList();
	}

	void OnChange_Search(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CHANGE)
			return;

		m_SearchFilter = m_SearchBox.GetText();

		//! Debounced: this scans every CfgVehicles/CfgWeapons/CfgMagazines
		//! entry, so running it on every keystroke lags typing once the
		//! server has a large modded item set.
		DeferCall( "UpdateItemList", 300 );
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

		JMSearchMatcher matcher = new JMSearchMatcher(m_SearchFilter);

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
				if (!matcher.Matches(strNameLower))
					continue;

				names.Insert(strName);
			}
		}

		m_ItemList.SetItems(names);

		if (m_SearchBox)
			m_SearchBox.SetTextPreview(matcher.GetClosestMatch());
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

		if (GetActiveTabIndex() == m_TabIdDistribution)
			m_Module.ShowCESpawnLocations(className);
		else
			m_Module.ShowItemsOnMap(className);
	}

	// =========================================================================
	//  Right column - tabs
	// =========================================================================

	protected void InitWidgetsRight()
	{
		m_TabItemScan     = UIActionManager.CreatePanel(m_RightContent, 0x00000000);
		m_TabDistribution = UIActionManager.CreatePanel(m_RightContent, 0x00000000);


		m_Tabs = UIActionManager.CreateTabStrip( m_RightTabStrip, this, "OnChange_Tab" );

		m_TabIdItemScan = m_Tabs.AddTab( "#STR_COT_LOOT_TAB_ITEM_SCAN", JMConstants.Lucide("scan-search"), m_TabItemScan );
		m_TabIdDistribution = m_Tabs.AddTab( "#STR_COT_LOOT_TAB_DISTRIBUTION", JMConstants.Lucide("map-pin"), m_TabDistribution );

		DeclareTabs(2);

		m_Tabs.SetSelection(m_TabIdItemScan, false);

		InitTabFocus(m_TabIdItemScan);
	}

	override protected void OnTabCreate( int tab, Widget panel )
	{
		if (tab == m_TabIdItemScan)
		{
			m_TabItemScanCtrl = new JMLootAnalysisFormTabItemScan( this );
			RegisterTab( m_TabIdItemScan, m_TabItemScanCtrl );
		}
		else if (tab == m_TabIdDistribution)
		{
			m_TabDistributionCtrl = new JMLootAnalysisFormTabDistribution( this );
			RegisterTab( m_TabIdDistribution, m_TabDistributionCtrl );
		}

		super.OnTabCreate(tab, panel);

		//! A tab built after the form's first resize has missed every layout
		//! pass so far - apply the cached height now instead of waiting on a
		//! window resize that may never come.
		if (m_LastRightContentH > 0)
			OnTabResize(tab, 0, m_LastRightContentH);
	}

	protected override COT_ScriptedWidgetEventHandler GetTabStrip()
	{
		return m_Tabs;
	}

	void OnChange_Tab(UIEvent eid, UIActionBase action)
	{
		if (eid == UIEvent.CHANGE)
			HandleTabChange();
	}

	override void OnHide()
	{
		super.OnHide();

		if (m_TabItemScanCtrl)
			m_TabItemScanCtrl.HideHoverInfo();
	}

	// =========================================================================
	//  CE-save confirmation chain - stays here, see file header.
	// =========================================================================

	//! Reads the four fields now, before the confirmation dialog even opens -
	//! GetSelectedItem()/m_LastTypeInfo could both have moved on by the time
	//! the admin answers it, the same reason OnClick_DeleteAll captures
	//! m_PendingDeleteClassName up front instead of re-reading the list.
	void SaveCEData(JMLootCEEditRow editRow)
	{
		if (!editRow || !m_LastTypeInfo || !m_LastTypeInfo.m_Found || !m_Module)
			return;

		string className = m_LastTypeInfo.m_ClassName;
		if (className == "")
			return;

		editRow.ReadValues(m_LastTypeInfo, m_PendingSaveCENominal, m_PendingSaveCEMin, m_PendingSaveCELifetime, m_PendingSaveCERestock);
		m_PendingSaveCEClassName = className;

		PromptInput( "#STR_COT_LOOTANALYSIS_SAVE_CE_DATA", "Overwrite " + className + "'s Nominal/Min/Lifetime/Restock in its source types.xml? A server restart or economy reload is needed for the change to affect spawns.", "SaveCEData_Yes", "#STR_COT_LOOTANALYSIS_SAVE" );
	}

	void SaveCEData_Yes(JMConfirmation confirmation)
	{
		if (!m_Module || m_PendingSaveCEClassName == "")
			return;

		m_Module.SaveItemTypeInfo(m_PendingSaveCEClassName, m_PendingSaveCENominal, m_PendingSaveCEMin, m_PendingSaveCELifetime, m_PendingSaveCERestock);

		m_PendingSaveCEClassName = "";
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

	//! Whichever classname Item Info is currently showing - the last lookup's
	//! own answer if there is one, falling back to the list/search selection
	//! so Reload still works before any lookup has happened yet. Shared by
	//! both tabs' Reload button, which is why it lives here rather than on
	//! either tab class.
	void RequestTypeInfoReload()
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

	// =========================================================================
	//  Module -> form marker / stats hand-off - the module calls back on the
	//  form, so these stay here and forward into whichever tab owns the data.
	// =========================================================================

	void AddItemScanMarkers(array<string> names, array<vector> positions, array<ref JMLootScanItemDetail> details)
	{
		if (m_TabItemScanCtrl)
			m_TabItemScanCtrl.AddMarkers(names, positions, details);
	}

	void AddDistributionMarkers(array<string> names, array<vector> positions, array<string> types)
	{
		if (m_TabDistributionCtrl)
			m_TabDistributionCtrl.AddMarkers(names, positions, types);
	}

	//! Tiers only - Nominal/Min/Lifetime/Restock moved out to JMLootCEEditRow
	//! so they can be edited, not just read. Shared by both tab classes.
	void ApplyTypeInfoToList(UIActionKeyValueList list, JMLootItemTypeInfo info)
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

	string JoinOrDash(array<string> values)
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

	string StatsListToText(UIActionKeyValueList list)
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
			if (m_TabItemScanCtrl)
				m_TabItemScanCtrl.OnMarkerHoverEnter(action);
			return;
		}

		if (eid == UIEvent.MOUSE_LEAVE)
		{
			if (m_TabItemScanCtrl)
				m_TabItemScanCtrl.HideHoverInfo();
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

		//! Distribution markers carry no on-map label (see AddDistributionMarkers) -
		//! their name lives in the side table instead.
		string fallbackLabel = "";
		if (m_TabDistributionCtrl && clickedMap == m_TabDistributionCtrl.GetMap())
			fallbackLabel = m_TabDistributionCtrl.GetMarkerName(markerId);

		m_MarkerMenu.Show(clickedMap, markerId, fallbackLabel);
	}
}
