class JMLootAnalysisForm: JMFormBase
{
	//! protected, not private: sub-mods reach for the module through the form.
	protected JMLootAnalysisModule m_Module;

	// Mode selection - a tab strip now, not two hand-coloured buttons.
	protected UIActionTabs m_Tabs;
	protected bool m_IsItemScanMode = true;

	static const int TAB_ITEM_SCAN    = 0;
	static const int TAB_DISTRIBUTION = 1;

	// Search and filters
	protected UIActionSearchBox m_SearchBox;
	protected string m_SearchFilter = "";
	protected string m_CurrentCategory = "";

	// Item list
	protected TextListboxWidget m_ItemList;
	protected ref map<string, string> m_ItemCategories;

	// Item Scan mode widgets
	protected Widget m_ItemScanPanel;
	protected UIActionButton m_ShowOnMapButton;
	protected UIActionButton m_ClearMarkersButton;
	protected UIActionConfirmInline m_DeleteAllButton;

	// Loot Distribution mode widgets
	protected Widget m_LootDistributionPanel;
	protected UIActionButton m_FindSpawnsButton;
	protected UIActionButton m_ClearDistMarkersButton;
	protected UIActionText m_TierWarningText;

	// Stored classname for Delete All confirmation (prevents stale selection)
	protected string m_PendingDeleteClassName;

	void JMLootAnalysisForm()
	{
		m_ItemCategories = new map<string, string>;
	}

	protected override bool SetModule(JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}

	override void OnInit()
	{
		// Category buttons
		Widget categoryPanel = Widget.Cast(layoutRoot.FindAnyWidget("category_panel"));
		if (categoryPanel)
		{
			Widget categoryGrid = UIActionManager.CreateGridSpacer(categoryPanel, 2, 4);
			AddItemCategory(categoryGrid, "All", "");
			AddItemCategory(categoryGrid, "Food", "edible_base");
			AddItemCategory(categoryGrid, "Vehicles", "transport");
			AddItemCategory(categoryGrid, "Firearms", "weapon_base");
			AddItemCategory(categoryGrid, "Ammo/Mags", "magazine_base");
			AddItemCategory(categoryGrid, "Clothing", "clothing_base");
			AddItemCategory(categoryGrid, "Containers", "container_base");
			AddItemCategory(categoryGrid, "Items", "inventory_base");
		}

		// Search box - shared between both modes, lives in search_panel
		Widget searchPanel = Widget.Cast(layoutRoot.FindAnyWidget("search_panel"));
		if (searchPanel)
		{
			Widget searchSpacer = UIActionManager.CreateWrapSpacer(searchPanel, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER);
			m_SearchBox = UIActionManager.CreateSearchBox(searchSpacer, this, "OnChange_Search", "Search:");
			m_SearchBox.SetWidth(1.0);

			UIActionImageButton resetButton = UIActionManager.CreateImageButton(searchSpacer, "set:dayz_gui image:icon_x", this, "OnClick_ResetSearch");
			resetButton.SetFixedSize(ICON_BUTTON_PX, ICON_BUTTON_PX);
		}

		// Shared item list - visible in both modes
		m_ItemList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("loot_item_list"));

		// Item Scan mode actions
		m_ItemScanPanel = Widget.Cast(layoutRoot.FindAnyWidget("item_scan_panel"));
		if (m_ItemScanPanel)
		{
			Widget actionsPanel = Widget.Cast(layoutRoot.FindAnyWidget("item_actions_panel"));
			if (actionsPanel)
			{
				Widget scanGrid = UIActionManager.CreateGridSpacer(actionsPanel, 1, 3);
				m_ShowOnMapButton = UIActionManager.CreateButton(scanGrid, "Show On Map", this, "OnClick_ShowOnMap");
				m_ShowOnMapButton.SetTooltip( "Place a marker on the world map for every selected item" );
				m_ClearMarkersButton = UIActionManager.CreateButton(scanGrid, "Clear Markers", this, "OnClick_ClearMarkers");
				m_ClearMarkersButton.SetTooltip( "Remove every map marker added by this module" );
				m_DeleteAllButton = UIActionManager.CreateConfirmInline(scanGrid, "Delete All", this, "OnClick_DeleteAll");
				UIActionIconGrid.ApplyDeletePreset( m_DeleteAllButton );
				m_DeleteAllButton.SetTooltip( "Delete every world item matching the current filter" );
			}
		}

