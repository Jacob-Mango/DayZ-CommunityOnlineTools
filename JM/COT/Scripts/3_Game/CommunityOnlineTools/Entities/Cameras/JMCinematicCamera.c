enum JMTravelMode
{
	ONCE     = 0,
	LOOP     = 1,
	PINGPONG = 2
}

class JMCinematicCamera: JMCameraBase
{
	static JMCinematicCamera s_COT_CinematicCamera;

	vector linearVelocity;
	vector angularVelocity;

	vector orientation;

	vector positionOffset;

	// Traveling
	ref array< ref JMCameraWaypoint > travelWaypoints;

	private int   currentTargetIndex;
	private float currentTime;
	private bool  shouldTravel;
	private bool  m_TravelPaused;
	private int   m_TravelDirection = 1;  // +1 forward, -1 pingpong reverse

	// Hold state
	private bool  m_Holding;
	private float m_HoldTimer;

	// Segment start/end cache (rebuilt each segment transition)
	private vector startPosition;
	private vector endPosition;
	private vector startOrientation;
	private vector endOrientation;
	private bool   m_InterpolateOrientation;  // true only when both endpoints have captured orientation
	private float  targetTime;
	private bool   targetCatmull;
	private JMCameraEasing targetEasing;
	private int    m_FromIdx;   // cached from-waypoint index for effects interpolation
	private int    m_ToIdx;     // cached to-waypoint index for effects interpolation

	// Speed-based travel (smooth speed transitions between waypoints)
	private float  m_CurrentSpeed;    // actual instantaneous speed (m/s), lerped each frame
	private float  m_TargetSpeed;     // destination speed for this segment (from waypoint)
	private float  m_SegmentLength;   // arc length of current segment (metres)
	private float  m_TravelDistance;  // distance traveled in current segment (metres)
	private bool   m_UseSpeedTravel;  // true when the current segment uses speed-based travel

	// Arc-length reparameterization lookup table (per segment, 20 samples)
	private static const int ARC_SAMPLES = 20;
	private ref array< float > m_ArcTable;  // normalized cumulative arc lengths [0..1]

	// Interpolated travel effects — read by JMCameraModule.OnUpdate
	float m_TravelExposure;
	float m_TravelVignette;
	float m_TravelBlur = 1.0;
	float m_TravelFOV  = 1.0;
	float m_TravelShakeIntensity;
	float m_TravelShakeFrequency = 1.0;

	// Playback config set by module before SetupTraveling
	JMTravelMode m_TravelMode   = JMTravelMode.ONCE;
	float        m_SpeedMult    = 1.0;  // > 1 = faster, < 1 = slower

	bool IsTraveling()  { return shouldTravel; }
	bool IsPaused()     { return m_TravelPaused; }

	// Camera shake (set by module or by travel)
	float m_ShakeIntensity;
	float m_ShakeFrequency;
	private float m_ShakeTime;

	autoptr TStringArray m_PossibleInputExcludes = {"menu", "inventory", "map"};

	private float m_Strafe;
	private float m_Altitude;
	private float m_Forward;

	void JMCinematicCamera()
	{
		travelWaypoints = new array< ref JMCameraWaypoint >;
		m_ArcTable      = new array< float >;

		if (s_COT_CinematicCamera)
			g_Game.ObjectDeleteOnClient(s_COT_CinematicCamera);

		s_COT_CinematicCamera = this;
	}

	// ----------------------------------------------------------------
	//  Public controls
	// ----------------------------------------------------------------

	void TogglePause()
	{
		m_TravelPaused = !m_TravelPaused;
	}

	// ----------------------------------------------------------------
	//  OnUpdate
	// ----------------------------------------------------------------

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		if ( !IsActive() )
			return;

		vector transform[4];
		GetTransform( transform );

		// ================ Inputs ================
		Input input = g_Game.GetInput();

		float forward  = input.LocalValue( "UAMoveForward" )  - input.LocalValue( "UAMoveBack" );
		float strafe   = input.LocalValue( "UAMoveRight" )    - input.LocalValue( "UAMoveLeft" );
		float altitude = input.LocalValue( "UALeanLeft" )     - input.LocalValue( "UALeanRight" );

		float yawDiff   = input.LocalValue( "UAAimLeft" )  - input.LocalValue( "UAAimRight" );
		float pitchDiff = input.LocalValue( "UAAimDown" )  - input.LocalValue( "UAAimUp" );

