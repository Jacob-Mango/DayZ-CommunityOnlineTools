class CameraTool: EditorModule
{
    static float m_CurrentSmoothBlur;

    void CameraTool()
    {
        GetRPCManager().AddRPC( "COT_Camera", "EnterCamera", this, SingeplayerExecutionType.Client );
        GetRPCManager().AddRPC( "COT_Camera", "LeaveCamera", this, SingeplayerExecutionType.Client );

        GetRPCManager().AddRPC( "COT_Camera", "UpdateCameraNetworkBubble", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "CameraTools.View" );
        GetPermissionsManager().RegisterPermission( "CameraTools.EnterCamera" );
        GetPermissionsManager().RegisterPermission( "CameraTools.LeaveCamera" );
        GetPermissionsManager().RegisterPermission( "CameraTools.UpdateNetworkBubble" );
    }

    void ~CameraTool()
    {
    }

    override bool HasAccess()
    {
        return GetPermissionsManager().HasPermission( "CameraTools.View" );
    }

    override string GetLayoutRoot()
    {
        return "JM/COT/gui/layouts/Camera/CameraSettings.layout";
    }
    
    override void OnUpdate( float timeslice )
    {
        if ( GetGame().IsClient() )
        {
            float speed = 0.2;
            m_CurrentSmoothBlur = Math.Lerp( m_CurrentSmoothBlur, CAMERA_SMOOTH_BLUR, speed );
            PPEffects.SetBlur( m_CurrentSmoothBlur );

             if ( CAMERA_DOF && CurrentActiveCamera ) // DOF enabled
            {
                vector from = GetGame().GetCurrentCameraPosition();

                float dist = 0.0;

                if ( CurrentActiveCamera.SelectedTarget )
                {
                    dist = vector.Distance( from, CurrentActiveCamera.SelectedTarget.GetPosition() );
                } else if ( CAMERA_AFOCUS )
                {
                    vector to = from + (GetGame().GetCurrentCameraDirection() * 9999);
                    vector contact_pos;
                    
                    DayZPhysics.RaycastRV( from, to, contact_pos, NULL, NULL, NULL , NULL, NULL, false, false, ObjIntersectIFire);
                    dist = vector.Distance( from, contact_pos );
                }

                if ( dist > 0 ) CAMERA_FDIST = dist;
                
                PPEffects.OverrideDOF(true, CAMERA_FDIST, CAMERA_FLENGTH, CAMERA_FNEAR, CAMERA_BLUR, CAMERA_DOFFSET);
            }
        }
    }
    
    override void RegisterKeyMouseBindings() 
    {
        KeyMouseBinding toggleCamera  = new KeyMouseBinding( GetModuleType(), "ToggleCamera", "Toggle camera.", true );
        toggleCamera.AddBinding( "kInsert" );
        RegisterKeyMouseBinding( toggleCamera );
    }

    Camera GetCamera()
    {
        return CurrentActiveCamera;
    }

    void UpdateCameraNetworkBubble( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "CameraTools.UpdateNetworkBubble", sender ) )
            return;

        Param1< vector > data;
        if ( !ctx.Read( data ) ) return;

        if( type == CallType.Server )
        {
            GetGame().UpdateSpectatorPosition( data.param1 );
        }
    }

    void EnterCamera( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "CameraTools.EnterCamera", sender ) )
            return;

        if( type == CallType.Server )
        {
            vector position = Vector( 0, 0, 0 );

            if ( target )
            {
                position = target.GetPosition();
            }

            Human human = Human.Cast( target );

            if ( human )
            {
                position = human.GetBonePositionWS( human.GetBoneIndexByName("Head") );
            }
            
            GetGame().SelectSpectator( sender, "CinematicCamera", position );

            GetGame().SelectPlayer( sender, NULL );

            GetRPCManager().SendRPC( "COT_Camera", "EnterCamera", new Param, true, sender );

            COTLog( sender, "Entered the Free Camera");
        }

        if( type == CallType.Client )
        {
            if ( GetGame().IsMultiplayer() )
            {
                CurrentActiveCamera = COTCamera.Cast( Camera.GetCurrentCamera() );
            } else 
            {
                CurrentActiveCamera = COTCamera.Cast( GetGame().CreateObject( "CinematicCamera", target.GetPosition(), false ) );
            }
            
            if ( CurrentActiveCamera )
            {
                CurrentActiveCamera.SetActive( true );
                GetPlayer().GetInputController().SetDisabled( true );
            }
        }
    }

    void LeaveCamera( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
    {
        if ( !GetPermissionsManager().HasPermission( "CameraTools.LeaveCamera", sender ) )
            return;

        Param1< vector > data;

        if( type == CallType.Server )
        {
            GetGame().SelectPlayer( sender, target );

            if ( ctx.Read( data ) ) 
            {
                if ( target ) 
                {
                    target.SetPosition( data.param1 );
                }
            } 

            if ( GetGame().IsMultiplayer() )
            {
                GetRPCManager().SendRPC( "COT_Camera", "LeaveCamera", new Param1<vector>(data.param1), true, sender );
            } 

            COTLog( sender, "Left the Free Camera");
        }

        if( type == CallType.Client )
        {
            if ( !GetGame().IsMultiplayer() )
            {
                GetGame().SelectPlayer( sender, target );

                if ( ctx.Read( data ) ) 
                {
                    if ( target ) 
                    {
                        target.SetPosition( data.param1 );
                    }
                } 
            }

            CurrentActiveCamera.SetActive( false );
            CurrentActiveCamera = NULL;
            
            PPEffects.ResetDOFOverride();

            GetPlayer().GetInputController().SetDisabled( false );
        }
    }

    void EnableCamera()
    {
        if ( CurrentActiveCamera )
        {
            return;
        }

        GetRPCManager().SendRPC( "COT_Camera", "EnterCamera", new Param, true, NULL, GetPlayer() );
    }

    void DisableCamera()
    {
        if ( CurrentActiveCamera )
        {
            SetFreezeMouse(false);

            vector position = "0 0 0";

            if( CTRL() || SHIFT() )
            {
                position = CurrentActiveCamera.GetPosition();
                position[1] = GetGame().SurfaceY( position[0], position[2] );
            }
            else
            {
                position = GetCursorPos();
            }

            GetRPCManager().SendRPC( "COT_Camera", "LeaveCamera", new Param1<vector>( position ), true, NULL, GetPlayer() );
        }
    }
    
    void ToggleCamera() 
    {
        if ( GetGame().IsMultiplayer() == false ) return;

        if ( CurrentActiveCamera )
        {
            DisableCamera();
        }
        else
        {
            EnableCamera();
        }
    }
    
    Object GetTargetObject()
    {
        if ( !CurrentActiveCamera ) return NULL;
        return CurrentActiveCamera.SelectedTarget;
    }

    vector GetTargetPos() 
    {
        if ( !CurrentActiveCamera ) return "0 0 0";
        return CurrentActiveCamera.TargetPosition;
    }
    
    static void SetFreezeCam( bool freeze ) 
    {
        if ( !CurrentActiveCamera ) return;
        CurrentActiveCamera.MoveFreeze = freeze;
    }
    
    static void SetFreezeMouse( bool freeze ) 
    {
        if ( !CurrentActiveCamera ) return;
        CurrentActiveCamera.LookFreeze = freeze;
    }
}