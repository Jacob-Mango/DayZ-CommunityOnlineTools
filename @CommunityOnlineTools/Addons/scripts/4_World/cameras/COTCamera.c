class COTCamera extends Camera
{
    float SendUpdateAccumalator = 0.0;
    
    bool LookFreeze;
    bool MoveFreeze;

    Object SelectedTarget;
    vector TargetPosition;

    void COTCamera()
    {
        SetEventMask( EntityEvent.FRAME );

        SelectedTarget( NULL );
    }

    void SelectedTarget( Object target )
    {
        SelectedTarget = target;

        if ( SelectedTarget )
        {
            TargetPosition = target.GetPosition();
            MoveFreeze = true;
            LookFreeze = true;
        } else 
        {
            TargetPosition = "0 0 0";
            MoveFreeze = false;
            LookFreeze = false;
        }
    }

    override void EOnFrame( IEntity other, float timeSlice )
    {
        if ( SendUpdateAccumalator > 0.5 )
        {
            GetRPCManager().SendRPC( "COT_Camera", "UpdateCameraNetworkBubble", new Param1<vector>( GetPosition() ), true );

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