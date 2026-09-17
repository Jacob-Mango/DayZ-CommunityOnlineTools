// =============================================================================
//  JMActionHistory
//
//  A single chronological undo/redo stack spanning every action type that
//  pushes onto it, so a global Ctrl+Z always undoes whatever actually
//  happened last, regardless of which module did it - not "the last thing
//  this one module did", the way each module's own bespoke history (Map
//  Editor's, the teleport history's per-target one) works.
//
//  Server-side and global for the whole session, the same scope
//  JMMapEditorModule's own undo stack already uses - COT's admin tooling has
//  never modeled a separate history per admin, and this doesn't start now.
//
//  Usage:
//      JMActionHistory.Push( new JMHealHistoryEntry( target ) );  // before healing
//      JMActionHistory.Undo( senderIdentity );
//      JMActionHistory.Redo( senderIdentity );
// =============================================================================
class JMActionHistory
{
	static const int MAX_ENTRIES = 64;

	protected static ref array<ref JMActionHistoryEntry> s_Undo;
	protected static ref array<ref JMActionHistoryEntry> s_Redo;

	static void Push( JMActionHistoryEntry entry )
	{
		if ( !entry )
			return;

		if ( !s_Undo )
			s_Undo = new array<ref JMActionHistoryEntry>;

		s_Undo.Insert( entry );

		while ( s_Undo.Count() > MAX_ENTRIES )
			s_Undo.Remove( 0 );

		//! A fresh action invalidates whatever redo history was sitting there -
		//! the same rule every undo stack in this mod already follows.
		if ( s_Redo )
			s_Redo.Clear();
	}

	//! Undo the most recent action, provided `ident` still holds the
	//! permission that action itself required. Denied silently, same as any
	//! other permission-gated action in this mod, and the stack is left
	//! untouched so a later, permitted caller can still undo it.
	static void Undo( PlayerIdentity ident )
	{
		if ( !s_Undo || s_Undo.Count() == 0 )
			return;

		JMActionHistoryEntry entry = s_Undo[s_Undo.Count() - 1];

		JMPlayerInstance instance;
		if ( entry.RequiredPermission != "" && !GetPermissionsManager().HasPermission( entry.RequiredPermission, ident, instance ) )
			return;

		s_Undo.Remove( s_Undo.Count() - 1 );
		entry.Undo();

		if ( !s_Redo )
			s_Redo = new array<ref JMActionHistoryEntry>;
		s_Redo.Insert( entry );
	}

	static void Redo( PlayerIdentity ident )
	{
		if ( !s_Redo || s_Redo.Count() == 0 )
			return;

		JMActionHistoryEntry entry = s_Redo[s_Redo.Count() - 1];

		JMPlayerInstance instance;
		if ( entry.RequiredPermission != "" && !GetPermissionsManager().HasPermission( entry.RequiredPermission, ident, instance ) )
			return;

		s_Redo.Remove( s_Redo.Count() - 1 );
		entry.Redo();

		if ( !s_Undo )
			s_Undo = new array<ref JMActionHistoryEntry>;
		s_Undo.Insert( entry );
	}
}
