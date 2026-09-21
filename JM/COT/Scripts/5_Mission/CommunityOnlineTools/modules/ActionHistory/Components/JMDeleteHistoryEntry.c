// =============================================================================
//  JMDeleteHistoryEntry
//
//  Undo/redo for ESP's generic object delete: undo brings the object back
//  (see JMSnapshotHistoryEntry for what is captured), redo deletes the
//  restored stand-in again.
//
//  Must be constructed BEFORE the target is actually deleted - it reads the
//  target's own state to know what to bring back.
// =============================================================================
class JMDeleteHistoryEntry: JMSnapshotHistoryEntry
{
	void JMDeleteHistoryEntry( Object target )
	{
		RequiredPermission = JMConstants.PERM_ESP_OBJECT_DELETE;

		if ( !target )
			return;

		Capture( target );
		CollectDependents( target );
	}

	override bool CanUndo()
	{
		return !m_Live && HasSnapshot();
	}

	override bool CanRedo()
	{
		return m_Live != NULL;
	}

	override bool Undo()
	{
		return Restore();
	}

	override bool Redo()
	{
		return Remove();
	}

	override string GetDescription()
	{
		return "Delete " + m_ClassName;
	}
}
