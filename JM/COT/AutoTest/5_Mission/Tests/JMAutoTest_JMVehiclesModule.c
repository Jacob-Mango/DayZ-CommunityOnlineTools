#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMVehiclesModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Vehicles/JMVehiclesModule.c).
//!
//! sender=NULL is the identity ceiling for this boot-time test - there is no
//! PlayerIdentity mock in this framework, so every call below goes in with a
//! NULL PlayerIdentity and an empty (truncated) ParamsReadContext. That is
//! safe here because every RPC_* handler on JMVehiclesModule either checks
//! IsMissionHost()/IsMissionClient() before touching the context, or checks
//! "if ( !senderRPC ) return;" before its first ctx.Read - confirmed by
//! reading the handler bodies directly, and consistent with the empirically
//! proven Phase 0 safety probe pattern in JMAutoTest_SafetyProbe.c. This does
//! not re-derive that safety property; it reuses it.
//!
//! This asserts CURRENT behaviour as a regression baseline: every rpc_type
//! the module's OnRPC switch actually has a case for must survive a NULL
//! sender / empty context call without halting the suite. It is not a
//! functional test of what each handler does - only that it does not crash
//! the server when handed a malformed/absent payload from an unauthenticated
//! caller.
//!
//! 20 rpc_types total, matching every "case JMVehiclesModuleRPC.X:" in the
//! module's OnRPC. 4 of them (DeleteVehicleUnclaimed, CoverVehicle,
//! LockVehicle, UnPairVehicle) only have a case under #ifdef
//! EXPANSIONMODVEHICLE in the module itself, so this file gates the same 4
//! calls behind the same #ifdef - on a non-Expansion build those rpc_types
//! hit no case in the real switch, so exercising them here would test
//! nothing the production switch would ever route.

static void JMAutoTest_JMVehiclesModule_Sweep(JMVehiclesModule mod)
{
	JMAutoTest_Group("JMVehiclesModule.OnRPC(NULL, NULL, <every handled rpc_type>, emptyCtx)");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.RequestServerVehicles, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.RequestServerVehicles");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.SendServerVehicles, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.SendServerVehicles");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.DeleteVehicle, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.DeleteVehicle");

	#ifdef EXPANSIONMODVEHICLE
	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.DeleteVehicleUnclaimed, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.DeleteVehicleUnclaimed");
	#endif

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.DeleteVehicleDestroyed, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.DeleteVehicleDestroyed");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.DeleteVehicleAll, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.DeleteVehicleAll");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.TeleportToVehicle, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.TeleportToVehicle");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.TeleportVehicleToMe, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.TeleportVehicleToMe");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.TeleportVehicleTo, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.TeleportVehicleTo");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.ClearVehicleCargo, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.ClearVehicleCargo");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.SpawnVehicleKey, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.SpawnVehicleKey");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.RepairVehicle, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.RepairVehicle");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.RefuelVehicle, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.RefuelVehicle");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.UnstuckVehicle, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.UnstuckVehicle");

	#ifdef EXPANSIONMODVEHICLE
	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.CoverVehicle, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.CoverVehicle");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.LockVehicle, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.LockVehicle");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.UnPairVehicle, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.UnPairVehicle");
	#endif

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.SendVehicleUpsert, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.SendVehicleUpsert");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.SendVehicleRemove, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.SendVehicleRemove");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.RequestVehicleUpsert, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMVehiclesModuleRPC.RequestVehicleUpsert");
}

static void JMAutoTest_JMVehiclesModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMVehiclesModule full RPC sweep, NULL sender");
	PrintFormat("================================================================");

	JMVehiclesModule mod;
	if (!CF_Modules<JMVehiclesModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMVehiclesModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_JMVehiclesModule_Sweep(mod);

	JMAutoTest_CanaryCheck("JMVehiclesModule full RPC sweep");
}

#endif
