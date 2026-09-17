// =============================================================================
//  JMActionHistoryEntry
//
//  One undoable/redoable action on JMActionHistory's shared stack. A subclass
//  captures whatever "before" state its action type needs at construction
//  time (before the action itself runs) and implements the inverse in
//  Undo(); Redo() re-applies the original action.
// =============================================================================
class JMActionHistoryEntry
{
	//! Checked against the caller before Undo/Redo runs - the SAME permission
	//! the original action itself needed, since undoing it is not a new grant
	//! of capability, just replaying an admin's own already-authorized
	//! change. Left "" only refuses nobody, which no entry here should do.
	string RequiredPermission;

	void Undo() {}
	void Redo() {}
}
