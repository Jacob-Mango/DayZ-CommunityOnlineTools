#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMPlayerModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/Player/JMPlayerModule.c),
//! the highest RPC-count module in the mod. The switch inside its
//! override void OnRPC(...) was read directly (not just the enum member
//! list) and handles every one of the 51 non-sentinel JMPlayerModuleRPC
//! values declared in Scripts/3_Game/CommunityOnlineTools/RPC.c (INVALID and
//! COUNT are sentinels, not real wire values, and are excluded here). This
//! file exercises all 51.
//!
//! sender=NULL, target=NULL is the identity ceiling for this boot-time test -
//! there is no PlayerIdentity mock available this early in boot, so no
//! handler here ever reaches a real GetPermissionsManager().HasPermission(...)
//! check against a live client identity. Every handler examined reads its
//! payload via ctx.Read(...) before touching the sender identity; against
//! JMAutoTest_EmptyReadContext() that read fails immediately and the handler
//! returns before any identity dereference. This exact pattern (NULL sender +
//! empty ctx, one call per module) was already proven not to crash the
//! dedicated server in JMAutoTest_SafetyProbe.c's phase 0 pass, including for
//! JMPlayerModuleRPC.SetStat specifically - this file just extends that same
//! proven-safe call shape to every remaining handler on this module.
//!
//! What this DOES assert: "every JMPlayerModuleRPC handler survives a NULL
//! sender with a malformed/empty payload without crashing or hanging the
//! server" - a regression baseline. What this does NOT assert: "permission
//! enforcement works correctly for a real client" - that needs a
//! PlayerIdentity/JMPlayerInstance fixture this boot-time harness does not
//! have, so it is untestable here and out of scope.

static void JMAutoTest_PlayerModule_AllRPCsSurviveNullSender(JMPlayerModule mod)
{
	JMAutoTest_Group("JMPlayerModule.OnRPC(NULL, NULL, <every handled rpc_type>, emptyCtx) survives");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetStat, JMAutoTest_ReadContextForSetStat());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetStat");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetBloodyHands, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetBloodyHands");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.RepairTransport, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.RepairTransport");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.TeleportTo, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.TeleportTo");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.TeleportSenderTo, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.TeleportSenderTo");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.TeleportToPrevious, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.TeleportToPrevious");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.StartSpectating, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.StartSpectating");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.EndSpectating, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.EndSpectating");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.EndSpectating_Finish, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.EndSpectating_Finish");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetCannotBeTargetedByAI, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetCannotBeTargetedByAI");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetGodMode, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetGodMode");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetFreeze, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetFreeze");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetInvisible, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetInvisible");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetReceiveDamageDealt, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetReceiveDamageDealt");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetRemoveCollision, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetRemoveCollision");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetUnlimitedAmmo, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetUnlimitedAmmo");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetUnlimitedStamina, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetUnlimitedStamina");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetAdminNVG, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetAdminNVG");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetBrokenLegs, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetBrokenLegs");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.Heal, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.Heal");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.Strip, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.Strip");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.ClearCargo, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.ClearCargo");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.Dry, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.Dry");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.Kick, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.Kick");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.KickMessage, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.KickMessage");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.Ban, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.Ban");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.BanMessage, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.BanMessage");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.Message, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.Message");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.Notif, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.Notif");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.StopBleeding, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.StopBleeding");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.Vomit, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.Vomit");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetScale, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetScale");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetPermissions, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SetPermissions");

	//! RPC_SetRoles's first read is array<string> - an empty buffer crashes
	//! that read the same way a bare string does (see JMAutoTestRPCFixtures.c).
	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SetRoles, JMAutoTest_ReadContextWithOneStringArray(new array<string>()));
	JMAutoTest_Canary("JMPlayerModuleRPC.SetRoles");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.VONStartedTransmitting, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.VONStartedTransmitting");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.VONStoppedTransmitting, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.VONStoppedTransmitting");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.AddDisease, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.AddDisease");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.RemoveDisease, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.RemoveDisease");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.RemoveAllDiseases, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.RemoveAllDiseases");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SendDiseaseMask, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SendDiseaseMask");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.AddBleedingPart, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.AddBleedingPart");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.StopBleedingPart, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.StopBleedingPart");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.SendBleedingState, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.SendBleedingState");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.RequestInventory, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.RequestInventory");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.InventoryDelete, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.InventoryDelete");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.InventoryRepair, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.InventoryRepair");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.InventoryTake, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.InventoryTake");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.RequestPlayerStats, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.RequestPlayerStats");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.InventoryModify, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.InventoryModify");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.InventoryGroupOp, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.InventoryGroupOp");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMPlayerModuleRPC.RequestExpansionInfo, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMPlayerModuleRPC.RequestExpansionInfo");

	JMAutoTest_CanaryCheck("JMPlayerModule full RPC sweep");
}

static void JMAutoTest_JMPlayerModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMPlayerModule - full per-RPC sweep, NULL sender");
	PrintFormat("================================================================");

	JMPlayerModule mod;
	if (!CF_Modules<JMPlayerModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMPlayerModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_PlayerModule_AllRPCsSurviveNullSender(mod);
}

#endif
