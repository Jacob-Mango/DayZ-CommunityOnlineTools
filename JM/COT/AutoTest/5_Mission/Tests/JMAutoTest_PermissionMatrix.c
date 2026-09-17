#ifdef JM_COT_AUTOTEST

//! Target: every module's HasAccess() permission string, tested against the
//! underlying JMPlayerInstance/JMPermission tree directly (JMPlayerInstance.c,
//! JMPermission.c) rather than against HasAccess() itself.
//!
//! WHY NOT HasAccess() DIRECTLY: HasAccess() calls the CLIENT-side
//! JMPermissionManager.HasPermission(string) overload, which unconditionally
//! returns true when IsMissionHost() - always true in this boot-time server
//! context - before ever consulting the permission tree, and asserts/errors if
//! called when !IsMissionClient(). Testing HasAccess() itself in this
//! environment would either always trivially pass or error. Testing the tree
//! logic directly (JMPlayerInstance.HasPermission, no host-bypass, no
//! client/server branch) exercises the real decision logic every module's
//! HasAccess() ultimately delegates to, honestly - this is the answer to
//! "can player see/use module X with/without permission Y" within what a
//! headless boot-time test can actually prove (see JMAutoTest_PermissionModel.c
//! for the underlying tri-state tree semantics).
//!
//! Every permission string below must already be registered by its owning
//! module's constructor - by mission start, when this boot pass runs, every
//! module has already run its own constructor. Each string was re-verified
//! against its module's actual HasAccess() source this session.
//!
//! Action-layer note: client-side action/permission checks (UIActionBase,
//! Actions.QuickActions, JMESPActionMenu's Perm() wrapper) are cosmetic only -
//! the real enforcement boundary is the server-side RPC handler, covered by
//! the per-module JMAutoTest_<Module>.c suites, not by this matrix. No
//! separate "action" or "quick action" suite exists for that reason.

static void JMAutoTest_PermMatrix_Row(string moduleName, string permission, bool hasGate = true)
{
	if (!hasGate)
	{
		JMAutoTest_Pass(moduleName + " - documented as ungated (HasAccess() always true), not a permission-tree row");
		return;
	}

	JMAutoTest_AssertPermissionGrantable(moduleName + " (" + permission + ")", permission);
	JMAutoTest_AssertPermissionDenyable(moduleName + " (" + permission + ")", permission);
}

static void JMAutoTest_PermissionMatrix()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: Permission matrix - one row per module HasAccess() gate");
	PrintFormat("================================================================");

	JMAutoTest_Group("Module HasAccess() permission strings - ALLOW/DISALLOW round trip");

	JMAutoTest_PermMatrix_Row("JMAntiCheatModule", "Admin.AntiCheat.View");
	JMAutoTest_PermMatrix_Row("JMBanModule", "Admin.Ban.View");
	JMAutoTest_PermMatrix_Row("JMCameraModule", "Camera.View");
	JMAutoTest_PermMatrix_Row("JMWebhookCOTModule", "Webhook.View");
	JMAutoTest_PermMatrix_Row("JMMapModule", "Admin.Map.View");
	JMAutoTest_PermMatrix_Row("JMESPModule", "ESP.View");
	JMAutoTest_PermMatrix_Row("JMItemStatsModule", "ItemStats.View");
	JMAutoTest_PermMatrix_Row("JMCompensationsModule", "Compensations.View");
	JMAutoTest_PermMatrix_Row("JMWeatherModule", "Weather.View");
	JMAutoTest_PermMatrix_Row("JMObjectSpawnerModule", "Entity.View");
	JMAutoTest_PermMatrix_Row("JMRoleManagerModule", "Admin.Roles.View");
	JMAutoTest_PermMatrix_Row("JMLoadoutModule", "Loadouts.View");
	JMAutoTest_PermMatrix_Row("JMLootAnalysisModule", "Admin.LootAnalysis.View");
	JMAutoTest_PermMatrix_Row("JMVehiclesModule", "Vehicles.View");
	//! JMNamalskEventManagerModule deliberately excluded: its constructor
	//! (which registers "Namalsk.View") only runs on the Namalsk map, never
	//! on this project's Chernarus test map - AddPermission's
	//! requireRegistered check silently no-ops for an unregistered string,
	//! making this row fail here for reasons unrelated to the permission
	//! model itself (confirmed - see JMAutoTest_SafetyProbe.c's identical
	//! map-conditional skip for the same module's RPC sweep).
	JMAutoTest_PermMatrix_Row("JMTerritoryModule", "Expansion.Territory.View");
	JMAutoTest_PermMatrix_Row("JMMapEditorModule", "Admin.MapEditor.View");
	JMAutoTest_PermMatrix_Row("JMTeleportModule", "Admin.Player.Teleport.View");
	JMAutoTest_PermMatrix_Row("JMPlayerModule", "", false); // always true, no gate (JMPlayerModule.c:125-128)

	JMAutoTest_Group("A subset of individual RPC-level permission strings (JMPlayerModule)");

	//! Representative sample of JMPlayerModule's ~30 distinct registered
	//! permission strings - the per-module RPC-safety suites cover the
	//! handlers themselves; this exercises the underlying permission-string
	//! grant/deny round trip for a few of the highest-impact ones.
	JMAutoTest_PermMatrix_Row("JMPlayerModule.RPC_Kick", "Admin.Player.Kick");
	JMAutoTest_PermMatrix_Row("JMPlayerModule.RPC_Ban", "Admin.Player.Ban");
	JMAutoTest_PermMatrix_Row("JMPlayerModule.RPC_Message", "Admin.Player.Message");
	JMAutoTest_PermMatrix_Row("JMPlayerModule.RPC_Notif", "Admin.Player.Notif");
	JMAutoTest_PermMatrix_Row("JMPlayerModule.RPC_SetGodMode", "Admin.Player.GodMode");

	JMAutoTest_Group("A subset of ESP object-action permission strings");

	JMAutoTest_PermMatrix_Row("JMESPModule.ESP.Object.Delete", "ESP.Object.Delete");
	JMAutoTest_PermMatrix_Row("JMESPModule.ESP.Object.SetHealth", "ESP.Object.SetHealth");
	JMAutoTest_PermMatrix_Row("JMESPModule.ESP.Object.DeleteAll", "ESP.Object.DeleteAll");
}

#endif
