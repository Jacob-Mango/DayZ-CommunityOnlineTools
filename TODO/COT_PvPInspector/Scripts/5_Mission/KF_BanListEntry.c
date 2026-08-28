// Une ligne de la fenetre "Liste des bans" : Identifiant | Duree restante | Message | [Debannir].
// - La colonne Identifiant affiche le SteamID64 si on le connait, sinon le GUID (repli).
//   Elle est CLIQUABLE : un clic ouvre le profil Steam (si le SteamID est connu).
// - Le bouton Debannir utilise TOUJOURS le GUID (COT indexe les bans par GUID, pas par SteamID).
class KF_BanListEntry: ScriptedWidgetEventHandler
{
	protected Widget m_Root;
	protected ButtonWidget m_NameBtn;   // colonne identifiant cliquable (-> profil Steam)
	protected TextWidget m_Name;        // texte affiche dans cette colonne
	protected TextWidget m_Duration;
	protected TextWidget m_Admin;
	protected ButtonWidget m_Unban;

	protected int m_Sort;

	protected ref KF_BanRecord m_Ban;
	protected ref JMKillFeedMenu m_Menu;

	void KF_BanListEntry(Widget parent, JMKillFeedMenu menu, KF_BanRecord ban, int serverNow)
	{
		m_Root = g_Game.GetWorkspace().CreateWidgets("COT_PvPInspector/GUI/layouts/killfeed/KillFeed_Ban_Entry.layout", parent);

		m_NameBtn  = ButtonWidget.Cast(m_Root.FindAnyWidget("col_ban_name"));
		m_Name     = TextWidget.Cast(m_Root.FindAnyWidget("col_ban_name_text"));
		m_Duration = TextWidget.Cast(m_Root.FindAnyWidget("col_ban_duration"));
		m_Admin    = TextWidget.Cast(m_Root.FindAnyWidget("col_ban_admin"));
		m_Unban    = ButtonWidget.Cast(m_Root.FindAnyWidget("kf_ban_unban"));

		m_Menu = menu;
		m_Ban = ban;

		m_Root.SetHandler(this);

		SetEntry(serverNow);
	}

	void ~KF_BanListEntry()
	{
		if (g_Game && m_Root)
			m_Root.Unlink();
	}

	void SetSort(int sort, bool immedUpdate = true)
	{
		m_Sort = sort;
		m_Root.SetSort(sort, immedUpdate);
	}

	void SetEntry(int serverNow)
	{
		if (!m_Ban)
			return;

		// Affiche le SteamID si connu, sinon le GUID en repli.
		string disp = m_Ban.SteamId;
		if (disp == "")
			disp = m_Ban.Identifier;
		m_Name.SetText(disp);

		// Indice visuel : identifiant cliquable (bleu Steam) si on a un SteamID, sinon gris.
		if (m_Ban.SteamId != "")
			m_Name.SetColor(ARGB(255, 102, 192, 244)); // bleu Steam -> cliquable
		else
			m_Name.SetColor(ARGB(255, 180, 180, 180)); // gris -> pas de profil a ouvrir

		m_Duration.SetText(m_Ban.RemainingStr(serverNow));
		if (m_Ban.BanDuration <= 0)
			m_Duration.SetColor(ARGB(255, 231, 76, 60));   // permanent : rouge
		else
			m_Duration.SetColor(ARGB(255, 243, 156, 18));  // temporaire : orange

		m_Admin.SetText(m_Ban.Message);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		// Clic sur l'identifiant -> profil Steam (si SteamID connu).
		if (m_NameBtn && w == m_NameBtn && m_Ban && m_Ban.SteamId != "")
		{
			g_Game.OpenURL("https://steamcommunity.com/profiles/" + m_Ban.SteamId);
			return true;
		}
		// Clic sur Debannir -> deban par GUID (cle COT).
		if (m_Unban && w == m_Unban && m_Menu && m_Ban)
		{
			m_Menu.UnbanFromList(m_Ban.Identifier);
			return true;
		}
		return false;
	}
}
