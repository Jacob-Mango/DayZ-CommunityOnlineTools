class JMAnimalsMenu: JMFormBase
{
	private JMAnimalsModule m_Module;

	protected Widget m_AnimalMapPanel;
	protected Widget m_AnimalListPanel;
	protected Widget m_AnimalsListContent;
	protected ButtonWidget m_RefreshButton;
	protected TextWidget m_RefreshButtonLabel;

	protected Widget m_MapWidgetPanel;
	protected MapWidget m_MapWidget;

	protected ButtonWidget m_DeleteAllButton;
	protected TextWidget m_DeleteAllButtonLabel;
	protected EditBoxWidget m_RadiusInput;
	protected ButtonWidget m_DeleteRadiusButton;
	protected TextWidget m_DeleteRadiusButtonLabel;

	protected Widget m_AnimalInfoPanel;
	protected TextWidget m_InfoNetworkID;
	protected TextWidget m_InfoPersistentABID;
	protected TextWidget m_InfoPersistentCDID;
	protected TextWidget m_InfoName;
	protected TextWidget m_InfoClassName;
	protected TextWidget m_InfoPos;
	protected TextWidget m_InfoOri;
	protected TextWidget m_InfoType;
	protected TextWidget m_InfoDestroyed;

	protected Widget m_AnimalOptionsPanel;
	protected ButtonWidget m_DeleteAnimalButton;
	protected TextWidget m_DeleteAnimalButtonLabel;
	protected ButtonWidget m_CancelEdit;
	protected TextWidget m_CancelEditLabel;
	protected ButtonWidget m_TeleportButton;
	protected TextWidget m_TeleportButtonLabel;
	protected ButtonWidget m_HealButton;
	protected TextWidget m_HealButtonLabel;

	protected ref array<ref JMAnimalsMapMarker> m_MapMarkers;
	protected ref array<ref JMAnimalsListEntry> m_AnimalEntries;

	protected JMAnimalMetaData m_CurrentAnimal;
	protected bool m_IsInAnimalInfo = false;

	void JMAnimalsMenu()
	{
		m_MapMarkers = new array<ref JMAnimalsMapMarker>;
		m_AnimalEntries = new array<ref JMAnimalsListEntry>;
	}

	protected override bool SetModule(JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}

	override void OnInit()
	{
		m_AnimalMapPanel = Widget.Cast(layoutRoot.FindAnyWidget("map_window_panel"));

		m_AnimalListPanel = Widget.Cast(layoutRoot.FindAnyWidget("vehicles_list_panel"));
		m_AnimalsListContent = Widget.Cast(layoutRoot.FindAnyWidget("vehicles_list_content"));

		m_MapWidgetPanel = Widget.Cast(layoutRoot.FindAnyWidget("vehicles_map_panel"));
		m_MapWidget = MapWidget.Cast(layoutRoot.FindAnyWidget("vehicles_map"));

		m_RefreshButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("vehicles_refresh_button"));
		m_RefreshButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("vehicles_refresh_button_label"));
		m_DeleteAllButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("delete_all_button"));
		m_DeleteAllButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("delete_all_button_label"));
		m_RadiusInput = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("animals_radius_input"));
		m_DeleteRadiusButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("delete_radius_button"));
		m_DeleteRadiusButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("delete_radius_button_label"));

		m_AnimalInfoPanel = Widget.Cast(layoutRoot.FindAnyWidget("vehicle_info_panel"));
		m_InfoNetworkID = TextWidget.Cast(layoutRoot.FindAnyWidget("info_network_id_value"));
		m_InfoPersistentABID = TextWidget.Cast(layoutRoot.FindAnyWidget("info_persistent_ab_id_value"));
		m_InfoPersistentCDID = TextWidget.Cast(layoutRoot.FindAnyWidget("info_persistent_cd_id_value"));
		m_InfoName = TextWidget.Cast(layoutRoot.FindAnyWidget("info_name_value"));
		m_InfoClassName = TextWidget.Cast(layoutRoot.FindAnyWidget("info_classname_value"));
		m_InfoPos = TextWidget.Cast(layoutRoot.FindAnyWidget("info_pos_value"));
		m_InfoOri = TextWidget.Cast(layoutRoot.FindAnyWidget("info_ori_value"));
		m_InfoType = TextWidget.Cast(layoutRoot.FindAnyWidget("info_type_value"));
		m_InfoDestroyed = TextWidget.Cast(layoutRoot.FindAnyWidget("info_destroyed_value"));

		// Lignes d'info specifiques vehicules : on les montre pour reutiliser PV / etat.
		if (m_InfoType)
			m_InfoType.GetParent().Show(true);

		m_AnimalOptionsPanel = Widget.Cast(layoutRoot.FindAnyWidget("vehicle_info_buttons_panel"));
		m_DeleteAnimalButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("delete_vehicle_button"));
		m_DeleteAnimalButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("delete_vehicle_button_label"));
		m_TeleportButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("teleport_to_button"));
		m_TeleportButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("teleport_to_button_label"));
		m_HealButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("heal_animal_button"));
		m_HealButtonLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("heal_animal_button_label"));
		m_CancelEdit = ButtonWidget.Cast(layoutRoot.FindAnyWidget("cancel_edit_button"));
		m_CancelEditLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("cancel_edit_button_label"));

		// Bouton supprimer de la fiche en majuscules.
		if (m_DeleteAnimalButtonLabel)
			m_DeleteAnimalButtonLabel.SetText("SUPPRIMER");
	}

	void LoadAnimals()
	{
		m_MapMarkers.Clear();
		m_AnimalEntries.Clear();

		if (m_Module)
		{
			auto animals = m_Module.GetServerAnimals();
			TStringArray entryLabels = {};

			foreach (JMAnimalMetaData currentAnimal: animals)
			{
				int color = ARGB(255, 46, 204, 113);

				if (!m_IsInAnimalInfo)
				{
					JMAnimalsMapMarker marker = new JMAnimalsMapMarker(m_AnimalMapPanel, m_MapWidget, currentAnimal.m_Position, color, "", currentAnimal, this);
					m_MapMarkers.Insert(marker);
				}

				JMAnimalsListEntry listEntry = new JMAnimalsListEntry(m_AnimalsListContent, this, currentAnimal);
				m_AnimalEntries.Insert(listEntry);

				int idx = 0;
				foreach (string entryLabel: entryLabels)
				{
					if (JMStatics.StrCaseCmp(listEntry.GetLabel(), entryLabel) < 0)
						break;
					idx++;
				}
				entryLabels.InsertAt(listEntry.GetLabel(), idx);
			}

			int sort;
			foreach (auto le: m_AnimalEntries)
			{
				sort = entryLabels.Find(le.GetLabel());
				if (sort > -1)
					le.SetSort(sort, false);
			}
		}
	}

	void SetAnimalInfo(JMAnimalMetaData animal)
	{
		m_IsInAnimalInfo = true;

		m_MapWidgetPanel.Show(false);
		HideMapMarkers();

		m_AnimalInfoPanel.Show(true);
		m_AnimalOptionsPanel.Show(true);

		m_InfoNetworkID.SetText(animal.m_NetworkIDHigh.ToString() + "    " + animal.m_NetworkIDLow.ToString());
		m_InfoPersistentABID.SetText(animal.m_PersistentIDA.ToString() + "    " + animal.m_PersistentIDB.ToString());
		m_InfoPersistentCDID.SetText(animal.m_PersistentIDC.ToString() + "    " + animal.m_PersistentIDD.ToString());
		m_InfoName.SetText(animal.m_DisplayName);
		m_InfoClassName.SetText(animal.m_ClassName);
		m_InfoPos.SetText("X: " + animal.m_Position[0] + " Z: " + animal.m_Position[1] + " Y: " + animal.m_Position[2]);
		m_InfoOri.SetText("Yaw: " + animal.m_Orientation[0] + " Pitch: " + animal.m_Orientation[1] + " Roll: " + animal.m_Orientation[2]);

		string hp = animal.m_Health.ToString() + " / " + animal.m_MaxHealth.ToString();
		m_InfoType.SetText(hp);

		if (animal.m_IsAlive)
			m_InfoDestroyed.SetText("#STR_GMA_YES");
		else
			m_InfoDestroyed.SetText("#STR_GMA_NO");

		m_CurrentAnimal = animal;

		UpdateMapPosition(false, animal.m_Position);
	}

	void SyncAndRefreshAnimals()
	{
		m_Module.RequestServerAnimals();
	}

	void ShowMapMarkers()
	{
		for (int i = 0; i < m_MapMarkers.Count(); i++)
		{
			JMAnimalsMapMarker marker = m_MapMarkers.Get(i);
			marker.ShowMarker();
		}
	}

	void HideMapMarkers()
	{
		for (int i = 0; i < m_MapMarkers.Count(); i++)
		{
			JMAnimalsMapMarker marker = m_MapMarkers.Get(i);
			marker.HideMarker();
		}
	}

	void BackToList()
	{
		m_IsInAnimalInfo = false;

		ShowMapMarkers();

		m_AnimalListPanel.Show(true);
		m_MapWidgetPanel.Show(true);

		m_AnimalInfoPanel.Show(false);
		m_AnimalOptionsPanel.Show(false);

		m_InfoNetworkID.SetText("");
		m_InfoPersistentABID.SetText("");
		m_InfoPersistentCDID.SetText("");
		m_InfoName.SetText("");
		m_InfoClassName.SetText("");
		m_InfoPos.SetText("");
		m_InfoOri.SetText("");
		m_InfoType.SetText("");
		m_InfoDestroyed.SetText("");

		m_CurrentAnimal = NULL;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_RefreshButton)
		{
			SyncAndRefreshAnimals();
		}

		if (w == m_CancelEdit)
		{
			BackToList();
		}

		if (w == m_DeleteAllButton)
		{
			CreateConfirmation_Two(JMConfirmationType.INFO, "#STR_GMA_DELETE_ALL", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "DeleteAnimalAll");
		}

		if (w == m_DeleteRadiusButton)
		{
			CreateConfirmation_Two(JMConfirmationType.INFO, "#STR_GMA_DELETE_RADIUS", "#STR_GMA_DELETE_RADIUS_CONFIRM", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "DeleteAnimalRadius");
		}

		if (w == m_DeleteAnimalButton)
		{
			if (m_CurrentAnimal)
			{
				CreateConfirmation_Two(JMConfirmationType.INFO, "#STR_GMA_DELETE_ONE", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "DeleteAnimal");
			}
		}

		if (w == m_TeleportButton)
		{
			if (m_CurrentAnimal)
			{
				TeleportToAnimal();
			}
		}

		if (w == m_HealButton)
		{
			if (m_CurrentAnimal)
			{
				HealCurrentAnimal();
			}
		}

		return false;
	}

	void DeleteAnimal(JMConfirmation confirmation)
	{
		m_Module.DeleteAnimal(m_CurrentAnimal);
		BackToList();
	}

	void DeleteAnimalAll(JMConfirmation confirmation)
	{
		m_Module.DeleteAnimalAll();
	}

	void DeleteAnimalRadius(JMConfirmation confirmation)
	{
		float radius = 0;
		if (m_RadiusInput)
			radius = m_RadiusInput.GetText().ToFloat();

		if (radius <= 0)
			return;

		m_Module.DeleteAnimalRadius(radius);
	}

	override void OnShow()
	{
		super.OnShow();

		SyncAndRefreshAnimals();

		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateMapPosition, 34, false, true, vector.Zero);
		// Marqueurs joueurs rafraichis chaque seconde, liste des animaux toutes les 10s.
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateMapMarkers, 1000, true);
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(AutoRefreshAnimals, 10000, true);
	}

	override void OnHide()
	{
		super.OnHide();

		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateMapMarkers);
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(AutoRefreshAnimals);
	}

	// Auto-refresh de la liste/carte des animaux (pas pendant la consultation d'une fiche).
	void AutoRefreshAnimals()
	{
		if (m_IsInAnimalInfo)
			return;

		SyncAndRefreshAnimals();
	}

	// Affiche la position des joueurs sur la carte (marqueurs natifs).
	void UpdateMapMarkers()
	{
		if (m_IsInAnimalInfo)
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

	void TeleportToAnimal()
	{
		m_Module.RequestTeleportToAnimal(m_CurrentAnimal);
	}

	void HealCurrentAnimal()
	{
		if (!m_CurrentAnimal)
			return;

		m_Module.HealAnimal(m_CurrentAnimal);

		// Mise a jour immediate de la fiche (le refresh serveur confirmera ensuite).
		m_CurrentAnimal.m_Health = m_CurrentAnimal.m_MaxHealth;
		m_CurrentAnimal.m_IsAlive = true;

		m_InfoType.SetText(m_CurrentAnimal.m_Health.ToString() + " / " + m_CurrentAnimal.m_MaxHealth.ToString());
		m_InfoDestroyed.SetText("#STR_GMA_YES");
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (w == m_DeleteAllButton)
			m_DeleteAllButtonLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_DeleteRadiusButton)
			m_DeleteRadiusButtonLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_DeleteAnimalButton)
			m_DeleteAnimalButtonLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_CancelEdit)
			m_CancelEditLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_TeleportButton)
			m_TeleportButtonLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_HealButton)
			m_HealButtonLabel.SetColor(ARGB(255, 0, 0, 0));
		else if (w == m_RefreshButton)
			m_RefreshButtonLabel.SetColor(ARGB(255, 0, 0, 0));

		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (w == m_DeleteAllButton)
			m_DeleteAllButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_DeleteRadiusButton)
			m_DeleteRadiusButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_DeleteAnimalButton)
			m_DeleteAnimalButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_CancelEdit)
			m_CancelEditLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_TeleportButton)
			m_TeleportButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_HealButton)
			m_HealButtonLabel.SetColor(ARGB(255, 255, 255, 255));
		else if (w == m_RefreshButton)
			m_RefreshButtonLabel.SetColor(ARGB(255, 255, 255, 255));

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
