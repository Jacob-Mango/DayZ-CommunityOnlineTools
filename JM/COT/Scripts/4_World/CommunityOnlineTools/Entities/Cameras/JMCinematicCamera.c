class JMCinematicCamera extends JMCameraBase
{
	vector linearVelocity;
	vector angularVelocity;

	vector orientation;
	
	void JMCinematicCamera()
	{
		SetEventMask( EntityEvent.FRAME );
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );
		
		vector transform[4];
		GetTransform( transform );

		Input input = GetGame().GetInput();

		float forward = input.LocalValue( "UAMoveForward" ) - input.LocalValue( "UAMoveBack" );
		float strafe = input.LocalValue( "UAMoveRight" ) - input.LocalValue( "UAMoveLeft" );
		float altitude = input.LocalValue( "UALeanLeft" ) - input.LocalValue( "UALeanRight" );

		float yawDiff = input.LocalValue( "UAAimLeft" ) - input.LocalValue( "UAAimRight" );
		float pitchDiff = input.LocalValue( "UAAimDown" ) - input.LocalValue( "UAAimUp" );

		float speedInc = 0;
		if ( input.HasGameFocus() )
		{
			float zoomAmt = input.LocalValue( "UACameraToolZoomForwards" ) - input.LocalValue( "UACameraToolZoomBackwards" );

			if ( zoomAmt == 0 )
				speedInc = input.LocalValue( "UACameraToolSpeedIncrease" ) - input.LocalValue( "UACameraToolSpeedDecrease" );
				if (speedInc)
					Print(speedInc);
		}


		bool shouldRoll = input.LocalValue( "UALookAround" );
		bool increaseSpeeds = input.LocalValue( "UATurbo" );

		if ( !MoveFreeze )
		{
			float cam_speed = CAMERA_SPEED;

			if ( !shouldRoll && CAMERA_BOOST_MULT > 0 )
			{
				CAMERA_SPEED += Math.Clamp( timeslice * 40.0 * CAMERA_SPEED * speedInc / CAMERA_BOOST_MULT, -CAMERA_BOOST_MULT, CAMERA_BOOST_MULT );
				
				if ( CAMERA_SPEED < 0.001 ) 
				{
					CAMERA_SPEED = 0.001;
				}
				
				cam_speed = CAMERA_SPEED;

				if ( increaseSpeeds ) 
				{
					cam_speed = cam_speed * CAMERA_BOOST_MULT;
				}
			}
			
			linearVelocity = linearVelocity * CAMERA_VELDRAG;

			linearVelocity = linearVelocity + ( transform[0] * strafe * cam_speed );
			linearVelocity = linearVelocity + ( transform[1] * altitude * cam_speed );
			linearVelocity = linearVelocity + ( transform[2] * forward * cam_speed );

			transform[3] = transform[3] + ( linearVelocity * timeslice );

			//float surfaceY = GetGame().SurfaceY( newPos[0], newPos[2] ) + 0.25;
			//if ( newPos[1] < surfaceY ) 
			//{
			//	newPos[1] = surfaceY;
			//}
		}

		SetTransform( transform );
			
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
}