		// Loot Distribution mode actions + tier warning
		m_LootDistributionPanel = Widget.Cast(layoutRoot.FindAnyWidget("loot_distribution_panel"));
		if (m_LootDistributionPanel)
		{
			Widget distInfoPanel = Widget.Cast(layoutRoot.FindAnyWidget("distribution_info_panel"));
			if (distInfoPanel)
			{
				Widget infoSpacer = UIActionManager.CreateWrapSpacer(distInfoPanel, WidgetAlignment.WA_CENTER, WidgetAlignment.WA_CENTER);
				m_TierWarningText = UIActionManager.CreateText(infoSpacer, "#STR_COT_LOOT_TIER_WARNING");
			}

			Widget distButtonPanel = Widget.Cast(layoutRoot.FindAnyWidget("distribution_buttons"));
			if (distButtonPanel)
			{
				Widget distGrid = UIActionManager.CreateGridSpacer(distButtonPanel, 1, 2);
				m_FindSpawnsButton = UIActionManager.CreateButton(distGrid, "Find CE Spawn Locations", this, "OnClick_FindCESpawns");
				m_FindSpawnsButton.SetTooltip( "Show every CE area that can spawn the selected item" );
				m_ClearDistMarkersButton = UIActionManager.CreateButton(distGrid, "Clear Markers", this, "OnClick_ClearDistMarkers");
				m_ClearDistMarkersButton.SetTooltip( "Remove distribution markers from the map" );
			}
		}

