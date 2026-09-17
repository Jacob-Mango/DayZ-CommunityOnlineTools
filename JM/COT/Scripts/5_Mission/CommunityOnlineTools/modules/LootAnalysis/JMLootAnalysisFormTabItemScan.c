//! "Item Scan" tab of JMLootAnalysisForm - the map of world item instances,
//! its Item Info / CE-edit card, and the hover preview. Back-reference to the
//! owning form, same shape as JMPlayerRowWidget.Menu.
class JMLootAnalysisFormTabItemScan
{
	protected JMLootAnalysisForm m_Form;

	protected Widget m_ScanMapWrapper;
	protected Widget m_ScanStatsWrapper;
	protected UIActionMap m_ItemScanMap;
	protected UIActionImageButton m_ShowOnMapButton;
	protected UIActionImageButton m_ClearMarkersButton;
	protected UIActionImageButton m_DeleteAllButton;
	protected UIActionCard m_ScanStatsCard;
	protected UIActionKeyValueList m_ScanStatsList;

	//! Real height of m_ScanStatsCard, corrected from STATS_H (a seed, not a
	//! measurement) once the card has something to measure - see Measure().
	//! A card sizes itself to its rows, and the row count here varies with
	//! whether CE data was found, so a fixed band either cut the Tiers row
	//! off or left dead space under a short one.
	protected float m_ScanStatsH = JMLootAnalysisForm.STATS_H;

	protected ref JMLootCEEditRow m_ScanCEEdit;
	protected UIActionImageButton m_ScanSaveCEButton;
	protected UIActionImageButton m_ScanReloadButton;

	//! Item Scan marker id -> per-entity health/quantity/lifetime/attachments,
	//! for the hover preview - see OnMarkerHoverEnter.
	protected ref map<string, ref JMLootScanItemDetail> m_ScanItemDetails = new map<string, ref JMLootScanItemDetail>;

	//! Floating hover-preview panel over the Item Scan map, built once and
	//! reused - mirrors JMVehiclesForm's own m_HoverInfo.
	protected ref JMLootAnalysisHoverInfo m_HoverInfo;

	void JMLootAnalysisFormTabItemScan( JMLootAnalysisForm form )
	{
		m_Form = form;
	}

	//! The map is a real UIActionCard, not a bare panel: its title bar is
	//! where "Show On Map" / "Clear Markers" / "Delete All" now live, next to
	//! the map's own name, instead of a separate button row above it.
	void Build(Widget parent)
	{
		m_ScanMapWrapper   = UIActionManager.CreatePanel(parent, 0x00000000);
		m_ScanStatsWrapper = UIActionManager.CreatePanel(parent, 0x00000000, JMLootAnalysisForm.STATS_H);

		UIActionCard mapCard = UIActionManager.CreateCard(m_ScanMapWrapper, "#STR_COT_LOOTANALYSIS_ITEM_SCAN_MAP_TITLE");

		m_ShowOnMapButton = mapCard.AddCardHeaderAction(JMConstants.Lucide("crosshair"), this, "OnClick_ShowOnMap", "Place a marker on the map for every selected item");
		m_ClearMarkersButton = mapCard.AddCardHeaderAction(JMConstants.Lucide("eraser"), this, "OnClick_ClearMarkers", "Remove every map marker added by this module");

		//! A plain icon button, the same premade JMPlayerForm/JMTeleportForm use
		//! for every other destructive header action.
		m_DeleteAllButton = mapCard.AddDeleteButton(this, "OnClick_DeleteAll", "Delete every world item matching the current filter");

		m_Form.RegisterPermission(m_ShowOnMapButton, JMConstants.PERM_LOOTANALYSIS_ITEMSCAN);
		m_Form.RegisterPermission(m_ClearMarkersButton, JMConstants.PERM_MAP_VIEW);
		m_Form.RegisterPermission(m_DeleteAllButton, JMConstants.PERM_LOOTANALYSIS_DELETE);

		//! A fixed height, not CreateMapFill - the card sizes ITSELF to its
		//! content, so the map is told its height explicitly and Layout()
		//! keeps it in step with the band on every resize.
		m_ItemScanMap = UIActionManager.CreateMap(mapCard.GetContent(), m_Form, "OnClick_Map");
		m_Form.RegisterPermission(m_ItemScanMap, JMConstants.PERM_MAP_VIEW);

		//! A real card, like the map above it, with its own title and a Copy
		//! header action - and a key/value LIST inside, one row per stat.
		m_ScanStatsCard = UIActionManager.CreateCard(m_ScanStatsWrapper, "#STR_COT_LOOTANALYSIS_ITEM_INFO_TITLE");
		m_ScanStatsCard.AddCopyButton(this, "OnClick_CopyScanStats");
		m_ScanReloadButton = m_ScanStatsCard.AddRefreshButton(this, "OnClick_ReloadTypeInfo", "Reload CE data for this item");
		m_ScanSaveCEButton = m_ScanStatsCard.AddSaveButton(this, "OnClick_SaveScanCEData", "Save Nominal/Min/Lifetime/Restock to the source types.xml");
		m_Form.RegisterPermission(m_ScanSaveCEButton, JMConstants.PERM_LOOTANALYSIS_EDIT);

		m_ScanStatsList = UIActionManager.CreateKeyValueList(m_ScanStatsCard.GetContent());
		m_ScanCEEdit = new JMLootCEEditRow(m_ScanStatsCard.GetContent());
		RefreshStats();
	}

	UIActionMap GetMap()
	{
		return m_ItemScanMap;
	}

