// =============================================================================
//  JMEditorGizmos.c
//
//  Renders 3D transform handles for the selected object in the Map Editor:
//
//    MOVE    -- three axis arrows (X red, Y green, Z blue) starting at the
//               object origin and pointing along each axis. 1 m long.
//    ROTATE  -- a Y-axis ring at the object origin, 1.5 m radius.
//    SCALE   -- a unit-cube outline centred on the origin, edges coloured
//               per-axis.
//
//  Rendering uses the vanilla DayZ Debug.DrawLine / Debug.DrawArrow API
//  (the same one COT's JMESPModule uses for its bone overlay). These take
//  ShapeFlags for one-shot vs persistent, which the form's per-frame Draw()
//  call refreshes each tick.
//
//  Hit testing is screen-space: a click position is compared against the
//  projected screen position of each axis's tip via g_Game.GetScreenPos.
//  The currently-hovered axis is set so the form can render a hover
//  highlight (brighter colour).
// =============================================================================

class JMEditorGizmos
{
	static const int AXIS_NONE  = -1;
	static const int AXIS_X     = 0;
	static const int AXIS_Y     = 1;
	static const int AXIS_Z     = 2;
	static const int AXIS_ROTY  = 3;
	static const int AXIS_SCALE = 4;

	// Modes
	static const int MODE_MOVE   = 0;
	static const int MODE_ROTATE = 1;
	static const int MODE_SCALE  = 2;

	vector m_Position;
	vector m_Orientation;
	float  m_Scale;

	int    m_Mode;
	int    m_HoveredAxis;

	void JMEditorGizmos()
	{
		m_Position    = vector.Zero;
		m_Orientation = vector.Zero;
		m_Scale       = 1.0;
		m_Mode        = -1;
		m_HoveredAxis = AXIS_NONE;
	}

	// Per-axis arrows
	void DrawMoveGizmo()
	{
		float s = m_Scale;
		vector tipX = m_Position + Vector( 1.0 * s, 0, 0 );
		vector tipY = m_Position + Vector( 0, 1.0 * s, 0 );
		vector tipZ = m_Position + Vector( 0, 0, 1.0 * s );

		int colX = ARGB( 200, 220,  60,  60 );
		int colY = ARGB( 200,  60, 220,  60 );
		int colZ = ARGB( 200,  60,  60, 220 );

		if ( m_HoveredAxis == AXIS_X ) colX = ARGB( 255, 255, 120, 120 );
		if ( m_HoveredAxis == AXIS_Y ) colY = ARGB( 255, 120, 255, 120 );
		if ( m_HoveredAxis == AXIS_Z ) colZ = ARGB( 255, 120, 120, 255 );

		// Debug.DrawArrow(from, to, size, color, ShapeFlags). ShapeFlags.ONCE
		// tells the engine to clear the shape on next frame; the form's
		// per-frame Draw() repaints each tick.
		Debug.DrawArrow( m_Position, tipX, 0.15, colX, ShapeFlags.ONCE );
		Debug.DrawArrow( m_Position, tipY, 0.15, colY, ShapeFlags.ONCE );
		Debug.DrawArrow( m_Position, tipZ, 0.15, colZ, ShapeFlags.ONCE );
	}

	void DrawRotateGizmo()
	{
		int colour = ARGB( 200, 220, 220,  60 );
		if ( m_HoveredAxis == AXIS_ROTY )
			colour = ARGB( 255, 255, 255,  60 );

		float r = 1.5 * Math.Max( m_Scale, 0.2 );
		int segs = 32;
		for ( int i = 0; i < segs; i++ )
		{
			float a1 = ( i     / segs ) * 6.2831853;
			float a2 = ( (i+1) / segs ) * 6.2831853;
			vector p1 = m_Position + Vector( Math.Cos(a1) * r, 0, Math.Sin(a1) * r );
			vector p2 = m_Position + Vector( Math.Cos(a2) * r, 0, Math.Sin(a2) * r );
			Debug.DrawLine( p1, p2, colour, ShapeFlags.ONCE );
		}
	}

