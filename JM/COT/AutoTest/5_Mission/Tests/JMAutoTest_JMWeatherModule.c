#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMWeatherModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Weather/jmweathermodule.c -
//! lowercase filename, PascalCase class name, both correct as-is).
//!
//! Every case actually handled by that file's OnRPC switch is called once,
//! as OnRPC(sender=NULL, target=NULL, rpc_type, emptyCtx). A NULL sender is
//! the identity ceiling for this boot-time test: it proves the handler
//! survives the earliest possible failure point (no sender at all, no
//! payload to read) without needing a connected client. This is not a
//! functional test of weather behaviour - it asserts CURRENT behaviour
//! (handler returns early without crashing) as a regression baseline, per
//! the phase 0 safety probe (JMAutoTest_SafetyProbe.c) that already proved
//! this pattern safe for every module, including this one.
//!
//! RPC_Load has a null-guard on senderRPC inside the g_Game.IsDedicatedServer()
//! branch before calling Server_Load() - already fixed production code this
//! session, nothing for this suite to change. It is exercised here like every
//! other id, not treated specially.

static void JMAutoTest_JMWeatherModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMWeatherModule full RPC sweep");
	PrintFormat("================================================================");

	JMWeatherModule mod;
	if (!CF_Modules<JMWeatherModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMWeatherModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <id>, emptyCtx) survives for every JMWeatherModuleRPC id");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.Load, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.Load");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.Storm, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.Storm");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.Fog, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.Fog");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.DynamicFog, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.DynamicFog");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.Rain, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.Rain");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.RainThresholds, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.RainThresholds");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.Snow, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.Snow");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.SnowThresholds, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.SnowThresholds");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.Overcast, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.Overcast");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.WindFunctionParams, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.WindFunctionParams");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.WindMagnitude, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.WindMagnitude");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.WindDirection, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.WindDirection");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.Date, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.Date");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.UsePreset, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.UsePreset");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.CreatePreset, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.CreatePreset");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.UpdatePreset, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.UpdatePreset");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.RemovePreset, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.RemovePreset");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.FreezeTime, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.FreezeTime");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMWeatherModuleRPC.DynamicWeather, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMWeatherModuleRPC.DynamicWeather");

	JMAutoTest_CanaryCheck("JMWeatherModule full RPC sweep");
}

#endif
