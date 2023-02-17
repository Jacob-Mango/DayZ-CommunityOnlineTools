enum JMCameraModuleRPC
{
	INVALID = 10160,
	Enter,
	Leave,
	UpdatePosition,
	COUNT
};

class JMCameraModule: JMRenderableModuleBase
{
	protected float m_CurrentSmoothBlur;
	protected float m_CurrentFOV;
	protected float m_TargetFOV;
	protected float m_UpdateTime;
	bool m_EnableFullmapCamera;

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
	
	#ifdef SERVER
	override void EnableUpdate()
	{
	}
	#else
	override void OnUpdate( float timeslice )
	{
		m_UpdateTime += timeslice;

		if ( IsMissionClient() && CurrentActiveCamera )
		{
			float speed = 0.2;
			m_CurrentSmoothBlur = Math.Lerp( m_CurrentSmoothBlur, CAMERA_SMOOTH_BLUR, speed );
			PPEffects.SetBlur( m_CurrentSmoothBlur );

			m_CurrentFOV = CurrentActiveCamera.GetCurrentFOV();
			if ( m_CurrentFOV != m_TargetFOV && (!CurrentActiveCamera.m_JM_IsADS || CurrentActiveCamera.m_JM_3rdPerson) ) 
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
				
				// CurrentActiveCamera.SetFocus( CAMERA_FDIST, CAMERA_BLUR );
				PPEffects.OverrideDOF( true, CAMERA_FDIST, CAMERA_FLENGTH, CAMERA_FNEAR, CAMERA_BLUR, CAMERA_DOFFSET );
				// PPEffects.SetBlurOptics( 0 );
			}

			if (m_UpdateTime > 1.0)
			{
				m_UpdateTime = 0.0;

				if (m_EnableFullmapCamera)
				{
					auto player = PlayerBase.Cast(GetGame().GetPlayer());
					if (GetGame().IsClient())
					{
						ScriptRPC rpc = new ScriptRPC();
						rpc.Write(CurrentActiveCamera.GetPosition());
						rpc.Send(player, JMCameraModuleRPC.UpdatePosition, true, NULL);
					}
					else if (!player.m_JM_SpectatedPlayer)
					{
						EnterFullmap(player);
						player.m_JM_CameraPosition = CurrentActiveCamera.GetPosition();
						player.COTUpdateSpectatorPosition();
					}
				}
			}
		}
	}
	#endif
	
	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();
		
		RegisterBinding( new JMModuleBinding( "ToggleCamera",		"UACameraToolToggleCamera",		true 	) );
		RegisterBinding( new JMModuleBinding( "ZoomForwards",		"UACameraToolZoomForwards",		true 	) );
		RegisterBinding( new JMModuleBinding( "ZoomBackwards",		"UACameraToolZoomBackwards",	true 	) );
		RegisterBinding( new JMModuleBinding( "Toggle3rdPerson",	"UAPersonView",	true 	) );
		RegisterBinding( new JMModuleBinding( "LeftShoulder",		"UALeanLeft",	true 	) );
		RegisterBinding( new JMModuleBinding( "RightShoulder",		"UALeanRight",	true 	) );
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
		case JMCameraModuleRPC.UpdatePosition:
			RPC_UpdatePosition( ctx, sender, target );
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

	private void RPC_Enter( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_Enter").Add(senderRPC);
		#endif

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
			vector spectatorPosition = player.GetPosition();
			if (!player.m_JM_SpectatedPlayer && player.m_JM_CameraPosition != vector.Zero)
				player.COTResetSpectator();

			player.m_JM_CameraPosition = vector.Zero;

			//player.GetInputController().SetDisabled( false );

			if ( GetGame().IsMultiplayer() )
			{
				ScriptRPC rpc = new ScriptRPC();
				rpc.Send( NULL, JMCameraModuleRPC.Leave, true, sender );
			} else
			{
				Client_Leave();
			}

			int delay;
			if ( player.HasLastPosition() )
			{
				float distance = vector.Distance( player.GetLastPosition(), spectatorPosition );
				if ( distance >= 1000 )
					delay = 1000;
			}

			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( GetGame().SelectPlayer, delay, false, sender, player );

			GetCommunityOnlineToolsBase().Log( sender, "Left the Free Camera");
		}
	}

	private void RPC_Leave( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_Leave").Add(senderRPC);
		#endif

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

	private void RPC_UpdatePosition( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_UpdatePosition").Add(senderRPC);
		#endif

		if ( GetGame().IsDedicatedServer() )
		{
			if ( !GetPermissionsManager().HasPermission( "Camera.View", senderRPC ) )
				return;

			vector position;
			if (!ctx.Read(position) || position == vector.Zero)
				return;

			PlayerBase player;
			if (Class.CastTo(player, target))
			{
				if (!player.m_JM_SpectatedPlayer)
				{
					EnterFullmap(player);
					player.m_JM_CameraPosition = position;
					player.COTUpdateSpectatorPosition();
				}
			}
		}
	}
	
	void EnterFullmap(PlayerBase player)
	{
		if (player.m_JM_CameraPosition == vector.Zero)
		{
			player.SetLastPosition();
			player.COTSetGodMode( true );  //! Enable godmode and remember previous state of GetAllowDamage
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

	void Toggle3rdPerson( UAInput input )
	{
		if ( input.LocalPress() && CurrentActiveCamera )
			CurrentActiveCamera.m_JM_3rdPerson = !CurrentActiveCamera.m_JM_3rdPerson;
	}

	void LeftShoulder( UAInput input )
	{
		if ( input.LocalPress() && CurrentActiveCamera )
			CurrentActiveCamera.m_JM_LeftShoulder = true;
	}

	void RightShoulder( UAInput input )
	{
		if ( input.LocalPress() && CurrentActiveCamera )
			CurrentActiveCamera.m_JM_LeftShoulder = false;
	}

	void SetTargetFOV( float fov )
	{
		m_TargetFOV = fov;
	}
}