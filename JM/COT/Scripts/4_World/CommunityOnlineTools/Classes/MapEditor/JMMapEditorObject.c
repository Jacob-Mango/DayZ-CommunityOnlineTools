//  JMMapEditorObject - per-object Map Editor record: spawn-time params to
//  re-create the object on server load and drive transform RPCs. Plain
//  serialisable class (no refs).

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