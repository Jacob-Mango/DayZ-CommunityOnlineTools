enum JMCameraModuleRPC
{
    INVALID = 10160,
    Enter,
    Leave,
    COUNT
};

class JMCameraModule: JMRenderableModuleBase
{
	protected float m_CurrentSmoothBlur;
	protected float m_CurrentFOV;
	protected float m_TargetFOV;

	void JMCameraModule()
	{
		GetPermissionsManager().RegisterPermission( "Camera.View" );

		m_CurrentSmoothBlur = 0.0;
		m_CurrentFOV = 1.0;
		m_TargetFOV = 1.0;
	}

	void ~JMCameraModule()
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Camera.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleCamera";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/camera_form.layout";
	}

	override string GetTitle()
	{
		return "Camera Tool";
	}
	
	override string GetImageSet()
	{
		return "rover_imageset";
	}

	override string GetIconName()
	{
		return "camera";
	}

	override bool ImageIsIcon()
	{
		return true;
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
		super.RegisterKeyMouseBindings();
		
		RegisterBinding( new JMModuleBinding( "ToggleCamera",		"UACameraToolToggleCamera",		true 	) );
		RegisterBinding( new JMModuleBinding( "ZoomForwards",		"UACameraToolZoomForwards",		true 	) );
		RegisterBinding( new JMModuleBinding( "ZoomBackwards",		"UACameraToolZoomBackwards",	true 	) );
	}

	Camera GetCamera()
	{
		return CurrentActiveCamera;
	}

	int GetRPCMin()
	{
		return JMCameraModuleRPC.INVALID;
	}

	int GetRPCMax()
	{
		return JMCameraModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMCameraModuleRPC.Enter:
			RPC_Enter( ctx, sender, target );
			break;
		case JMCameraModuleRPC.Leave:
			RPC_Leave( ctx, sender, target );
			break;
		}
    }

	void Enter()
	{

		if ( !IsMissionOffline() && IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( GetGame().GetPlayer(), JMCameraModuleRPC.Enter, true, NULL );
		}
	}

	private void Client_Enter()
	{		
		if ( Class.CastTo( CurrentActiveCamera, Camera.GetCurrentCamera() ) )
		{
			CurrentActiveCamera.SetActive( true );
			
			if ( GetGame().GetPlayer() )
			{
				GetGame().GetPlayer().GetInputController().SetDisabled( true );
			}
		}
	}

	private void Server_Enter( PlayerIdentity sender, Object target )
	{
		vector position = Vector( 0, 0, 0 );

		if ( target )
		{
			position = target.GetPosition();
		}

		GetGame().SelectPlayer( sender, NULL );

		PlayerBase player;
		if ( Class.CastTo( player, target ) )
		{
			position = player.GetBonePositionWS( player.GetBoneIndexByName( "Head" ) );
			//player.GetInputController().SetDisabled( true );
		}

		if ( GetGame().IsMultiplayer() )
		{
			GetGame().SelectSpectator( sender, "JMCinematicCamera", position );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMCameraModuleRPC.Enter, true, sender );
		} else
		{
			CurrentActiveCamera = JMCameraBase.Cast( GetGame().CreateObject( "JMCinematicCamera", position, false ) );

			Client_Enter();
		}

		GetCommunityOnlineToolsBase().Log( sender, "Entered the Free Camera");
	}

	private void RPC_Enter( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Camera.View", senderRPC ) )
			return;

		if ( !IsMissionOffline() && IsMissionHost() )
		{
			Server_Enter( senderRPC, target );
		} else
		{
			Client_Enter();
		}
	}

	void Leave()
	{
		if ( !IsMissionOffline() && IsMissionClient() )
		{
			SetFreezeMouse( false );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( GetGame().GetPlayer(), JMCameraModuleRPC.Leave, true, NULL );
		}
	}

	private void Client_Leave()
	{
		CurrentActiveCamera.SetActive( false );
		CurrentActiveCamera = NULL;
		
		PPEffects.ResetDOFOverride();

		if ( GetGame().GetPlayer() )
		{
			GetGame().GetPlayer().GetInputController().SetDisabled( false );
		}
	}

	private void Server_Leave( PlayerIdentity sender, Object target )
	{
		PlayerBase player;
		if ( Class.CastTo( player, target ) )
		{
			GetGame().SelectPlayer( sender, player );

			//player.GetInputController().SetDisabled( false );

			if ( GetGame().IsMultiplayer() )
			{
				ScriptRPC rpc = new ScriptRPC();
				rpc.Send( NULL, JMCameraModuleRPC.Leave, true, sender );
			} else
			{
				Client_Leave();
			}

			GetCommunityOnlineToolsBase().Log( sender, "Left the Free Camera");
		}
	}

	private void RPC_Leave( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Camera.View", senderRPC ) )
			return;

		if ( IsMissionHost() )
		{
			Server_Leave( senderRPC, target );
		} else
		{
			Client_Leave();
		}
	}
	
	void ToggleCamera( UAInput input )
	{
		if ( !( input.LocalPress() ) )
		{
			return;
		}

		if ( !GetCommunityOnlineToolsBase().IsActive() )
		{
			CreateLocalAdminNotification( "Community Online Tools is currently toggled off." );
			return;
		}

		if ( CurrentActiveCamera )
		{
			if ( !COTPlayerIsRemoved )
			{
				Leave();
			}
		} else
		{
			Enter();
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