// Une ligne de la liste de gauche = une zone/groupe de constructions.
class JMBuildingsGroupEntry: ScriptedWidgetEventHandler
{
	protected Widget m_Root;
	protected TextWidget m_GroupName;
	protected TextWidget m_GroupInfo;
	protected ButtonWidget m_EditButton;
	protected ImageWidget m_StatusIcon;

	protected ref JMBuildGroup m_Group;
	protected int m_Index;
	protected ref JMBuildingsMenu m_Menu;

	void JMBuildingsGroupEntry(Widget parent, JMBuildingsMenu menu, JMBuildGroup group, int index)
	{
		m_Root = g_Game.GetWorkspace().CreateWidgets("COT_BaseBuildingManager/GUI/layouts/buildings/Buildings_Group_Entry.layout", parent);
		m_GroupName = TextWidget.Cast(m_Root.FindAnyWidget("group_name"));
		m_GroupInfo = TextWidget.Cast(m_Root.FindAnyWidget("group_info"));
		m_EditButton = ButtonWidget.Cast(m_Root.FindAnyWidget("edit_button"));
		m_StatusIcon = ImageWidget.Cast(m_Root.FindAnyWidget("group_status_icon"));

		m_Menu = menu;
		m_Group = group;
		m_Index = index;

		m_Root.SetHandler(this);

		SetEntry();
	}

	void ~JMBuildingsGroupEntry()
	{
		if (g_Game && m_Root)
			m_Root.Unlink();
	}

	JMBuildGroup GetGroup()
	{
		return m_Group;
	}

	void SetEntry()
	{
		if (!m_Group)
			return;

		m_GroupName.SetText("Zone " + (m_Index + 1).ToString() + "  -  " + m_Group.Count().ToString() + " elem.");

		// Duree restante MOYENNE de la zone. "Reste" identique dans toutes les langues.
		int days = Math.Round(m_Group.GetAvgLifetimeDays());
		m_GroupInfo.SetText(Widget.TranslateString("#STR_BBM_REMAINING") + " : " + days.ToString() + Widget.TranslateString("#STR_BBM_DAYS_SHORT"));

		if (m_StatusIcon)
			m_StatusIcon.SetColor(ARGB(255, 241, 196, 15)); // jaune "zone"
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_EditButton && w == m_EditButton)
		{
			if (m_Menu && m_Group)
			{
				m_Menu.OpenGroup(m_Group);
				return true;
			}
		}
		return false;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_EditButton && w == m_EditButton)
		{
			m_GroupName.SetColor(ARGB(255, 0, 0, 0));
			m_GroupInfo.SetColor(ARGB(255, 0, 0, 0));
		}
		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_EditButton && w == m_EditButton)
		{
			m_GroupName.SetColor(ARGB(255, 255, 255, 255));
			m_GroupInfo.SetColor(ARGB(255, 255, 255, 255));
		}
		return false;
	}
}