		float speedInc = 0;
		if ( input.HasGameFocus() && !IsAnyInputExcludeActive() )
		{
			float zoomAmt = input.LocalValue( "UACameraToolZoomForwards" ) - input.LocalValue( "UACameraToolZoomBackwards" );
			if ( zoomAmt == 0 )
				speedInc = input.LocalValue( "UACameraToolSpeedIncrease" ) - input.LocalValue( "UACameraToolSpeedDecrease" );
		}

		bool shouldRoll    = input.LocalValue( "UALookAround" );
		bool increaseSpeeds = input.LocalValue( "UATurbo" );
		bool decreaseSpeeds = input.LocalValue( "UAWalkRunTemp" );
		// ================ Inputs ================

		if ( !MoveFreeze && !shouldTravel )
		{
			float cam_speed = s_CurrentSpeed;

			if ( !shouldRoll && CAMERA_BOOST_MULT > 0 )
			{
				s_CurrentSpeed += Math.Clamp( timeslice * 40.0 * s_CurrentSpeed * speedInc / CAMERA_BOOST_MULT, -CAMERA_BOOST_MULT, CAMERA_BOOST_MULT );

				if ( s_CurrentSpeed < 0.001 )
					s_CurrentSpeed = 0.001;
				else if ( s_CurrentSpeed > 10 )
					s_CurrentSpeed = 10;

				cam_speed = s_CurrentSpeed;

				if ( decreaseSpeeds )
					cam_speed *= 0.2;
				else if ( increaseSpeeds )
					cam_speed *= CAMERA_BOOST_MULT;
			}

			linearVelocity = linearVelocity * CAMERA_VELDRAG;

			CalcAccelerationRate(m_Strafe,   strafe,   timeslice, increaseSpeeds);
			CalcAccelerationRate(m_Altitude, altitude, timeslice, increaseSpeeds);
			CalcAccelerationRate(m_Forward,  forward,  timeslice, increaseSpeeds);

			linearVelocity = linearVelocity + ( transform[0] * strafe   * cam_speed );
			linearVelocity = linearVelocity + ( transform[1] * altitude * cam_speed );
			linearVelocity = linearVelocity + ( transform[2] * forward  * cam_speed );

			transform[3] = transform[3] + ( linearVelocity * timeslice );
			SetTransform( transform );
		}
		else if ( shouldTravel )
		{
			if ( !m_TravelPaused )
			{
				// ---- Hold phase ----
				if ( m_Holding )
				{
					m_HoldTimer -= timeslice;
					if ( m_HoldTimer <= 0 )
					{
						m_Holding = false;
						AdvanceSegment();
					}
					// During hold: keep last frame's position & orientation, still apply effects
					ApplyTravelEffectsAtT(1.0, timeslice);
				}
				else
				{
					float t;

					if ( m_UseSpeedTravel )
					{
						// Smoothly lerp instantaneous speed toward the target waypoint speed
						float speedRamp = timeslice * 2.0;
						m_CurrentSpeed = Math.Lerp( m_CurrentSpeed, m_TargetSpeed * m_SpeedMult, speedRamp );

						m_TravelDistance += m_CurrentSpeed * timeslice;
						t = Math.Clamp( m_TravelDistance / m_SegmentLength, 0.0, 1.0 );
					}
					else
					{
						currentTime += timeslice;
						t = Math.Clamp( currentTime / targetTime, 0.0, 1.0 );
					}

					bool segmentDone;
					if ( m_UseSpeedTravel )
						segmentDone = m_TravelDistance >= m_SegmentLength;
					else
						segmentDone = currentTime >= targetTime;

					if ( segmentDone )
					{
						// Snap to exact end of segment
						ApplyTravelEffectsAtT(1.0, timeslice);
						ApplyTravelPosition(1.0);
						ApplyTravelOrientation(1.0);

						// Check for hold
						int holdIdx = GetCurrentEndIndex();
						float holdTime = travelWaypoints[holdIdx].HoldTime;
						if ( holdTime > 0.0 )
						{
							m_Holding   = true;
							m_HoldTimer = holdTime;
						}
						else
						{
							AdvanceSegment();
						}
					}
					else
					{
						ApplyTravelPosition(t);
						ApplyTravelOrientation(t);
						ApplyTravelEffectsAtT(t, timeslice);
					}
				}
			}
		}

