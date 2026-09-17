#ifdef JM_COT_AUTOTEST
#ifdef EXPANSIONMODBASEBUILDING

//! Full per-RPC sweep for JMTerritoryModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Territory/JMTerritoryModule.c).
//!
//! Unlike most modules, JMTerritoryModule has its own OnRPC override (it does
//! not reuse JMEntityManagerModule.OnRPC). Calls that override once per
//! JMTerritoryModuleRPC value the switch statement actually handles, with
//! sender=NULL, target=NULL, ctx=an empty ParamsReadContext. sender=NULL is
//! the identity ceiling for this boot-time test - no PlayerIdentity mock
//! exists at this point in the boot sequence, so every handler is exercised
//! exactly the way JMAutoTest_SafetyProbe.c already proved safe for one
//! representative rpc_type per module this session (every handler reads its
//! payload via ctx.Read, or checks sender/g_Game.IsClient(), before ever
//! reaching risky work, so the empty context and NULL sender make it bail out
//! early). This suite widens that single-sample probe to every rpc_type this
//! module's switch knows about, so it is asserting CURRENT behavior as a
//! regression baseline, not proving new safety properties.
//!
//! All three JMTerritoryModuleRPC members have a case in the switch:
//! RequestTerritories (guarded by `if (!sender) return;`, so a NULL sender
//! bails immediately), SendTerritories (guarded by
//! `if (!g_Game.IsClient()) return;`, so on a dedicated server it bails
//! immediately regardless of sender/ctx), and SetLevel (also guarded by
//! `if (!sender) return;`).

static void JMAutoTest_JMTerritoryModule_Sweep(JMTerritoryModule mod)
{
	JMAutoTest_Group("JMTerritoryModule.OnRPC(NULL, NULL, <rpc_type>, emptyCtx) - one call per handled rpc_type");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMTerritoryModuleRPC.RequestTerritories, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMTerritoryModuleRPC.RequestTerritories");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMTerritoryModuleRPC.SendTerritories, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMTerritoryModuleRPC.SendTerritories");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMTerritoryModuleRPC.SetLevel, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMTerritoryModuleRPC.SetLevel");
}

static void JMAutoTest_JMTerritoryModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMTerritoryModule full RPC sweep - every rpc_type, NULL sender");
	PrintFormat("================================================================");

	JMTerritoryModule mod;
	if (!CF_Modules<JMTerritoryModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMTerritoryModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_JMTerritoryModule_Sweep(mod);

	JMAutoTest_CanaryCheck("JMTerritoryModule full RPC sweep");
}

#endif
#endif
