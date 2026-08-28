class JMKillFeedMenu: JMFormBase
{
	private JMKillFeedModule m_Module;

	// --- Pages ---
	protected Widget m_PageList;
	protected Widget m_PageDetail;
	protected Widget m_PageBanInput;
	protected Widget m_PageBanList;

	// --- Page liste ---
	protected Widget m_ListContent;
	protected ButtonWidget m_RefreshButton;
	protected ButtonWidget m_ClearButton;
	protected ButtonWidget m_HideAllButton;
	protected ButtonWidget m_BanListButton;

	// --- Filtre (boutons en ligne) ---
	static const int KF_FILTER_ALL      = 0; // Tous
	static const int KF_FILTER_SUSPECT  = 1; // Suspect (vue ou cadrage)
	static const int KF_FILTER_NO_LOS   = 2; // Tir aveugle
	static const int KF_FILTER_BAD_AIM  = 3; // Angle incorrect
	static const int KF_FILTER_LONG     = 4; // Longue distance
	static const int KF_FILTER_TRAPS    = 5; // Explosifs / pieges
	static const int KF_FILTER_VEHICLES = 6; // Vehicules
	static const int KF_FILTER_COUNT    = 7;
	static const int KF_LONG_RANGE_M    = 300;

	protected int m_Filter = 0;
	protected ref array<ButtonWidget> m_FilterButtons;
	protected ref array<TextWidget> m_FilterLabels;

	// --- Page detail : valeurs ---
	protected TextWidget m_vKiller;
	protected TextWidget m_vKillerGUID;
	protected TextWidget m_vVictim;
	protected TextWidget m_vVictimGUID;
	protected ButtonWidget m_KillerSteamBtn;
	protected ButtonWidget m_VictimSteamBtn;
	protected TextWidget m_vWeapon;
	protected TextWidget m_vAmmo;
	protected TextWidget m_vDistance;
	protected TextWidget m_vVisibility;
	protected TextWidget m_vAiming;
	protected TextWidget m_vAngle;
	protected TextWidget m_vKillerPos;
	protected TextWidget m_vVictimPos;
	protected TextWidget m_vTime;
	protected TextWidget m_vCause;

	// --- Page detail : boutons + carte ---
	protected ButtonWidget m_TpKillerButton;
	protected ButtonWidget m_TpVictimButton;
	protected ButtonWidget m_ShowButton;
	protected ButtonWidget m_HideButton;
	protected ButtonWidget m_BanButton;
	protected ButtonWidget m_ShowKillerButton;
	protected ButtonWidget m_HideKillerButton;
	protected ButtonWidget m_BackButton;
	protected MapWidget m_MapWidget;

	// --- Fenetre de saisie de duree (Bannir) ---
	protected TextWidget m_BanInputTitle;
	protected EditBoxWidget m_BanDaysEdit;
	protected ButtonWidget m_BanConfirm;
	protected ButtonWidget m_BanCancel;
	protected string m_BanTargetGuid;
	protected string m_BanTargetName;
	protected string m_BanTargetSteam;

	// --- Fenetre liste des bans ---
	protected Widget m_BanListContent;
	protected ButtonWidget m_BanListBack;
	protected ref array<ref KF_BanListEntry> m_BanEntries;

	protected ref array<ref JMKillFeedListEntry> m_Entries;
	protected KF_KillRecord m_Current;
	protected bool m_IsInInfo = false;
	protected bool m_SpawnAvailable = false;

	void JMKillFeedMenu()
	{
		m_Entries = new array<ref JMKillFeedListEntry>;
		m_FilterButtons = new array<ButtonWidget>;
		m_FilterLabels = new array<TextWidget>;
		m_BanEntries = new array<ref KF_BanListEntry>;
	}

	protected override bool SetModule(JMRenderableModuleBase mdl)
	{
		return Class.CastTo(m_Module, mdl);
	}

	protected string KF_PosStr(vector v)
	{
		int x = (int)Math.Round(v[0]);
		int y = (int)Math.Round(v[1]);
		int z = (int)Math.Round(v[2]);
		return "X: " + x + "  Z: " + z + "  Y: " + y;
	}

	override void OnInit()
	{
		m_PageList = Widget.Cast(layoutRoot.FindAnyWidget("page_list"));
		m_PageDetail = Widget.Cast(layoutRoot.FindAnyWidget("page_detail"));
		m_PageBanInput = Widget.Cast(layoutRoot.FindAnyWidget("page_ban_input"));
		m_PageBanList = Widget.Cast(layoutRoot.FindAnyWidget("page_banlist"));

		m_ListContent = Widget.Cast(layoutRoot.FindAnyWidget("list_content"));
		m_RefreshButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_refresh_button"));
		m_ClearButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_clear_button"));
		m_HideAllButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_hideall_button"));
		m_BanListButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_banlist_button"));

		// Boutons de filtre kf_f0..kf_f7.
		m_FilterButtons.Clear();
		m_FilterLabels.Clear();
		for (int i = 0; i < KF_FILTER_COUNT; i++)
		{
			ButtonWidget b = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_f" + i.ToString()));
			TextWidget bl = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_f" + i.ToString() + "_label"));
			m_FilterButtons.Insert(b);
			m_FilterLabels.Insert(bl);
		}

		// Valeurs du detail.
		m_vKiller     = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_killer_value"));
		m_vKillerGUID = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_killerguid_text"));
		m_KillerSteamBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_d_killerguid_value"));
		m_vVictim     = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_victim_value"));
		m_vVictimGUID = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_victimguid_text"));
		m_VictimSteamBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_d_victimguid_value"));
		m_vWeapon     = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_weapon_value"));
		m_vAmmo       = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_ammo_value"));
		m_vDistance   = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_dist_value"));
		m_vVisibility = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_vis_value"));
		m_vAiming     = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_aim_value"));
		m_vAngle      = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_angle_value"));
		m_vKillerPos  = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_kpos_value"));
		m_vVictimPos  = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_vpos_value"));
		m_vTime       = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_time_value"));
		m_vCause      = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_d_cause_value"));

		// Boutons + carte du detail.
		m_TpKillerButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_btn_tp_killer"));
		m_TpVictimButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_btn_tp_victim"));
		m_ShowButton     = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_btn_show"));
		m_HideButton     = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_btn_hide"));
		m_BanButton      = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_btn_ban"));
		m_ShowKillerButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_btn_show_killer"));
		m_HideKillerButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_btn_hide_killer"));
		m_BackButton     = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_btn_back"));
		m_MapWidget      = MapWidget.Cast(layoutRoot.FindAnyWidget("kf_map"));

		// Fenetre de saisie de duree.
		m_BanInputTitle = TextWidget.Cast(layoutRoot.FindAnyWidget("kf_ban_input_title"));
		m_BanDaysEdit   = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("kf_ban_days"));
		m_BanConfirm    = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_ban_confirm"));
		m_BanCancel     = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_ban_cancel"));

		// Fenetre liste des bans.
		m_BanListContent = Widget.Cast(layoutRoot.FindAnyWidget("kf_banlist_content"));
		m_BanListBack    = ButtonWidget.Cast(layoutRoot.FindAnyWidget("kf_banlist_back"));

		// Afficher/Masquer la victime n'ont de sens que si COT_LoadoutManager est charge.
		m_SpawnAvailable = KF_LoadoutProvider.Get().IsAvailable();
		if (!m_SpawnAvailable)
		{
			if (m_ShowButton)
				m_ShowButton.Show(false);
			if (m_HideButton)
				m_HideButton.Show(false);
			if (m_ShowKillerButton)
				m_ShowKillerButton.Show(false);
			if (m_HideKillerButton)
				m_HideKillerButton.Show(false);
		}

		RefreshFilterButtons();
		ShowListPage();
	}

	// Bascule entre les pages : "list", "detail", "baninput", "banlist".
	protected void ShowPage(string which)
	{
		if (m_PageList)
			m_PageList.Show(which == "list");
		if (m_PageDetail)
			m_PageDetail.Show(which == "detail");
		if (m_PageBanInput)
			m_PageBanInput.Show(which == "baninput");
		if (m_PageBanList)
			m_PageBanList.Show(which == "banlist");
	}

	protected void ShowListPage()
	{
		m_IsInInfo = false;
		m_Current = NULL;

		ShowPage("list");

		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateMapMarkers);
	}

	void LoadKills()
	{
		m_Entries.Clear();

		if (!m_Module)
			return;

		array<ref KF_KillRecord> kills = m_Module.GetKills();
		foreach (KF_KillRecord k: kills)
		{
			if (!KF_Matches(m_Filter, k))
				continue;
			JMKillFeedListEntry entry = new JMKillFeedListEntry(m_ListContent, this, k);
			m_Entries.Insert(entry);
		}

		// La liste arrive deja triee (plus recent en premier). On respecte cet ordre.
		int sort = 0;
		foreach (JMKillFeedListEntry e: m_Entries)
		{
			e.SetSort(sort, false);
			sort++;
		}

		// Si une fiche detail etait ouverte (ex : COT ferme puis rouvert), on la restaure.
		if (!m_IsInInfo && m_Module.GetViewedKillId() >= 0)
		{
			KF_KillRecord vk = FindKillById(m_Module.GetViewedKillId());
			if (vk)
				OpenDetail(vk);
		}
	}

	protected KF_KillRecord FindKillById(int id)
	{
		if (!m_Module)
			return NULL;
		array<ref KF_KillRecord> kills = m_Module.GetKills();
		foreach (KF_KillRecord k: kills)
		{
			if (k && k.Id == id)
				return k;
		}
		return NULL;
	}

	// --- Filtre ---
	bool KF_Matches(int f, KF_KillRecord k)
	{
		if (!k)
			return false;
		if (f == KF_FILTER_ALL)
			return true;
		if (f == KF_FILTER_SUSPECT)
			return k.CombatInfo && (!k.LoS || !k.AimLegit);
		if (f == KF_FILTER_NO_LOS)
			return k.CombatInfo && !k.LoS;
		if (f == KF_FILTER_BAD_AIM)
			return k.CombatInfo && !k.AimLegit;
		if (f == KF_FILTER_LONG)
			return k.Distance >= KF_LONG_RANGE_M;
		if (f == KF_FILTER_TRAPS)
			return k.Cause == "Explosif" || k.Cause == "Grenade" || k.Cause == "Piege";
		if (f == KF_FILTER_VEHICLES)
			return k.Cause == "Vehicule";
		return true;
	}

	void SetFilter(int f)
	{
		m_Filter = f;
		RefreshFilterButtons();
		LoadKills();
	}

	void RefreshFilterButtons()
	{
		for (int i = 0; i < m_FilterLabels.Count(); i++)
		{
			TextWidget lbl = m_FilterLabels[i];
			if (!lbl)
				continue;
			if (i == m_Filter)
				lbl.SetColor(ARGB(255, 255, 220, 120)); // actif : jaune
			else
				lbl.SetColor(ARGB(255, 255, 255, 255)); // inactif : blanc
		}
	}

	protected bool IsFilterButton(Widget w)
	{
		for (int i = 0; i < m_FilterButtons.Count(); i++)
		{
			if (w == m_FilterButtons[i])
				return true;
		}
		return false;
	}

	// Ouvre la fiche detail (double-clic sur une ligne, ou restauration).
	void OpenDetail(KF_KillRecord k)
	{
		if (!k)
			return;

		m_IsInInfo = true;
		m_Current = k;

		if (m_Module)
			m_Module.SetViewedKillId(k.Id);

		ShowPage("detail");

		m_vKiller.SetText(k.KillerName);
		string kSteam = k.KillerSteamId;
		if (kSteam == "")
			kSteam = k.KillerGUID;
		m_vKillerGUID.SetText(kSteam);
		m_vVictim.SetText(k.VictimName);
		string vSteam = k.VictimSteamId;
		if (vSteam == "")
			vSteam = k.VictimGUID;
		m_vVictimGUID.SetText(vSteam);
		m_vWeapon.SetText(k.Weapon);
		m_vAmmo.SetText(k.Ammo);

		int dist = (int)Math.Round(k.Distance);
		m_vDistance.SetText(dist.ToString() + " m");

		if (k.CombatInfo)
		{
			if (k.LoS)
			{
				m_vVisibility.SetText("#STR_GKFM_LOS_YES");
				m_vVisibility.SetColor(ARGB(255, 46, 204, 113));
			}
			else
			{
				m_vVisibility.SetText("#STR_GKFM_NOLOS");
				m_vVisibility.SetColor(ARGB(255, 231, 76, 60));
			}

			if (k.AimLegit)
			{
				m_vAiming.SetText("#STR_GKFM_AIM_OK");
				m_vAiming.SetColor(ARGB(255, 46, 204, 113));
			}
			else
			{
				m_vAiming.SetText("#STR_GKFM_AIM_SUSPECT");
				m_vAiming.SetColor(ARGB(255, 231, 76, 60));
			}

			int ang = (int)Math.Round(k.AimAngle);
			m_vAngle.SetText(ang.ToString() + " deg");
		}
		else
		{
			m_vVisibility.SetText("-");
			m_vVisibility.SetColor(ARGB(255, 150, 150, 150));
			m_vAiming.SetText("-");
			m_vAiming.SetColor(ARGB(255, 150, 150, 150));
			m_vAngle.SetText("-");
		}

		m_vCause.SetText(KF_CauseKey(k.Cause));
		m_vKillerPos.SetText(KF_PosStr(k.KillerPos));
		m_vVictimPos.SetText(KF_PosStr(k.VictimPos));
		m_vTime.SetText(k.TimeStr);

		FocusMapOnKill(k);
		// Premier affichage : le MapWidget n'est pas encore dimensionne, on recentre a la frame suivante.
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(FocusMapOnKill);
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(FocusMapOnKill, 100, false, k);
		UpdateMapMarkers();
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateMapMarkers);
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateMapMarkers, 1000, true);
	}

	void BackToList()
	{
		if (m_Module)
			m_Module.SetViewedKillId(-1);

		ShowListPage();
		SyncAndRefresh();
	}

	void SyncAndRefresh()
	{
		if (m_Module)
			m_Module.RequestKills();
	}

	override void OnShow()
	{
		super.OnShow();

		ShowListPage();
		SyncAndRefresh();

		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(AutoRefresh, 15000, true);
	}

	override void OnHide()
	{
		super.OnHide();

		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateMapMarkers);
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(FocusMapOnKill);
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(AutoRefresh);
	}

	void AutoRefresh()
	{
		if (m_IsInInfo)
			return;
		SyncAndRefresh();
	}

	void UpdateMapMarkers()
	{
		if (!m_MapWidget || !m_IsInInfo || !m_Current)
			return;

		m_MapWidget.ClearUserMarks();
		string lblKiller = Widget.TranslateString("#STR_GKFM_KILLER") + ": " + m_Current.KillerName;
		string lblVictim = Widget.TranslateString("#STR_GKFM_VICTIM") + ": " + m_Current.VictimName;
		m_MapWidget.AddUserMark(m_Current.KillerPos, lblKiller, ARGB(255, 231, 76, 60), JM_COT_ICON_DOT + ".paa");
		m_MapWidget.AddUserMark(m_Current.VictimPos, lblVictim, ARGB(255, 243, 156, 18), JM_COT_ICON_DOT + ".paa");
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_RefreshButton)
			SyncAndRefresh();

		if (w == m_ClearButton)
			CreateConfirmation_Two(JMConfirmationType.INFO, "#STR_GKFM_CLEAR_CONFIRM", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "ClearKills");

		if (w == m_HideAllButton && m_Module)
			m_Module.RequestCleanAll();

		// Boutons de filtre.
		for (int i = 0; i < m_FilterButtons.Count(); i++)
		{
			if (w == m_FilterButtons[i])
			{
				SetFilter(i);
				return false;
			}
		}

		if (w == m_BackButton)
			BackToList();

		if (w == m_TpKillerButton && m_Current)
			TeleportWithOri(m_Current.KillerPos, m_Current.KillerOri);

		if (w == m_TpVictimButton && m_Current)
			TeleportWithOri(m_Current.VictimPos, m_Current.VictimOri);

		if (w == m_ShowButton && m_Current && m_Module)
			m_Module.RequestSpawnDummy(m_Current.VictimSteamId, m_Current.VictimPos, m_Current.VictimOri, m_Current.VictimStance, m_Current.VictimHands, m_Current.TimeStr, m_Current.Id, 0);

		if (w == m_HideButton && m_Current && m_Module)
			m_Module.RequestCleanScene(m_Current.Id, 0);

		// --- Bans ---
		if (w == m_BanButton && m_Current)
			OpenBanInput(m_Current.KillerGUID, m_Current.KillerName, m_Current.KillerSteamId);

		if (w == m_ShowKillerButton && m_Current && m_Module)
			m_Module.RequestSpawnDummy(m_Current.KillerSteamId, m_Current.KillerPos, m_Current.KillerOri, m_Current.KillerStance, m_Current.KillerHands, m_Current.TimeStr, m_Current.Id, 1);

		if (w == m_HideKillerButton && m_Current && m_Module)
			m_Module.RequestCleanScene(m_Current.Id, 1);

		if (w == m_KillerSteamBtn && m_Current && m_Current.KillerSteamId != "")
			g_Game.OpenURL("https://steamcommunity.com/profiles/" + m_Current.KillerSteamId);

		if (w == m_VictimSteamBtn && m_Current && m_Current.VictimSteamId != "")
			g_Game.OpenURL("https://steamcommunity.com/profiles/" + m_Current.VictimSteamId);

		if (w == m_BanListButton)
			OpenBanList();

		if (w == m_BanConfirm)
			ConfirmBan();

		if (w == m_BanCancel)
			ShowPage("detail");

		if (w == m_BanListBack)
			ShowListPage();

		return false;
	}

	void ClearKills(JMConfirmation confirmation)
	{
		if (m_Module)
			m_Module.ClearKills();
		BackToList();
	}

	protected string GetClientGUID()
	{
		JMPlayerInstance client = GetPermissionsManager().GetClientPlayer();
		if (!client)
			return "";
		return client.GetGUID();
	}

	void TeleportPos(vector pos)
	{
		string guid = GetClientGUID();
		if (guid == "")
			return;
		CF_Modules<JMTeleportModule>.Get().Position(pos, {guid});
	}

	void TeleportWithOri(vector pos, vector ori)
	{
		TeleportPos(pos);
		if (m_Module)
			m_Module.RequestSetSelfOri(ori); // l'orientation n'est pas geree par le module Teleport COT
	}

	// Double-clic carte -> teleportation de l'admin a l'endroit clique (sans rotation imposee).
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w == NULL)
			return false;

		if (w == m_MapWidget)
		{
			vector tpPos = SnapToGround(m_MapWidget.ScreenToMap(Vector(x, y, 0)));
			TeleportPos(tpPos);
			return true;
		}

		return false;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		HoverLabel(w, true);
		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		// Les boutons de filtre conservent leur etat actif/inactif (jaune/blanc).
		if (IsFilterButton(w))
		{
			RefreshFilterButtons();
			return false;
		}

		HoverLabel(w, false);
		return false;
	}

	protected void HoverLabel(Widget w, bool on)
	{
		int c = ARGB(255, 255, 255, 255);
		if (on)
			c = ARGB(255, 0, 0, 0);

		ButtonWidget b = ButtonWidget.Cast(w);
		if (!b)
			return;

		TextWidget lbl = TextWidget.Cast(b.FindAnyWidget(b.GetName() + "_label"));
		if (lbl)
			lbl.SetColor(c);
	}

	// Traduit la cause stockee (code FR interne) en cle #STR localisee.
	// Le stockage et les filtres continuent d'utiliser le code FR ; seule la cle
	// renvoyee ici est traduite a l'affichage (SetText traduit les #STR).
	private string KF_CauseKey(string cause)
	{
		if (cause == "Arme a feu") return "#STR_GKFM_CAUSE_FIREARM";
		if (cause == "Melee")      return "#STR_GKFM_CAUSE_MELEE";
		if (cause == "Mains nues") return "#STR_GKFM_CAUSE_BAREHANDS";
		if (cause == "Grenade")    return "#STR_GKFM_CAUSE_GRENADE";
		if (cause == "Piege")      return "#STR_GKFM_CAUSE_TRAP";
		if (cause == "Explosif")   return "#STR_GKFM_CAUSE_EXPLOSIVE";
		if (cause == "Vehicule")   return "#STR_GKFM_CAUSE_VEHICLE";
		return cause;
	}

	void FocusMapOnKill(KF_KillRecord k)
	{
		if (!m_MapWidget || !k)
			return;

		vector mid = (k.KillerPos + k.VictimPos) * 0.5;

		float scale = k.Distance / 4000.0 + 0.045;
		if (scale < 0.045)
			scale = 0.045;
		if (scale > 0.6)
			scale = 0.6;

		m_MapWidget.SetScale(scale);
		m_MapWidget.SetMapPos(mid);
	}

	// ===================== BANS (UI) =====================

	// Ouvre la fenetre de saisie de duree pour bannir le tireur courant.
	void OpenBanInput(string guid, string name, string steamId)
	{
		m_BanTargetGuid = guid;
		m_BanTargetName = name;
		m_BanTargetSteam = steamId;

		if (m_BanInputTitle)
			m_BanInputTitle.SetText(Widget.TranslateString("#STR_GKFM_BAN") + " : " + name);
		if (m_BanDaysEdit)
			m_BanDaysEdit.SetText("");

		ShowPage("baninput");
	}

	// Valide la saisie : vide ou <= 0 => permanent.
	void ConfirmBan()
	{
		if (!m_Module || m_BanTargetGuid == "")
		{
			ShowPage("detail");
			return;
		}

		int days = 0;
		if (m_BanDaysEdit)
		{
			string txt = m_BanDaysEdit.GetText();
			if (txt != "")
				days = txt.ToInt();
		}
		if (days < 0)
			days = 0;

		m_Module.RequestBan(m_BanTargetGuid, m_BanTargetName, days, m_BanTargetSteam);

		ShowPage("detail");
	}

	// Ouvre la fenetre liste des bans et demande la liste au serveur.
	void OpenBanList()
	{
		if (m_Module)
			m_Module.RequestBanList();
		ShowPage("banlist");
	}

	// (Re)construit les lignes de la liste des bans depuis le cache du module.
	void LoadBans()
	{
		m_BanEntries.Clear();

		if (!m_Module || !m_BanListContent)
			return;

		int now = m_Module.GetBansServerNow();
		array<ref KF_BanRecord> bans = m_Module.GetBans();
		int sort = 0;
		foreach (KF_BanRecord b: bans)
		{
			KF_BanListEntry e = new KF_BanListEntry(m_BanListContent, this, b, now);
			e.SetSort(sort, false);
			sort++;
			m_BanEntries.Insert(e);
		}
	}

	// Appele par une ligne de la liste (bouton Debannir).
	void UnbanFromList(string steamId)
	{
		if (m_Module)
			m_Module.RequestUnban(steamId);
	}
}
