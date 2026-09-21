#ifdef JM_COT_AUTOTEST

//! Runs every registered suite once at mission start and prints one verdict.
//! Enforce Script has no reflection/function pointers, so there is no way to
//! auto-discover suites - registration in JMAutoTest_RunAll() below is a
//! manual, hand-written call list. Add one pB/fB/call/EndSuite block per new
//! suite. Ported from EgoLand's AutoTestRunner.c, minus the modded MissionServer
//! hook - COT calls JMAutoTest_TryRun() from its own CommunityOnlineTools.OnStart().

static int g_JMAutoTestSuitesPassed = 0;
static int g_JMAutoTestSuitesFailed = 0;

//! One-shot guard against a double call from the single OnStart() call site.
static bool g_JMAutoTestHasRun = false;

//! Public entrypoint. Called from the modded CommunityOnlineTools.OnStart()
//! (Scripts/5_Mission/CommunityOnlineTools/CommunityOnlineTools.c), itself
//! gated by the same #ifdef JM_COT_AUTOTEST.
void JMAutoTest_TryRun()
{
	if (g_JMAutoTestHasRun)
		return;
	g_JMAutoTestHasRun = true;

	if (g_Game && !g_Game.IsServer())
		return;

	JMAutoTest_RunAll();
}

static void JMAutoTest_RunAll()
{
	PrintFormat("");
	PrintFormat("#################################################################");
	PrintFormat("#  JM COT AUTOTESTS                                             #");
	PrintFormat("#################################################################");

	g_JMAutoTestPassCount = 0;
	g_JMAutoTestFailCount = 0;
	g_JMAutoTestGroupCount = 0;
	g_JMAutoTestSuitesPassed = 0;
	g_JMAutoTestSuitesFailed = 0;

	// -- Suites -----------------------------------------------------------
	int pB;
	int fB;

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMConstants();
	JMAutoTest_EndSuite("JMConstants.Lucide()", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMRPCThrottle();
	JMAutoTest_EndSuite("JMRPCThrottle Rate Limiting", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_SafetyProbe();
	JMAutoTest_EndSuite("Phase 0 Safety Probe (one RPC per module, NULL sender)", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_PermissionModel();
	JMAutoTest_EndSuite("Permission Model (core tree logic)", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_PermissionMatrix();
	JMAutoTest_EndSuite("Permission Matrix (module + RPC gate strings)", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_ESPActionMenuMapping();
	JMAutoTest_EndSuite("JMESPModule.PermissionForAction completeness", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_ChatCommandsAndQuickActions();
	JMAutoTest_EndSuite("Chat commands + Actions.QuickActions", pB, fB);

	// -- Phase 3: full per-module RPC sweeps ---------------------------------

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMPlayerModule();
	JMAutoTest_EndSuite("JMPlayerModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMESPModule();
	JMAutoTest_EndSuite("JMESPModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMVehiclesModule();
	JMAutoTest_EndSuite("JMVehiclesModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMTeleportModule();
	JMAutoTest_EndSuite("JMTeleportModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMWeatherModule();
	JMAutoTest_EndSuite("JMWeatherModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMObjectSpawnerModule();
	JMAutoTest_EndSuite("JMObjectSpawnerModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMCameraModule();
	JMAutoTest_EndSuite("JMCameraModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMCommandModule();
	JMAutoTest_EndSuite("JMCommandModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMWebhookCOTModule();
	JMAutoTest_EndSuite("JMWebhookCOTModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMLoadoutModule();
	JMAutoTest_EndSuite("JMLoadoutModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMCompensationsModule();
	JMAutoTest_EndSuite("JMCompensationsModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMRoleManagerModule();
	JMAutoTest_EndSuite("JMRoleManagerModule full RPC sweep", pB, fB);

#ifdef EXPANSIONMODBASEBUILDING
	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMTerritoryModule();
	JMAutoTest_EndSuite("JMTerritoryModule full RPC sweep", pB, fB);
#endif

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMMapEditorModule();
	JMAutoTest_EndSuite("JMMapEditorModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMAntiCheatModule();
	JMAutoTest_EndSuite("JMAntiCheatModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMServerStatsModule();
	JMAutoTest_EndSuite("JMServerStatsModule (no client->server surface)", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMActionHistory();
	JMAutoTest_EndSuite("JMActionHistory stack rules + module RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMBanModule();
	JMAutoTest_EndSuite("JMBanModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMNamalskEventManagerModule();
	JMAutoTest_EndSuite("JMNamalskEventManagerModule full RPC sweep (map-conditional)", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMEventsModule();
	JMAutoTest_EndSuite("JMEventsModule full RPC sweep", pB, fB);

	pB = g_JMAutoTestPassCount; fB = g_JMAutoTestFailCount;
	JMAutoTest_JMLootAnalysisModule();
	JMAutoTest_EndSuite("JMLootAnalysisModule (public-helper fallback)", pB, fB);

	// -- Final verdict ------------------------------------------------------
	int total = g_JMAutoTestPassCount + g_JMAutoTestFailCount;

	PrintFormat("");
	PrintFormat("#################################################################");
	PrintFormat("#  JM COT AUTOTEST SUMMARY                                      #");
	PrintFormat("#################################################################");
	PrintFormat("  Assertions: %1/%2 passed (%3 failed)", g_JMAutoTestPassCount, total, g_JMAutoTestFailCount);
	PrintFormat("  Suites:     %1 passed, %2 failed", g_JMAutoTestSuitesPassed, g_JMAutoTestSuitesFailed);

	if (g_JMAutoTestFailCount == 0)
	{
		PrintFormat("  STATUS: ALL TESTS PASSED");
	}
	else
	{
		PrintFormat("  STATUS: FAILURES DETECTED");
		PrintFormat("  AUTOTEST FAILURES: %1", g_JMAutoTestFailCount);
		Error("[JM_COT_AUTOTEST] AUTOTEST FAILURES: " + g_JMAutoTestFailCount + " (search log for [FAIL])");
	}

	PrintFormat("#################################################################");
	PrintFormat("");
}

//! Records one suite's result from pass/fail snapshots taken before it ran.
//! A suite "passes" only if it added >=1 passing assertion and 0 failing ones.
static void JMAutoTest_EndSuite(string suiteName, int passBefore, int failBefore)
{
	int suiteFails = g_JMAutoTestFailCount - failBefore;
	int suitePasses = g_JMAutoTestPassCount - passBefore;

	if (suiteFails == 0 && suitePasses > 0)
		g_JMAutoTestSuitesPassed++;
	else
		g_JMAutoTestSuitesFailed++;
}

#endif
