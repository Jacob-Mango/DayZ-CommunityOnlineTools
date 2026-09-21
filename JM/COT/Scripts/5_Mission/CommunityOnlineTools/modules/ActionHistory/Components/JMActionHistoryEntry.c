// =============================================================================
//  JMActionHistoryEntry
//
//  One undoable/redoable action on JMActionHistory's shared stack. A subclass
//  captures whatever "before" state its action type needs at construction
//  time (before the action itself runs) and implements the inverse in
//  Undo(); Redo() re-applies the original action.
//
//  A sub-mod adds its own action type by extending this class, setting
//  RequiredPermission in the constructor, and pushing an instance from the
//  server-side handler that performs the action:
//
//      JMActionHistory.Push( new MyEntry( target ), JMActionHistory.OwnerOf( ident ) );
// =============================================================================
class JMActionHistoryEntry
{
	//! Checked against the caller before Undo/Redo runs - the SAME permission
	//! the original action itself needed, since undoing it is not a new grant
	//! of capability, just replaying an admin's own already-authorized
	//! change. MUST be set: an entry left "" is refused by Push and can never
	//! be undone, rather than being undoable by anybody.
	string RequiredPermission;

	//! Who performed the action - filled in by JMActionHistory.Push. "" means
	//! a local (offline) session or a system action. Without the
	//! ActionHistory.UndoOthers permission a caller can only step back
	//! entries they own.
	string OwnerId;

	//! g_Game.GetTickTime() of the last change to this entry; TryMerge uses it
	//! to coalesce a burst of the same edit into one step.
	float Time;

	void JMActionHistoryEntry()
	{
		Time = g_Game.GetTickTime();
	}

	//! Short human text for notifications and logs, e.g. "Heal Barrel_Blue".
	string GetDescription()
	{
		return "action";
	}

	//! The world object this entry acts on, or NULL. Lets JMActionHistory find
	//! every entry that references an object when that object is deleted and
	//! later recreated by an undo - see CollectTargeting / SetTarget.
	Object GetTarget()
	{
		return NULL;
	}

	//! Point this entry at a replacement for its target (a deleted object that
	//! an undo just brought back is a NEW object, so older entries would
	//! otherwise be left holding a dead reference).
	void SetTarget( Object target )
	{
	}

	//! Return false when the inverse could not be applied (target gone, a
	//! dependency missing). The entry is then dropped instead of being kept
	//! and silently retried on every press.
	bool Undo()
	{
		return false;
	}

	bool Redo()
	{
		return false;
	}

	//! Whether Undo()/Redo() could do anything right now. A false here makes
	//! JMActionHistory skip the entry - without consuming it - and try the one
	//! below, so an entry whose target was deleted does not swallow a keypress.
	bool CanUndo()
	{
		return true;
	}

	bool CanRedo()
	{
		return true;
	}

	//! Add this entry (or, for a grouping entry, the children) to `results` when
	//! it acts on `target`.
	void CollectTargeting( Object target, array<JMActionHistoryEntry> results )
	{
		if ( target && GetTarget() == target )
			results.Insert( this );
	}

	//! Called on the most recent entry (same owner) when `incoming` is pushed.
	//! Return true to absorb it - the incoming entry is then discarded - so a
	//! slider dragged through ten values is one undo step, not ten.
	bool TryMerge( JMActionHistoryEntry incoming )
	{
		return false;
	}

	//! "Barrel_Blue" for an object, or "object" when it is gone.
	static string DescribeObject( Object obj )
	{
		if ( !obj )
			return "object";

		return obj.GetType();
	}
}
