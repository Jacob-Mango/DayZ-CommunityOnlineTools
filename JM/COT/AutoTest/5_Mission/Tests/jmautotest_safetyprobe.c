#ifdef JM_COT_AUTOTEST

//! Phase 0 empirical safety probe. Before writing a full per-RPC sweep for
//! every module, confirm the core assumption behind that whole plan: that
//! calling a module's OnRPC(sender=NULL, target=NULL, rpcType, emptyCtx) is
//! safe across the board, because every handler examined this session reads
//! its payload (ctx.Read) before ever touching senderRPC - an empty context
//! makes ctx.Read fail and the handler return early, never reaching the
//! identity dereference in JMPermissionManager.HasPermission that would
//! otherwise be a null-pointer hazard (Enforce Script has no try/catch, and
//! this exact dedicated server has been observed to go down hard on an
//! unrelated VM exception this session).
//!
//! One representative rpc_type per OnRPC-implementing module (19 total -
//! CommunityOnlineTools's own base RPCs were already exercised and proven
//! safe earlier this session). If this suite's canary check passes for every
//! module, the full per-RPC sweep (JMAutoTest_<Module>.c files) is safe to
//! write without any production null-guard changes. If the whole boot pass
//! instead comes back as a build CRASH (not a normal [FAIL] line), that
//! module is the one whose handler checks permission before reading its
//! payload - identify it from the build log and fix ONLY that handler before
//! continuing, per the plan's phase 0 design.

//! Some modules (e.g. JMNamalskEventManagerModule) only construct themselves
//! on a matching map - not finding one here is a benign skip, not a test
//! infra defect, so this does not call JMAutoTest_Fail.
static void JMAutoTest_SafetyProbe_Module(string moduleName, bool didGet)
{
	if (!didGet)
	{
		JMAutoTest_Pass(moduleName + " not constructed on this map - skipped (not a failure)");
	}
}

