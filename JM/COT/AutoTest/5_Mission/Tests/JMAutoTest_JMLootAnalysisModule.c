#ifdef JM_COT_AUTOTEST

//! Targeted unit test for JMLootAnalysisModule
//! (Scripts/5_Mission/CommunityOnlineTools/modules/LootAnalysis/JMLootAnalysisModule.c).
//!
//! This module is structurally different from every other module in this
//! test set: it does not use OnRPC / module-range dispatch at all. Its
//! constructor registers named RPCs directly via
//! GetRPCManager().AddRPC("JM_COT_RPC", "RPC_*", this, ...), and every
//! RPC_* handler method (RPC_RequestItemScan, RPC_DeleteAllItems, etc.) is
//! declared private on JMLootAnalysisModule. Enforce Script enforces
//! `private` across classes, so this test file - a different class - cannot
//! call those handlers directly, and GetRPCManager().SendRPC(...)'s
//! local-loopback behavior with no connected clients is unverified/unreliable
//! at boot time, so neither approach is used here.
//!
//! Instead this test calls the public static helper RPC_DeleteAllItems
//! itself delegates to: JMEntityTracker.DeleteAllEntities(string className)
//! (Scripts/4_World/CommunityOnlineTools/Classes/JMEntityTracker.c). Reading
//! its body confirms it iterates a snapshot from GetByClassname(className)
//! and returns the count of entities it deleted - for a classname with zero
//! matches (guaranteed by using a nonsense name) the loop body never runs and
//! it returns 0, a pure array scan with no null-deref or permission-check
//! hazard. No CF_Modules<T> fetch or module instance is needed for this test:
//! it calls a static/public utility directly, not the module instance or the
//! RPC layer.

static void JMAutoTest_JMLootAnalysisModule()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMLootAnalysisModule - JMEntityTracker.DeleteAllEntities unit test");
	PrintFormat("================================================================");

	JMAutoTest_Group("JMEntityTracker.DeleteAllEntities survives and returns 0 for a nonexistent classname");

	JMAutoTest_ExpectCanary();
	int deleteCount = JMEntityTracker.DeleteAllEntities("JMAutoTest_Nonexistent_Classname_ZZZ");
	JMAutoTest_Canary("JMEntityTracker.DeleteAllEntities(nonexistent classname)");

	JMAutoTest_AssertEqualInt("JMEntityTracker.DeleteAllEntities(nonexistent classname) deletes nothing", deleteCount, 0);

	JMAutoTest_CanaryCheck("JMLootAnalysisModule DeleteAllEntities unit test");
}

#endif
