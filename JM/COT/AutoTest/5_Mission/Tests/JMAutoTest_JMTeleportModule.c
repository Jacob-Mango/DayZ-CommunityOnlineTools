#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMTeleportModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Teleport/JMTeleportModule.c).
//!
//! Every case actually handled by that file's OnRPC switch is called once,
//! as OnRPC(sender=NULL, target=NULL, rpc_type, emptyCtx). A NULL sender is
//! the identity ceiling for this boot-time test: it proves the handler
//! survives the earliest possible failure point (no sender at all, no
//! payload to read) without needing a connected client. This is not a
//! functional test of teleport behaviour - it asserts CURRENT behaviour
//! (handler returns early without crashing) as a regression baseline, per
//! the phase 0 safety probe (JMAutoTest_SafetyProbe.c) that already proved
//! this pattern safe for every module, including this one.
//!
//! RPC_Load has a null-guard on senderRPC inside the g_Game.IsDedicatedServer()
//! branch before calling Server_Load() - already fixed production code this
//! session, nothing for this suite to change. It is exercised here like every
//! other id, not treated specially.

static void JMAutoTest_JMTeleportModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMTeleportModule full RPC sweep");
	PrintFormat("================================================================");

	JMTeleportModule mod;
	if (!CF_Modules<JMTeleportModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMTeleportModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <id>, emptyCtx) survives for every JMTeleportModuleRPC id");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMTeleportModuleRPC.Load, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMTeleportModuleRPC.Load");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMTeleportModuleRPC.Position, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMTeleportModuleRPC.Position");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMTeleportModuleRPC.PositionRaycast, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMTeleportModuleRPC.PositionRaycast");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMTeleportModuleRPC.Location, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMTeleportModuleRPC.Location");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMTeleportModuleRPC.AddLocation, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMTeleportModuleRPC.AddLocation");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMTeleportModuleRPC.RemoveLocation, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMTeleportModuleRPC.RemoveLocation");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMTeleportModuleRPC.EditLocation, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMTeleportModuleRPC.EditLocation");

	JMAutoTest_CanaryCheck("JMTeleportModule full RPC sweep");
}

#endif
