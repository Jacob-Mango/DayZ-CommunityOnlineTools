// =============================================================================
//  JMMapEditorUndoEntry.c
//
//  Per-mutation snapshot for the map editor's undo/redo stack. Captures the
//  state of one or more objects at the moment a mutation fires so a replay
//  restores them. Server-side only.
// =============================================================================

class JMMapEditorUndoEntry
{
	string Type;        // "Transform" | "Spawn" | "Delete" | "Bulk" | "Clone"
	ref array< ref JMMapEditorObject > Pre;   // snapshot before the mutation
	ref array< ref JMMapEditorObject > Post;  // snapshot after the mutation (for redo)

	void JMMapEditorUndoEntry()
	{
		Pre  = new array< ref JMMapEditorObject >;
		Post = new array< ref JMMapEditorObject >;
	}
}