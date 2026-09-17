#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMEventsModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Events/JMEventsModule.c).
//!
//! JMEventsModule does not override OnRPC itself - it extends
//! JMEntityManagerModule and inherits that class's generic dispatcher
//! (Scripts/5_Mission/CommunityOnlineTools/gui/EntityManager/JMEntityManagerModule.c),
//! which resolves ids dynamically via the virtuals GetRPCRequest() /
//! GetRPCSend() / GetRPCAction() / GetRPCSendUpsert() / GetRPCSendRemove()
//! rather than a hardcoded switch. JMEventsModule overrides those five
//! virtuals to return JMEventsModuleRPC.Request / .Send / .Action /
//! .SendUpsert / .SendRemove (RPC.c), so this sweep calls OnRPC with each of
//! those five ids in turn - each is matched by JMEntityManagerModule.OnRPC's
//! if/else-if chain and routed to RPC_Request / RPC_Send / RPC_Action /
//! RPC_SendUpsert / RPC_SendRemove respectively.
//!
//! Request and Action are client -> server; Send, SendUpsert and SendRemove
//! are server -> client. This suite calls all five anyway: the goal here is
//! "does calling this survive", not "does it do something" - a server -> client
//! handler invoked with sender=NULL is expected to no-op (RPC_Send checks
//! IsMissionClient(), for example), not crash, and that no-op is itself the
//! thing being asserted safe.
//!
//! Calls OnRPC(sender=NULL, target=NULL, rpc_type, emptyCtx) once per id.
//! sender=NULL is the identity ceiling for this boot-time test - there is no
//! live PlayerIdentity to attach at this point in the boot sequence, so every
//! handler that checks permissions is exercised at the "no sender" edge
//! rather than as a real admin or a real player. This asserts CURRENT
//! behavior as a regression baseline (Phase 0 safety probe already proved
//! this call shape survives for this exact module, including the generic
//! JMEntityManagerModule dispatch it shares with JMEventsModule), not a
//! claim that the handlers are correct.

static void JMAutoTest_JMEventsModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMEventsModule full RPC sweep");
	PrintFormat("================================================================");

	JMEventsModule mod;
	if (!CF_Modules<JMEventsModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMEventsModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <id>, emptyCtx) survives for every id the JMEntityManagerModule adapter maps to");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMEventsModuleRPC.Request, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMEventsModuleRPC.Request");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMEventsModuleRPC.Send, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMEventsModuleRPC.Send");

	//! RPC_Action's first read is a string (actionId, JMEntityManagerModule.c)
	//! - an empty buffer throws a VM exception decoding it (see
	//! JMAutoTestRPCFixtures.c). Give it one real empty string instead.
	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMEventsModuleRPC.Action, JMAutoTest_ReadContextWithOneString(""));
	JMAutoTest_Canary("JMEventsModuleRPC.Action");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMEventsModuleRPC.SendUpsert, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMEventsModuleRPC.SendUpsert");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMEventsModuleRPC.SendRemove, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMEventsModuleRPC.SendRemove");

	JMAutoTest_CanaryCheck("JMEventsModule full RPC sweep");
}

#endif
