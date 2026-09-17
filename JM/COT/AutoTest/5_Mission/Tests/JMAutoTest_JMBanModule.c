#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMBanModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/BanManager/JMBanModule.c).
//!
//! Calls OnRPC(sender=NULL, target=NULL, rpc_type, emptyCtx) once for every
//! rpc_type the module's OnRPC switch actually handles. sender=NULL is the
//! identity ceiling for this boot-time test - there is no live PlayerIdentity
//! to attach at this point in the boot sequence, so every handler that checks
//! permissions is exercised at the "no sender" edge rather than as a real
//! admin or a real player. This asserts CURRENT behavior as a regression
//! baseline, not a claim that the handlers are correct.
//!
//! JMBanModuleRPC is defined in JMBanModule.c itself (not RPC.c), starting at
//! INVALID = 10800: RequestBanList, RequestBan, UnbanPlayer and
//! EditBanDuration are client -> server, BanList is server -> client. All
//! five have a real case in the OnRPC switch (RPC_RequestBanList,
//! RPC_RequestBan, RPC_UnbanPlayer, RPC_EditBanDuration, RPC_BanList), so all
//! five are swept - BanList is included because the switch genuinely routes
//! to a handler for it, even though nothing but the server itself is meant to
//! send it.

static void JMAutoTest_JMBanModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMBanModule full RPC sweep");
	PrintFormat("================================================================");

	JMBanModule mod;
	if (!CF_Modules<JMBanModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMBanModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <id>, emptyCtx) survives for every handled rpc_type");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMBanModuleRPC.RequestBanList, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMBanModuleRPC.RequestBanList");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMBanModuleRPC.RequestBan, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMBanModuleRPC.RequestBan");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMBanModuleRPC.UnbanPlayer, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMBanModuleRPC.UnbanPlayer");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMBanModuleRPC.EditBanDuration, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMBanModuleRPC.EditBanDuration");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMBanModuleRPC.BanList, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMBanModuleRPC.BanList");

	JMAutoTest_CanaryCheck("JMBanModule full RPC sweep");
}

#endif
