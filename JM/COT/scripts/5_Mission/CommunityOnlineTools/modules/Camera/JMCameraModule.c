class JMCameraModule: JMRenderableModuleBase
{
	protected float m_CurrentSmoothBlur;
	protected float m_CurrentFOV;
	protected float m_TargetFOV;

	void JMCameraModule()
	{
		GetRPCManager().AddRPC( "COT_Camera", "EnterCamera", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT_Camera", "LeaveCamera", this, SingeplayerExecutionType.Client );

		GetRPCManager().AddRPC( "COT_Camera", "UpdateCameraNetworkBubble", this, SingeplayerExecutionType.Server );

		GetPermissionsManager().RegisterPermission( "CameraTools.View" );
		GetPermissionsManager().RegisterPermission( "CameraTools.EnterCamera" );
		GetPermissionsManager().RegisterPermission( "CameraTools.LeaveCamera" );
		GetPermissionsManager().RegisterPermission( "CameraTools.UpdateNetworkBubble" );

		m_CurrentSmoothBlur = 0.0;
		m_CurrentFOV = 1.0;
		m_TargetFOV = 1.0;
	}

	void ~JMCameraModule()
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "CameraTools.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/camera_form.layout";
	}
	
	override void OnUpdate( float timeslice )
	{
		if ( !GetGame().IsMultiplayer() && CurrentActiveCamera )
		{
			float speed = 0.2;
			m_CurrentSmoothBlur = Math.Lerp( m_CurrentSmoothBlur, CAMERA_SMOOTH_BLUR, speed );
			PPEffects.SetBlur( m_CurrentSmoothBlur );

			if ( m_CurrentFOV != m_TargetFOV ) 
			{
				m_CurrentFOV = Math.Lerp( m_CurrentFOV, m_TargetFOV, timeslice * CAMERA_FOV_SPEED_MODIFIER );
				CurrentActiveCamera.SetFOV( m_CurrentFOV );
			}

			if ( CAMERA_DOF )
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

				if ( dist > 0 )
					CAMERA_FDIST = dist;
				
				CurrentActiveCamera.SetFocus( CAMERA_FDIST, CAMERA_BLUR );
				// PPEffects.OverrideDOF( true, CAMERA_FDIST, CAMERA_FLENGTH, CAMERA_FNEAR, CAMERA_BLUR, CAMERA_DOFFSET );
			}
		}
	}
	
	override void RegisterKeyMouseBindings() 
	{
		RegisterBinding( new JMModuleBinding( "ToggleCamera",		"UACameraToolToggleCamera",		true 	) );
		RegisterBinding( new JMModuleBinding( "ZoomForwards",		"UACameraToolZoomForwards",		true 	) );
		RegisterBinding( new JMModuleBinding( "ZoomBackwards",		"UACameraToolZoomBackwards",	true 	) );
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

		if ( type == CallType.Server )
		{
			GetGame().UpdateSpectatorPosition( data.param1 );
		}
	}

	void EnterCamera( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "CameraTools.EnterCamera", sender ) )
			return;

		if ( type == CallType.Server )
		{
			vector position = Vector( 0, 0, 0 );

			if ( target )
			{
				position = target.GetPosition();
			}

			GetGame().SelectPlayer( sender, NULL );

			PlayerBase human = PlayerBase.Cast( target );

			if ( human )
			{
				// human.Save();
				// human.Delete();

				position = human.GetBonePositionWS( human.GetBoneIndexByName("Head") );
			}

			GetGame().SelectSpectator( sender, "JMCinematicCamera", position );

			GetRPCManager().SendRPC( "COT_Camera", "EnterCamera", new Param, true, sender );

			GetCommunityOnlineToolsBase().Log( sender, "Entered the Free Camera");
		}

		if ( type == CallType.Client && !COTPlayerIsRemoved )
		{
			if ( GetGame().IsMultiplayer() )
			{
				CurrentActiveCamera = JMCameraBase.Cast( Camera.GetCurrentCamera() );
			} else 
			{
				CurrentActiveCamera = JMCameraBase.Cast( GetGame().CreateObject( "JMCinematicCamera", target.GetPosition(), false ) );
			}
			
			if ( CurrentActiveCamera )
			{
				CurrentActiveCamera.SetActive( true );
				
				if ( GetPlayer() )
				{
					GetPlayer().GetInputController().SetDisabled( true );
				}
			}
		}
	}

	void LeaveCamera( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "CameraTools.LeaveCamera", sender ) )
			return;

		if ( type == CallType.Server )
		{
			GetGame().SelectPlayer( sender, target );

			if ( GetGame().IsMultiplayer() )
			{
				GetRPCManager().SendRPC( "COT_Camera", "LeaveCamera", new Param, true, sender );
			} 

			GetCommunityOnlineToolsBase().Log( sender, "Left the Free Camera");
		}

		if ( type == CallType.Client && !COTPlayerIsRemoved )
		{
			if ( !GetGame().IsMultiplayer() )
			{
				GetGame().SelectPlayer( sender, target );
			}

			CurrentActiveCamera.SetActive( false );
			CurrentActiveCamera = NULL;
			
			PPEffects.ResetDOFOverride();

			if ( GetPlayer() )
			{
				GetPlayer().GetInputController().SetDisabled( false );
			}
		}
	}

	void EnableCamera()
	{
		if ( CurrentActiveCamera || COTPlayerIsRemoved )
		{
			return;
		}

		GetRPCManager().SendRPC( "COT_Camera", "EnterCamera", new Param, false, NULL, GetPlayer() );
	}

	void DisableCamera()
	{
		if ( CurrentActiveCamera && !COTPlayerIsRemoved )
		{
			SetFreezeMouse( false );

			GetRPCManager().SendRPC( "COT_Camera", "LeaveCamera", new Param, false, NULL, GetPlayer() );
		}
	}

	void ZoomForwards( UAInput input )
	{
		if ( input.LocalValue() != 0 )
		{
			m_TargetFOV += input.LocalValue() * 0.01;
		}
	}

	void ZoomBackwards( UAInput input )
	{
		if ( input.LocalValue() != 0 )
		{
			m_TargetFOV -= input.LocalValue() * 0.01;
					
			if ( m_TargetFOV < 0.01 ) 
			{
				m_TargetFOV = 0.01;
			}
		}
	}
	
	void ToggleCamera( UAInput input )
	{
		if ( !( input.LocalPress() ) )
			return;

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
			return;
		}

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
		if ( !CurrentActiveCamera )
			return NULL;

		return CurrentActiveCamera.SelectedTarget;
	}

	vector GetTargetPos() 
	{
		if ( !CurrentActiveCamera )
			return "0 0 0";

		return CurrentActiveCamera.TargetPosition;
	}
	
	static void SetFreezeCam( bool freeze ) 
	{
		if ( !CurrentActiveCamera )
			return;

		CurrentActiveCamera.MoveFreeze = freeze;
	}
	
	static void SetFreezeMouse( bool freeze ) 
	{
		if ( !CurrentActiveCamera )
			return;

		CurrentActiveCamera.LookFreeze = freeze;
	}
}