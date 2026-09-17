#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMNamalskEventManagerModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Namalsk/JMNamalskEventManagerModule.c).
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
//! JMNamalskEventManagerRPC (RPC.c) has four members: LoadEvents and
//! RequestEvents, StartEvent and CancelEvent all have a real case in the
//! OnRPC switch, so all four are swept.
//!
//! IMPORTANT: JMNamalskEventManagerModule only constructs itself on the
//! Namalsk map, not on the Chernarus test map this project's dev server
//! normally boots. CF_Modules<JMNamalskEventManagerModule>.Get() returning
//! false/null here is expected on that map, not a test infra defect - this
//! suite treats it as a benign skip (JMAutoTest_Pass), matching how
//! JMAutoTest_SafetyProbe.c already handles this same module's absence.

static void JMAutoTest_JMNamalskEventManagerModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMNamalskEventManagerModule full RPC sweep");
	PrintFormat("================================================================");

	JMNamalskEventManagerModule mod;
	if (!CF_Modules<JMNamalskEventManagerModule>.Get(mod) || !mod)
	{
		JMAutoTest_Pass("JMNamalskEventManagerModule not constructed on this map - skipped (not a failure)");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <id>, emptyCtx) survives for every handled rpc_type");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMNamalskEventManagerRPC.LoadEvents, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMNamalskEventManagerRPC.LoadEvents");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMNamalskEventManagerRPC.RequestEvents, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMNamalskEventManagerRPC.RequestEvents");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMNamalskEventManagerRPC.StartEvent, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMNamalskEventManagerRPC.StartEvent");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMNamalskEventManagerRPC.CancelEvent, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMNamalskEventManagerRPC.CancelEvent");

	JMAutoTest_CanaryCheck("JMNamalskEventManagerModule full RPC sweep");
}

#endif
