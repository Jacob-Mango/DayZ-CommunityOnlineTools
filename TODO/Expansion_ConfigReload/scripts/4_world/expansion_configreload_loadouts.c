#ifdef SERVER
class ExpansionConfigReloadLoadoutService
{
	static bool ReloadAll(string reason = "manual", PlayerIdentity admin = null)
	{
		if (!GetGame() || !GetGame().IsServer())
		{
			return false;
		}

		if (!FileExist(EXPANSION_LOADOUT_FOLDER))
		{
			Print("[Expansion_ConfigReload] Loadout folder not found: " + EXPANSION_LOADOUT_FOLDER);
			return false;
		}

		TStringArray files = ExpansionStatic.FindFilesInLocation(EXPANSION_LOADOUT_FOLDER, ".json");
		int reloaded = 0;

		foreach (string fileName : files)
		{
			if (!fileName || fileName == string.Empty)
			{
				continue;
			}

			string loadoutName = fileName;
			int extIndex = loadoutName.IndexOf(".json");
			if (extIndex > 0)
			{
				loadoutName = loadoutName.Substring(0, extIndex);
			}

			if (ExpansionLoadout.Load(loadoutName, true))
			{
				reloaded++;
			}
			else
			{
				Print("[Expansion_ConfigReload] Failed to reload loadout: " + loadoutName);
			}
		}

		string adminText = "";
		if (admin)
		{
			adminText = " by " + admin.GetName() + " (" + admin.GetPlainId() + ")";
		}

		Print("[Expansion_ConfigReload] Loadouts reloaded: " + reloaded + "/" + files.Count() + " (" + reason + ")" + adminText);
		Print("[Expansion_ConfigReload] Path: ExpansionMod/Loadouts");
		return reloaded > 0 || files.Count() == 0;
	}
}
#endif
