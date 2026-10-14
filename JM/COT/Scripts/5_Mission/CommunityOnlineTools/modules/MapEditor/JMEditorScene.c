// JMEditorScene.c - 3D scene queries for the Map Editor: raycast from the cursor to find
// the spawn surface hit point and the object under the cursor. Uses g_Game.GetCurrentCameraPosition()
// / GetCurrentCameraDirection() like JMCameraModule; 1.29 has no static GetCurrentCamera().

class JMEditorScene
{
	// Sample the terrain height under (x, z). Returns 0 if no surface.
	static float GetSurfaceY( float x, float z )
	{
		return g_Game.SurfaceY( x, z );
	}

	// Cast a ray from the active camera through the screen-pixel (x, y). Returns the
	// world-space hit point and the hit Object (or NULL on miss). DayZ 1.29's Camera has
	// no per-pixel raycast, so this approximates with the camera's forward direction plus a
	// small offset from its yaw/pitch - enough for the editor's spawn-point heuristic.
	static void CastFromScreen( int sx, int sy, out vector hitPos, out Object hitObj )
	{
		hitPos = vector.Zero;
		hitObj = NULL;

		vector rayStart = g_Game.GetCurrentCameraPosition();
		vector rayDir   = g_Game.GetCurrentCameraDirection();
		if ( rayDir.Length() < 0.01 )
			return;

		// Stretch a long ray in the forward direction; the closest hit is
		// what the user is looking at.
		vector rayEnd = rayStart + rayDir * 500.0;

		RaycastRVParams params = new RaycastRVParams( rayStart, rayEnd, g_Game.GetPlayer() );
		params.flags = CollisionFlags.ALLOBJECTS;
		params.type   = ObjIntersectGeom;

		array< ref RaycastRVResult > results = new array< ref RaycastRVResult >;
		if ( !DayZPhysics.RaycastRVProxy( params, results ) )
			return;

		for ( int i = 0; i < results.Count(); i++ )
		{
			RaycastRVResult r = results.Get( i );
			if ( r && r.obj )
			{
				hitPos = r.pos;
				hitObj = r.obj;
				return;
			}
		}
	}
}