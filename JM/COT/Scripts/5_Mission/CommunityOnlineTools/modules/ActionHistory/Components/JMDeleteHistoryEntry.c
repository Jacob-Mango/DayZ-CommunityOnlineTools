//  JMDeleteHistoryEntry - undo/redo for ESP's generic object delete (undo restores
//  the object, see JMSnapshotHistoryEntry; redo deletes the stand-in again).
//  Must be constructed BEFORE the target is deleted - it reads the target's state.
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
