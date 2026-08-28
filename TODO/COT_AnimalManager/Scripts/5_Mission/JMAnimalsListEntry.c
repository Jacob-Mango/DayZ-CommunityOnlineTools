class JMAnimalsListEntry: ScriptedWidgetEventHandler
{
	protected Widget m_Root;
	protected TextWidget m_AnimalName;
	protected TextWidget m_AnimalID;
	protected int m_Sort;
	protected string m_Label;
	protected ButtonWidget m_EditButton;
	protected ImageWidget m_StatusIcon;

	protected ref JMAnimalMetaData m_Animal;
	protected ref JMAnimalsMenu m_Menu;

	void JMAnimalsListEntry(Widget parent, JMAnimalsMenu menu, JMAnimalMetaData animal)
	{
		m_Root = g_Game.GetWorkspace().CreateWidgets("COT_AnimalManager/GUI/layouts/animals/Animals_List_Entry.layout", parent);
		m_AnimalName = TextWidget.Cast(m_Root.FindAnyWidget("vehicle_name"));
		m_AnimalID = TextWidget.Cast(m_Root.FindAnyWidget("vehicle_id"));
		m_EditButton = ButtonWidget.Cast(m_Root.FindAnyWidget("edit_button"));
		m_StatusIcon = ImageWidget.Cast(m_Root.FindAnyWidget("vehicle_status_icon"));

		m_Menu = menu;
		m_Animal = animal;

		m_Root.SetHandler(this);

		SetEntry();
	}

	void ~JMAnimalsListEntry()
	{
		if (g_Game && m_Root)
			m_Root.Unlink();
	}

	void SetSort(int sort, bool immedUpdate = true)
	{
		m_Sort = sort;
		m_Root.SetSort(sort, immedUpdate);
		SetEntry();
	}

	string GetLabel()
	{
		return m_Label;
	}

	void SetEntry()
	{
		if (m_Animal)
		{
			m_AnimalName.SetText((m_Sort + 1).ToString() + " - " + m_Animal.m_DisplayName);

			string id = " - ID: " + m_Animal.m_NetworkIDHigh.ToString() + " " + m_Animal.m_NetworkIDLow.ToString();
			m_AnimalID.SetText(id);

			m_Label = m_Animal.m_DisplayName + id;

			if (m_StatusIcon)
			{
				if (m_Animal.m_IsAlive)
					m_StatusIcon.SetColor(ARGB(255, 0, 255, 0));
				else
					m_StatusIcon.SetColor(ARGB(255, 255, 0, 0));
			}
		}
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_EditButton && w == m_EditButton)
		{
			if (m_Menu && m_Animal)
			{
				m_Menu.SetAnimalInfo(m_Animal);
				return true;
			}
		}

		return false;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_EditButton && w == m_EditButton)
		{
			m_AnimalName.SetColor(ARGB(255, 0, 0, 0));
			m_AnimalID.SetColor(ARGB(255, 0, 0, 0));
		}

		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_EditButton && w == m_EditButton)
		{
			m_AnimalName.SetColor(ARGB(255, 255, 255, 255));
			m_AnimalID.SetColor(ARGB(255, 255, 255, 255));
		}

		return false;
	}
}
