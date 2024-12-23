//! These are consts. Treat them as such. If you need them mutable, change from const to JMCameraModule member vars and rename to CamelCase.
static const float CAMERA_FOV_SPEED_MODIFIER = 5.0;
static const float CAMERA_BOOST_MULT = 5.0;
static const float CAMERA_VELDRAG = 0.9;
static const float CAMERA_MSENS = 35.0;
static const float CAMERA_SMOOTH = 0.8;
static const float CAMERA_DOFFSET = 10.0;
static const float CAMERA_SMOOTH_BLUR = 0.0;

//! TODO: These really should be static members on JMCameraBase, but I can't be arsed to change this rn
static JMCameraBase CurrentActiveCamera; // active static camera "staticcamera"
static JMCameraBase COT_PreviousActiveCamera;

enum JMCamera3rdPersonMode
{
	OFF,
	DEFAULT,
	DOLLY
}

class JMCameraBase: Camera
{
	static float s_CurrentSpeed = 3.0;

	float SendUpdateAccumalator = 0.0;
	
	bool LookFreeze;
	bool MoveFreeze;

	Object SelectedTarget;
	vector TargetPosition;

	bool m_JM_3rdPerson;
	bool m_JM_LeftShoulder;
	bool m_JM_IsADS;
	bool m_JM_1stPersonADS_HideScope;

	void JMCameraBase()
	{
		SetEventMask( EntityEvent.FRAME );

	#ifndef SERVER
		if (COT_PreviousActiveCamera)
			SetFOV(COT_PreviousActiveCamera.GetCurrentFOV());
		else
			SetFOV(GetDayZGame().GetUserFOV());
	#endif

		SelectedTarget( NULL );
	}

	void ~JMCameraBase()
	{
		SelectedTarget( NULL );
	}

	void OnTargetSelected( Object target )
	{
	}

	void OnTargetDeselected( Object target )
	{
	}

	void SelectedTarget( Object target )
	{
		if ( target && target != SelectedTarget )
		{
			TargetPosition = target.GetPosition();
			MoveFreeze = true;
			LookFreeze = true;

			OnTargetSelected( target );
		} else if ( target == NULL && SelectedTarget )
		{
			TargetPosition = "0 0 0";
			MoveFreeze = false;
			LookFreeze = false;

			OnTargetDeselected( SelectedTarget );
		}

		SelectedTarget = target;
	}

	override void EOnFrame( IEntity other, float timeSlice )
	{
		if ( SendUpdateAccumalator > 0.5 )
		{
			GetGame().UpdateSpectatorPosition( GetPosition() );

			SendUpdateAccumalator = 0;
		}
			
		SendUpdateAccumalator = SendUpdateAccumalator + timeSlice;

		OnUpdate( timeSlice );
	}

	void OnUpdate( float timeslice )
	{
	}
};
