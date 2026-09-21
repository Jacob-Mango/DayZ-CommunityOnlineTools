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

	override void DescribeModule( JMModuleInfo info )
	{
		super.DescribeModule( info );

		info.Title = "Territories";
		info.Icon = "land-plot";
		info.Layout = "JM/COT/GUI/layouts/territory_form.layout";
		info.Category = JMSideBarConfig.CATEGORY_EXPANSION;
		info.ViewPermission = JMConstants.PERM_EXPANSION_TERRITORY_VIEW;
		info.WebhookTitle = "Territory Module";
		info.SetRPCRange( JMTerritoryModuleRPC.INVALID, JMTerritoryModuleRPC.COUNT );
		info.AddPermission( JMConstants.PERM_EXPANSION_TERRITORY );
	}

	//! The set-level permission and its webhook type come from the action.
	override void RegisterActions()
	{
		super.RegisterActions();

		DefineAction( JMTerritorySetLevel );
	}

	override void EnableUpdate()
	{
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if (g_Game.IsServer())
			return;

		if (JMPermissions.Has(JMConstants.PERM_EXPANSION_TERRITORY))
			RequestTerritories();
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		if (JMPermissions.Has(JMConstants.PERM_EXPANSION_TERRITORY))
			RequestTerritories();
	}

	override void RequestData()
	{
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
				if (!sender) return;
				if (!JMPermissions.HasRPC(JMConstants.PERM_EXPANSION_TERRITORY, sender, instance))
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

			default:
			{
				RunAction(sender, rpc_type, ctx);
				return;
			}
		}
	}

	protected void SendTerritoriesToClient(PlayerIdentity recipient)
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
		JMTerritorySetLevel action = new JMTerritorySetLevel();
		action.TerritoryID = territoryID;
		action.NewLevel = newLevel;
		SubmitAction(action);
	}
}
#endif
