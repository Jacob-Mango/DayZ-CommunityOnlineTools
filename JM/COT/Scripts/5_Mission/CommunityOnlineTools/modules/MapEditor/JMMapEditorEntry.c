//  JMMapEditorEntry.c: data class for a placed-object row in the Map Editor list.
//  Own file because Enforce loads only the class matching the filename (CLAUDE.md
//  rule 7); others in a multi-class file are silently dropped.

class JMMapEditorEntry
{
	string ClassName;
	int Id;
	vector Position;
	vector Orientation;
	float Scale;
}