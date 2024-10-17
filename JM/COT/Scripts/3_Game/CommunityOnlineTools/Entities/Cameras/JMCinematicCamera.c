class JMCinematicCamera: JMCameraBase
{
	vector linearVelocity;
	vector angularVelocity;

	vector orientation;

	vector positionOffset;

	// Traveling stuff
	ref TVectorArray travelPositions;
	ref TFloatArray travelTimes; // in seconds
	ref TBoolArray travelSmooth;
	
    private int currentTargetIndex;
    private vector startPosition;
    private vector endPosition;
    private float currentTime;
	private float targetTime;
	private bool targetSmooth;
	private bool shouldTravel;

	autoptr TStringArray m_PossibleInputExcludes = {"menu", "inventory", "map"};

	private float m_Strafe;
	private float m_Altitude;
	private float m_Forward;

	void JMCinematicCamera()
	{
		//positionOffset = "0 1.5 0";
		travelPositions = new TVectorArray;
		travelTimes = new TFloatArray;
		travelSmooth = new TBoolArray;
		SetEventMask( EntityEvent.FRAME );
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );
		
		vector transform[4];
		GetTransform( transform );

		// ================ Inputs ================
		Input input = GetGame().GetInput();

		float forward = input.LocalValue( "UAMoveForward" ) - input.LocalValue( "UAMoveBack" );
		float strafe = input.LocalValue( "UAMoveRight" ) - input.LocalValue( "UAMoveLeft" );
		float altitude = input.LocalValue( "UALeanLeft" ) - input.LocalValue( "UALeanRight" );

		float yawDiff = input.LocalValue( "UAAimLeft" ) - input.LocalValue( "UAAimRight" );
		float pitchDiff = input.LocalValue( "UAAimDown" ) - input.LocalValue( "UAAimUp" );

		float speedInc = 0;
		if ( input.HasGameFocus() && !IsAnyInputExcludeActive() )
		{
			float zoomAmt = input.LocalValue( "UACameraToolZoomForwards" ) - input.LocalValue( "UACameraToolZoomBackwards" );

			if ( zoomAmt == 0 )
				speedInc = input.LocalValue( "UACameraToolSpeedIncrease" ) - input.LocalValue( "UACameraToolSpeedDecrease" );
		}

		bool shouldRoll = input.LocalValue( "UALookAround" );
		bool increaseSpeeds = input.LocalValue( "UATurbo" );
		// ================ Inputs ================

		if ( !MoveFreeze && !shouldTravel )
		{
			float cam_speed = CAMERA_SPEED;

			if ( !shouldRoll && CAMERA_BOOST_MULT > 0 )
			{
				CAMERA_SPEED += Math.Clamp( timeslice * 40.0 * CAMERA_SPEED * speedInc / CAMERA_BOOST_MULT, -CAMERA_BOOST_MULT, CAMERA_BOOST_MULT );
				
				if ( CAMERA_SPEED < 0.001 ) 
				{
					CAMERA_SPEED = 0.001;
				}
				else if ( CAMERA_SPEED > 10 ) 
				{
					CAMERA_SPEED = 10;
				}
				
				cam_speed = CAMERA_SPEED;

				if ( increaseSpeeds ) 
				{
					cam_speed = cam_speed * CAMERA_BOOST_MULT;
				}
			}
			
			linearVelocity = linearVelocity * CAMERA_VELDRAG;

			CalcAccelerationRate(m_Strafe, strafe, increaseSpeeds);
			CalcAccelerationRate(m_Altitude, altitude, increaseSpeeds);
			CalcAccelerationRate(m_Forward, forward, increaseSpeeds);

			linearVelocity = linearVelocity + ( transform[0] * strafe * cam_speed );
			linearVelocity = linearVelocity + ( transform[1] * altitude * cam_speed );
			linearVelocity = linearVelocity + ( transform[2] * forward * cam_speed );

			transform[3] = transform[3] + ( linearVelocity * timeslice );

			SetTransform( transform );
		}
		else if ( shouldTravel )
		{
            currentTime += timeslice;

            float t = Math.Clamp(currentTime / targetTime, 0.0, 1.0);

            if (currentTime >= targetTime)
            {
                currentTargetIndex++;
                if (currentTargetIndex >= travelPositions.Count())
                {
					shouldTravel = false;
                }
				else
				{
					currentTime = 0.0;
					startPosition = travelPositions[currentTargetIndex - 1];
					endPosition = travelPositions[currentTargetIndex];
					targetTime = travelTimes[currentTargetIndex];
					targetSmooth = travelSmooth[currentTargetIndex];
				}
            }
            else
            {
                // Smooth step interpolation
				if ( targetSmooth )
                	t = SmoothStep(t);

				vector pos = vector.Lerp(startPosition, endPosition, t);
				SetPosition(pos + positionOffset);
            }
		}
			
		if ( !LookFreeze )
		{
			angularVelocity = angularVelocity * CAMERA_SMOOTH;

			angularVelocity[0] = angularVelocity[0] + ( yawDiff * CAMERA_MSENS );
			angularVelocity[1] = angularVelocity[1] + ( pitchDiff * CAMERA_MSENS );
			
			if ( shouldRoll ) 
			{
				angularVelocity[2] = angularVelocity[2] + ( speedInc * CAMERA_MSENS );
			}

			if (orientation == vector.Zero)
				orientation = GetOrientation();

			orientation[0] = orientation[0] - ( angularVelocity[0] * timeslice );
			orientation[1] = orientation[1] - ( angularVelocity[1] * timeslice );
			orientation[2] = orientation[2] - ( angularVelocity[2] * timeslice );

			if ( orientation[1] <= -90 )
			{
				angularVelocity[1] = Math.Min( angularVelocity[1], 0 );
				orientation[1] = -90;
			} else if ( orientation[1] >= 90 )
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

	void CalcAccelerationRate(inout float velocity, inout float rate, bool increaseSpeeds = false)
	{
		float mult = 0.05;

		//! Adjust for increased camera speed so that effective acceleration stays the same
		if (increaseSpeeds)
			mult *= 0.447214;  //! 0.05 * Math.Sqrt(0.2) == 0.05 / Math.Sqrt(CAMERA_BOOST_MULT)

		//! Slow acceleration, instant deceleration
		if (rate)
			velocity = Math.Clamp(velocity + rate * mult, -1.0, 1.0);
		else
			velocity = 0;

		if (rate < 0)
			rate = velocity * -velocity;
		else
			rate = velocity * velocity;
	}
	
	void SetupTraveling(TVectorArray positions, TFloatArray time, TBoolArray smooth)
	{
		travelPositions = positions;
		travelTimes = time;
		travelSmooth = smooth;

		currentTargetIndex = 1;
		currentTime = 0.0;
		startPosition = travelPositions[0];
		endPosition = travelPositions[1];

		shouldTravel = true;
	}

    // Custom smooth step interpolation function
    private float SmoothStep(float t)
    {
        return t * t * (3 - 2 * t);
    }

	bool IsAnyInputExcludeActive()
	{
		foreach (string exclude: m_PossibleInputExcludes)
		{
			if (GetGame().GetMission().IsInputExcludeActive(exclude))
				return true;
		}

		return false;
	}

	void AngleToQuat( float angle, vector dir, out float d[4] )
	{
		float sin = Math.Sin( angle * 0.5 * Math.DEG2RAD );
		float cos = Math.Cos( angle * 0.5 * Math.DEG2RAD );

		d[3] = cos;
		d[2] = dir[2] * sin;
		d[1] = dir[1] * sin;
		d[0] = dir[0] * sin;
	}
};
