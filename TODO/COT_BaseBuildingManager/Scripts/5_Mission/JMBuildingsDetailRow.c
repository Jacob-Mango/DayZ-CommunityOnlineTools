// Une ligne du tableau de detail = un element de construction de la zone.
// Colonnes : Type/Nom | Sante | Duree de vie | Etat | [TP] [Supprimer]
class JMBuildingsDetailRow: ScriptedWidgetEventHandler
{
	protected Widget m_Root;
	protected TextWidget m_ColName;
	protected TextWidget m_ColHealth;
	protected TextWidget m_ColLife;
	protected TextWidget m_ColState;
	protected ButtonWidget m_TpButton;
	protected TextWidget m_TpLabel;
	protected ButtonWidget m_DeleteButton;
	protected TextWidget m_DeleteLabel;

	protected ref JMBuildMetaData m_Meta;
	protected ref JMBuildingsMenu m_Menu;

	void JMBuildingsDetailRow(Widget parent, JMBuildingsMenu menu, JMBuildMetaData meta)
	{
		m_Root = g_Game.GetWorkspace().CreateWidgets("COT_BaseBuildingManager/GUI/layouts/buildings/Buildings_Detail_Row.layout", parent);

		m_ColName = TextWidget.Cast(m_Root.FindAnyWidget("row_name"));
		m_ColHealth = TextWidget.Cast(m_Root.FindAnyWidget("row_health"));
		m_ColLife = TextWidget.Cast(m_Root.FindAnyWidget("row_life"));
		m_ColState = TextWidget.Cast(m_Root.FindAnyWidget("row_state"));
		m_TpButton = ButtonWidget.Cast(m_Root.FindAnyWidget("row_tp_button"));
		m_TpLabel = TextWidget.Cast(m_Root.FindAnyWidget("row_tp_label"));
		m_DeleteButton = ButtonWidget.Cast(m_Root.FindAnyWidget("row_delete_button"));
		m_DeleteLabel = TextWidget.Cast(m_Root.FindAnyWidget("row_delete_label"));

		m_Menu = menu;
		m_Meta = meta;

		m_Root.SetHandler(this);

		SetEntry();
	}

	void ~JMBuildingsDetailRow()
	{
		if (g_Game && m_Root)
			m_Root.Unlink();
	}

	JMBuildMetaData GetMeta()
	{
		return m_Meta;
	}

	void SetEntry()
	{
		if (!m_Meta)
			return;

		string typeTag = "[B]";
		if (m_Meta.m_Category == 1)
			typeTag = "[T]";

		m_ColName.SetText(typeTag + " " + m_Meta.m_DisplayName);

		int pct = Math.Round(m_Meta.GetHealthPercent());
		m_ColHealth.SetText(pct.ToString() + " %");

		// Couleur de la sante : vert -> rouge.
		if (pct >= 66)
			m_ColHealth.SetColor(ARGB(255, 46, 204, 113));
		else if (pct >= 33)
			m_ColHealth.SetColor(ARGB(255, 241, 196, 15));
		else
			m_ColHealth.SetColor(ARGB(255, 231, 76, 60));

		if (m_ColLife)
		{
			int days = Math.Round(m_Meta.GetLifetimeDays());
			m_ColLife.SetText(days.ToString() + " " + Widget.TranslateString("#STR_BBM_DAYS_SHORT"));
		}

		if (m_Meta.m_IsRuined)
		{
			m_ColState.SetText("#STR_BBM_RUINED");
			m_ColState.SetColor(ARGB(255, 231, 76, 60));
		}
		else
		{
			m_ColState.SetText("#STR_BBM_OK");
			m_ColState.SetColor(ARGB(255, 46, 204, 113));
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_TpButton && w == m_TpButton)
		{
			if (m_Menu && m_Meta)
			{
				m_Menu.TeleportToMeta(m_Meta);
				return true;
			}
		}

		if (m_DeleteButton && w == m_DeleteButton)
		{
			if (m_Menu && m_Meta)
			{
				m_Menu.RequestDeleteSingle(m_Meta);
				return true;
			}
		}
		return false;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_TpButton && w == m_TpButton && m_TpLabel)
			m_TpLabel.SetColor(ARGB(255, 0, 0, 0));
		if (m_DeleteButton && w == m_DeleteButton && m_DeleteLabel)
			m_DeleteLabel.SetColor(ARGB(255, 0, 0, 0));
		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_TpButton && w == m_TpButton && m_TpLabel)
			m_TpLabel.SetColor(ARGB(255, 255, 255, 255));
		if (m_DeleteButton && w == m_DeleteButton && m_DeleteLabel)
			m_DeleteLabel.SetColor(ARGB(255, 255, 255, 255));
		return false;
	}
}
