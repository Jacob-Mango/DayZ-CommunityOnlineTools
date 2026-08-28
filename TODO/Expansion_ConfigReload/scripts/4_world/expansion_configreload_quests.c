#ifdef EXPANSIONMODQUESTS
#ifdef SERVER
class ExpansionConfigReloadQuestService
{
	static bool ReloadAll(string reason = "manual", PlayerIdentity admin = null)
	{
		if (!GetGame() || !GetGame().IsServer())
		{
			return false;
		}

		ExpansionQuestModule questModule = ExpansionQuestModule.GetModuleInstance();
		if (!questModule)
		{
			Print("[Expansion_ConfigReload] Quest reload failed: ExpansionQuestModule is not available");
			return false;
		}

		if (!ExpansionConfigReloadQuestHelper.IsQuestSystemEnabled())
		{
			Print("[Expansion_ConfigReload] Quest reload skipped: quest system is disabled in ExpansionQuestSettings");
			return false;
		}

		if (!questModule.ConfigReload_ReloadQuestConfigsFromDisk())
		{
			Print("[Expansion_ConfigReload] Quest reload failed while reading configs from disk");
			return false;
		}

		string adminText = "";
		if (admin)
		{
			adminText = " by " + admin.GetName() + " (" + admin.GetPlainId() + ")";
		}

		Print("[Expansion_ConfigReload] Expansion Quest configs reloaded: " + reason + adminText);
		Print("[Expansion_ConfigReload] Quest paths: ExpansionMod/Quests (Quests, Objectives, NPCs)");
		Print("[Expansion_ConfigReload] Active player quests were not reset; players may need to relog if quest definitions changed drastically.");
		return true;
	}
}

modded class ExpansionQuestModule
{
	bool ConfigReload_ReloadQuestConfigsFromDisk()
	{
		if (!GetGame().IsServer())
		{
			return false;
		}

		ConfigReload_DespawnAllQuestNPCs();
		ConfigReload_ClearQuestConfigCaches();

		ServerModuleInit();
		SpawnQuestNPCs();

		if (!ExpansionConfigReloadQuestHelper.ReloadSettingsFromDisk())
		{
			return false;
		}

		ExpansionConfigReloadQuestHelper.PushSettingsToAllPlayers();
		ConfigReload_PushQuestConfigsToPlayers();
		return true;
	}

	protected void ConfigReload_DespawnAllQuestNPCs()
	{
		array<int> npcIDs = s_QuestNPCEntities.GetKeyArray();
		foreach (int npcID : npcIDs)
		{
			RemoveQuestNPC(npcID);
		}

		array<int> objectIDs = s_QuestObjectEntities.GetKeyArray();
		foreach (int objectID : objectIDs)
		{
			RemoveStaticQuestObject(objectID);
		}

	#ifdef EXPANSIONMODAI
		array<int> aiIDs = s_QuestNPCAIEntities.GetKeyArray();
		foreach (int aiID : aiIDs)
		{
			RemoveQuestNPCAI(aiID);
		}
	#endif
	}

	protected void ConfigReload_ClearQuestConfigCaches()
	{
		m_QuestConfigs.Clear();
		m_QuestsNPCs.Clear();
		m_QuestObjectSets.Clear();

		m_TravelObjectivesConfigs.Clear();
		m_DeliveryObjectivesConfigs.Clear();
		m_TargetObjectivesConfigs.Clear();
		m_CollectionObjectivesConfigs.Clear();
		m_TreasureHuntObjectivesConfigs.Clear();
		m_ActionObjectivesConfigs.Clear();
		m_CraftingObjectivesConfigs.Clear();

	#ifdef EXPANSIONMODAI
		m_AIPatrolObjectivesConfigs.Clear();
		m_AICampObjectivesConfigs.Clear();
		m_AIEscortObjectivesConfigs.Clear();
	#endif
	}

	protected void ConfigReload_PushQuestConfigsToPlayers()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (player && player.GetIdentity())
			{
				SendClientQuestConfigs(player.GetIdentity());
			}
		}
	}

}
#endif
#endif
