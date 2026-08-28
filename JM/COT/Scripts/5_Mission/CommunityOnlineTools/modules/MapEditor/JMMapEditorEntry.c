// =============================================================================
//  JMMapEditorEntry.c
//
//  Plain data class for a placed-object row in the Map Editor's left list
//  panel. Lives in its own file because Enforce Script loads only the class
//  matching the filename (CLAUDE.md rule 7); everything else in a multi-class
//  file is silently dropped.
// =============================================================================

class JMMapEditorEntry
{
	string ClassName;
	int Id;
	vector Position;
	vector Orientation;
	float Scale;
}