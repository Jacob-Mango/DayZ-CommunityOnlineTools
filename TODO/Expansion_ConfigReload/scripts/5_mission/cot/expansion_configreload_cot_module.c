#ifdef JM_COT
class ExpansionConfigReloadCOTModule: JMRenderableModuleBase
{
	void ExpansionConfigReloadCOTModule()
	{
		ExpansionConfigReloadPermissions.RegisterPermissions();
	}

	override void OnInit()
	{
		super.OnInit();

		Expansion_EnableRPCManager();

		#ifdef EXPANSIONMODMARKET
		Expansion_RegisterServerRPC("RPC_ReloadMarket");
		#endif
		#ifdef EXPANSIONMODQUESTS
		Expansion_RegisterServerRPC("RPC_ReloadQuests");
		#endif
		#ifdef EXPANSIONMODMISSIONS
		Expansion_RegisterServerRPC("RPC_ReloadAirdrop");
		#endif
		Expansion_RegisterServerRPC("RPC_ReloadLoadouts");
		Expansion_RegisterClientRPC("RPC_ReloadResult");
	}

	override void EnableUpdate()
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission(ExpansionConfigReloadPermissions.PERMISSION_ROOT);
	}

	override bool HasButton()
	{
		return ExpansionConfigReloadFeatures.GetAvailableActionCount() > 0;
	}

	override string GetLayoutRoot()
	{
		return ExpansionConfigReloadConstants.COT_LAYOUT_PATH;
	}

	override string GetTitle()
	{
		return "STR_ECR_COT_Module_Name";
	}

	override string GetIconName()
	{
		return "R";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	#ifdef EXPANSIONMODMARKET
	void RequestReloadMarket()
	{
		if (GetGame().IsServer())
		{
			return;
		}

		auto rpc = Expansion_CreateRPC("RPC_ReloadMarket");
		rpc.Expansion_Send(true);
	}
	#endif

	#ifdef EXPANSIONMODQUESTS
	void RequestReloadQuests()
	{
		if (GetGame().IsServer())
		{
			return;
		}

		auto rpc = Expansion_CreateRPC("RPC_ReloadQuests");
		rpc.Expansion_Send(true);
	}
	#endif

	#ifdef EXPANSIONMODMISSIONS
	void RequestReloadAirdrop()
	{
		if (GetGame().IsServer())
		{
			return;
		}

		auto rpc = Expansion_CreateRPC("RPC_ReloadAirdrop");
		rpc.Expansion_Send(true);
	}
	#endif

	void RequestReloadLoadouts()
	{
		if (GetGame().IsServer())
		{
			return;
		}

		auto rpc = Expansion_CreateRPC("RPC_ReloadLoadouts");
		rpc.Expansion_Send(true);
	}

#ifdef SERVER
	#ifdef EXPANSIONMODMARKET
	protected void RPC_ReloadMarket(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		if (!sender)
		{
			return;
		}

		string message;
		bool success = ExpansionConfigReloadReloadHandler.ReloadMarket(sender, message);
		ExpansionConfigReloadReloadHandler.SendResult(this, sender, success, message);
	}
	#endif

	#ifdef EXPANSIONMODQUESTS
	protected void RPC_ReloadQuests(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		if (!sender)
		{
			return;
		}

		string message;
		bool success = ExpansionConfigReloadReloadHandler.ReloadQuests(sender, message);
		ExpansionConfigReloadReloadHandler.SendResult(this, sender, success, message);
	}
	#endif

	protected void RPC_ReloadLoadouts(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		if (!sender)
		{
			return;
		}

		string message;
		bool success = ExpansionConfigReloadReloadHandler.ReloadLoadouts(sender, message);
		ExpansionConfigReloadReloadHandler.SendResult(this, sender, success, message);
	}

	#ifdef EXPANSIONMODMISSIONS
	protected void RPC_ReloadAirdrop(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		if (!sender)
		{
			return;
		}

		string message;
		bool success = ExpansionConfigReloadReloadHandler.ReloadAirdrop(sender, message);
		ExpansionConfigReloadReloadHandler.SendResult(this, sender, success, message);
	}
	#endif
#endif

	protected void RPC_ReloadResult(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		if (GetGame().IsServer())
		{
			return;
		}

		bool success;
		string message;
		if (!ctx.Read(success) || !ctx.Read(message))
		{
			return;
		}

		ExpansionConfigReloadReloadHandler.ShowNotification(success, message);
	}
}
#endif
