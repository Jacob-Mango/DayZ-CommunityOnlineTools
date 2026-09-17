#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMLoadoutModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Loadout/JMLoadoutModule.c).
//!
//! Calls OnRPC(sender=NULL, target=NULL, rpc_type, emptyCtx) once for every
//! rpc_type the module's OnRPC switch actually handles. sender=NULL is the
//! identity ceiling for this boot-time test - there is no logged-in
//! PlayerIdentity to hand in at this point in the boot sequence, so every
//! handler here is exercised at the lowest-privilege, most-hostile identity
//! it can ever legally be called with (an empty ParamsReadContext makes
//! ctx.Read fail first, before any identity dereference). This asserts
//! CURRENT behavior as a regression baseline, not correctness of any
//! particular handler's logic.
//!
//! RPC_Load already had a null-guard fix applied this session (inside the
//! g_Game.IsDedicatedServer() branch, before calling Server_Load()) - nothing
//! further to change here.

static void JMAutoTest_JMLoadoutModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMLoadoutModule full RPC sweep, NULL sender");
	PrintFormat("================================================================");

	JMLoadoutModule mod;
	if (!CF_Modules<JMLoadoutModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMLoadoutModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <every JMLoadoutModuleRPC id>, emptyCtx) survives");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMLoadoutModuleRPC.Load, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMLoadoutModuleRPC.Load");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMLoadoutModuleRPC.Create, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMLoadoutModuleRPC.Create");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMLoadoutModuleRPC.Delete, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMLoadoutModuleRPC.Delete");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMLoadoutModuleRPC.SpawnCursor, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMLoadoutModuleRPC.SpawnCursor");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMLoadoutModuleRPC.SpawnTarget, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMLoadoutModuleRPC.SpawnTarget");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMLoadoutModuleRPC.SpawnPlayers, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMLoadoutModuleRPC.SpawnPlayers");

	JMAutoTest_CanaryCheck("JMLoadoutModule full RPC sweep");
}

#endif
