#ifdef EXPANSIONMODMISSIONS
modded class ExpansionSettings
{
	bool ConfigReload_AirdropSettingsFromDisk()
	{
		if (!GetGame() || !GetGame().IsServer())
		{
			return false;
		}

		ExpansionSettingBase existing;
		if (!m_Settings.Find(ExpansionAirdropSettings, existing))
		{
			Print("[Expansion_ConfigReload] Airdrop settings not initialized (Expansion Missions required).");
			return false;
		}

		bool registerClientRpc = m_RPCIDs.Contains(ExpansionAirdropSettings);

		if (existing)
		{
			existing.Unload();
			m_Settings.Remove(ExpansionAirdropSettings);
			m_SettingsOrdered.RemoveItem(existing);
		}

		Init(ExpansionAirdropSettings, registerClientRpc);

		ExpansionAirdropSettings airdrop = GetAirdrop(false);
		if (!airdrop || !airdrop.Load())
		{
			Print("[Expansion_ConfigReload] Failed to reload: " + EXPANSION_AIRDROP_SETTINGS);
			return false;
		}

		if (registerClientRpc)
		{
			array<Man> players = new array<Man>;
			GetGame().GetPlayers(players);

			foreach (Man man : players)
			{
				PlayerIdentity identity = man.GetIdentity();
				if (identity)
				{
					airdrop.Send(identity);
				}
			}
		}

		Print("[Expansion_ConfigReload] Reloaded " + EXPANSION_AIRDROP_SETTINGS);
		return true;
	}
}
#endif
