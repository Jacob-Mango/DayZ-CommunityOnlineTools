class JMCameraBase extends Camera
{
	float SendUpdateAccumalator = 0.0;
	
	bool LookFreeze;
	bool MoveFreeze;

	Object SelectedTarget;
	vector TargetPosition;

	bool m_JM_3rdPerson;

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
}