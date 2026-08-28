class JMAnimalsMapMarker: ScriptedWidgetEventHandler
{
	protected Widget m_Root;
	protected TextWidget m_Name;
	protected ImageWidget m_Icon;
	protected ButtonWidget m_MarkerButton;
	protected MapWidget m_MapWidget;
	protected vector m_MarkerPos;
	protected string m_MarkerName;
	protected string m_MarkerIcon;
	protected int m_MarkerColor;

	protected ref JMAnimalMetaData m_Animal;
	protected ref JMAnimalsMenu m_Menu;

	protected ref Timer m_MarkerUpdateTimer;

	void JMAnimalsMapMarker(Widget parent, MapWidget mapwidget, vector pos, int color, string icon, JMAnimalMetaData animal, JMAnimalsMenu menu)
	{
		m_Root = g_Game.GetWorkspace().CreateWidgets("COT_AnimalManager/GUI/layouts/animals/Animals_Marker.layout", parent);

		m_Name = TextWidget.Cast(m_Root.FindAnyWidget("marker_name"));
		m_Icon = ImageWidget.Cast(m_Root.FindAnyWidget("marker_icon"));
		m_MarkerButton = ButtonWidget.Cast(m_Root.FindAnyWidget("marker_button"));

		m_MapWidget = mapwidget;
		m_MarkerPos = pos;
		m_MarkerIcon = icon;
		m_MarkerColor = color;
		m_Animal = animal;
		m_Menu = menu;

		m_MarkerName = m_Animal.m_DisplayName;

		if (m_MarkerIcon != string.Empty)
			m_Icon.LoadImageFile(0, m_MarkerIcon);

		m_Icon.SetColor(m_MarkerColor);
		m_Name.SetText(m_MarkerName);
		m_Name.SetColor(m_MarkerColor);

		m_Root.SetHandler(this);

		RunUpdateTimer();
	}

	void ~JMAnimalsMapMarker()
	{
		StopUpdateTimer();

		if (g_Game && m_Root)
			m_Root.Unlink();
	}

	void Update(float timeslice)
	{
		if (!m_MapWidget)
			return;

		if (!m_MarkerPos)
			return;

		vector mapPos = m_MapWidget.MapToScreen(m_MarkerPos);

		float x;
		float y;
		m_Root.GetParent().GetScreenPos(x, y);

		m_Root.SetPos(mapPos[0] - x, mapPos[1] - y, true);
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_MarkerButton && w == m_MarkerButton)
		{
			StopUpdateTimer();
			m_Icon.SetColor(ARGB(255, 255, 255, 255));
			m_Name.SetColor(ARGB(255, 255, 255, 255));
			return true;
		}

		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_MarkerButton && w == m_MarkerButton)
		{
			RunUpdateTimer();
			m_Icon.SetColor(m_MarkerColor);
			m_Name.SetColor(m_MarkerColor);
			return true;
		}

		return false;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_MarkerButton && w == m_MarkerButton)
		{
			m_Menu.SetAnimalInfo(m_Animal);
			return true;
		}

		return false;
	}

	void HideMarker()
	{
		m_Root.Show(false);
	}

	void ShowMarker()
	{
		m_Root.Show(true);
	}

	void StopUpdateTimer()
	{
		if (m_MarkerUpdateTimer && m_MarkerUpdateTimer.IsRunning())
		{
			m_MarkerUpdateTimer.Stop();
			m_MarkerUpdateTimer = NULL;
		}
	}

	void RunUpdateTimer()
	{
		if (!m_MarkerUpdateTimer)
		{
			m_MarkerUpdateTimer = new Timer(CALL_CATEGORY_GUI);
			if (!m_MarkerUpdateTimer.IsRunning())
			{
				m_MarkerUpdateTimer.Run(0.01, this, "Update", NULL, true);
			}
		}
	}
}
