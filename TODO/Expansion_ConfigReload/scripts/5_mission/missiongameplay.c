#ifndef JM_COT
modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		ExpansionConfigReloadAdmin.EnsureInitialized();
	}
}

modded class MissionGameplay
{
	protected bool m_Expansion_ConfigReloadMenuPressed;

	override void Expansion_OnUpdate(float timeslice, PlayerBase player, bool isAliveConscious, Input input, bool inputIsFocused, UIScriptedMenu menu, ExpansionScriptViewMenuBase viewMenu)
	{
		super.Expansion_OnUpdate(timeslice, player, isAliveConscious, input, inputIsFocused, menu, viewMenu);

		if (!isAliveConscious || inputIsFocused || menu)
		{
			return;
		}

		if (!ExpansionConfigReloadModule.IsClientAdmin())
		{
			if (!GetGame().IsMultiplayer())
			{
				ExpansionConfigReloadModule.SyncOfflineAdminFlag();
			}

			if (!ExpansionConfigReloadModule.IsClientAdmin())
			{
				return;
			}
		}

		if (input.LocalPress("UAExpansionConfigReloadMenu", false) && !m_Expansion_ConfigReloadMenuPressed)
		{
			m_Expansion_ConfigReloadMenuPressed = true;
			ToggleConfigReloadMenu();
		}
		else if (input.LocalRelease("UAExpansionConfigReloadMenu", false) || input.LocalValue("UAExpansionConfigReloadMenu", false) == 0)
		{
			m_Expansion_ConfigReloadMenuPressed = false;
		}
	}

	protected void ToggleConfigReloadMenu()
	{
		UIScriptedMenu currentMenu = GetGame().GetUIManager().GetMenu();
		ExpansionConfigReloadMenu reloadMenu = ExpansionConfigReloadMenu.Cast(currentMenu);

		if (reloadMenu)
		{
			reloadMenu.CloseMenu();
			return;
		}

		if (currentMenu)
		{
			return;
		}

		GetGame().GetUIManager().EnterScriptedMenu(ExpansionConfigReloadConstants.MENU_ID, null);
	}
}
#endif
