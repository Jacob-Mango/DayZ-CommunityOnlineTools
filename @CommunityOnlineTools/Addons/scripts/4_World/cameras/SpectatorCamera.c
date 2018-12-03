class SpectatorCamera extends COTCamera
{
    void SpectatorCamera()
    {
    }

    override void OnUpdate( float timeslice )
    {
        super.OnUpdate( timeslice );

        Human human = Human.Cast( SelectedTarget );

        if ( human )
        {
            vector pos;
            float rot[4];

            int bone = human.GetBoneIndexByName( "Head" );

            pos = human.GetBonePositionWS( bone );

            human.GetBoneRotationWS( bone, rot );

            vector orientation = Math3D.QuatToAngles( rot );

            orientation[0] = orientation[0] - 90;
            orientation[1] = orientation[1];
            orientation[2] = 0;

            SetOrientation( orientation );
            SetPosition( pos + "0.04 0.04 0" );
        }
    }
}