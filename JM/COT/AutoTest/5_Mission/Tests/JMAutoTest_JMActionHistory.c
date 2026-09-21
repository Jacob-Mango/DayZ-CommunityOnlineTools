#ifdef JM_COT_AUTOTEST

//! JMActionHistory stack rules + the JMActionHistoryModule RPC surface.
//!
//! The stack tests never depend on whether the boot-time caller (NULL identity)
//! is allowed to undo: a refused step hands back the entry it refused as
//! `performed`, so the assertions read WHICH entry the stack chose, which is
//! the property under test, not the permission outcome.

//! A do-nothing entry with a settable label and permission.
class JMAutoTest_HistoryEntry: JMActionHistoryEntry
{
	string m_Label;
	bool m_MergeAll;
	int m_Undone;

	void JMAutoTest_HistoryEntry( string label, string permission )
	{
		m_Label = label;
		RequiredPermission = permission;
	}

	override bool Undo()
	{
		m_Undone++;
		return true;
	}

	override bool Redo()
	{
		return true;
	}

	override string GetDescription()
	{
		return m_Label;
	}

	override bool TryMerge( JMActionHistoryEntry incoming )
	{
		return m_MergeAll;
	}
}

static void JMAutoTest_JMActionHistory()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMActionHistory stack rules + JMActionHistoryModule");
	PrintFormat("================================================================");

	JMActionHistoryEntry performed;
	int skipped;
	JMActionHistoryResult result;
	int i;

	JMAutoTest_Group("Push refuses an entry with no RequiredPermission (fail closed)");
	JMActionHistory.Clear();
	JMActionHistory.Push( new JMAutoTest_HistoryEntry( "unguarded", "" ) );
	result = JMActionHistory.Undo( NULL, performed, skipped );
	JMAutoTest_Assert( "refused entry never reaches the stack", result == JMActionHistoryResult.EMPTY );

	JMAutoTest_Group("Newest entry is stepped first, also after the stack trims");
	JMActionHistory.Clear();
	for ( i = 0; i < JMActionHistory.MAX_ENTRIES + 6; i++ )
		JMActionHistory.Push( new JMAutoTest_HistoryEntry( "e" + i, JMConstants.PERM_ESP_OBJECT_DELETE ) );

	JMActionHistory.Undo( NULL, performed, skipped );
	JMAutoTest_Assert( "an entry was chosen", performed != NULL );
	if ( performed )
		JMAutoTest_AssertEqualStr( "trim keeps chronological order (newest first)", performed.GetDescription(), "e" + ( JMActionHistory.MAX_ENTRIES + 5 ) );

	JMAutoTest_Group("A merge absorbs the incoming entry instead of stacking it");
	JMActionHistory.Clear();
	JMAutoTest_HistoryEntry first = new JMAutoTest_HistoryEntry( "first", JMConstants.PERM_ESP_OBJECT_DELETE );
	first.m_MergeAll = true;
	JMActionHistory.Push( first );
	JMActionHistory.Push( new JMAutoTest_HistoryEntry( "second", JMConstants.PERM_ESP_OBJECT_DELETE ) );
	JMActionHistory.Undo( NULL, performed, skipped );
	if ( performed )
		JMAutoTest_AssertEqualStr( "merged entry is the one on the stack", performed.GetDescription(), "first" );

	JMAutoTest_Group("A composite steps its children and reports its size");
	JMCompositeHistoryEntry group = new JMCompositeHistoryEntry( JMConstants.PERM_ESP_OBJECT_DELETEALL, "Delete" );
	JMAutoTest_HistoryEntry childA = new JMAutoTest_HistoryEntry( "a", JMConstants.PERM_ESP_OBJECT_DELETE );
	JMAutoTest_HistoryEntry childB = new JMAutoTest_HistoryEntry( "b", JMConstants.PERM_ESP_OBJECT_DELETE );
	group.Add( childA );
	group.Add( childB );
	JMAutoTest_AssertEqualStr( "description carries the count", group.GetDescription(), "Delete (2)" );
	JMAutoTest_Assert( "composite undo succeeds when a child does", group.Undo() );
	JMAutoTest_AssertEqualInt( "child a undone once", childA.m_Undone, 1 );
	JMAutoTest_AssertEqualInt( "child b undone once", childB.m_Undone, 1 );

	JMActionHistory.Clear();

	JMActionHistoryModule mod;
	if ( !CF_Modules<JMActionHistoryModule>.Get( mod ) || !mod )
	{
		JMAutoTest_Fail( "JMActionHistoryModule instance available via CF_Modules", "not constructed yet?" );
		return;
	}

	JMAutoTest_Group("OnRPC(NULL, NULL, <id>, emptyCtx) survives for every handled rpc_type");

	JMAutoTest_ExpectCanary();
	mod.OnRPC( NULL, NULL, JMActionHistoryModuleRPC.Undo, JMAutoTest_EmptyReadContext() );
	JMAutoTest_Canary( "JMActionHistoryModuleRPC.Undo" );

	JMAutoTest_ExpectCanary();
	mod.OnRPC( NULL, NULL, JMActionHistoryModuleRPC.Redo, JMAutoTest_EmptyReadContext() );
	JMAutoTest_Canary( "JMActionHistoryModuleRPC.Redo" );

	JMAutoTest_CanaryCheck( "JMActionHistoryModule RPC sweep" );
}

#endif
