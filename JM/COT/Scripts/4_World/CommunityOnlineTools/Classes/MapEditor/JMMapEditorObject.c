// =============================================================================
//  JMMapEditorObject.c
//
//  Per-object record used by the Map Editor module. Holds the spawn-time
//  parameters needed to re-create the object on server load and to drive
//  live transform RPCs. Plain serialisable class (no refs).
// =============================================================================

class JMMapEditorObject
{
	string ClassName;
	vector Position;
	vector Orientation;
	float Scale;
	int Id;            // server-assigned unique id within this session

	void JMMapEditorObject()
	{
		Scale = 1.0;
		Id = -1;
	}

	void Set( string className, vector pos, vector ori, float scale, int id )
	{
		ClassName   = className;
		Position    = pos;
		Orientation = ori;
		Scale       = scale;
		Id          = id;
	}
}