#ifndef JM_COT
modded class MissionBase
{
	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		UIScriptedMenu menu;

		if (id == ExpansionConfigReloadConstants.MENU_ID)
		{
			menu = new ExpansionConfigReloadMenu;
		}

		if (menu)
		{
			menu.SetID(id);
			return menu;
		}

		return super.CreateScriptedMenu(id);
	}
}
#endif
