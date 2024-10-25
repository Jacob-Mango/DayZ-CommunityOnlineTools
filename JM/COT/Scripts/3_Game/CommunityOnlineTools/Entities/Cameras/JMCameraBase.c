static float CAMERA_FOV = 1.0;
static float CAMERA_TARGETFOV = 1.0;
static float CAMERA_FOV_SPEED_MODIFIER = 5.0;
static float CAMERA_SPEED = 3.0;
static float CAMERA_BOOST_MULT = 5.0;
static float CAMERA_VELDRAG = 0.9;
static float CAMERA_MSENS = 35.0;
static float CAMERA_SMOOTH = 0.8;
static bool  CAMERA_DOF = false;
static bool  CAMERA_AFOCUS = true;

static float CAMERA_FDIST = 0.2;
static float CAMERA_FLENGTH = 20.0;
static float CAMERA_FNEAR = 185.0;
static float CAMERA_BLUR = 4.0;
static float CAMERA_DOFFSET = 10.0;

static float CAMERA_SMOOTH_BLUR = 0.0;

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
