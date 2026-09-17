//! "Loot Distribution" tab of JMLootAnalysisForm - CE spawn-location map, its
//! Item Info / CE-edit card, and the usage legend. Back-reference to the
//! owning form, same shape as JMPlayerRowWidget.Menu.
class JMLootAnalysisFormTabDistribution
{
	protected JMLootAnalysisForm m_Form;

	protected Widget m_DistMapWrapper;
	protected Widget m_DistStatsWrapper;

	//! Legend row, built directly under m_DistStatsCard.GetContent() - not a
	//! wrapper panel nested inside a card's content, which stacks a SECOND
	//! auto-sizing widget on top of the card's own one. UIActionCard's header
	//! comment calls exactly-one-Size-To-Content-V a hard rule.
	protected Widget m_DistLegendRow;
	protected UIActionMap m_DistributionMap;
	protected UIActionImageButton m_FindSpawnsButton;
	protected UIActionImageButton m_ClearDistMarkersButton;
	protected UIActionCard m_DistStatsCard;
	protected UIActionKeyValueList m_DistStatsList;
	protected float m_DistStatsH = JMLootAnalysisForm.STATS_H;

	//! Distinct matched-usage strings in the current Distribution result set,
	//! in first-seen order - what RebuildLegend() renders. Parallel to
	//! m_LegendBadges, the same way JMLootAnalysisForm's m_RecentIds is
	//! parallel to m_RecentButtons: one build pass, so the two can never
	//! drift apart.
	protected ref TStringArray m_DistTypesSeen = new TStringArray;
	protected ref array<ref UIActionBadge> m_LegendBadges = new array<ref UIActionBadge>;

	//! Usages the legend has toggled off - RepaintMarkers skips any marker
	//! whose usage is in here. Not "deleted", just hidden: toggling the
	//! badge again brings them straight back from m_DistMarker*All below, no
	//! re-request to the server needed.
	protected ref TStringArray m_HiddenUsages = new TStringArray;

	//! Marker id -> building display name. Distribution markers draw as bare
	//! coloured dots (a name label per marker was unreadable clutter at CE
	//! density), so the name is kept here instead for the right-click menu's
	//! Copy Name, rather than lost with it.
	protected ref map<string, string> m_DistMarkerNames = new map<string, string>;

	//! Every Distribution marker from the last result set, regardless of
	//! whether its usage is currently hidden - the source of truth
	//! RepaintMarkers repaints from. Parallel arrays, same shape as the
	//! RPC's own names/positions/types.
	protected ref TStringArray m_DistMarkerIdsAll = new TStringArray;
	protected ref array<vector> m_DistMarkerPositionsAll = new array<vector>;
	protected ref TStringArray m_DistMarkerUsagesAll = new TStringArray;

	protected ref JMLootCEEditRow m_DistCEEdit;
	protected UIActionImageButton m_DistSaveCEButton;
	protected UIActionImageButton m_DistReloadButton;

	void JMLootAnalysisFormTabDistribution( JMLootAnalysisForm form )
	{
		m_Form = form;
	}

	void Build(Widget parent)
	{
		m_DistMapWrapper    = UIActionManager.CreatePanel(parent, 0x00000000);
		m_DistStatsWrapper  = UIActionManager.CreatePanel(parent, 0x00000000, JMLootAnalysisForm.STATS_H);

		UIActionCard mapCard = UIActionManager.CreateCard(m_DistMapWrapper, "#STR_COT_LOOTANALYSIS_DISTRIBUTION_MAP_TITLE");

		m_FindSpawnsButton = mapCard.AddCardHeaderAction(JMConstants.Lucide("search"), this, "OnClick_FindCESpawns", "Show every CE area that can spawn the selected item");
		m_ClearDistMarkersButton = mapCard.AddCardHeaderAction(JMConstants.Lucide("eraser"), this, "OnClick_ClearDistMarkers", "Remove distribution markers from the map");

		m_Form.RegisterPermission(m_FindSpawnsButton, JMConstants.PERM_LOOTANALYSIS_DISTRIBUTION);
		m_Form.RegisterPermission(m_ClearDistMarkersButton, JMConstants.PERM_MAP_VIEW);

		m_DistributionMap = UIActionManager.CreateMap(mapCard.GetContent(), m_Form, "OnClick_Map");
		m_Form.RegisterPermission(m_DistributionMap, JMConstants.PERM_MAP_VIEW);

		m_DistStatsCard = UIActionManager.CreateCard(m_DistStatsWrapper, "Item Info");
		m_DistStatsCard.AddCopyButton(this, "OnClick_CopyDistStats");
		m_DistReloadButton = m_DistStatsCard.AddRefreshButton(this, "OnClick_ReloadTypeInfo", "Reload CE data for this item");
		m_DistSaveCEButton = m_DistStatsCard.AddSaveButton(this, "OnClick_SaveDistCEData", "Save Nominal/Min/Lifetime/Restock to the source types.xml");
		m_Form.RegisterPermission(m_DistSaveCEButton, JMConstants.PERM_LOOTANALYSIS_EDIT);

		m_DistStatsList = UIActionManager.CreateKeyValueList(m_DistStatsCard.GetContent());
		m_DistCEEdit = new JMLootCEEditRow(m_DistStatsCard.GetContent());

		RefreshStats();

		RebuildLegend();
	}

