class ExpansionConfigReloadFeatures
{
	static bool HasMarket()
	{
		#ifdef EXPANSIONMODMARKET
		return true;
		#else
		return false;
		#endif
	}

	static bool HasQuests()
	{
		#ifdef EXPANSIONMODQUESTS
		return true;
		#else
		return false;
		#endif
	}

	static bool HasAirdrop()
	{
		#ifdef EXPANSIONMODMISSIONS
		return true;
		#else
		return false;
		#endif
	}

	static bool HasLoadouts()
	{
		return true;
	}

	static int GetAvailableActionCount()
	{
		int count = 0;

		if (HasMarket())
		{
			count++;
		}

		if (HasQuests())
		{
			count++;
		}

		if (HasAirdrop())
		{
			count++;
		}

		if (HasLoadouts())
		{
			count++;
		}

		return count;
	}
}
