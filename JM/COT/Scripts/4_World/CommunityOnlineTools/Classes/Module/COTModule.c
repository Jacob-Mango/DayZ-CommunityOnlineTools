class COTModule extends CF_ModuleWorld
{
	static ref array<COTModule> s_All = new array<COTModule>(); 

	ref CF_Window m_Window;

	ButtonWidget m_MenuButton;

	int m_MenuButtonColour = 0;
	
	override void OnInit()
	{
		super.OnInit();

		s_All.Insert(this);
		
		SetMenuButtonColour(1, 1, 0, 0);
		
		EnableSettingsChanged();
		EnablePermissionsChanged();
		
		if (GetInputToggle() != "")
		{
			Bind("Input_ToggleShow", GetInputToggle(), true);
		}
	}

	override void OnSettingsChanged(Class sender, CF_EventArgs args)
	{
		if (GetForm())
		{
			GetForm().OnSettingsUpdated();
		}
	}

	override void OnPermissionsChanged(Class sender, CF_EventArgs args)
	{
		bool hasAccess = HasAccess();

		if (GetForm() && hasAccess)
		{
			GetForm().OnClientPermissionsUpdated();
		}

		if (IsVisible() && !hasAccess)
		{
			Hide();
		}
	}

	bool InitButton(Widget button_bkg)
	{
		Class.CastTo(m_MenuButton, button_bkg.FindAnyWidget("btn"));
		if (!button_bkg || !m_MenuButton)
		{
			return false;
		}
		
		m_MenuButton.SetColor(m_MenuButtonColour);

		TextWidget ttl = TextWidget.Cast(button_bkg.FindAnyWidget("ttl"));
		ttl.SetText(GetLocalisedTitle());

		ImageWidget btn_img = ImageWidget.Cast(button_bkg.FindAnyWidget("btn_img"));
		TextWidget btn_txt = TextWidget.Cast(button_bkg.FindAnyWidget("btn_txt"));

		if (ImageIsIcon())
		{
			btn_txt.Show(false);
			btn_img.Show(true);

			btn_img.LoadImageFile(0, "set:" + GetImageSet() + " image:" + GetIconName());
		}
		else
		{
			btn_txt.Show(true);
			btn_img.Show(false);
			
			btn_txt.SetText(GetIconName());
		}

		return true;
	}

	void SetMenuButtonColour(float r, float g, float b, float alpha)
	{
		m_MenuButtonColour = ARGB(r * 255, g * 255, b * 255, alpha * 255);
		
		if (m_MenuButton)
		{
			m_MenuButton.SetColor(m_MenuButtonColour);
		}
	}

	ButtonWidget GetMenuButton()
	{
		return m_MenuButton;
	}

	JMFormBase GetForm()
	{
		return null;
	}

	string GetInputToggle()
	{
		return "";
	}

	string GetLayoutRoot()
	{
		return "";
	}

	override string GetLayoutFile()
	{
		return GetLayoutRoot();
	}

	string GetTitle()
	{
		return "";
	}

	string GetLocalisedTitle()
	{
		string text = Widget.TranslateString("#" + GetTitle());

		if (text == "" || text.Get(0) == " ")
		{
			return GetTitle();
		}

		return text;
	}

	string GetImageSet()
	{
		return "";
	}

	string GetIconName()
	{
		return "";
	}

	bool ImageIsIcon()
	{
		return false;
	}

	bool HasAccess()
	{
		return true;
	}

	bool HasButton()
	{
		return true;
	}

	bool IsVisible()
	{
		return m_Window != null;
	}

	void Show()
	{
		if (!HasAccess())
			return;

		m_Window = new CF_Window();
		m_Window.OnClose.AddSubscriber(OnHide);

		m_Window.SetModel(this);

		OnShow(this, new CF_EventArgs());
	}

	void Hide()
	{
		delete m_Window;
	}

	void OnShow(Class sender, CF_EventArgs args)
	{

	}

	void OnHide(Class sender, CF_EventArgs args)
	{

	}

	void ToggleShow()
	{
		if (IsVisible())
		{
			Hide();
		}
		else
		{
			if (GetGame().GetUIManager().GetMenu())
			{
				return;
			}

			if (!GetCommunityOnlineToolsBase().IsActive())
			{
				COTCreateLocalAdminNotification(new StringLocaliser("STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF"));
				return;
			}

			Show();
		}
	}

	void Input_ToggleShow(UAInput input)
	{
		if (!input.LocalClick())
			return;

		ToggleShow();
	}
};
