#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMObjectSpawnerModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Object/JMObjectSpawnerModule.c).
//!
//! Calls OnRPC(sender=NULL, target=NULL, rpc_type, emptyCtx) once for every
//! rpc_type its switch actually handles. sender=NULL is the identity ceiling
//! for this boot-time test - there is no live PlayerIdentity to hand in this
//! early, so every handler that would normally dereference the caller's
//! identity is exercised at the point right before that dereference, same as
//! JMAutoTest_SafetyProbe already proved safe for JMObjectSpawnerModuleRPC.Position
//! this session (including RPC_SpawnEntity_Position's now-added null-sender
//! guard). This suite is a regression baseline for CURRENT behavior, not a
//! correctness spec - a future change that makes any of these calls halt the
//! suite (canary mismatch) is the regression this test exists to catch.

static void JMAutoTest_JMObjectSpawnerModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMObjectSpawnerModule full RPC sweep");
	PrintFormat("================================================================");

	JMObjectSpawnerModule mod;
	if (!CF_Modules<JMObjectSpawnerModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMObjectSpawnerModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <rpc id>, emptyCtx) survives for every JMObjectSpawnerModuleRPC id");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMObjectSpawnerModuleRPC.Position, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMObjectSpawnerModuleRPC.Position");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMObjectSpawnerModuleRPC.Inventory, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMObjectSpawnerModuleRPC.Inventory");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMObjectSpawnerModuleRPC.Delete, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMObjectSpawnerModuleRPC.Delete");

	JMAutoTest_CanaryCheck("JMObjectSpawnerModule full RPC sweep");
}

#endif
