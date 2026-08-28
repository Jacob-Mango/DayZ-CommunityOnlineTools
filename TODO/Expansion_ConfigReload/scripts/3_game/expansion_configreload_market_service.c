#ifdef EXPANSIONMODMARKET
#ifdef SERVER
class ExpansionConfigReloadMarketService
{
	static void PushConfigToPlayers()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		foreach (Man man : players)
		{
			PlayerIdentity identity = man.GetIdentity();
			if (!identity)
			{
				continue;
			}

			if (GetExpansionSettings().GetMarket(false))
			{
				GetExpansionSettings().GetMarket(false).Send(identity);
			}
		}
	}
}
#endif
#endif
