#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMWebhookCOTModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Webhook/JMWebhookCOTModule.c).
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

static void JMAutoTest_JMWebhookCOTModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMWebhookCOTModule full RPC sweep, NULL sender");
	PrintFormat("================================================================");

	JMWebhookCOTModule mod;
	if (!CF_Modules<JMWebhookCOTModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMWebhookCOTModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <every JMWebhookCOTModuleRPC id>, emptyCtx) survives");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWebhookCOTModuleRPC.Load, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWebhookCOTModuleRPC.Load");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWebhookCOTModuleRPC.AddConnectionGroup, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWebhookCOTModuleRPC.AddConnectionGroup");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWebhookCOTModuleRPC.RemoveConnectionGroup, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWebhookCOTModuleRPC.RemoveConnectionGroup");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWebhookCOTModuleRPC.EditConnectionGroup, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWebhookCOTModuleRPC.EditConnectionGroup");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWebhookCOTModuleRPC.AddType, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWebhookCOTModuleRPC.AddType");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWebhookCOTModuleRPC.RemoveType, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWebhookCOTModuleRPC.RemoveType");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWebhookCOTModuleRPC.TypeState, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWebhookCOTModuleRPC.TypeState");

	JMAutoTest_CanaryCheck("JMWebhookCOTModule full RPC sweep");
}

#endif
