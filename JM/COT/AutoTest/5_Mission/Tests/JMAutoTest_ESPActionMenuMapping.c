#ifdef JM_COT_AUTOTEST

//! Target: JMESPModule.PermissionForAction (JMESPModule.c) - a pure
//! string-table lookup with no identity, no RPC, no widget involved. The
//! switch has no `default:` case, falling through to `return "";` for
//! anything unmatched (RPC_ObjectAction, JMESPModule.c). INVALID and COUNT
//! (JMESPObjectAction, RPC.c) are sentinels, not real actions - every value
//! strictly between them must map to a non-empty permission string, or a
//! context-menu action silently has NO server-side permission gate at all -
//! a real, exploitable gap this test exists to catch.

static void JMAutoTest_ESPActionMenuMapping()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMESPModule.PermissionForAction completeness");
	PrintFormat("================================================================");

	JMAutoTest_Group("Every JMESPObjectAction value (except INVALID/COUNT sentinels) maps to a non-empty permission string");

	for (int action = JMESPObjectAction.INVALID + 1; action < JMESPObjectAction.COUNT; action++)
	{
		string permission = JMESPModule.PermissionForAction(action);
		JMAutoTest_AssertFalse("JMESPObjectAction id=" + action + " has a permission mapping (not empty)", permission == "");
	}
}

#endif
