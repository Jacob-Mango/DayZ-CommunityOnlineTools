#ifndef JM_COT
class ExpansionConfigReloadMenu: ExpansionUIScriptedMenu
{
	protected Widget m_Root;
	protected Widget m_HeaderDrag;
	protected ButtonWidget m_CloseButton;
	protected GridSpacerWidget m_ButtonGrid;
	protected ref array<ref ExpansionConfigReloadMenuAction> m_Actions;

	void ExpansionConfigReloadMenu()
	{
		m_Actions = new array<ref ExpansionConfigReloadMenuAction>;
	}

	override Widget Init()
	{
		m_Root = GetGame().GetWorkspace().CreateWidgets(ExpansionConfigReloadConstants.LAYOUT_PATH);
		layoutRoot = m_Root;

		m_HeaderDrag = m_Root.FindAnyWidget("Header");
		m_CloseButton = ButtonWidget.Cast(m_Root.FindAnyWidget("BtnClose"));
		m_ButtonGrid = GridSpacerWidget.Cast(m_Root.FindAnyWidget("ButtonGrid"));

		ApplyLocalizedTexts();
		RegisterButtons();

		if (m_ButtonGrid)
		{
			m_ButtonGrid.Update();
		}

		return layoutRoot;
	}

	override void OnShow()
	{
		super.OnShow();

		SetFocus(layoutRoot);
		GetGame().GetInput().ChangeGameFocus(1);
		GetGame().GetUIManager().ShowUICursor(true);
	}

	override void OnHide()
	{
		super.OnHide();

		GetGame().GetInput().ResetGameFocus();
		GetGame().GetUIManager().ShowUICursor(false);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_CloseButton)
		{
			CloseMenu();
			return true;
		}

		for (int i = 0; i < m_Actions.Count(); i++)
		{
			ExpansionConfigReloadMenuAction action = m_Actions.Get(i);
			if (action && w == action.GetButton())
			{
				SendAction(action);
				return true;
			}
		}

		return false;
	}

	protected string Localize(string stringId)
	{
		if (!stringId || stringId == string.Empty)
		{
			return string.Empty;
		}

		if (stringId[0] == "#")
		{
			return Widget.TranslateString(stringId);
		}

		return stringId;
	}

	protected void ApplyLocalizedTexts()
	{
		TextWidget title = TextWidget.Cast(m_Root.FindAnyWidget("Title"));
		if (title)
		{
			title.SetText(Localize("#STR_ECR_Menu_Title"));
		}

		MultilineTextWidget description = MultilineTextWidget.Cast(m_Root.FindAnyWidget("Description"));
		if (description)
		{
			description.SetText(Localize("#STR_ECR_Menu_Description"));
		}

		TextWidget sectionTitle = TextWidget.Cast(m_Root.FindAnyWidget("SectionTitle"));
		if (sectionTitle)
		{
			sectionTitle.SetText(Localize("#STR_ECR_Section_Actions"));
		}
	}

	protected void RegisterButtons()
	{
		if (ExpansionConfigReloadFeatures.HasMarket())
		{
			AddReloadAction("#STR_ECR_Button_Market", ExpansionConfigReloadActionType.MARKET);
		}

		if (ExpansionConfigReloadFeatures.HasQuests())
		{
			AddReloadAction("#STR_ECR_Button_Quests", ExpansionConfigReloadActionType.QUESTS);
		}

		if (ExpansionConfigReloadFeatures.HasAirdrop())
		{
			AddReloadAction("#STR_ECR_Button_Airdrop", ExpansionConfigReloadActionType.AIRDROP);
		}

		if (ExpansionConfigReloadFeatures.HasLoadouts())
		{
			AddReloadAction("#STR_ECR_Button_Loadouts", ExpansionConfigReloadActionType.LOADOUTS);
		}
	}

	protected void AddReloadAction(string displayName, ExpansionConfigReloadActionType actionType)
	{
		if (!m_ButtonGrid || displayName == string.Empty)
		{
			return;
		}

		Widget rowWidget = GetGame().GetWorkspace().CreateWidgets(ExpansionConfigReloadConstants.BUTTON_LAYOUT_PATH, m_ButtonGrid);
		if (!rowWidget)
		{
			return;
		}

		ButtonWidget buttonWidget = ButtonWidget.Cast(rowWidget.FindAnyWidget("ReloadActionButton"));
		if (!buttonWidget)
		{
			rowWidget.Unlink();
			return;
		}

		buttonWidget.SetText(Localize(displayName));
		m_Actions.Insert(new ExpansionConfigReloadMenuAction(buttonWidget, actionType));
	}

	protected void SendAction(ExpansionConfigReloadMenuAction action)
	{
		if (!action)
		{
			return;
		}

		ExpansionConfigReloadModule module = ExpansionConfigReloadModule.GetInstance();
		if (!module)
		{
			return;
		}

		switch (action.GetActionType())
		{
			#ifdef EXPANSIONMODMARKET
			case ExpansionConfigReloadActionType.MARKET:
				module.RequestReloadMarket();
				break;
			#endif
			#ifdef EXPANSIONMODQUESTS
			case ExpansionConfigReloadActionType.QUESTS:
				module.RequestReloadQuests();
				break;
			#endif
			#ifdef EXPANSIONMODMISSIONS
			case ExpansionConfigReloadActionType.AIRDROP:
				module.RequestReloadAirdrop();
				break;
			#endif
			case ExpansionConfigReloadActionType.LOADOUTS:
				module.RequestReloadLoadouts();
				break;
		}
	}

	void CloseMenu()
	{
		GetGame().GetUIManager().HideScriptedMenu(this);
	}
}

class ExpansionConfigReloadMenuAction
{
	protected ButtonWidget m_Button;
	protected ExpansionConfigReloadActionType m_ActionType;

	void ExpansionConfigReloadMenuAction(ButtonWidget button, ExpansionConfigReloadActionType actionType)
	{
		m_Button = button;
		m_ActionType = actionType;
	}

	ButtonWidget GetButton()
	{
		return m_Button;
	}

	ExpansionConfigReloadActionType GetActionType()
	{
		return m_ActionType;
	}
}
#endif
