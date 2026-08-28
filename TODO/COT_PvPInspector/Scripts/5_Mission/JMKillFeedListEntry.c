// Une ligne du journal : 5 colonnes (Tueur | Victime | Distance | Tir a vue ? | Cadrage).
// Un bouton transparent couvre toute la ligne pour capter le double-clic,
// qui ouvre la fenetre de detail (page_detail).
class JMKillFeedListEntry: ScriptedWidgetEventHandler
{
	protected Widget m_Root;
	protected ButtonWidget m_Button;

	protected TextWidget m_Date;
	protected TextWidget m_Killer;
	protected TextWidget m_Victim;
	protected TextWidget m_Dist;
	protected TextWidget m_Weapon;
	protected TextWidget m_LoS;
	protected TextWidget m_Aim;

	protected int m_Sort;

	protected ref KF_KillRecord m_Kill;
	protected ref JMKillFeedMenu m_Menu;

	void JMKillFeedListEntry(Widget parent, JMKillFeedMenu menu, KF_KillRecord kill)
	{
		m_Root = g_Game.GetWorkspace().CreateWidgets("COT_PvPInspector/GUI/layouts/killfeed/KillFeed_List_Entry.layout", parent);

		m_Button = ButtonWidget.Cast(m_Root.FindAnyWidget("entry_button"));
		m_Date   = TextWidget.Cast(m_Root.FindAnyWidget("col_date"));
		m_Killer = TextWidget.Cast(m_Root.FindAnyWidget("col_killer"));
		m_Victim = TextWidget.Cast(m_Root.FindAnyWidget("col_victim"));
		m_Dist   = TextWidget.Cast(m_Root.FindAnyWidget("col_dist"));
		m_Weapon = TextWidget.Cast(m_Root.FindAnyWidget("col_weapon"));
		m_LoS    = TextWidget.Cast(m_Root.FindAnyWidget("col_los"));
		m_Aim    = TextWidget.Cast(m_Root.FindAnyWidget("col_aim"));

		m_Menu = menu;
		m_Kill = kill;

		m_Root.SetHandler(this);

		SetEntry();
	}

	void ~JMKillFeedListEntry()
	{
		if (g_Game && m_Root)
			m_Root.Unlink();
	}

	void SetSort(int sort, bool immedUpdate = true)
	{
		m_Sort = sort;
		m_Root.SetSort(sort, immedUpdate);
	}

	int GetId()
	{
		if (m_Kill)
			return m_Kill.Id;
		return 0;
	}

	void SetEntry()
	{
		if (!m_Kill)
			return;

		string dateStr = m_Kill.TimeStr;
		if (dateStr.Length() > 16)
			dateStr = dateStr.Substring(0, 16);
		m_Date.SetText(dateStr);

		m_Killer.SetText(m_Kill.KillerName);
		m_Victim.SetText(m_Kill.VictimName);

		int dist = (int)Math.Round(m_Kill.Distance);
		m_Dist.SetText(dist.ToString() + " m");

		m_Weapon.SetText(m_Kill.Weapon);

		// Colonne "Tir a vue ?"
		if (!m_Kill.CombatInfo)
		{
			// Explosif / piege / vehicule : visibilite non pertinente.
			m_LoS.SetText("-");
			m_LoS.SetColor(ARGB(255, 150, 150, 150));
		}
		else if (m_Kill.LoS)
		{
			m_LoS.SetText("#STR_GKFM_YES");
			m_LoS.SetColor(ARGB(255, 46, 204, 113));
		}
		else
		{
			m_LoS.SetText("#STR_GKFM_NO_BLIND");
			m_LoS.SetColor(ARGB(255, 231, 76, 60));
		}

		// Colonne "Cadrage / angle"
		if (!m_Kill.CombatInfo)
		{
			m_Aim.SetText("-");
			m_Aim.SetColor(ARGB(255, 150, 150, 150));
		}
		else
		{
			int ang = (int)Math.Round(m_Kill.AimAngle);
			if (m_Kill.AimLegit)
			{
				m_Aim.SetText("OK (" + ang.ToString() + " deg)");
				m_Aim.SetColor(ARGB(255, 46, 204, 113));
			}
			else
			{
				m_Aim.SetText("Suspect (" + ang.ToString() + " deg)");
				m_Aim.SetColor(ARGB(255, 231, 76, 60));
			}
		}
	}

	void Highlight(bool on)
	{
		int c = ARGB(255, 255, 255, 255);
		if (on)
			c = ARGB(255, 255, 220, 120);

		// On ne recolore pas LoS/Aim (couleurs porteuses de sens), seulement les noms.
		m_Date.SetColor(c);
		m_Killer.SetColor(c);
		m_Victim.SetColor(c);
		m_Dist.SetColor(c);
		m_Weapon.SetColor(c);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		// Simple clic : selection visuelle uniquement.
		return false;
	}

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (m_Button && w == m_Button && m_Menu && m_Kill)
		{
			m_Menu.OpenDetail(m_Kill);
			return true;
		}
		return false;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_Button && w == m_Button)
			Highlight(true);
		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_Button && w == m_Button)
			Highlight(false);
		return false;
	}
}
