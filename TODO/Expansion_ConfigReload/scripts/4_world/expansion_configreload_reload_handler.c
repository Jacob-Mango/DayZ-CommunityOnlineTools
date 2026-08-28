class ExpansionConfigReloadReloadHandler
{
#ifdef SERVER
	#ifdef EXPANSIONMODMARKET
	static bool ReloadMarket(PlayerIdentity sender, out string message)
	{
		if (!ExpansionConfigReloadPermissions.CanReloadMarket(sender))
		{
			message = "STR_ECR_Result_Denied_Market";
			return false;
		}

		bool success = ExpansionConfigReloadMarketWorld.ReloadAll("cot", sender);
		if (success)
		{
			message = "STR_ECR_Result_Market_OK";
		}
		else
		{
			message = "STR_ECR_Result_Market_Fail";
		}

		return success;
	}
	#endif

	#ifdef EXPANSIONMODQUESTS
	static bool ReloadQuests(PlayerIdentity sender, out string message)
	{
		if (!ExpansionConfigReloadPermissions.CanReloadQuests(sender))
		{
			message = "STR_ECR_Result_Denied_Quests";
			return false;
		}

		bool success = ExpansionConfigReloadQuestService.ReloadAll("cot", sender);
		if (success)
		{
			message = "STR_ECR_Result_Quests_OK";
		}
		else
		{
			message = "STR_ECR_Result_Quests_Fail";
		}

		return success;
	}
	#endif

	#ifdef EXPANSIONMODMISSIONS
	static bool ReloadAirdrop(PlayerIdentity sender, out string message)
	{
		if (!ExpansionConfigReloadPermissions.CanReloadAirdrop(sender))
		{
			message = "STR_ECR_Result_Denied_Airdrop";
			return false;
		}

		bool success = GetExpansionSettings().ConfigReload_AirdropSettingsFromDisk();
		if (success)
		{
			message = "STR_ECR_Result_Airdrop_OK";
		}
		else
		{
			message = "STR_ECR_Result_Airdrop_Fail";
		}

		return success;
	}
	#endif

	static bool ReloadLoadouts(PlayerIdentity sender, out string message)
	{
		if (!ExpansionConfigReloadPermissions.CanReloadLoadouts(sender))
		{
			message = "STR_ECR_Result_Denied_Loadouts";
			return false;
		}

		bool success = ExpansionConfigReloadLoadoutService.ReloadAll("cot", sender);
		if (success)
		{
			message = "STR_ECR_Result_Loadouts_OK";
		}
		else
		{
			message = "STR_ECR_Result_Loadouts_Fail";
		}

		return success;
	}
#endif

	static void SendResult(CF_ModuleGame module, PlayerIdentity identity, bool success, string message)
	{
		if (!module || !identity)
		{
			return;
		}

		auto rpc = module.Expansion_CreateRPC("RPC_ReloadResult");
		rpc.Write(success);
		rpc.Write(message);
		rpc.Expansion_Send(true, identity);
	}

	static void ShowNotification(bool success, string message)
	{
		string title = Widget.TranslateString("#STR_ECR_Notif_Title");
		string body = LocalizeMessage(message);

		if (success)
		{
			ExpansionNotification(title, body, "set:expansion_notification_iconset image:icon_info", COLOR_EXPANSION_NOTIFICATION_SUCCESS, 7).Create();
		}
		else
		{
			ExpansionNotification(title, body, "Error", COLOR_EXPANSION_NOTIFICATION_ERROR, 7).Create();
		}
	}

	static string LocalizeMessage(string message)
	{
		if (!message || message == string.Empty)
		{
			return string.Empty;
		}

		if (message[0] == "#")
		{
			return Widget.TranslateString(message);
		}

		return Widget.TranslateString("#" + message);
	}
}
