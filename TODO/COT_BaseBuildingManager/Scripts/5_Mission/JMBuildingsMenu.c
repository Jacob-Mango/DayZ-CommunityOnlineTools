class JMBuildingsMenu: JMFormBase
{
	private JMBuildingsModule m_Module;

	// Panneaux
	protected Widget m_MapWindowPanel;     // parent des marqueurs
	protected Widget m_ListPanel;          // liste de gauche (groupes)
	protected Widget m_GroupsContent;      // conteneur des entrees de groupe
	protected Widget m_MapPanel;           // panneau carte
	protected MapWidget m_MapWidget;

	protected ButtonWidget m_RefreshButton;
	protected TextWidget m_RefreshButtonLabel;
	protected EditBoxWidget m_GroupSizeInput;
	protected ButtonWidget m_RegroupButton;
	protected TextWidget m_RegroupButtonLabel;
	protected ButtonWidget m_DeleteAllButton;
	protected TextWidget m_DeleteAllButtonLabel;

	// Vue detail
	protected Widget m_DetailPanel;
	protected TextWidget m_DetailLabel;
	protected Widget m_DetailContent;
	protected ButtonWidget m_DeleteGroupButton;
	protected TextWidget m_DeleteGroupButtonLabel;
	protected ButtonWidget m_TpGroupButton;
	protected TextWidget m_TpGroupButtonLabel;
	protected ButtonWidget m_BackButton;
	protected TextWidget m_BackButtonLabel;

	protected ref array<ref JMBuildingsMapMarker> m_MapMarkers;
	protected ref array<ref JMBuildingsGroupEntry> m_GroupEntries;
	protected ref array<ref JMBuildingsDetailRow> m_DetailRows;

	protected ref array<ref JMBuildGroup> m_Groups;
	protected ref JMBuildGroup m_CurrentGroup;
	protected ref JMBuildMetaData m_PendingMeta;
	protected bool m_IsInDetail = false;

	void JMBuildingsMenu()
	{
		m_MapMarkers = new array<ref JMBuildingsMapMarker>;
		m_GroupEntries = new array<ref JMBuildingsGroupEntry>;
		m_DetailRows = new array<ref JMBuildingsDetailRow>;
		m_Groups = new array<ref JMBuildGroup>;
	}

	protected override bool SetModule(JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}

	override void OnInit()
	{
		m_MapWindowPanel = Widget.Cast(layoutRoot.FindAnyWidget("bbm_map_window_panel"));

		m_ListPanel = Widget.Cast(layoutRoot.FindAnyWidget("bbm_list_panel"));
		m_GroupsContent = Widget.Cast(layoutRoot.FindAnyWidget("bbm_groups_content"));

		m_MapPanel = Widget.Cast(layoutRoot.FindAnyWidget("bbm_map_panel"));
		m_MapWidget = MapWidget.Cast(layoutRoot.FindAnyWidget("bbm_map"));

		m_RefreshButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("bbm_refresh_button"));
		m_RefreshButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("bbm_refresh_button_label"));
		m_GroupSizeInput = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("bbm_groupsize_input"));
		m_RegroupButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("bbm_regroup_button"));
		m_RegroupButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("bbm_regroup_button_label"));
		m_DeleteAllButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("bbm_delete_all_button"));
		m_DeleteAllButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("bbm_delete_all_button_label"));

		m_DetailPanel = Widget.Cast(layoutRoot.FindAnyWidget("bbm_detail_panel"));
		m_DetailLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("bbm_detail_label"));
		m_DetailContent = Widget.Cast(layoutRoot.FindAnyWidget("bbm_detail_content"));
		m_DeleteGroupButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("bbm_delete_group_button"));
		m_DeleteGroupButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("bbm_delete_group_button_label"));
		m_TpGroupButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("bbm_tp_group_button"));
		m_TpGroupButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("bbm_tp_group_button_label"));
		m_BackButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("bbm_back_button"));
		m_BackButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("bbm_back_button_label"));

		if (m_DetailPanel)
			m_DetailPanel.Show(false);
	}

	float GetGroupRadius()
	{
		float r = 150.0;
		if (m_GroupSizeInput)
			r = m_GroupSizeInput.GetText().ToFloat();
		if (r <= 0)
			r = 150.0;
		return r;
	}

	// ------------------------------------------------------------------------
	//  Construction des groupes + remplissage liste / carte
	// ------------------------------------------------------------------------
	void LoadGroups()
	{
		if (!m_Module)
			return;

		// Re-clustering a partir des dernieres donnees serveur.
		m_Groups = JMBuildGroup.BuildGroups(m_Module.GetServerBuildings(), GetGroupRadius());

		// Si on consulte une zone, on laisse la vue detail intacte (elle se gere
		// localement). Les groupes en arriere-plan restent neanmoins a jour.
		if (m_IsInDetail)
			return;

		m_MapMarkers.Clear();
		m_GroupEntries.Clear();

		// Jaune : bien plus lisible que le bleu sur la carte.
		int markerColor = ARGB(255, 255, 210, 0);

		for (int i = 0; i < m_Groups.Count(); i++)
		{
			JMBuildGroup grp = m_Groups.Get(i);

			JMBuildingsMapMarker marker = new JMBuildingsMapMarker(m_MapWindowPanel, m_MapWidget, grp, i, markerColor, this);
			m_MapMarkers.Insert(marker);

			JMBuildingsGroupEntry entry = new JMBuildingsGroupEntry(m_GroupsContent, this, grp, i);
			m_GroupEntries.Insert(entry);
		}
	}

	// ------------------------------------------------------------------------
	//  Ouverture d'une zone -> vue detail (tableau)
	// ------------------------------------------------------------------------
	void OpenGroup(JMBuildGroup group)
	{
		if (!group)
			return;

		m_IsInDetail = true;
		m_CurrentGroup = group;

		HideMapMarkers();
		if (m_MapPanel)
			m_MapPanel.Show(false);

		if (m_DetailPanel)
			m_DetailPanel.Show(true);

		PopulateDetail();
	}

	void PopulateDetail()
	{
		m_DetailRows.Clear();

		if (!m_CurrentGroup)
			return;

		int cx = Math.Round(m_CurrentGroup.m_Center[0]);
		int cz = Math.Round(m_CurrentGroup.m_Center[2]);
		if (m_DetailLabel)
			m_DetailLabel.SetText("ZONE  -  " + m_CurrentGroup.Count().ToString() + " elements   (X " + cx.ToString() + "  Z " + cz.ToString() + ")");

		foreach (JMBuildMetaData meta: m_CurrentGroup.m_Items)
		{
			JMBuildingsDetailRow row = new JMBuildingsDetailRow(m_DetailContent, this, meta);
			m_DetailRows.Insert(row);
		}
	}

	void BackToList()
	{
		m_IsInDetail = false;
		m_CurrentGroup = NULL;
		m_PendingMeta = NULL;

		m_DetailRows.Clear();

		if (m_DetailPanel)
			m_DetailPanel.Show(false);
		if (m_MapPanel)
			m_MapPanel.Show(true);

		// Reconstruit liste + marqueurs a partir des donnees serveur les plus recentes.
		LoadGroups();
		ShowMapMarkers();
	}

	// ------------------------------------------------------------------------
	//  Suppressions
	// ------------------------------------------------------------------------
	void RequestDeleteSingle(JMBuildMetaData meta)
	{
		if (!meta)
			return;

		m_PendingMeta = meta;
		CreateConfirmation_Two(JMConfirmationType.INFO, "#STR_BBM_DELETE_ONE", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "DeleteSingleConfirmed");
	}

	void DeleteSingleConfirmed(JMConfirmation confirmation)
	{
		if (!m_PendingMeta || !m_CurrentGroup)
			return;

		m_Module.DeleteBuilding(m_PendingMeta);

		int idx = m_CurrentGroup.m_Items.Find(m_PendingMeta);
		if (idx > -1)
			m_CurrentGroup.m_Items.Remove(idx);

		m_PendingMeta = NULL;

		if (m_CurrentGroup.Count() == 0)
			BackToList();
		else
			PopulateDetail();
	}

	void RequestDeleteGroup()
	{
		if (!m_CurrentGroup || m_CurrentGroup.Count() == 0)
			return;

		CreateConfirmation_Two(JMConfirmationType.INFO, "#STR_BBM_DELETE_GROUP", "#STR_BBM_DELETE_GROUP_CONFIRM", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "DeleteGroupConfirmed");
	}

	void DeleteGroupConfirmed(JMConfirmation confirmation)
	{
		if (!m_CurrentGroup)
			return;

		m_Module.DeleteBuildingGroup(m_CurrentGroup.m_Items);
		BackToList();
	}

	void DeleteBuildingAll(JMConfirmation confirmation)
	{
		m_Module.DeleteBuildingAll();
	}

	// ------------------------------------------------------------------------
	//  Sync / refresh
	// ------------------------------------------------------------------------
	void SyncAndRefresh()
	{
		m_Module.RequestServerBuildings();
	}

	void ShowMapMarkers()
	{
		for (int i = 0; i < m_MapMarkers.Count(); i++)
			m_MapMarkers.Get(i).ShowMarker();
	}

	void HideMapMarkers()
	{
		for (int i = 0; i < m_MapMarkers.Count(); i++)
			m_MapMarkers.Get(i).HideMarker();
	}

	override void OnShow()
	{
		super.OnShow();

		SyncAndRefresh();

		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateMapPosition, 34, false, true, vector.Zero);
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdatePlayerMarkers, 1000, true);
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(AutoRefresh, 10000, true);
	}

	override void OnHide()
	{
		super.OnHide();

		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdatePlayerMarkers);
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(AutoRefresh);
	}

	void AutoRefresh()
	{
		if (m_IsInDetail)
			return;
		SyncAndRefresh();
	}

	// Position des joueurs sur la carte (marqueurs natifs), pour le contexte.
	void UpdatePlayerMarkers()
	{
		if (m_IsInDetail)
			return;
		if (!m_MapWidget)
			return;

		m_MapWidget.ClearUserMarks();

		array<JMPlayerInstance> players = GetPermissionsManager().GetPlayers();
		foreach (JMPlayerInstance player: players)
		{
			m_MapWidget.AddUserMark(player.GetPosition(), player.GetName(), ARGB(255, 52, 152, 219), JM_COT_ICON_DOT + ".paa");
		}

		GetCommunityOnlineTools().RefreshClientPositions();
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_RefreshButton)
		{
			SyncAndRefresh();
		}

		if (w == m_RegroupButton)
		{
			if (!m_IsInDetail)
				LoadGroups();
		}

		if (w == m_DeleteAllButton)
		{
			CreateConfirmation_Two(JMConfirmationType.INFO, "#STR_BBM_DELETE_ALL", "#STR_BBM_DELETE_ALL_CONFIRM", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "DeleteBuildingAll");
		}

		if (w == m_DeleteGroupButton)
		{
			RequestDeleteGroup();
		}

		if (w == m_TpGroupButton)
		{
			TeleportToGroup();
		}

		if (w == m_BackButton)
		{
			BackToList();
		}

		return false;
	}

	void TeleportToGroup()
	{
		if (!m_CurrentGroup)
			return;

		string guid = GetPermissionsManager().GetClientPlayer().GetGUID();
		CF_Modules<JMTeleportModule>.Get().Position(SnapToGround(m_CurrentGroup.m_Center), {guid});
	}

	// Teleporte l'admin sur un element precis du tableau de detail.
	void TeleportToMeta(JMBuildMetaData meta)
	{
		if (!meta)
			return;

		string guid = GetPermissionsManager().GetClientPlayer().GetGUID();
		CF_Modules<JMTeleportModule>.Get().Position(SnapToGround(meta.m_Position), {guid});
	}

	// Double-clic sur la carte -> teleportation de l'admin a l'emplacement clique.
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w == NULL)
			return false;

		if (w == m_MapWidget)
		{
			vector tpPos = SnapToGround(m_MapWidget.ScreenToMap(Vector(x, y, 0)));
			string guid = GetPermissionsManager().GetClientPlayer().GetGUID();
			CF_Modules<JMTeleportModule>.Get().Position(tpPos, {guid});
			return true;
		}

		return false;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (w == m_RefreshButton)
			m_RefreshButtonLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_RegroupButton)
			m_RegroupButtonLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_DeleteAllButton)
			m_DeleteAllButtonLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_DeleteGroupButton)
			m_DeleteGroupButtonLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_TpGroupButton)
			m_TpGroupButtonLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_BackButton)
			m_BackButtonLabel.SetColor(ARGB(255, 0, 0, 0));

		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (w == m_RefreshButton)
			m_RefreshButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_RegroupButton)
			m_RegroupButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_DeleteAllButton)
			m_DeleteAllButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_DeleteGroupButton)
			m_DeleteGroupButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_TpGroupButton)
			m_TpGroupButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_BackButton)
			m_BackButtonLabel.SetColor(ARGB(255, 255, 255, 255));

		return false;
	}

	void UpdateMapPosition(bool usePlayerPosition, vector mapPosition = vector.Zero)
	{
		if (usePlayerPosition)
		{
			PlayerBase player;
			float scale;
			if (Class.CastTo(player, g_Game.GetPlayer()) && !player.GetLastMapInfo(scale, mapPosition))
			{
				scale = 0.33;
				mapPosition = player.GetWorldPosition();
			}

			m_MapWidget.SetScale(scale);
		}

		m_MapWidget.SetMapPos(mapPosition);
	}
}
