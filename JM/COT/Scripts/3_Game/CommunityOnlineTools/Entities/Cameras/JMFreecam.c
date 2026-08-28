// =============================================================================
//  JMFreecam.c
//
//  Free-roam 6-DOF camera used by the Map Editor module when the admin
//  opens the editor. Detaches the player's view and lets the admin fly
//  around the world to inspect + place / move / rotate objects in 3D.
//
//  Controls:
//    Movement:  W / S   - forward / back along camera direction
//               A / D   - strafe left / right
//               Q / E   - down / up (world Y axis)
//    Rotation:  Mouse   - yaw + pitch (held RMB to rotate)
//    Boost:     Shift   - 5x speed multiplier
//    Slow:      Ctrl    - 0.2x speed multiplier for precision
//
//  Input binding convention: every action name MUST exist in
//  JM/COT/Scripts/Data/Inputs.xml under the Map Editor module's binding
//  scope. The bindings are looked up each tick via GetUApi().GetInputByName().
//
//  Frame-rate independent: velocity is computed from speed * dt (not per-tick).
// =============================================================================

class JMFreecam : JMCameraBase
{
	static const float BASE_SPEED  = 6.0;   // m/s walk pace
	static const float BOOST_MULT  = 5.0;   // shift
	static const float SLOW_MULT   = 0.2;   // ctrl
	static const float MOUSE_SENS  = 0.22;  // deg per pixel
	static const float PITCH_LIMIT = 85.0;  // degrees

	float m_Yaw;     // rotation around world Z (0..360)
	float m_Pitch;   // rotation around camera right (clamped)

	vector m_AccumVelocity;  // smooth out per-tick velocity jumps
	float  m_CurSpeed;       // current m/s

	int    m_PrevMouseX;
	int    m_PrevMouseY;
	bool   m_HasPrevMouse;

	void JMFreecam()
	{
		// Seed yaw/pitch from current camera orientation so we don't snap.
		vector dir = GetDirection();
		m_Yaw = Math.Atan2( dir[0], dir[2] ) * 57.2957795;
		m_Pitch = -Math.Asin( dir[1] ) * 57.2957795;

		m_AccumVelocity = vector.Zero;
		m_CurSpeed = BASE_SPEED;

		m_PrevMouseX = 0;
		m_PrevMouseY = 0;
		m_HasPrevMouse = false;
	}

	override void EOnFrame( IEntity other, float timeSlice )
	{
		super.EOnFrame( other, timeSlice );

		if ( LookFreeze || MoveFreeze )
			return;

		// -------- Mouse delta -> yaw/pitch (only while RMB held) --------
		// RMB state via GetMouseState - RMB=1, LMB=0 in vanilla.
		bool rmbHeld = ( GetMouseState( MouseState.RIGHT ) & MB_PRESSED_MASK ) != 0;
		if ( rmbHeld )
		{
			int mx, my;
			GetMousePos( mx, my );
			int dx = 0, dy = 0;
			if ( m_HasPrevMouse )
			{
				dx = mx - m_PrevMouseX;
				dy = my - m_PrevMouseY;
			}
			m_PrevMouseX = mx;
			m_PrevMouseY = my;
			m_HasPrevMouse = true;

			m_Yaw   = m_Yaw   + dx * MOUSE_SENS;
			m_Pitch = m_Pitch - dy * MOUSE_SENS;
			if ( m_Pitch >  PITCH_LIMIT ) m_Pitch =  PITCH_LIMIT;
			if ( m_Pitch < -PITCH_LIMIT ) m_Pitch = -PITCH_LIMIT;
		}
		else
		{
			m_HasPrevMouse = false;
		}

		// -------- Input bindings -> movement vector --------
		// GetUApi().GetInputByName() returns a UAInput per the matching
		// entry in Inputs.xml; .LocalHold() = true while the key is down.
		vector fwd   = vector.Zero;
		fwd[0] = Math.Sin( m_Yaw * 0.0174532925 ) * Math.Cos( m_Pitch * 0.0174532925 );
		fwd[1] = -Math.Sin( m_Pitch * 0.0174532925 );
		fwd[2] = Math.Cos( m_Yaw * 0.0174532925 ) * Math.Cos( m_Pitch * 0.0174532925 );

		vector right = Vector( Math.Cos( m_Yaw * 0.0174532925 ), 0, -Math.Sin( m_Yaw * 0.0174532925 ) );

		vector wish = vector.Zero;
		if ( IsInputHeld( "UAMapEditorMoveForward"  ) ) wish = wish + fwd;
		if ( IsInputHeld( "UAMapEditorMoveBackward" ) ) wish = wish - fwd;
		if ( IsInputHeld( "UAMapEditorMoveLeft"     ) ) wish = wish - right;
		if ( IsInputHeld( "UAMapEditorMoveRight"    ) ) wish = wish + right;
		if ( IsInputHeld( "UAMapEditorMoveUp"       ) ) wish[1] = wish[1] + 1;
		if ( IsInputHeld( "UAMapEditorMoveDown"     ) ) wish[1] = wish[1] - 1;

		if ( wish.Length() > 0.001 )
			wish.Normalize();

		float speed = BASE_SPEED;
		if ( IsInputHeld( "UAMapEditorBoost" ) ) speed = BASE_SPEED * BOOST_MULT;
		if ( IsInputHeld( "UAMapEditorSlow"  ) ) speed = BASE_SPEED * SLOW_MULT;
		m_CurSpeed = speed;

		vector targetVel = wish * speed;
		m_AccumVelocity = vector.Lerp( m_AccumVelocity, targetVel, Math.Clamp( timeSlice / 0.15, 0, 1 ) );

		vector pos = GetPosition() + m_AccumVelocity * timeSlice;
		SetPosition( pos );

		vector orient = Vector( m_Pitch, m_Yaw, 0 );
		SetOrientation( orient );
		SetDirection( fwd );
	}

	// Helper: poll a named input via GetUApi. Returns false on missing input.
	private bool IsInputHeld( string inputName )
	{
		UAInput inp = GetUApi().GetInputByName( inputName );
		if ( !inp )
			return false;
		return inp.LocalHold();
	}
}