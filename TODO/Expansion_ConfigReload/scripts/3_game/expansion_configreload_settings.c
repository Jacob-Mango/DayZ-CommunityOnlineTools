#ifdef EXPANSIONMODMARKET
modded class ExpansionSettings
{
	override void RPC_MarketSettings(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		if (!g_Game.IsDedicatedServer())
		{
			ExpansionMarketSettings market = GetMarket(false);
			if (market)
			{
				market.ClearMarketCaches();
			}
		}

		super.RPC_MarketSettings(sender, target, ctx);
	}

	bool ConfigReload_MarketFromDisk()
	{
		if (!GetGame() || !GetGame().IsServer())
		{
			return false;
		}

		ExpansionMarketSettings market = GetMarket(false);
		if (market)
		{
			market.NetworkCategories.Clear();
			market.ClearMarketCaches();
			market.Unload();
			m_Settings.Remove(ExpansionMarketSettings);
			m_SettingsOrdered.RemoveItem(market);
		}

		Init(ExpansionMarketSettings, true);
		market = GetMarket(false);
		if (!market)
		{
			return false;
		}

		return market.Load();
	}
}
#endif
