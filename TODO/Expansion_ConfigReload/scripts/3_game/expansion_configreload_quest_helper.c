#ifdef EXPANSIONMODQUESTS
#ifdef SERVER
class ExpansionConfigReloadQuestHelper
{
	static bool IsQuestSystemEnabled()
	{
		if (!GetExpansionSettings().GetQuest(false))
		{
			return false;
		}

		return GetExpansionSettings().GetQuest(false).EnableQuests;
	}

	static bool ReloadSettingsFromDisk()
	{
		if (!GetExpansionSettings().GetQuest(false))
		{
			return false;
		}

		GetExpansionSettings().GetQuest(false).Unload();
		return GetExpansionSettings().GetQuest(false).Load();
	}

	static void PushSettingsToAllPlayers()
	{
		if (!GetExpansionSettings().GetQuest(false))
		{
			return;
		}

		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		foreach (Man man : players)
		{
			PlayerIdentity identity = man.GetIdentity();
			if (identity)
			{
				GetExpansionSettings().GetQuest(false).Send(identity);
			}
		}
	}
}
#endif
#endif
