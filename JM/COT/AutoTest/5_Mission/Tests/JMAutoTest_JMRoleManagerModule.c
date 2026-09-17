#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMRoleManagerModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/RoleManager/JMRoleManagerModule.c).
//!
//! Calls the module's own OnRPC(sender, target, rpc_type, ctx) once per
//! JMRoleManagerModuleRPC value the switch statement actually handles, with
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
//! Note: JMRoleManagerModuleRPC.RoleList is normally a server->client only
//! message (see RPC.c), but the switch in JMRoleManagerModule.OnRPC DOES
//! contain a case for it (routes to RPC_RoleList), so it is included here for
//! completeness - a dedicated server calling its own OnRPC with this rpc_type
//! is not a path a real client can trigger, but the sweep asserts it does not
//! crash regardless.

static void JMAutoTest_JMRoleManagerModule_Sweep(JMRoleManagerModule mod)
{
	JMAutoTest_Group("JMRoleManagerModule.OnRPC(NULL, NULL, <rpc_type>, emptyCtx) - one call per handled rpc_type");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMRoleManagerModuleRPC.RequestRoleList, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMRoleManagerModuleRPC.RequestRoleList");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMRoleManagerModuleRPC.CreateRole, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMRoleManagerModuleRPC.CreateRole");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMRoleManagerModuleRPC.DeleteRole, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMRoleManagerModuleRPC.DeleteRole");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMRoleManagerModuleRPC.SetRolePermissions, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMRoleManagerModuleRPC.SetRolePermissions");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMRoleManagerModuleRPC.RoleList, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMRoleManagerModuleRPC.RoleList");
}

static void JMAutoTest_JMRoleManagerModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMRoleManagerModule full RPC sweep - every rpc_type, NULL sender");
	PrintFormat("================================================================");

	JMRoleManagerModule mod;
	if (!CF_Modules<JMRoleManagerModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMRoleManagerModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_JMRoleManagerModule_Sweep(mod);

	JMAutoTest_CanaryCheck("JMRoleManagerModule full RPC sweep");
}

#endif
