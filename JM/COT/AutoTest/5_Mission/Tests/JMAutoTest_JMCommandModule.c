#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMCommandModule
//! (Scripts/4_World/CommunityOnlineTools/Classes/Commands/JMCommandModule.c -
//! note this class lives in the 4_World layer, unlike the 5_Mission modules
//! covered by the other two sweep files, but CF_Modules<JMCommandModule>.Get()
//! resolves it the same way regardless of declaring layer, as already
//! confirmed by JMAutoTest_SafetyProbe's own use of that exact call).
//!
//! Calls OnRPC(sender=NULL, target=NULL, rpc_type, emptyCtx) once for every
//! rpc_type its switch actually handles. sender=NULL is the identity ceiling
//! for this boot-time test - there is no live PlayerIdentity to hand in this
//! early. For JMCommandModuleRPC.PerformCommand specifically, the handler's
//! very first read (ctx.Read(input), reading a string from a truly empty
//! context) is expected to throw a VM exception at the read call itself,
//! before any identity is ever touched - already observed and confirmed safe
//! this session (Enforce Script catches it internally, the canary fires
//! immediately after, and execution resumes normally). That is included here
//! like any other id, not guarded against. This suite is a regression
//! baseline for CURRENT behavior, not a correctness spec - a future change
//! that makes this call halt the suite (canary mismatch) is the regression
//! this test exists to catch.

static void JMAutoTest_JMCommandModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMCommandModule full RPC sweep");
	PrintFormat("================================================================");

	JMCommandModule mod;
	if (!CF_Modules<JMCommandModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMCommandModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <rpc id>, emptyCtx) survives for every JMCommandModuleRPC id");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMCommandModuleRPC.PerformCommand, JMAutoTest_ReadContextWithOneString(""));
	JMAutoTest_Canary("JMCommandModuleRPC.PerformCommand");

	JMAutoTest_CanaryCheck("JMCommandModule full RPC sweep");
}

#endif
