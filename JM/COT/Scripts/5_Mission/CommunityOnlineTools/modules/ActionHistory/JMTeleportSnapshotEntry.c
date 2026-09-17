// =============================================================================
//  JMTeleportSnapshotEntry
//
//  Undo/redo for a player teleport, on the shared global stack. This is a
//  companion to JMTeleportHistory, not a replacement - the per-target
//  Undo/Redo rows on the ESP menu still walk JMTeleportHistory's own stack
//  unchanged; this is only what a blanket Ctrl+Z replays, and it moves the
//  player directly rather than touching that history at all.
//
//  Only player teleports push one of these (see JMTeleportModule.Server_
//  Position) - a raw object position set shares its server call with
//  continuous mouse-drag, which has no natural "one gesture" boundary to
//  record a single undo step at.
// =============================================================================
class JMTeleportSnapshotEntry: JMActionHistoryEntry
{
	protected PlayerBase m_Target;
	protected vector m_Position;

	void JMTeleportSnapshotEntry( PlayerBase target, vector previousPosition )
	{
		RequiredPermission = "Admin.Player.Teleport.Position";

		m_Target   = target;
		m_Position = previousPosition;
	}

	//! Undo and Redo are the same swap in either direction: move the target
	//! to the stored position and remember where it just was, so the next
	//! press (whichever button it is) goes right back.
	override void Undo()
	{
		if ( !m_Target )
			return;

		vector current = m_Target.GetPosition();
		m_Target.SetPosition( m_Position );
		m_Position = current;
	}

	override void Redo()
	{
		Undo();
	}
}
