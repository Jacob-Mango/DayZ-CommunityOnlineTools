class ExpansionConfigReloadPermissions
{
	static const string PERMISSION_ROOT = "Expansion.ConfigReload";

	#ifdef JM_COT
	static void RegisterPermissions()
	{
		GetPermissionsManager().RegisterPermission(PERMISSION_ROOT);

		#ifdef EXPANSIONMODMARKET
		GetPermissionsManager().RegisterPermission(PERMISSION_ROOT + ".Market");
		#endif
		#ifdef EXPANSIONMODQUESTS
		GetPermissionsManager().RegisterPermission(PERMISSION_ROOT + ".Quests");
		#endif
		#ifdef EXPANSIONMODMISSIONS
		GetPermissionsManager().RegisterPermission(PERMISSION_ROOT + ".Airdrop");
		#endif
		GetPermissionsManager().RegisterPermission(PERMISSION_ROOT + ".Loadouts");
	}
	#endif

	static bool HasAccess(PlayerIdentity identity = null)
	{
		#ifdef JM_COT
		return GetPermissionsManager().HasPermission(PERMISSION_ROOT, identity);
		#else
		if (identity)
		{
			return ExpansionConfigReloadAdmin.IsAdmin(identity.GetPlainId());
		}

		return ExpansionConfigReloadModule.IsClientAdmin();
		#endif
	}

	static bool CanReloadMarket(PlayerIdentity identity)
	{
		if (!identity)
		{
			return false;
		}

		#ifndef EXPANSIONMODMARKET
		return false;
		#endif

		#ifdef JM_COT
		if (GetPermissionsManager().HasPermission(PERMISSION_ROOT + ".Market", identity))
		{
			return true;
		}

		return GetPermissionsManager().HasPermission(PERMISSION_ROOT, identity);
		#else
		return ExpansionConfigReloadAdmin.IsAdmin(identity.GetPlainId());
		#endif
	}

	static bool CanReloadQuests(PlayerIdentity identity)
	{
		if (!identity)
		{
			return false;
		}

		#ifndef EXPANSIONMODQUESTS
		return false;
		#endif

		#ifdef JM_COT
		if (GetPermissionsManager().HasPermission(PERMISSION_ROOT + ".Quests", identity))
		{
			return true;
		}

		return GetPermissionsManager().HasPermission(PERMISSION_ROOT, identity);
		#else
		return ExpansionConfigReloadAdmin.IsAdmin(identity.GetPlainId());
		#endif
	}

	static bool CanReloadAirdrop(PlayerIdentity identity)
	{
		if (!identity)
		{
			return false;
		}

		#ifndef EXPANSIONMODMISSIONS
		return false;
		#endif

		#ifdef JM_COT
		if (GetPermissionsManager().HasPermission(PERMISSION_ROOT + ".Airdrop", identity))
		{
			return true;
		}

		return GetPermissionsManager().HasPermission(PERMISSION_ROOT, identity);
		#else
		return ExpansionConfigReloadAdmin.IsAdmin(identity.GetPlainId());
		#endif
	}

	static bool CanReloadLoadouts(PlayerIdentity identity)
	{
		if (!identity)
		{
			return false;
		}

		#ifdef JM_COT
		if (GetPermissionsManager().HasPermission(PERMISSION_ROOT + ".Loadouts", identity))
		{
			return true;
		}

		return GetPermissionsManager().HasPermission(PERMISSION_ROOT, identity);
		#else
		return ExpansionConfigReloadAdmin.IsAdmin(identity.GetPlainId());
		#endif
	}
}
