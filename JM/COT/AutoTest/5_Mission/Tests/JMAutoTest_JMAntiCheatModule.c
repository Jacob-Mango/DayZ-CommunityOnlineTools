#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMAntiCheatModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/AntiCheat/JMAntiCheatModule.c).
//!
//! Calls OnRPC(sender=NULL, target=NULL, rpc_type, emptyCtx) once for every
//! rpc_type the module's OnRPC switch actually handles. sender=NULL is the
//! identity ceiling for this boot-time test - there is no live PlayerIdentity
//! to attach at this point in the boot sequence, so every handler that checks
//! permissions is exercised at the "no sender" edge rather than as a real
//! admin or a real player. This asserts CURRENT behavior as a regression
//! baseline (Phase 0 safety probe already proved this call shape survives for
//! this exact module), not a claim that the handlers are correct.
//!
//! JMAntiCheatModuleRPC (RPC.c) has three members: RequestFlags and ClearFlag
//! are client -> server, Flags is server -> client. All three have a real
//! case in the OnRPC switch (RPC_RequestFlags, RPC_ClearFlag, RPC_Flags), so
//! all three are swept - Flags is included because the switch genuinely
//! routes to a handler for it, even though nothing but the server itself is
//! meant to send it.

static void JMAutoTest_JMAntiCheatModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMAntiCheatModule full RPC sweep");
	PrintFormat("================================================================");

	JMAntiCheatModule mod;
	if (!CF_Modules<JMAntiCheatModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMAntiCheatModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <id>, emptyCtx) survives for every handled rpc_type");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMAntiCheatModuleRPC.RequestFlags, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMAntiCheatModuleRPC.RequestFlags");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMAntiCheatModuleRPC.ClearFlag, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMAntiCheatModuleRPC.ClearFlag");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMAntiCheatModuleRPC.Flags, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMAntiCheatModuleRPC.Flags");

	JMAutoTest_CanaryCheck("JMAntiCheatModule full RPC sweep");
}

#endif
