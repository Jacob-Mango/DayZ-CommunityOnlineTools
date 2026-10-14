//  JMSpawnHistoryEntry - undo/redo for an Object Spawner world spawn: undo removes what was
//  spawned, redo puts it back (mirror of JMDeleteHistoryEntry, see JMSnapshotHistoryEntry).
//  Constructed AFTER the spawn, from the spawned object. Only ground spawns push one: an item
//  spawned into an inventory has no world position to restore.
class JMSpawnHistoryEntry: JMSnapshotHistoryEntry
{
	void JMSpawnHistoryEntry( Object spawned )
	{
		RequiredPermission = JMConstants.PERM_ENTITY_SPAWN_POSITION;

		if ( !spawned )
			return;

		Class.CastTo( m_Live, spawned );

		Capture( spawned );
	}

	override bool CanUndo()
	{
		return m_Live != NULL;
	}

	override bool CanRedo()
	{
		return !m_Live && HasSnapshot();
	}

	override bool Undo()
	{
		return Remove();
	}

	override bool Redo()
	{
		return Restore();
	}

	override string GetDescription()
	{
		return "Spawn " + m_ClassName;
	}
}
