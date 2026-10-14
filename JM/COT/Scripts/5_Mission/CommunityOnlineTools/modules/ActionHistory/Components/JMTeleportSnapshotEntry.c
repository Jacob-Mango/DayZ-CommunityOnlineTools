//  JMTeleportSnapshotEntry - global-stack undo/redo for a player teleport; companion to
//  JMTeleportHistory (ESP per-target rows keep its own stack), replayed by a blanket Ctrl+Z.
//  Only player teleports push one: a raw object position set shares its call with mouse-drag,
//  which has no "one gesture" boundary.
class JMTeleportSnapshotEntry: JMActionHistoryEntry
{
	protected PlayerBase m_Target;
	protected vector m_Position;

	void JMTeleportSnapshotEntry( PlayerBase target, vector previousPosition )
	{
		RequiredPermission = JMConstants.PERM_PLAYER_TELEPORT_POSITION;

		m_Target   = target;
		m_Position = previousPosition;
	}

	//! A player who disconnected is NULL here, and there is nobody to move back.
	override bool CanUndo()
	{
		return m_Target != NULL;
	}

	override bool CanRedo()
	{
		return m_Target != NULL;
	}

	//! Undo and Redo are the same swap in either direction: move the target
	//! to the stored position and remember where it just was, so the next
	//! press (whichever button it is) goes right back.
	override bool Undo()
	{
		if ( !m_Target )
			return false;

		vector current = m_Target.GetPosition();
		m_Target.SetPosition( m_Position );
		m_Position = current;

		return true;
	}

	override bool Redo()
	{
		return Undo();
	}

	override Object GetTarget()
	{
		return m_Target;
	}

	override void SetTarget( Object target )
	{
		Class.CastTo( m_Target, target );
	}

	override string GetDescription()
	{
		string name = "player";
		if ( m_Target && m_Target.GetIdentity() )
			name = m_Target.GetIdentity().GetName();

		return "Teleport " + name;
	}
}
