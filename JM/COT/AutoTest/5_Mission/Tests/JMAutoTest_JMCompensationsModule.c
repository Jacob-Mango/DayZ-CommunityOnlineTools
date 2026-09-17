#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMCompensationsModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Compensations/JMCompensationsModule.c).
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

static void JMAutoTest_JMCompensationsModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMCompensationsModule full RPC sweep, NULL sender");
	PrintFormat("================================================================");

	JMCompensationsModule mod;
	if (!CF_Modules<JMCompensationsModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMCompensationsModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <every JMCompensationsModuleRPC id>, emptyCtx) survives");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMCompensationsModuleRPC.Load, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMCompensationsModuleRPC.Load");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMCompensationsModuleRPC.SpawnCursor, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMCompensationsModuleRPC.SpawnCursor");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMCompensationsModuleRPC.SpawnTarget, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMCompensationsModuleRPC.SpawnTarget");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMCompensationsModuleRPC.SpawnPlayers, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMCompensationsModuleRPC.SpawnPlayers");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMCompensationsModuleRPC.Delete, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMCompensationsModuleRPC.Delete");

	JMAutoTest_CanaryCheck("JMCompensationsModule full RPC sweep");
}

#endif
