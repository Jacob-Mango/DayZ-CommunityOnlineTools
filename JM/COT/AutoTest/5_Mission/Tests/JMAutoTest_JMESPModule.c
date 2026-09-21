#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMESPModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/ESP/JMESPModule.c).
//!
//! Calls the module's own OnRPC(sender, target, rpc_type, ctx) once per
//! JMESPModuleRPC value the switch statement actually handles, with
//! sender=NULL, target=NULL, ctx=an empty ParamsReadContext. sender=NULL is
//! the identity ceiling for this boot-time test - no PlayerIdentity mock
//! exists at this point in the boot sequence, so every handler is exercised
//! exactly the way JMAutoTest_SafetyProbe.c already proved safe for one
//! representative rpc_type per module this session (every handler reads its
//! payload via ctx.Read before ever touching senderRPC, so the empty context
//! makes it bail out before any identity dereference). This suite widens that
//! single-sample probe to every rpc_type this module's switch knows about, so
//! it is asserting CURRENT behavior as a regression baseline, not proving new
//! safety properties.
//!
//! Fixed, not just flagged: SetHealth, ObjectAction, BaseBuilding_Build/
//! Dismantle/Repair/SetPartHealth, Vehicle_Unstuck/Refuel and Heal used to
//! send a targeted ScriptRPC with a non-NULL target from the client side,
//! which per docs/systems/rpc.md crashed the server natively before any
//! script ran. All 9 now send untargeted with the network id in the payload
//! (netLow/netHigh, read first by each handler and resolved via
//! g_Game.GetObjectByNetworkId) - the same pattern RPC_DeleteObject already
//! used. This suite calls the SERVER's OnRPC directly with target=NULL and
//! an empty/malformed ctx, so it cannot reproduce the original native crash
//! either way (that only happened on the wire, from a real client send) -
//! it is testing the handler's own null/malformed-payload robustness, which
//! is a separate property from the transport fix.

static void JMAutoTest_JMESPModule_Sweep(JMESPModule mod)
{
	JMAutoTest_Group("JMESPModule.OnRPC(NULL, NULL, <rpc_type>, emptyCtx) - one call per handled rpc_type");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.Log, JMAutoTest_ReadContextWithOneString(""));
	JMAutoTest_Canary("JMESPModuleRPC.Log");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.SetPosition, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.SetPosition");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.SetOrientation, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.SetOrientation");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.SetHealth, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.SetHealth");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.DeleteObject, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.DeleteObject");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.BaseBuilding_Build, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.BaseBuilding_Build");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.BaseBuilding_Dismantle, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.BaseBuilding_Dismantle");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.BaseBuilding_Repair, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.BaseBuilding_Repair");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.BaseBuilding_SetPartHealth, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.BaseBuilding_SetPartHealth");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.Vehicle_Unstuck, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.Vehicle_Unstuck");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.Vehicle_Refuel, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.Vehicle_Refuel");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.Heal, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.Heal");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.MakeItemSet, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.MakeItemSet");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.DuplicateAll, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.DuplicateAll");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.DeleteAll, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.DeleteAll");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.MoveToCursor, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.MoveToCursor");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.ObjectAction, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.ObjectAction");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.ObjectActionResult, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.ObjectActionResult");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMESPModuleRPC.RecordTransformHistory, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMESPModuleRPC.RecordTransformHistory");
}

static void JMAutoTest_JMESPModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMESPModule full RPC sweep - every rpc_type, NULL sender");
	PrintFormat("================================================================");

	JMESPModule mod;
	if (!CF_Modules<JMESPModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMESPModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_JMESPModule_Sweep(mod);

	JMAutoTest_CanaryCheck("JMESPModule full RPC sweep");
}

#endif
