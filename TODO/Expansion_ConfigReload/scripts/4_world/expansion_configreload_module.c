[CF_RegisterModule(ExpansionConfigReloadModule)]
class ExpansionConfigReloadModule: CF_ModuleWorld
{
	protected static ExpansionConfigReloadModule s_Instance;

	protected static bool s_ClientIsAdmin;

	void ExpansionConfigReloadModule()
	{
		s_Instance = this;
	}

	static ExpansionConfigReloadModule GetInstance()
	{
		return s_Instance;
	}

	static bool IsClientAdmin()
	{
		return s_ClientIsAdmin;
	}

	static void SyncOfflineAdminFlag()
	{
		if (GetGame().IsMultiplayer())
		{
			return;
		}

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player || !player.GetIdentity())
		{
			s_ClientIsAdmin = false;
			return;
		}

		s_ClientIsAdmin = ExpansionConfigReloadAdmin.IsAdmin(player.GetIdentity().GetPlainId());
	}

	override void OnInit()
	{
		super.OnInit();

		#ifndef JM_COT
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
		Expansion_RegisterClientRPC("RPC_SetAdminAccess");
		Expansion_RegisterClientRPC("RPC_ReloadResult");
		#endif

		#ifndef JM_COT
		EnableInvokeConnect();
		#endif

		#ifndef JM_COT
		if (GetGame().IsServer())
		{
			ExpansionConfigReloadAdmin.EnsureInitialized();
		}
		#endif
	}

	#ifndef JM_COT
	override void OnInvokeConnect(Class sender, CF_EventArgs args)
	{
		super.OnInvokeConnect(sender, args);

	#ifdef SERVER
		if (!GetGame().IsServer())
		{
			return;
		}

		auto cArgs = CF_EventPlayerArgs.Cast(args);
		if (!cArgs || !cArgs.Identity)
		{
			return;
		}

		ExpansionConfigReloadAdmin.EnsureInitialized();
		bool isAdmin = ExpansionConfigReloadAdmin.IsAdmin(cArgs.Identity.GetPlainId());
		auto rpc = Expansion_CreateRPC("RPC_SetAdminAccess");
		rpc.Write(isAdmin);
		rpc.Expansion_Send(true, cArgs.Identity);
	#endif
	}
	#endif

	override void OnMissionStart(Class sender, CF_EventArgs args)
	{
		super.OnMissionStart(sender, args);

		#ifndef JM_COT
		if (GetGame().IsServer())
		{
			ExpansionConfigReloadAdmin.EnsureInitialized();
		}

		if (!GetGame().IsMultiplayer())
		{
			SyncOfflineAdminFlag();
		}
		#endif
	}

	#ifndef JM_COT
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
	#endif

	#ifndef JM_COT
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

	protected void RPC_SetAdminAccess(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		if (GetGame().IsServer())
		{
			return;
		}

		bool isAdmin;
		if (!ctx.Read(isAdmin))
		{
			s_ClientIsAdmin = false;
			return;
		}

		s_ClientIsAdmin = isAdmin;
	}

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
	#endif
}

