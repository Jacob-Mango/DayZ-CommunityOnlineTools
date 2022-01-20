enum JMCameraModuleRPC
{
	INVALID = 10160,
	Enter,
	Leave,
	COUNT
};

[CF_RegisterModule(JMCameraModule)]
class JMCameraModule: JMRenderableModuleBase
{
	protected float m_CurrentSmoothBlur;
	protected float m_CurrentFOV;
	protected float m_TargetFOV;

	override void OnInit()
	{
		super.OnInit();

		Bind("ToggleCamera", "UACameraToolToggleCamera", true);
		Bind("ZoomForwards", "UACameraToolZoomForwards", true);
		Bind("ZoomBackwards", "UACameraToolZoomBackwards", true);
		
		GetPermissionsManager().RegisterPermission( "Camera.View" );

		m_CurrentSmoothBlur = 0.0;
		m_CurrentFOV = 1.0;
		m_TargetFOV = 1.0;
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
		return "#STR_COT_CAMERA_MODULE_NAME";
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
	
	override void OnUpdate(Class sender, CF_EventArgs args)
	{		
		if (!IsMissionClient() || !CurrentActiveCamera)
		{
			return;
		}
	
		float speed = 0.2;
		m_CurrentSmoothBlur = Math.Lerp( m_CurrentSmoothBlur, CAMERA_SMOOTH_BLUR, speed );
		PPEffects.SetBlur( m_CurrentSmoothBlur );

		if ( m_CurrentFOV != m_TargetFOV ) 
		{
			m_CurrentFOV = Math.Lerp( m_CurrentFOV, m_TargetFOV, CF_EventUpdateArgs.Cast(args).DeltaTime * CAMERA_FOV_SPEED_MODIFIER );
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
			
			// CurrentActiveCamera.SetFocus( CAMERA_FDIST, CAMERA_BLUR );
			PPEffects.OverrideDOF( true, CAMERA_FDIST, CAMERA_FLENGTH, CAMERA_FNEAR, CAMERA_BLUR, CAMERA_DOFFSET );
			// PPEffects.SetBlurOptics( 0 );
		}
	}
	
	Camera GetCamera()
	{
		return CurrentActiveCamera;
	}

	override int GetRPCMin()
	{
		return JMCameraModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMCameraModuleRPC.COUNT;
	}

	override void OnRPC(Class sender, CF_EventArgs args)
	{
		auto rpcArgs = CF_EventRPCArgs.Cast(args);

		switch (rpcArgs.ID)
		{
		case JMCameraModuleRPC.Enter:
			RPC_Enter(rpcArgs.Context, rpcArgs.Sender, rpcArgs.Target);
			break;
		case JMCameraModuleRPC.Leave:
			RPC_Leave(rpcArgs.Context, rpcArgs.Sender, rpcArgs.Target);
			break;
		}
	}

	void Enter()
	{
		if ( IsMissionOffline() )
		{
			Server_Enter( NULL, GetGame().GetPlayer() );
		} else if ( IsMissionClient() )
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

		PlayerBase player;
		if ( Class.CastTo( player, target ) )
		{
			position = player.GetBonePositionWS( player.GetBoneIndexByName( "Head" ) );
			//player.GetInputController().SetDisabled( true );
		}

		if ( IsMissionOffline() )
		{
			CurrentActiveCamera = JMCameraBase.Cast( GetGame().CreateObject( "JMCinematicCamera", position, false ) );

			CurrentActiveCamera.SetActive( true );
			
			if ( GetGame().GetPlayer() )
			{
				GetGame().GetPlayer().GetInputController().SetDisabled( true );
			}
		} else 
		{
			GetGame().SelectPlayer( sender, NULL );

			GetGame().SelectSpectator( sender, "JMCinematicCamera", position );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMCameraModuleRPC.Enter, true, sender );
		}

		GetCommunityOnlineToolsBase().Log( sender, "Entered the Free Camera");
	}

	private void RPC_Enter( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Camera.View", senderRPC ) )
				return;

			Server_Enter( senderRPC, target );
		} else
		{
			// RPC was sent from the server, permission would've been verified there.

			Client_Enter();
		}
	}

	void Leave()
	{
		if ( IsMissionOffline() )
		{
			Server_Leave( NULL, GetGame().GetPlayer() );
		} else if ( IsMissionClient() )
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
		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Camera.View", senderRPC ) )
				return;

			Server_Leave( senderRPC, target );
		} else
		{
			// RPC was sent from the server, permission would've been verified there.

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
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF" ) );
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