static void JMAutoTest_SafetyProbe()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: Phase 0 safety probe - one RPC per module, NULL sender");
	PrintFormat("================================================================");

	JMAutoTest_Group("OnRPC(NULL, NULL, <one rpc id>, emptyCtx) survives for every module");

	JMCameraModule camMod;
	if (CF_Modules<JMCameraModule>.Get(camMod) && camMod)
	{
		JMAutoTest_ExpectCanary();
		camMod.OnRPC(NULL, NULL, JMCameraModuleRPC.Enter, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMCameraModule.Enter");
	}
	else JMAutoTest_SafetyProbe_Module("JMCameraModule", false);

	JMVehiclesModule vehMod;
	if (CF_Modules<JMVehiclesModule>.Get(vehMod) && vehMod)
	{
		JMAutoTest_ExpectCanary();
		vehMod.OnRPC(NULL, NULL, JMVehiclesModuleRPC.RequestServerVehicles, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMVehiclesModule.RequestServerVehicles");
	}
	else JMAutoTest_SafetyProbe_Module("JMVehiclesModule", false);

	JMObjectSpawnerModule objMod;
	if (CF_Modules<JMObjectSpawnerModule>.Get(objMod) && objMod)
	{
		JMAutoTest_ExpectCanary();
		objMod.OnRPC(NULL, NULL, JMObjectSpawnerModuleRPC.Position, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMObjectSpawnerModule.Position");
	}
	else JMAutoTest_SafetyProbe_Module("JMObjectSpawnerModule", false);

	JMTeleportModule tpMod;
	if (CF_Modules<JMTeleportModule>.Get(tpMod) && tpMod)
	{
		JMAutoTest_ExpectCanary();
		tpMod.OnRPC(NULL, NULL, JMTeleportModuleRPC.Load, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMTeleportModule.Load");
	}
	else JMAutoTest_SafetyProbe_Module("JMTeleportModule", false);

	JMWeatherModule wthMod;
	if (CF_Modules<JMWeatherModule>.Get(wthMod) && wthMod)
	{
		JMAutoTest_ExpectCanary();
		wthMod.OnRPC(NULL, NULL, JMWeatherModuleRPC.Load, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMWeatherModule.Load");
	}
	else JMAutoTest_SafetyProbe_Module("JMWeatherModule", false);

	JMESPModule espMod;
	if (CF_Modules<JMESPModule>.Get(espMod) && espMod)
	{
		JMAutoTest_ExpectCanary();
		espMod.OnRPC(NULL, NULL, JMESPModuleRPC.Log, JMAutoTest_ReadContextWithOneString(""));
		JMAutoTest_Canary("JMESPModule.Log");
	}
	else JMAutoTest_SafetyProbe_Module("JMESPModule", false);

	JMPlayerModule plyMod;
	if (CF_Modules<JMPlayerModule>.Get(plyMod) && plyMod)
	{
		JMAutoTest_ExpectCanary();
		plyMod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetStat, JMAutoTest_ReadContextForSetStat());
		JMAutoTest_Canary("JMPlayerModule.SetStat");
	}
	else JMAutoTest_SafetyProbe_Module("JMPlayerModule", false);

	JMNamalskEventManagerModule namMod;
	if (CF_Modules<JMNamalskEventManagerModule>.Get(namMod) && namMod)
	{
		JMAutoTest_ExpectCanary();
		namMod.OnRPC(NULL, NULL, JMNamalskEventManagerRPC.LoadEvents, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMNamalskEventManagerModule.LoadEvents");
	}
	else JMAutoTest_SafetyProbe_Module("JMNamalskEventManagerModule", false);

	JMCommandModule cmdMod;
	if (CF_Modules<JMCommandModule>.Get(cmdMod) && cmdMod)
	{
		JMAutoTest_ExpectCanary();
		cmdMod.OnRPC(NULL, NULL, JMCommandModuleRPC.PerformCommand, JMAutoTest_ReadContextWithOneString(""));
		JMAutoTest_Canary("JMCommandModule.PerformCommand");
	}
	else JMAutoTest_SafetyProbe_Module("JMCommandModule", false);

	JMWebhookCOTModule webMod;
	if (CF_Modules<JMWebhookCOTModule>.Get(webMod) && webMod)
	{
		JMAutoTest_ExpectCanary();
		webMod.OnRPC(NULL, NULL, JMWebhookCOTModuleRPC.Load, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMWebhookCOTModule.Load");
	}
	else JMAutoTest_SafetyProbe_Module("JMWebhookCOTModule", false);

	JMLoadoutModule loadMod;
	if (CF_Modules<JMLoadoutModule>.Get(loadMod) && loadMod)
	{
		JMAutoTest_ExpectCanary();
		loadMod.OnRPC(NULL, NULL, JMLoadoutModuleRPC.Load, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMLoadoutModule.Load");
	}
	else JMAutoTest_SafetyProbe_Module("JMLoadoutModule", false);

	JMCompensationsModule compMod;
	if (CF_Modules<JMCompensationsModule>.Get(compMod) && compMod)
	{
		JMAutoTest_ExpectCanary();
		compMod.OnRPC(NULL, NULL, JMCompensationsModuleRPC.Load, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMCompensationsModule.Load");
	}
	else JMAutoTest_SafetyProbe_Module("JMCompensationsModule", false);

	JMRoleManagerModule roleMod;
	if (CF_Modules<JMRoleManagerModule>.Get(roleMod) && roleMod)
	{
		JMAutoTest_ExpectCanary();
		roleMod.OnRPC(NULL, NULL, JMRoleManagerModuleRPC.RequestRoleList, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMRoleManagerModule.RequestRoleList");
	}
	else JMAutoTest_SafetyProbe_Module("JMRoleManagerModule", false);

	JMEventsModule evtMod;
	if (CF_Modules<JMEventsModule>.Get(evtMod) && evtMod)
	{
		JMAutoTest_ExpectCanary();
		evtMod.OnRPC(NULL, NULL, JMEventsModuleRPC.Request, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMEventsModule.Request");
	}
	else JMAutoTest_SafetyProbe_Module("JMEventsModule", false);

#ifdef EXPANSIONMODBASEBUILDING
	JMTerritoryModule terrMod;
	if (CF_Modules<JMTerritoryModule>.Get(terrMod) && terrMod)
	{
		JMAutoTest_ExpectCanary();
		terrMod.OnRPC(NULL, NULL, JMTerritoryModuleRPC.RequestTerritories, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMTerritoryModule.RequestTerritories");
	}
	else JMAutoTest_SafetyProbe_Module("JMTerritoryModule", false);
#else
	JMAutoTest_SafetyProbe_Module("JMTerritoryModule", false);
#endif

	JMMapEditorModule mapEdMod;
	if (CF_Modules<JMMapEditorModule>.Get(mapEdMod) && mapEdMod)
	{
		JMAutoTest_ExpectCanary();
		mapEdMod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.RequestList, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMMapEditorModule.RequestList");
	}
	else JMAutoTest_SafetyProbe_Module("JMMapEditorModule", false);

	JMAntiCheatModule acMod;
	if (CF_Modules<JMAntiCheatModule>.Get(acMod) && acMod)
	{
		JMAutoTest_ExpectCanary();
		acMod.OnRPC(NULL, NULL, JMAntiCheatModuleRPC.RequestFlags, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMAntiCheatModule.RequestFlags");
	}
	else JMAutoTest_SafetyProbe_Module("JMAntiCheatModule", false);

	JMServerStatsModule statsMod;
	if (CF_Modules<JMServerStatsModule>.Get(statsMod) && statsMod)
	{
		JMAutoTest_ExpectCanary();
		statsMod.OnRPC(NULL, NULL, JMServerStatsModuleRPC.Stats, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMServerStatsModule.Stats");
	}
	else JMAutoTest_SafetyProbe_Module("JMServerStatsModule", false);

	JMBanModule banMod;
	if (CF_Modules<JMBanModule>.Get(banMod) && banMod)
	{
		JMAutoTest_ExpectCanary();
		banMod.OnRPC(NULL, NULL, JMBanModuleRPC.RequestBanList, JMAutoTest_EmptyReadContext());
		JMAutoTest_Canary("JMBanModule.RequestBanList");
	}
	else JMAutoTest_SafetyProbe_Module("JMBanModule", false);

	JMAutoTest_CanaryCheck("Phase 0 safety probe - one RPC per module");
}

#endif
