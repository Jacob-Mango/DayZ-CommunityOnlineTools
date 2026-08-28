#ifdef EXPANSIONMODBASEBUILDING
class JMTerritoryData: UIActionData
{
	int TerritoryID;
	string TerritoryName;
	string OwnerID;
	int CurrentLevel;
	int MaxLevel;
}

class JMTerritoryModule: JMRenderableModuleBase
{
	ref array<ref JMTerritoryData> m_Territories = new array<ref JMTerritoryData>();

	void JMTerritoryModule()
	{
		GetPermissionsManager().RegisterPermission("Expansion.Territory");
		GetPermissionsManager().RegisterPermission("Expansion.Territory.View");
		GetPermissionsManager().RegisterPermission("Expansion.Territory.SetLevel");
	}

	override void EnableUpdate()
	{
	}

	override bool HasButton()
	{
		return true;
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission("Expansion.Territory.View");
	}

	override string GetCategory()
	{
		return "Expansion";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/territory_form.layout";
	}

	override string GetTitle()
	{
		return "Territories";
	}

	override string GetIconName()
	{
		return JMConstants.Lucide( "land-plot" );
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
	}

	override int GetRPCMin()
	{
		return JMTerritoryModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMTerritoryModuleRPC.COUNT;
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if (g_Game.IsServer())
			return;

		if (GetPermissionsManager().HasPermission("Expansion.Territory"))
			RequestTerritories();
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		if (GetPermissionsManager().HasPermission("Expansion.Territory"))
			RequestTerritories();
	}

	void RequestTerritories()
	{
		if (IsMissionClient() && !IsMissionOffline())
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(null, JMTerritoryModuleRPC.RequestTerritories, true, null);
		}
	}

	override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		JMPlayerInstance instance;

		switch (rpc_type)
		{
			case JMTerritoryModuleRPC.RequestTerritories:
			{
				if (!GetPermissionsManager().HasPermission("Expansion.Territory", sender, instance))
					return;

				SendTerritoriesToClient(sender);
				return;
			}

			case JMTerritoryModuleRPC.SendTerritories:
			{
				if (!g_Game.IsClient())
					return;

				int count;
				if (!ctx.Read(count))
					return;

				m_Territories.Clear();
				for (int i = 0; i < count; i++)
				{
					int id;
					int level;
					int maxLevel;
					string name;
					string owner;
					if (!ctx.Read(id)) return;
					if (!ctx.Read(name)) return;
					if (!ctx.Read(owner)) return;
					if (!ctx.Read(level)) return;
					if (!ctx.Read(maxLevel)) return;

					JMTerritoryData data = new JMTerritoryData();
					data.TerritoryID = id;
					data.TerritoryName = name;
					data.OwnerID = owner;
					data.CurrentLevel = level;
					data.MaxLevel = maxLevel;
					m_Territories.Insert(data);
				}

				OnSettingsUpdated();
				return;
			}

			case JMTerritoryModuleRPC.SetLevel:
			{
				if (!GetPermissionsManager().HasPermission("Expansion.Territory.SetLevel", sender, instance))
					return;

				int territoryID;
				int newLevel;
				if (!ctx.Read(territoryID)) return;
				if (!ctx.Read(newLevel)) return;

				GetCommunityOnlineToolsBase().Log(sender, "Set territory " + territoryID + " to level " + newLevel);

				ExpansionTerritoryModule territoryModule = ExpansionTerritoryModule.Cast(CF_ModuleCoreManager.Get(ExpansionTerritoryModule));
				if (territoryModule)
					territoryModule.Exec_AdminSetTerritoryLevel(territoryID, newLevel, sender);

				return;
			}
		}
	}

	private void SendTerritoriesToClient(PlayerIdentity recipient)
	{
		ExpansionTerritoryModule territoryModule = ExpansionTerritoryModule.Cast(CF_ModuleCoreManager.Get(ExpansionTerritoryModule));
		if (!territoryModule)
			return;

		array<ref ExpansionTerritory> territories = new array<ref ExpansionTerritory>;
		territoryModule.JM_COT_GetTerritories(territories);

		//! Expansion has no UpgradeLevels setting -- levels are hardcoded
		//! constants. See JMTerritoryCompat.c.
		int maxLevel = territoryModule.JM_COT_GetMaxTerritoryLevel();

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(territories.Count());
		foreach (ExpansionTerritory territory: territories)
		{
			rpc.Write(territory.GetTerritoryID());
			rpc.Write(territory.GetTerritoryName());
			rpc.Write(territory.GetOwnerID());
			rpc.Write(territory.GetTerritoryLevel());
			rpc.Write(maxLevel);
		}
		rpc.Send(null, JMTerritoryModuleRPC.SendTerritories, true, recipient);
	}

	void SendSetLevel(int territoryID, int newLevel)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(territoryID);
		rpc.Write(newLevel);
		rpc.Send(null, JMTerritoryModuleRPC.SetLevel, true, null);
	}
}
#endif
