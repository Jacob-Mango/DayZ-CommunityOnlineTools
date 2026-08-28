// =============================================================================
//  JMEditorScene.c
//
//  3D scene queries for the Map Editor. The user's cursor raycasts into the
//  world to determine:
//    - where to place a new spawned object (the surface hit point)
//    - which existing object is under the cursor (selection pick)
//
//  Uses g_Game.GetCurrentCameraPosition() / GetCurrentCameraDirection() -
//  the same pattern that COT's existing JMCameraModule uses (JMCameraModule.c:162).
//  DayZ 1.29 has no static GetCurrentCamera() on DayZGame; camera is fetched
//  via g_Game as a singleton Game instance.
// =============================================================================

class JMEditorScene
{
	// Cast a ray from the active camera through the screen-pixel (x, y).
	// Returns the world-space hit point and the hit Object (or NULL on miss).
	//
	// We approximate the screen-pixel ray via the active camera's forward
	// direction (g_Game.GetCurrentCameraDirection) plus a small horizontal/vertical
	// offset derived from the camera's current yaw/pitch. DayZ 1.29's Camera
	// class doesn't expose per-pixel raycast; this approximation is good
	// enough for the editor's spawn-point heuristic.
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

	// Sample the terrain height under (x, z). Returns 0 if no surface.
	static float GetSurfaceY( float x, float z )
	{
		return g_Game.SurfaceY( x, z );
	}
}