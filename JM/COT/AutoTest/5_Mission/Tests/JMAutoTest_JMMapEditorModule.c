#ifdef JM_COT_AUTOTEST

//! Full per-RPC sweep for JMMapEditorModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/MapEditor/JMMapEditorModule.c).
//!
//! Calls the module's own OnRPC(sender, target, rpc_type, ctx) once per
//! JMMapEditorModuleRPC value the switch statement actually handles, with
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
//! Notes on specific cases:
//! - CloneObject has a case in the switch, but its body is an intentional
//!   no-op on the server side (comment: the form's Paste button calls
//!   PasteCloned directly on the module; no server RPC is needed) - included
//!   here anyway since the switch does match and "handle" this rpc_type.
//! - JMMapEditorModuleRPC.UndoState is EXCLUDED from this sweep: per a
//!   comment in RPC.c it is reserved and deliberately never sent, and the
//!   switch in OnRPC has no case for it at all.
//! - List is normally server->client only but the switch does contain a case
//!   for it (routes to RPC_List), so it is included for completeness.

static void JMAutoTest_JMMapEditorModule_Sweep(JMMapEditorModule mod)
{
	JMAutoTest_Group("JMMapEditorModule.OnRPC(NULL, NULL, <rpc_type>, emptyCtx) - one call per handled rpc_type");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.RequestList, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.RequestList");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.SpawnObject, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.SpawnObject");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.TransformObject, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.TransformObject");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.DeleteObject, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.DeleteObject");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.ClearAll, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.ClearAll");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.BulkTransform, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.BulkTransform");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.BulkDelete, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.BulkDelete");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.CloneObject, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.CloneObject");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.Undo, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.Undo");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.Redo, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.Redo");

	JMAutoTest_ExpectCanary();
	mod.OnRPC(NULL, NULL, JMMapEditorModuleRPC.List, JMAutoTest_EmptyReadContext());
	JMAutoTest_Canary("JMMapEditorModuleRPC.List");
}

static void JMAutoTest_JMMapEditorModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMMapEditorModule full RPC sweep - every rpc_type, NULL sender");
	PrintFormat("================================================================");

	JMMapEditorModule mod;
	if (!CF_Modules<JMMapEditorModule>.Get(mod) || !mod)
	{
		JMAutoTest_Fail("JMMapEditorModule instance available via CF_Modules", "not constructed yet?");
		return;
	}

	JMAutoTest_JMMapEditorModule_Sweep(mod);

	JMAutoTest_CanaryCheck("JMMapEditorModule full RPC sweep");
}

#endif
