#ifdef EXPANSIONMODMARKET
#ifdef SERVER
class ExpansionConfigReloadMarketWorld
{
	static bool ReloadAll(string reason = "manual", PlayerIdentity admin = null)
	{
		if (!GetGame() || !GetGame().IsServer())
		{
			return false;
		}

		array<ref Param2<ExpansionTraderObjectBase, string>> traders = CollectTraderFileNames();

		if (!GetExpansionSettings().ConfigReload_MarketFromDisk())
		{
			Print("[Expansion_ConfigReload] Market reload failed while reading configs from disk");
			return false;
		}

		ExpansionMarketModule marketModule = ExpansionMarketModule.GetInstance();
		if (marketModule)
		{
			marketModule.LoadMoneyPrice();
		}

		RefreshTraderObjects(traders);
		ResyncTraderObjectsToClients();
		ExpansionConfigReloadMarketService.PushConfigToPlayers();

		string adminText = "";
		if (admin)
		{
			adminText = " by " + admin.GetName() + " (" + admin.GetPlainId() + ")";
		}

		Print("[Expansion_ConfigReload] Expansion Market configs reloaded: " + reason + adminText);
		Print("[Expansion_ConfigReload] Refreshed " + traders.Count() + " trader object(s) on server");
		Print("[Expansion_ConfigReload] Market paths: ExpansionMod/Market | ExpansionMod/Traders | ExpansionMod/traderzones");
		return true;
	}

	protected static array<ref Param2<ExpansionTraderObjectBase, string>> CollectTraderFileNames()
	{
		array<ref Param2<ExpansionTraderObjectBase, string>> traders = new array<ref Param2<ExpansionTraderObjectBase, string>>;

		foreach (ExpansionTraderObjectBase traderObj : ExpansionTraderObjectBase.GetAll())
		{
			if (!traderObj)
			{
				continue;
			}

			string fileName = "";
			ExpansionMarketTrader trader = traderObj.GetTraderMarket();
			if (trader)
			{
				fileName = trader.m_FileName;
			}

			traders.Insert(new Param2<ExpansionTraderObjectBase, string>(traderObj, fileName));
		}

		return traders;
	}

	protected static void RefreshTraderObjects(array<ref Param2<ExpansionTraderObjectBase, string>> traders)
	{
		foreach (Param2<ExpansionTraderObjectBase, string> entry : traders)
		{
			if (!entry || !entry.param1 || entry.param2 == string.Empty)
			{
				continue;
			}

			entry.param1.LoadTraderHost(entry.param2);
		}
	}

	protected static void ResyncTraderObjectsToClients()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		foreach (ExpansionTraderObjectBase traderObj : ExpansionTraderObjectBase.GetAll())
		{
			if (!traderObj || !traderObj.GetTraderMarket())
			{
				continue;
			}

			foreach (Man man : players)
			{
				PlayerBase player = PlayerBase.Cast(man);
				if (player && player.GetIdentity())
				{
					traderObj.RPC_TraderObject(player.GetIdentity(), null);
				}
			}
		}
	}
}
#endif
#endif