		if ( !shouldTravel )
		{
			if ( !LookFreeze )
			{
				angularVelocity = angularVelocity * CAMERA_SMOOTH;

				angularVelocity[0] = angularVelocity[0] + ( yawDiff   * CAMERA_MSENS );
				angularVelocity[1] = angularVelocity[1] + ( pitchDiff * CAMERA_MSENS );

				if ( shouldRoll )
					angularVelocity[2] = angularVelocity[2] + ( speedInc * CAMERA_MSENS );

				if (orientation == vector.Zero)
					orientation = GetOrientation();

				orientation[0] = orientation[0] - ( angularVelocity[0] * timeslice );
				orientation[1] = orientation[1] - ( angularVelocity[1] * timeslice );
				orientation[2] = orientation[2] - ( angularVelocity[2] * timeslice );

				if ( orientation[1] <= -90 )
				{
					angularVelocity[1] = Math.Min( angularVelocity[1], 0 );
					orientation[1] = -90;
				}
				else if ( orientation[1] >= 90 )
				{
					orientation[1] = 90;
					angularVelocity[1] = Math.Max( angularVelocity[1], 0 );
				}

				orientation[0] = Math.NormalizeAngle( orientation[0] );
				orientation[2] = Math.NormalizeAngle( orientation[2] );

				SetOrientation( orientation );
			}
			else
			{
				if ( SelectedTarget )
				{
					vector position = SelectedTarget.GetPosition();
					LookAt(position + positionOffset);
				}
				else if ( TargetPosition )
				{
					LookAt(TargetPosition);
				}
			}
		}

