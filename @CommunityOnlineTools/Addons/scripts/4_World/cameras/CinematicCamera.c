class CinematicCamera extends COTCamera
{
    vector velocity;

    float yawVelocity;
    float pitchVelocity;
    
    void CinematicCamera()
    {
        SetEventMask( EntityEvent.FRAME );
    }

    override void OnUpdate( float timeslice )
    {
        super.OnUpdate( timeslice );
        
        vector oldOrient = GetOrientation();
        vector oldPos = GetPosition();
        vector oldDir = GetDirection();

        Input input = GetGame().GetInput();

        float forward = input.GetAction( UAMoveForward ) - input.GetAction( UAMoveBack );
        float strafe = input.GetAction( UATurnRight ) - input.GetAction( UATurnLeft );
        float altitude = input.GetAction( UALeanLeft ) - input.GetAction( UALeanRight );

        float yawDiff = input.GetAction( UAAimLeft ) - input.GetAction( UAAimRight );
        float pitchDiff = input.GetAction( UAAimDown ) - input.GetAction( UAAimUp );

        float speedInc = input.GetAction( UAPrevAction ) - input.GetAction( UANextAction );

        bool shouldRoll = input.GetAction( UALookAround );
        bool increaseSpeeds = input.GetAction( UATurbo );

        if ( !MoveFreeze )
        {
            float cam_speed = CAMERA_SPEED;

            if ( !shouldRoll && CAMERA_BOOST_MULT > 0 )
            {
                CAMERA_SPEED = CAMERA_SPEED + ( CAMERA_SPEED * speedInc / CAMERA_BOOST_MULT );
                
                if ( CAMERA_SPEED < 0.001 ) 
                {
                    CAMERA_SPEED = 0.001;
                }
                
                cam_speed = CAMERA_SPEED;

                if( increaseSpeeds ) 
                {
                    cam_speed = cam_speed * CAMERA_BOOST_MULT;
                }
            }

            vector up = vector.Up;
            vector direction = GetDirection();
            vector directionAside = vector.Up * direction;
            
            up = up * altitude * cam_speed;

            direction = direction * forward * cam_speed;
            directionAside = directionAside * strafe * cam_speed;

            velocity = velocity * CAMERA_VELDRAG;
            velocity = velocity + (direction + directionAside + up) * timeslice;

            vector newPos = oldPos + velocity;

            float surfaceY = GetGame().SurfaceY( newPos[0], newPos[2] ) + 0.25;

            if ( newPos[1] < surfaceY ) 
            {
                newPos[1] = surfaceY;
            }

            SetPosition(newPos);
        }

        if ( !LookFreeze )
        {
            yawVelocity = yawVelocity + yawDiff * CAMERA_MSENS;
            pitchVelocity = pitchVelocity + pitchDiff * CAMERA_MSENS;

            vector newOrient = oldOrient;

            newOrient[0] = oldOrient[0] - Math.RAD2DEG * yawVelocity * timeslice;
            newOrient[1] = oldOrient[1] - Math.RAD2DEG * pitchVelocity * timeslice;

            yawVelocity *= CAMERA_SMOOTH;
            pitchVelocity *= CAMERA_SMOOTH;

            if( newOrient[1] < -89 )
                newOrient[1] = -89;
            if( newOrient[1] > 89 )
                newOrient[1] = 89;

            if ( shouldRoll ) 
            {
                if ( increaseSpeeds && CAMERA_BOOST_MULT > 0 )
                {
                    speedInc = speedInc * CAMERA_BOOST_MULT;
                }

                newOrient[2] = newOrient[2] - ( speedInc );
            }

            SetOrientation( newOrient );
        }
    }
}