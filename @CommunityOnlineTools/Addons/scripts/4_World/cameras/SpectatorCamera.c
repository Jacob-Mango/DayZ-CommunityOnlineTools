class SpectatorCamera extends COTCamera
{
    static const float CONST_UD_MIN = -85.0;
    static const float CONST_UD_MAX = 85.0;

    static const float CONST_LR_MIN = -160.0;
    static const float CONST_LR_MAX = 160.0;

    protected float m_fLRAngleVel[1];
    protected float m_fUDAngleVel[1];

    protected float m_fUpDownAngle;
    protected float m_fUpDownAngleAdd;
    protected float m_fLeftRightAngle;

    protected float m_CurrentCameraPitch;

    void SpectatorCamera()
    {
        m_fLRAngleVel[0] = 0;
        m_fUDAngleVel[0] = 0;
        
        SetEventMask( EntityEvent.FRAME );
    }

    float UpdateUDAngle( Human human, out float pAngle, out float pAngleAdd, float pMin, float pMax, float pDt )
    {
        if ( Math.AbsFloat(pAngleAdd) > 0.001 )
        {
            float aimingUDAngle = human.GetCommandModifier_Weapons().GetBaseAimingAngleUD();

            //!    pAngle + pAngleAdd == aimingUDAngl
            pAngleAdd = pAngle + pAngleAdd - aimingUDAngle;
            pAngle = aimingUDAngle;
        }

        pAngle += human.GetInputController().GetAimChange()[1] * Math.RAD2DEG;
        pAngle = Limit( pAngle, pMin, pMax );

        pAngleAdd = Math.SmoothCD( pAngleAdd, 0, m_fUDAngleVel, 0.14, 1000, pDt );
        
        return Limit( pAngle + pAngleAdd, pMin, pMax );
    }

    float UpdateLRAngle( Human human, float pAngle, float pMin, float pMax, float pDt )
    {
        pAngle += human.GetInputController().GetAimChange()[0] * Math.RAD2DEG;
        pAngle = Limit( pAngle, pMin, pMax );

        m_fLRAngleVel[0] = 0;

        pAngle = Math.SmoothCD( pAngle, 0, m_fLRAngleVel, 0.14, 1000, pDt );

        return pAngle;
    }

    override void OnUpdate( float timeslice )
    {
        super.OnUpdate( timeslice );

        Human human = Human.Cast( SelectedTarget );

        if ( human )
        {
            vector pos;
            vector rot;

            int bone = human.GetBoneIndexByName( "Head" )

            pos = human.GetBonePositionWS( bone );

            float udAngle = UpdateUDAngle( human, m_fUpDownAngle, m_fUpDownAngleAdd, CONST_UD_MIN, CONST_UD_MAX, timeslice );
            m_CurrentCameraPitch = udAngle;
            m_fLeftRightAngle = UpdateLRAngle( human, m_fLeftRightAngle, CONST_LR_MIN, CONST_LR_MAX, timeslice );

            rot[0] = m_fLeftRightAngle;
            rot[1] = udAngle;
            rot[2] = 0;

            vector direction = rot.AnglesToVector();

            direction = direction * 0.1;

            pos = pos + direction;

            SetPosition( pos );
            SetOrientation( rot );
        }
    }
}