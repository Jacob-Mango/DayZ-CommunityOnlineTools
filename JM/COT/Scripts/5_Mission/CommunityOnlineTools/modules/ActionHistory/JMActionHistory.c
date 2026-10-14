//  JMActionHistory: one chronological, server-side, session-global undo/redo stack across
//  all action types, so Ctrl+Z undoes whatever happened last, whichever module did it.
//  Each entry records its owner: callers undo their OWN last action; only ActionHistory.UndoOthers
//  can touch another admin's. Push before acting; Undo/Redo are driven by JMActionHistoryModule.
enum JMActionHistoryResult
{
	//! Stepped an entry; `entry` is what was undone/redone.
	OK,
	//! Nothing the caller may step - the stack is empty, holds only other
	//! admins' entries, or every remaining entry has lost its target.
	EMPTY,
	//! The caller lacks the permission the action itself required. `entry` is
	//! the one that was refused; the stack is untouched.
	DENIED,
	//! The entry could not be applied (target gone, dependency missing). It has
	//! been dropped. `entry` is the one that failed.
	FAILED
}

class JMActionHistory
{
	static const int MAX_ENTRIES = 64;
	protected static ref array<ref JMActionHistoryEntry> s_Undo;
	protected static ref array<ref JMActionHistoryEntry> s_Redo;

	//! Stack owner id for a network caller. "" offline, where there is exactly
	//! one admin and no identity to tell them apart.
	static string OwnerOf( PlayerIdentity ident )
	{
		if ( !ident || IsMissionOffline() )
			return "";

		return ident.GetId();
	}

	//! Same id, for a handler that has the admin's JMPlayerInstance instead.
	static string OwnerOfInstance( JMPlayerInstance instance )
	{
		if ( !instance || IsMissionOffline() )
			return "";

		return instance.GetGUID();
	}

	static void Push( JMActionHistoryEntry entry, string ownerId = "" )
	{
		if ( !entry )
			return;

		//! Fail closed: an entry that names no permission would be undoable by
		//! any client that can send the undo RPC.
		if ( entry.RequiredPermission == "" )
		{
			CF_Log.Error( "JMActionHistory: %1 was pushed without a RequiredPermission and was refused", entry.Type().ToString() );
			return;
		}

		EnsureStacks();

		entry.OwnerId = ownerId;

		bool merged;
		if ( s_Undo.Count() > 0 )
		{
			JMActionHistoryEntry top = s_Undo[s_Undo.Count() - 1];
			if ( top.OwnerId == ownerId && top.TryMerge( entry ) )
				merged = true;
		}

		if ( !merged )
		{
			s_Undo.Insert( entry );
			Trim( s_Undo );
		}

		//! A fresh action invalidates that admin's redo history - the same
		//! rule every undo stack in this mod already follows. Another admin's
		//! redo entries are theirs and stay.
		for ( int i = s_Redo.Count() - 1; i >= 0; i-- )
		{
			if ( s_Redo[i].OwnerId == ownerId )
				s_Redo.RemoveOrdered( i );
		}
	}

	//! Undo the caller's most recent action. `skipped` counts entries passed
	//! over because their target no longer exists. The stack is left alone on
	//! DENIED, so a later, permitted caller can still undo it.
	static JMActionHistoryResult Undo( PlayerIdentity ident, out JMActionHistoryEntry entry, out int skipped )
	{
		return Perform( true, ident, entry, skipped );
	}

	static JMActionHistoryResult Redo( PlayerIdentity ident, out JMActionHistoryEntry entry, out int skipped )
	{
		return Perform( false, ident, entry, skipped );
	}

	//! Forget everything. Entries hold weak references to world objects, so
	//! nothing here needs releasing - this is for a clean slate (tests).
	static void Clear()
	{
		if ( s_Undo )
			s_Undo.Clear();

		if ( s_Redo )
			s_Redo.Clear();
	}

	//! Every entry, on either stack, that acts on `target` - so a delete can
	//! tell the entries below it to follow the object when an undo recreates it.
	static void FindEntriesTargeting( Object target, array<JMActionHistoryEntry> results )
	{
		if ( !target )
			return;

		int i;

		if ( s_Undo )
		{
			for ( i = 0; i < s_Undo.Count(); i++ )
				s_Undo[i].CollectTargeting( target, results );
		}

		if ( s_Redo )
		{
			for ( i = 0; i < s_Redo.Count(); i++ )
				s_Redo[i].CollectTargeting( target, results );
		}
	}

	protected static void EnsureStacks()
	{
		if ( !s_Undo )
			s_Undo = new array<ref JMActionHistoryEntry>;

		if ( !s_Redo )
			s_Redo = new array<ref JMActionHistoryEntry>;
	}

	//! RemoveOrdered, never Remove: array.Remove swaps the LAST element into
	//! the freed slot, which on a chronological stack would put the newest
	//! action at the bottom.
	protected static void Trim( array<ref JMActionHistoryEntry> stack )
	{
		while ( stack.Count() > MAX_ENTRIES )
			stack.RemoveOrdered( 0 );
	}

	protected static JMActionHistoryResult Perform( bool undo, PlayerIdentity ident, out JMActionHistoryEntry performed, out int skipped )
	{
		performed = NULL;
		skipped = 0;

		EnsureStacks();

		array<ref JMActionHistoryEntry> source;
		array<ref JMActionHistoryEntry> destination;
		if ( undo )
		{
			source = s_Undo;
			destination = s_Redo;
		} else
		{
			source = s_Redo;
			destination = s_Undo;
		}

		string caller = OwnerOf( ident );

		//! Has, not HasRPC: this is a probe for which entries the caller may
		//! see, and a role without the permission is the normal case - it must
		//! not be reported to anti-cheat as a denied attempt.
		bool others = JMPermissions.Has( JMConstants.PERM_ACTIONHISTORY_UNDOOTHERS, ident );

		JMActionHistoryEntry candidate;
		bool applicable;
		bool applied;

		for ( int i = source.Count() - 1; i >= 0; i-- )
		{
			candidate = source[i];

			if ( candidate.OwnerId != caller && !others )
				continue;

			if ( undo )
				applicable = candidate.CanUndo();
			else
				applicable = candidate.CanRedo();

			if ( !applicable )
			{
				skipped++;
				continue;
			}

			//! HasRPC: on a denial it also tells anti-cheat, like every other
			//! server-side permission gate.
			if ( candidate.RequiredPermission == "" || !JMPermissions.HasRPC( candidate.RequiredPermission, ident ) )
			{
				performed = candidate;
				return JMActionHistoryResult.DENIED;
			}

			source.RemoveOrdered( i );
			performed = candidate;

			if ( undo )
				applied = candidate.Undo();
			else
				applied = candidate.Redo();

			if ( !applied )
				return JMActionHistoryResult.FAILED;

			destination.Insert( candidate );
			Trim( destination );

			return JMActionHistoryResult.OK;
		}

		return JMActionHistoryResult.EMPTY;
	}
}
