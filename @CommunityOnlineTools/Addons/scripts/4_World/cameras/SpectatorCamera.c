class SpectatorCamera extends COTCamera
{
	void SpectatorCamera()
	{
		SetEventMask( EntityEvent.FRAME );
	}

	override void OnUpdate( float timeslice )
	{
        super.OnUpdate( timeslice );

        if ( SelectedTarget )
        {
            SetPosition( SelectedTarget.GetPosition() );
            SetDirection( SelectedTarget.GetDirection() )
            SetOrientation( SelectedTarget.GetOrientation() );
        }
    }
}