		InitModeTabs();
		UpdateItemList();
	}

	void AddItemCategory(Widget parent, string name, string category)
	{
		UIActionButton btn = UIActionManager.CreateButton(parent, name, this, "OnClick_SetCategory");
		m_ItemCategories.Insert(name, category);
	}

	void OnClick_SetCategory(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		string categoryName = action.GetButton();
		if (m_ItemCategories.Contains(categoryName))
		{
			m_CurrentCategory = m_ItemCategories.Get(categoryName);
			UpdateItemList();
		}
	}

	void OnClick_ResetSearch(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		m_SearchBox.SetText("");
		m_SearchBox.SetTextPreview("");
		m_SearchFilter = "";
		UpdateItemList();
	}

	void OnChange_Search(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CHANGE)
			return;

		m_SearchFilter = m_SearchBox.GetText();

		// Both modes use the same list - always update it on search change
		UpdateItemList();
	}

	// -------------------------------------------------------------------------
	//  UpdateItemList - same logic as ObjectSpawner: iterate CfgVehicles /
	//  CfgWeapons / CfgMagazines, apply category + keyword search, set
	//  autocomplete preview on the search box.
	//  Note: JMEntityTracker is server-side only and always returns 0 on the
	//  client, so we scan config directly here.  The server scan (ShowItemsOnMap)
	//  filters to actually-spawned entities when the user requests map markers.
	// -------------------------------------------------------------------------
	void UpdateItemList()
	{
		if (!m_ItemList)
			return;

		m_ItemList.ClearItems();

		COT_String strSearch = m_SearchFilter;
		bool requireAllKeywords;
		TStringArray keywords = strSearch.KeywordSearch_Prepare(requireAllKeywords);
		string closestMatch;

		TStringArray configs = new TStringArray;
		configs.Insert(CFG_VEHICLESPATH);
		configs.Insert(CFG_WEAPONSPATH);
		configs.Insert(CFG_MAGAZINESPATH);

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

				m_ItemList.AddItem(strName, NULL, 0);
			}
		}

		if (m_SearchBox)
			m_SearchBox.SetTextPreview(closestMatch);
	}

	//! UIActionTabs owns showing and hiding the two panels, so the switch
	//! handlers only carry what is actually mode-specific.
	protected void InitModeTabs()
	{
		if (!m_ItemScanPanel || !m_LootDistributionPanel)
			return;

		ref array<string> tabLabels = { "#STR_COT_LOOT_TAB_ITEM_SCAN", "#STR_COT_LOOT_TAB_DISTRIBUTION" };
		ref array<string> tabIcons  = { JMConstants.Lucide( "scan-search" ), JMConstants.Lucide( "map-pin" ) };

		m_Tabs = UIActionManager.CreateTabs( layoutRoot.FindAnyWidget( "panel_bottom_tabs" ), tabLabels, tabIcons, this, "OnChange_Tab" );

		m_Tabs.AddContent( m_ItemScanPanel );
		m_Tabs.AddContent( m_LootDistributionPanel );

		m_Tabs.SetSelection( TAB_ITEM_SCAN, false );

		SwitchToItemScanMode();
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

		if (GetActiveTabIndex() == TAB_ITEM_SCAN)
			SwitchToItemScanMode();
		else
			SwitchToLootDistributionMode();
	}

	void SwitchToItemScanMode()
	{
		m_IsItemScanMode = true;

		if (m_SearchBox)
			m_SearchBox.SetTextPreview("Search by classname...");

		UpdateItemList();
	}

	void SwitchToLootDistributionMode()
	{
		m_IsItemScanMode = false;

		if (m_SearchBox)
			m_SearchBox.SetTextPreview("Search by classname, then click Find CE Spawn Locations...");
	}

	void OnClick_ShowOnMap(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		string className = GetSelectedItem();
		if (className == "")
			return;

		if (m_Module)
			m_Module.ShowItemsOnMap(className);
	}

	void OnClick_ClearMarkers(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		JMMapModule mapMod;
		if (Class.CastTo(mapMod, GetModuleManager().GetModule(JMMapModule)))
		{
			if (mapMod)
			{
				mapMod.ClearLootMarkers();
				COTCreateLocalAdminNotification(new StringLocaliser("Loot markers cleared from map"));
			}
		}
	}

	void OnClick_DeleteAll(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CHANGE)
			return;

		string className = GetSelectedItemStrict();
		if (className == "")
			return;

		// Store now - the list selection may be cleared by the time the dialog is confirmed
		m_PendingDeleteClassName = className;

		CreateConfirmation_Two(JMConfirmationType.INFO, "Delete All Items", "Are you sure you want to delete ALL instances of: " + className + "?", "Cancel", "DeleteAll_No", "Confirm", "DeleteAll_Yes");
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

	void OnClick_FindCESpawns(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		string className = GetSelectedItem();
		if (className == "")
			return;

		if (m_Module)
			m_Module.ShowCESpawnLocations(className);
	}

	void OnClick_ClearDistMarkers(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		JMMapModule mapMod;
		if (Class.CastTo(mapMod, GetModuleManager().GetModule(JMMapModule)))
		{
			mapMod.ClearLootMarkers();
			COTCreateLocalAdminNotification(new StringLocaliser("CE spawn markers cleared from map"));
		}
	}

	// Returns the selected list row text, or the search bar text if nothing is
	// selected - used for Show On Map so a typed classname works without picking.
	string GetSelectedItem()
	{
		if (m_ItemList && m_ItemList.GetSelectedRow() != -1)
		{
			string result;
			m_ItemList.GetItemText(m_ItemList.GetSelectedRow(), 0, result);
			if (result != "")
				return result;
		}

		if (m_SearchBox)
		{
			string searchText = m_SearchBox.GetText();
			searchText.Trim();
			return searchText;
		}

		return "";
	}

	// Strict version - only returns a value when something is explicitly selected
	// in the list (used for Delete All to avoid accidental mass deletions).
	string GetSelectedItemStrict()
	{
		if (!m_ItemList || m_ItemList.GetSelectedRow() == -1)
			return "";

		string result;
		m_ItemList.GetItemText(m_ItemList.GetSelectedRow(), 0, result);
		return result;
	}

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w == m_ItemList && button == MouseState.LEFT)
		{
			string className = GetSelectedItem();
			if (className != "" && m_Module)
			{
				if (m_IsItemScanMode)
					m_Module.ShowItemsOnMap(className);
				else
					m_Module.ShowCESpawnLocations(className);
			}
			return true;
		}

		return super.OnDoubleClick(w, x, y, button);
	}

	void UpdateDistributionData(array<ref JMLootDistributionData> distributionData)
	{
		// Legacy stub - distribution data is now shown on the map, not in a scroller
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();
		
		if (m_ShowOnMapButton)
			m_ShowOnMapButton.SetEnabled(GetPermissionsManager().HasPermission("Admin.LootAnalysis.ItemScan"));

		if (m_ClearMarkersButton)
			m_ClearMarkersButton.SetEnabled(GetPermissionsManager().HasPermission("Admin.Map.View"));

		if (m_DeleteAllButton)
			m_DeleteAllButton.SetEnabled(GetPermissionsManager().HasPermission("Admin.LootAnalysis.Delete"));

		if (m_FindSpawnsButton)
			m_FindSpawnsButton.SetEnabled(GetPermissionsManager().HasPermission("Admin.LootAnalysis.Distribution"));

		if (m_ClearDistMarkersButton)
			m_ClearDistMarkersButton.SetEnabled(GetPermissionsManager().HasPermission("Admin.Map.View"));
	}
}