	UIActionMap GetMap()
	{
		return m_DistributionMap;
	}

	string GetMarkerName(string markerId)
	{
		if (!m_DistMarkerNames.Contains(markerId))
			return "";

		return m_DistMarkerNames.Get(markerId);
	}

	void Layout(float contentH)
	{
		if (!m_DistMapWrapper)
			return;

		float mapCardH = contentH - m_DistStatsH;
		if (mapCardH < 0)
			mapCardH = 0;

		m_Form.PinBand(m_DistMapWrapper, 0, mapCardH);
		m_Form.PinBand(m_DistStatsWrapper, mapCardH, m_DistStatsH);

		if (m_DistributionMap)
		{
			float mapH = mapCardH - JMLootAnalysisForm.MAP_CARD_CHROME;
			if (mapH < 0)
				mapH = 0;
			m_DistributionMap.SetHeight(mapH);
		}
	}

	//! See JMLootAnalysisFormTabItemScan.Tick() - same measure-and-relayout
	//! shape, against this tab's own stats card.
	void Tick()
	{
		if (m_DistributionMap)
			m_DistributionMap.TickMarkers();

		if (!m_DistStatsCard || m_Form.m_LastRightContentH <= 0)
			return;

		float cw, ch;
		m_DistStatsCard.GetLayoutRoot().GetScreenSize(cw, ch);

		if (ch < 1 || Math.AbsFloat(ch - m_DistStatsH) < 1)
			return;

		m_DistStatsH = ch;
		Layout(m_Form.m_LastRightContentH);
	}

	void OnClick_FindCESpawns(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		string className = m_Form.GetSelectedItem();
		if (className == "")
			return;

		action.AnimateFeedback();

		if (m_Form.m_Module)
			m_Form.m_Module.ShowCESpawnLocations(className);
	}

	void OnClick_ClearDistMarkers(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();

		if (m_DistributionMap)
			m_DistributionMap.ClearLayer(JMLootAnalysisForm.MAP_LAYER_DIST);

		m_DistTypesSeen.Clear();
		m_DistMarkerNames.Clear();
		m_DistMarkerIdsAll.Clear();
		m_DistMarkerPositionsAll.Clear();
		m_DistMarkerUsagesAll.Clear();
		m_HiddenUsages.Clear();

		RebuildLegend();
		RefreshStats();
	}

	void OnClick_ReloadTypeInfo(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();
		m_Form.RequestTypeInfoReload();
	}

	void OnClick_SaveDistCEData(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		m_Form.SaveCEData(m_DistCEEdit);
	}

	//! Called by the module (through the form) once the server answers a
	//! RPC_RequestLootDistribution. `types` is the usage tag that matched
	//! each position - what markers are coloured by and what the legend (in
	//! Item Info) groups on.
	//!
	//! A fresh result set replaces the last one outright, including which
	//! usages are hidden - a new search has nothing to do with what the last
	//! one's legend had toggled off.
	void AddMarkers(array<string> names, array<vector> positions, array<string> types)
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

		RepaintMarkers();

		if (positions.Count() > 0)
			m_DistributionMap.CenterOnDeferred(positions[0]);

		RebuildLegend();
	}

	//! Redraws every Distribution marker from m_DistMarker*All, skipping
	//! whichever usages are currently in m_HiddenUsages - the legend badges'
	//! only effect. Re-derives from the stored result set rather than hiding
	//! individual marker widgets, so toggling a usage back on needs nothing
	//! more than re-adding it from data already in hand.
	protected void RepaintMarkers()
	{
		if (!m_DistributionMap)
			return;

		m_DistributionMap.ClearLayer(JMLootAnalysisForm.MAP_LAYER_DIST);

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
			m_DistributionMap.AddMarker(m_DistMarkerIdsAll[i], m_DistMarkerPositionsAll[i], "", UsageColor(usage), "", JMLootAnalysisForm.MAP_LAYER_DIST);
		}

		RefreshStats();
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
	protected void RebuildLegend()
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

		RepaintMarkers();
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

	void RefreshStats()
	{
		if (!m_DistStatsList)
			return;

		int count = 0;
		if (m_DistributionMap)
			count = m_DistributionMap.GetMarkerCount();

		m_DistStatsList.SetValue("Spawn Locations", count.ToString());
		m_Form.ApplyTypeInfoToList(m_DistStatsList, m_Form.m_LastTypeInfo);

		if (m_DistCEEdit)
			m_DistCEEdit.SetValues(m_Form.m_LastTypeInfo);
	}

	void OnClick_CopyDistStats(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK)
			return;

		action.AnimateFeedback();

		if (m_DistStatsList)
			g_Game.CopyToClipboard(m_Form.StatsListToText(m_DistStatsList));
	}
}
