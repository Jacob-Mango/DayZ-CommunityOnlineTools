class JMSpectatorCamera: JMCameraBase
{
	vector linearVelocity;
	vector angularVelocity;

	vector orientation;
	bool m_COT_IsInFreeLook;
	
	override void OnTargetSelected( Object target )
	{
		DayZPlayerImplement impl;
		if ( !Class.CastTo( impl, target ) )
			return;

		impl.OnSpectateStart( this );
	}

	override void OnTargetDeselected( Object target )
	{
		DayZPlayerImplement impl;
		if ( !Class.CastTo( impl, target ) )
			return;

		impl.OnSpectateEnd();
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		Input input = GetGame().GetInput();
		bool freelook = input.LocalValue( "UALookAround" );

		if (m_COT_IsInFreeLook != freelook)
		{
			m_COT_IsInFreeLook = freelook;
			if (freelook)
				orientation = GetOrientation();
		}
		
		if ( !LookFreeze || freelook )
		{
			float yawDiff = input.LocalValue( "UAAimLeft" ) - input.LocalValue( "UAAimRight" );
			float pitchDiff = input.LocalValue( "UAAimDown" ) - input.LocalValue( "UAAimUp" );
		
			angularVelocity = angularVelocity * CAMERA_SMOOTH;

			angularVelocity[0] = angularVelocity[0] + ( yawDiff * CAMERA_MSENS );
			angularVelocity[1] = angularVelocity[1] + ( pitchDiff * CAMERA_MSENS );

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
};
