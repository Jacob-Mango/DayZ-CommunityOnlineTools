#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMCameraModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Camera/JMCameraModule.c).
//!
//! Calls OnRPC(sender=NULL, target=NULL, rpc_type, emptyCtx) once for every
//! rpc_type its switch actually handles. sender=NULL is the identity ceiling
//! for this boot-time test - there is no live PlayerIdentity to hand in this
//! early, so every handler that would normally dereference the caller's
//! identity is exercised at the point right before that dereference, same as
//! JMAutoTest_SafetyProbe already proved safe for JMCameraModuleRPC.Enter this
//! session. This suite is a regression baseline for CURRENT behavior, not a
//! correctness spec - a future change that makes any of these calls halt the
//! suite (canary mismatch) is the regression this test exists to catch.

static void JMAutoTest_JMCameraModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMCameraModule full RPC sweep");
	PrintFormat("================================================================");

	JMCameraModule mod;
	if (!CF_Modules<JMCameraModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMCameraModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <rpc id>, emptyCtx) survives for every JMCameraModuleRPC id");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMCameraModuleRPC.Enter, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMCameraModuleRPC.Enter");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMCameraModuleRPC.Leave, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMCameraModuleRPC.Leave");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMCameraModuleRPC.Leave_Finish, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMCameraModuleRPC.Leave_Finish");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMCameraModuleRPC.UpdatePosition, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMCameraModuleRPC.UpdatePosition");

	JMAutoTest_CanaryCheck("JMCameraModule full RPC sweep");
}

#endif