	void DrawScaleGizmo()
	{
		float h = 0.6 * Math.Max( m_Scale, 0.3 );
		int colX = ARGB( 200, 220,  60,  60 );
		int colY = ARGB( 200,  60, 220,  60 );
		int colZ = ARGB( 200,  60,  60, 220 );
		if ( m_HoveredAxis == AXIS_SCALE )
		{
			colX = ARGB( 255, 255, 200, 120 );
			colY = ARGB( 255, 255, 200, 120 );
			colZ = ARGB( 255, 255, 200, 120 );
		}

		vector c = m_Position;
		vector x1 = c + Vector( h, -h, -h );
		vector x2 = c + Vector( h,  h, -h );
		vector x3 = c + Vector( h,  h,  h );
		vector x4 = c + Vector( h, -h,  h );

		// Bottom rectangle
		Debug.DrawLine( x1, x2, colX, ShapeFlags.ONCE );
		Debug.DrawLine( x2, x3, colX, ShapeFlags.ONCE );
		Debug.DrawLine( x3, x4, colX, ShapeFlags.ONCE );
		Debug.DrawLine( x4, x1, colX, ShapeFlags.ONCE );
	}

	void Draw()
	{
		if ( m_Position == vector.Zero && m_Mode == MODE_SCALE )
			return;

		switch ( m_Mode )
		{
		case MODE_MOVE:   DrawMoveGizmo();   break;
		case MODE_ROTATE: DrawRotateGizmo(); break;
		case MODE_SCALE:  DrawScaleGizmo();  break;
		}
	}

	// Project a world position to screen space via the engine helper.
	static vector WorldToScreen( vector worldPos )
	{
		return g_Game.GetScreenPos( worldPos );
	}

	// Hit test: given a screen-space mouse pos, return the AXIS_* (or NONE).
	int HitTest( vector mouseScreen )
	{
		m_HoveredAxis = AXIS_NONE;

		if ( m_Position == vector.Zero )
			return AXIS_NONE;

		float threshold = 18.0; // screen px

		if ( m_Mode == MODE_MOVE )
		{
			float s = m_Scale;
			vector pts[3];
			pts[0] = m_Position + Vector( 1.0 * s, 0, 0 );
			pts[1] = m_Position + Vector( 0, 1.0 * s, 0 );
			pts[2] = m_Position + Vector( 0, 0, 1.0 * s );

			float bestDist = threshold;
			int   bestAxis = AXIS_NONE;
			for ( int i = 0; i < 3; i++ )
			{
				vector sp = g_Game.GetScreenPos( pts[i] );
				if ( sp[2] < 0 )
					continue;
				float d = vector.Distance( sp, mouseScreen );
				if ( d < bestDist )
				{
					bestDist = d;
					bestAxis = i;
				}
			}
			m_HoveredAxis = bestAxis;
			return bestAxis;
		}

		if ( m_Mode == MODE_ROTATE )
		{
			vector spRot = g_Game.GetScreenPos( m_Position );
			if ( spRot[2] < 0 )
				return AXIS_NONE;
			float r = 1.5 * Math.Max( m_Scale, 0.2 );
			float mouseDist = vector.Distance( spRot, mouseScreen );
			float ringDist = r * 60.0;
			if ( Math.AbsFloat( mouseDist - ringDist ) < 12 )
				m_HoveredAxis = AXIS_ROTY;
			return m_HoveredAxis;
		}

		if ( m_Mode == MODE_SCALE )
		{
			vector spScale = g_Game.GetScreenPos( m_Position );
			if ( spScale[2] < 0 )
				return AXIS_NONE;
			if ( vector.Distance( spScale, mouseScreen ) < 28 )
				m_HoveredAxis = AXIS_SCALE;
			return m_HoveredAxis;
		}

		return AXIS_NONE;
	}

	// Compute a world-space delta from screen-space mouse delta. Crude axis
	// mapping -- not camera-aligned, but enough for the editor's transform
	// mutation.
	vector ComputeDragDelta( vector mouseDelta, int axis )
	{
		if ( axis == AXIS_X ) return Vector( mouseDelta[0] * 0.01, 0, 0 );
		if ( axis == AXIS_Y ) return Vector( 0, mouseDelta[1] * -0.01, 0 );
		if ( axis == AXIS_Z ) return Vector( 0, 0, mouseDelta[0] * 0.01 );
		if ( axis == AXIS_ROTY ) return Vector( 0, mouseDelta[0] * 0.5, 0 );
		return vector.Zero;
	}
}