		// Camera shake — additive noise on top of final position/orientation
		if ( m_ShakeIntensity > 0 )
		{
			m_ShakeTime += timeslice * m_ShakeFrequency;

			float freq = m_ShakeTime;
			vector shakePos;
			shakePos[0] = Math.Sin( freq * 1.7  + 0.3 ) * Math.Sin( freq * 3.1  + 1.1 ) * m_ShakeIntensity;
			shakePos[1] = Math.Sin( freq * 2.3  + 2.7 ) * Math.Sin( freq * 1.9  + 0.7 ) * m_ShakeIntensity * 0.5;
			shakePos[2] = Math.Sin( freq * 1.13 + 1.3 ) * Math.Sin( freq * 2.71 + 3.3 ) * m_ShakeIntensity;

			SetPosition( GetPosition() + shakePos );

			vector shakeOrient = GetOrientation();
			shakeOrient[0] = shakeOrient[0] + Math.Sin( freq * 2.1 + 0.9 ) * m_ShakeIntensity * 0.3;
			shakeOrient[1] = shakeOrient[1] + Math.Sin( freq * 1.6 + 2.1 ) * m_ShakeIntensity * 0.3;
			SetOrientation( shakeOrient );
		}
	}

	// ----------------------------------------------------------------
	//  Travel helpers
	// ----------------------------------------------------------------

	// Returns the waypoint index at the current segment end (accounting for direction)
	private int GetCurrentEndIndex()
	{
		return m_ToIdx;
	}

	// Shortest-arc lerp for a single angle (degrees), handles wrap at ±180
	private float LerpAngle( float a, float b, float t )
	{
		float diff = b - a;
		// Wrap diff into (-180, 180]
		while ( diff >  180 ) diff -= 360;
		while ( diff < -180 ) diff += 360;
		return a + diff * t;
	}

	private void ApplyTravelOrientation( float t )
	{
		if ( !m_InterpolateOrientation )
			return;

		float tEased = ApplyEasing(t, targetEasing);

		orientation[0] = LerpAngle(startOrientation[0], endOrientation[0], tEased);
		orientation[1] = LerpAngle(startOrientation[1], endOrientation[1], tEased);
		orientation[2] = LerpAngle(startOrientation[2], endOrientation[2], tEased);

		SetOrientation(orientation);
	}

	private void ApplyTravelPosition( float t )
	{
		float tInterp;
		vector pos;

		if ( targetCatmull && travelWaypoints.Count() >= 4 )
		{
			// Reparameterized Catmull-Rom for constant perceived speed
			float tRemap = CatmullArcLengthRemap(t);
			int i1 = currentTargetIndex - 1;
			int i0 = Math.Max(i1 - 1, 0);
			int i2 = currentTargetIndex;
			int i3 = Math.Min(i2 + 1, travelWaypoints.Count() - 1);
			pos = CatmullRom(travelWaypoints[i0].Position, travelWaypoints[i1].Position,
			                 travelWaypoints[i2].Position, travelWaypoints[i3].Position, tRemap);
		}
		else
		{
			tInterp = ApplyEasing(t, targetEasing);
			pos = vector.Lerp(startPosition, endPosition, tInterp);
		}

		SetPosition(pos + positionOffset);
	}

	private void ApplyTravelEffectsAtT( float t, float timeslice )
	{
		JMCameraWaypoint wpStart = travelWaypoints[m_FromIdx];
		JMCameraWaypoint wpEnd   = travelWaypoints[m_ToIdx];

		float effectT = ApplyEasing(t, targetEasing);

		float targetExposure       = Math.Lerp(wpStart.Exposure,       wpEnd.Exposure,       effectT);
		float targetVignette       = Math.Lerp(wpStart.Vignette,       wpEnd.Vignette,       effectT);
		float targetBlur           = Math.Lerp(wpStart.Blur,           wpEnd.Blur,           effectT);
		float targetFOV            = Math.Lerp(wpStart.FOV,            wpEnd.FOV,            effectT);
		float targetShakeIntensity = Math.Lerp(wpStart.ShakeIntensity, wpEnd.ShakeIntensity, effectT);
		float targetShakeFrequency = Math.Lerp(wpStart.ShakeFrequency, wpEnd.ShakeFrequency, effectT);

		float smoothSpeed = timeslice * 5.0;
		m_TravelExposure       = Math.Lerp(m_TravelExposure,       targetExposure,       smoothSpeed);
		m_TravelVignette       = Math.Lerp(m_TravelVignette,       targetVignette,       smoothSpeed);
		m_TravelBlur           = Math.Lerp(m_TravelBlur,           targetBlur,           smoothSpeed);
		m_TravelFOV            = Math.Lerp(m_TravelFOV,            targetFOV,            smoothSpeed);
		m_TravelShakeIntensity = Math.Lerp(m_TravelShakeIntensity, targetShakeIntensity, smoothSpeed);
		m_TravelShakeFrequency = Math.Lerp(m_TravelShakeFrequency, targetShakeFrequency, smoothSpeed);

		if ( m_TravelFOV > 0 )
			SetFOV(m_TravelFOV);
	}

	// Advance to the next segment (or loop/pingpong)
	private void AdvanceSegment()
	{
		int nextIdx = currentTargetIndex + m_TravelDirection;

		bool reachedEnd   = nextIdx >= travelWaypoints.Count();
		bool reachedStart = nextIdx < 1;

		if ( reachedEnd || reachedStart )
		{
			if ( m_TravelMode == JMTravelMode.LOOP )
			{
				// Restart from the beginning
				currentTargetIndex = 1;
				m_TravelDirection  = 1;
				JMCameraWaypoint wp0 = travelWaypoints[0];
				m_TravelExposure       = wp0.Exposure;
				m_TravelVignette       = wp0.Vignette;
				m_TravelBlur           = wp0.Blur;
				m_TravelFOV            = wp0.FOV;
				m_TravelShakeIntensity = wp0.ShakeIntensity;
				m_TravelShakeFrequency = wp0.ShakeFrequency;
				LoadSegment(1);
			}
			else if ( m_TravelMode == JMTravelMode.PINGPONG )
			{
				// Reverse direction
				m_TravelDirection = -m_TravelDirection;
				currentTargetIndex = currentTargetIndex + m_TravelDirection;
				if ( currentTargetIndex >= 1 && currentTargetIndex < travelWaypoints.Count() )
					LoadSegment(currentTargetIndex);
				else
					shouldTravel = false;
			}
			else
			{
				// ONCE — stop
				shouldTravel = false;
			}
		}
		else
		{
			currentTargetIndex = nextIdx;
			LoadSegment(currentTargetIndex);
		}
	}

	// ----------------------------------------------------------------
	//  Setup
	// ----------------------------------------------------------------

	void SetupTraveling( array< ref JMCameraWaypoint > waypoints )
	{
		if ( !waypoints || waypoints.Count() < 2 )
			return;

		travelWaypoints    = waypoints;
		currentTargetIndex = 1;
		currentTime        = 0.0;
		shouldTravel       = true;
		m_TravelPaused     = false;
		m_TravelDirection  = 1;
		m_Holding          = false;
		m_HoldTimer        = 0.0;
		m_TravelDistance   = 0.0;
		m_CurrentSpeed     = 0.0;  // will ramp up from rest at start of first segment

		// Seed effect values from waypoint[0] to prevent visual jump
		JMCameraWaypoint wp0 = travelWaypoints[0];
		m_TravelExposure       = wp0.Exposure;
		m_TravelVignette       = wp0.Vignette;
		m_TravelBlur           = wp0.Blur;
		m_TravelFOV            = wp0.FOV;
		m_TravelShakeIntensity = wp0.ShakeIntensity;
		m_TravelShakeFrequency = wp0.ShakeFrequency;

		// Seed orientation from waypoint[0] if it has one, so the camera snaps to start orientation
		if ( wp0.OrientationCaptured )
		{
			orientation = wp0.Orientation;
			SetOrientation(orientation);
		}

		LoadSegment(1);
	}

	private void LoadSegment( int idx )
	{
		currentTime   = 0.0;
		targetEasing  = travelWaypoints[idx].m_Easing;
		targetCatmull = travelWaypoints[idx].UseCatmull;

		// Determine actual start/end depending on direction
		int fromIdx = idx - m_TravelDirection;
		int toIdx   = idx;
		if ( m_TravelDirection < 0 )
		{
			fromIdx = idx + 1;
			toIdx   = idx;
		}

		fromIdx = Math.Clamp(fromIdx, 0, travelWaypoints.Count() - 1);
		toIdx   = Math.Clamp(toIdx,   0, travelWaypoints.Count() - 1);

		m_FromIdx = fromIdx;
		m_ToIdx   = toIdx;

		startPosition = travelWaypoints[fromIdx].Position;
		endPosition   = travelWaypoints[toIdx].Position;

		JMCameraWaypoint wpTarget = travelWaypoints[toIdx];
		float segDist = vector.Distance( travelWaypoints[fromIdx].Position, travelWaypoints[toIdx].Position );
		if ( segDist < 0.01 ) segDist = 0.01;

		m_TravelDistance = 0.0;

		if ( wpTarget.Speed > 0 )
		{
			m_UseSpeedTravel = true;
			m_TargetSpeed    = wpTarget.Speed;
			m_SegmentLength  = segDist;

			// If m_CurrentSpeed is 0 (first segment), seed it at target speed so we don't start from a dead stop
			if ( m_CurrentSpeed <= 0 )
				m_CurrentSpeed = m_TargetSpeed;

			// targetTime is kept as an upper-bound safety timeout (dist / speed gives exact expected time)
			targetTime = ( segDist / wpTarget.Speed ) / m_SpeedMult;
		}
		else
		{
			m_UseSpeedTravel = false;
			m_SegmentLength  = segDist;
			targetTime       = wpTarget.Time / m_SpeedMult;
		}
		if ( targetTime <= 0 )
			targetTime = 1.0;

		// Orientation — only interpolate when both endpoints have a captured orientation
		JMCameraWaypoint wpFrom = travelWaypoints[fromIdx];
		JMCameraWaypoint wpTo   = travelWaypoints[toIdx];
		if ( wpFrom.OrientationCaptured && wpTo.OrientationCaptured )
		{
			startOrientation      = wpFrom.Orientation;
			endOrientation        = wpTo.Orientation;
			m_InterpolateOrientation = true;
		}
		else
		{
			m_InterpolateOrientation = false;
		}

		// Build arc-length table for catmull reparameterization
		if ( targetCatmull && travelWaypoints.Count() >= 4 )
			BuildArcTable(idx);
	}

	// ----------------------------------------------------------------
	//  Arc-length reparameterization (Catmull-Rom constant speed)
	// ----------------------------------------------------------------

	private void BuildArcTable( int idx )
	{
		m_ArcTable.Clear();

		int i1 = idx - 1;
		int i0 = Math.Max(i1 - 1, 0);
		int i2 = idx;
		int i3 = Math.Min(i2 + 1, travelWaypoints.Count() - 1);

		vector prev = travelWaypoints[i1].Position;
		float totalLen = 0.0;

		m_ArcTable.Insert(0.0);  // t=0 -> cumLen=0

		for ( int s = 1; s <= ARC_SAMPLES; s++ )
		{
			float tt = (float)s / ARC_SAMPLES;
			vector curr = CatmullRom(travelWaypoints[i0].Position, travelWaypoints[i1].Position,
			                         travelWaypoints[i2].Position, travelWaypoints[i3].Position, tt);
			totalLen += vector.Distance(prev, curr);
			m_ArcTable.Insert(totalLen);
			prev = curr;
		}

		// Normalize to [0..1]
		if ( totalLen > 0 )
		{
			for ( int n = 0; n < m_ArcTable.Count(); n++ )
				m_ArcTable[n] = m_ArcTable[n] / totalLen;
		}
	}

	// Map uniform t -> arc-parameterized t using the lookup table
	private float CatmullArcLengthRemap( float t )
	{
		if ( m_ArcTable.Count() < 2 )
			return t;

		// Binary search for the segment in the table that spans t
		int lo = 0;
		int hi = m_ArcTable.Count() - 1;

		while ( lo < hi - 1 )
		{
			int mid = (lo + hi) / 2;
			if ( m_ArcTable[mid] <= t )
				lo = mid;
			else
				hi = mid;
		}

		float arcLo = m_ArcTable[lo];
		float arcHi = m_ArcTable[hi];
		float range = arcHi - arcLo;

		float localT;
		if (range > 0.0)
			localT = (t - arcLo) / range;
		else
			localT = 0.0;

		// Map back to the spline parameter range
		float tLo = (float)lo / ARC_SAMPLES;
		float tHi = (float)hi / ARC_SAMPLES;
		return tLo + localT * (tHi - tLo);
	}

	// ----------------------------------------------------------------
	//  Easing
	// ----------------------------------------------------------------

	private float ApplyEasing( float t, JMCameraEasing easing )
	{
		switch ( easing )
		{
		case JMCameraEasing.LINEAR:
			return t;
		case JMCameraEasing.EASE_IN:
			return t * t * t;                        // cubic ease-in
		case JMCameraEasing.EASE_OUT:
			float inv = 1.0 - t;
			return 1.0 - inv * inv * inv;            // cubic ease-out
		case JMCameraEasing.SMOOTHER_STEP:
			return SmootherStep(t);
		case JMCameraEasing.EASE_IN_OUT:
		}
		
		return SmoothStep(t);
	}

	// ----------------------------------------------------------------
	//  Manual camera acceleration
	// ----------------------------------------------------------------

	void CalcAccelerationRate(inout float t, inout float rate, float dt, bool increaseSpeeds = false)
	{
		float step = 3.0 * dt;

		if (increaseSpeeds)
			step *= 0.447214;

		if (rate)
		{
			t = Math.Min(t + step, 1.0);
			if (rate < 0)
				rate = -SmootherStep(t);
			else
				rate = SmootherStep(t);
		}
		else
		{
			t = Math.Max(t - step, 0.0);
			rate = 0;
		}
	}

	// ----------------------------------------------------------------
	//  Math helpers
	// ----------------------------------------------------------------

	private vector CatmullRom( vector p0, vector p1, vector p2, vector p3, float t )
	{
		float t2 = t * t;
		float t3 = t2 * t;
		vector result;
		for (int i = 0; i < 3; i++)
		{
			float p0i = p0[i];
			float p1i = p1[i];
			float p2i = p2[i];
			float p3i = p3[i];

			float term0 = 2 * p1i;
			float term1 = (-p0i + p2i) * t;
			float term2 = (2*p0i - 5*p1i + 4*p2i - p3i) * t2;
			float term3 = (-p0i + 3*p1i - 3*p2i + p3i) * t3;

			result[i] = 0.5 * (term0 + term1 + term2 + term3);
		}
		return result;
	}

	private float SmoothStep(float t)
	{
		return t * t * (3 - 2 * t);
	}

	private float SmootherStep(float t)
	{
		return t * t * t * (t * (6 * t - 15) + 10);
	}

	bool IsAnyInputExcludeActive()
	{
		foreach (string exclude: m_PossibleInputExcludes)
		{
			if (g_Game.GetMission().IsInputExcludeActive(exclude))
				return true;
		}
		return false;
	}

	void AngleToQuat( float angle, vector dir, out float d[4] )
	{
		float sinV = Math.Sin( angle * 0.5 * Math.DEG2RAD );
		float cosV = Math.Cos( angle * 0.5 * Math.DEG2RAD );
		d[3] = cosV;
		d[2] = dir[2] * sinV;
		d[1] = dir[1] * sinV;
		d[0] = dir[0] * sinV;
	}
}