	void Layout(float contentH)
	{
		if (!m_ScanMapWrapper)
			return;

		float mapCardH = contentH - m_ScanStatsH;
		if (mapCardH < 0)
			mapCardH = 0;

		m_Form.PinBand(m_ScanMapWrapper, 0, mapCardH);
		m_Form.PinBand(m_ScanStatsWrapper, mapCardH, m_ScanStatsH);

		if (m_ItemScanMap)
		{
			float mapH = mapCardH - JMLootAnalysisForm.MAP_CARD_CHROME;
			if (mapH < 0)
				mapH = 0;
			m_ItemScanMap.SetHeight(mapH);
		}
	}

	//! m_ScanStatsH starts at the STATS_H seed and is corrected here once the
	//! card has laid out its rows for real - it cannot know that on the frame
	//! it is built, and the row count itself only settles once a scan result
	//! actually arrives. Re-checked every Tick() the tab is active; the
	//! comparison is a cheap float read, and it only ever relayouts when the
	//! measured height actually moved.
	void Tick()
	{
		if (m_ItemScanMap)
			m_ItemScanMap.TickMarkers();

		if (!m_ScanStatsCard || m_Form.m_LastRightContentH <= 0)
			return;

		float cw, ch;
		m_ScanStatsCard.GetLayoutRoot().GetScreenSize(cw, ch);

		if (ch < 1 || Math.AbsFloat(ch - m_ScanStatsH) < 1)
			return;

		m_ScanStatsH = ch;
		Layout(m_Form.m_LastRightContentH);
	}

	void OnClick_ShowOnMap(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		string className = m_Form.GetSelectedItem();
		if (className == "")
			return;

		action.AnimateFeedback();

		if (m_Form.m_Module)
			m_Form.m_Module.ShowItemsOnMap(className);
	}

	void OnClick_ClearMarkers(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();

		if (m_ItemScanMap)
			m_ItemScanMap.ClearLayer(JMLootAnalysisForm.MAP_LAYER_SCAN);

		m_ScanItemDetails.Clear();
		HideHoverInfo();

		RefreshStats();
	}

	void OnClick_DeleteAll(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		string className = m_Form.GetSelectedItemStrict();
		if (className == "")
			return;

		// Store now - the list selection may be cleared by the time the dialog is confirmed
		m_Form.m_PendingDeleteClassName = className;

		m_Form.CreateConfirmation_Two(JMConfirmationType.INFO, "Delete All Items", "Are you sure you want to delete ALL instances of: " + className + "?", "Cancel", "", "Confirm", "DeleteAll_Yes");
	}

	void OnClick_ReloadTypeInfo(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();
		m_Form.RequestTypeInfoReload();
	}

	void OnClick_SaveScanCEData(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		m_Form.SaveCEData(m_ScanCEEdit);
	}

	//! Called by the module (through the form) once the server answers a
	//! RPC_RequestItemScan. `details` is parallel to positions - one entry
	//! per entity, keyed into m_ScanItemDetails by the same marker id
	//! AddMarker gets, for the hover preview (see OnMarkerHoverEnter).
	void AddMarkers(array<string> names, array<vector> positions, array<ref JMLootScanItemDetail> details)
	{
		if (!m_ItemScanMap)
			return;

		m_ItemScanMap.ClearLayer(JMLootAnalysisForm.MAP_LAYER_SCAN);
		m_ScanItemDetails.Clear();

		for (int i = 0; i < positions.Count(); i++)
		{
			string label = "";
			if (i < names.Count())
				label = names[i];

			string markerId = "scan_" + i;

			m_ItemScanMap.AddMarker(markerId, positions[i], label, JMTheme.ACCENT, "", JMLootAnalysisForm.MAP_LAYER_SCAN);

			if (details && i < details.Count())
				m_ScanItemDetails.Set(markerId, details[i]);
		}

		if (positions.Count() > 0)
			m_ItemScanMap.CenterOnDeferred(positions[0]);

		RefreshStats();
	}

	void RefreshStats()
	{
		if (!m_ScanStatsList)
			return;

		int count = 0;
		if (m_ItemScanMap)
			count = m_ItemScanMap.GetMarkerCount();

		m_ScanStatsList.SetValue("Markers", count.ToString());
		m_Form.ApplyTypeInfoToList(m_ScanStatsList, m_Form.m_LastTypeInfo);

		if (m_ScanCEEdit)
			m_ScanCEEdit.SetValues(m_Form.m_LastTypeInfo);
	}

	void OnClick_CopyScanStats(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();

		if (m_ScanStatsList)
			g_Game.CopyToClipboard(m_Form.StatsListToText(m_ScanStatsList));
	}

	//! Item Scan markers only - Distribution markers are potential spawn
	//! points, not entities, and have nothing in m_ScanItemDetails to show.
	void OnMarkerHoverEnter(UIActionBase action)
	{
		UIActionMap hoveredMap;
		if (!Class.CastTo(hoveredMap, action) || hoveredMap != m_ItemScanMap)
			return;

		string markerId = hoveredMap.GetHoveredMarkerId();
		if (markerId == "" || !m_ScanItemDetails.Contains(markerId))
			return;

		if (!m_HoverInfo)
		{
			CF_Window window = m_Form.GetWindow();
			if (!window)
				return;

			m_HoverInfo = new JMLootAnalysisHoverInfo(window.GetWidgetRoot());
		}

		int mx, my;
		GetMousePos(mx, my);

		m_HoverInfo.ShowAt(markerId, m_ScanItemDetails.Get(markerId), mx, my);
	}

	void HideHoverInfo()
	{
		if (m_HoverInfo)
			m_HoverInfo.Hide();
	}
}
