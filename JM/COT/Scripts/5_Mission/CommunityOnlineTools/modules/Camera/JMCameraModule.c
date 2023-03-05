class JMCameraModule: JMRenderableModuleBase
{
	protected float m_CurrentSmoothBlur;
	protected float m_CurrentFOV;
	protected float m_TargetFOV;
	protected float m_UpdateTime;
	bool m_EnableFullmapCamera;
	bool m_HideGrass;
	int m_GrassPatchX;
	int m_GrassPatchY;

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

				if ((CurrentActiveCamera.IsInherited(JMCinematicCamera) && m_EnableFullmapCamera) || (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMSpectatorCamera)))
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

			if ( m_HideGrass )
			{
				vector pos = CurrentActiveCamera.GetPosition();

				//! @note: Flatten functions have a size/diameter limit of around 60
				//! We also want some overlap so that there are no visible seams
				float side = 50;
				float x = pos[0] - side * 2;
				float z = pos[2] - side * 2;
				/*    _ _ _
				 *  _|_|_|_|_
				 * |_|_|_|_|_|
				 * |_|_|X|_|_|
				 * |_|_|_|_|_|
				 *   |_|_|_|
				 */
				int row = m_GrassPatchX;
				int col = m_GrassPatchY;
				//for (int row = 0; row < 5; row++)
				//{
					//for (int col = 0; col < 5; col++)
					//{
						if ((row > 0 && row < 4) || (col > 0 && col < 4))
							GetGame().GetWorld().FlattenGrassBox(x + side * row, z + side * col, side * 1.2, 0, 0, 0.1, 1.0);
					//}
				//}
				m_GrassPatchY++;
				if (m_GrassPatchY == 5)
				{
					m_GrassPatchY = 0;
					m_GrassPatchX++;
					if (m_GrassPatchX == 5)
					{
						m_GrassPatchX = 0;
					}
				}
			}
		}
	}
	#endif
	
	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();
		
		Bind( new JMModuleBinding( "ToggleCamera",		"UACameraToolToggleCamera",		true 	) );
		Bind( new JMModuleBinding( "ZoomForwards",		"UACameraToolZoomForwards",		true 	) );
		Bind( new JMModuleBinding( "ZoomBackwards",		"UACameraToolZoomBackwards",	true 	) );
		Bind( new JMModuleBinding( "Toggle3rdPerson",	"UAPersonView",	true 	) );
		Bind( new JMModuleBinding( "LeftShoulder",		"UALeanLeft",	true 	) );
		Bind( new JMModuleBinding( "RightShoulder",		"UALeanRight",	true 	) );
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
		case JMCameraModuleRPC.Leave_Finish:
			RPC_Leave_Finish( ctx, sender, target );
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
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "Client_Enter");
		#endif

		COT_PreviousActiveCamera = CurrentActiveCamera;
		if (COT_PreviousActiveCamera)
			COT_PreviousActiveCamera.SetActive( false );

		if ( Class.CastTo( CurrentActiveCamera, Camera.GetCurrentCamera() ) )
		{
			CurrentActiveCamera.SetActive( true );

			if (COT_PreviousActiveCamera)
				CurrentActiveCamera.SetDirection(COT_PreviousActiveCamera.GetDirection());
			
			Human player = GetGame().GetPlayer();
			if ( player )
			{
				if (!COT_PreviousActiveCamera)
				{
					vector headTransform[4];
					player.GetBoneTransformWS(player.GetBoneIndexByName( "Head" ), headTransform);
					CurrentActiveCamera.SetDirection(headTransform[1]);
				}

				player.GetInputController().SetDisabled( true );
			}
		}
	}

	private void Server_Enter( PlayerIdentity sender, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Server_Enter").Add(sender).Add(target.ToString());
		#endif

		vector position = Vector( 0, 0, 0 );

		PlayerBase player;
		if ( Class.CastTo( player, target ) )
		{
			if (player.m_JM_SpectatedPlayer)
				player = player.m_JM_SpectatedPlayer;

			position = player.GetBonePositionWS( player.GetBoneIndexByName( "Head" ) ) + player.GetDirection() * 0.12;
			//player.GetInputController().SetDisabled( true );
		}
		else if ( target )
		{
			position = target.GetPosition();
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
		auto trace = CF_Trace_2(this, "RPC_Enter").Add(senderRPC).Add(target.ToString());
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

	private void Client_Leave(int waitForPlayerIdleTimeout = 0)
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "Client_Leave");
		#endif
Print("JMCameraModule::Client_Leave - current cam " + CurrentActiveCamera);
		CurrentActiveCamera.SetActive( false );
		CurrentActiveCamera = COT_PreviousActiveCamera;

Print("JMCameraModule::Client_Leave - previous cam " + COT_PreviousActiveCamera);
		if (COT_PreviousActiveCamera)
		{
Print("JMCameraModule::Client_Leave - switching to prev cam " + COT_PreviousActiveCamera);
			CurrentActiveCamera.SetActive(true);
			COT_PreviousActiveCamera = NULL;
			return;
		}
		
		PPEffects.ResetDOFOverride();

Print("JMCameraModule::Client_Leave - player " + GetGame().GetPlayer());
		if ( GetGame().GetPlayer() )
		{
			GetGame().GetPlayer().GetInputController().SetDisabled( false );
		}

		PlayerBase player;
		if (waitForPlayerIdleTimeout && Class.CastTo(player, GetGame().GetPlayer()))
		{
Print("JMCameraModule::Client_Leave - waiting for player to be idle, timestamp " + GetGame().GetTickTime());
			player.COT_EnableBonePositionUpdate(true);
			Client_Check_Leave(player, waitForPlayerIdleTimeout);
			if (waitForPlayerIdleTimeout > 1000)
				COTCreateLocalAdminNotification(new StringLocaliser("Leaving freecam..."));
		}
Print("JMCameraModule::Client_Leave - left cam");
	}

	void Client_Check_Leave(PlayerBase player, int waitForPlayerIdleTimeout)
	{
		if (!player.COT_IsAnimationIdle() && waitForPlayerIdleTimeout > 0)
		{
			GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Client_Check_Leave, 250, false, player, waitForPlayerIdleTimeout - 250 );
		}
		else
		{
Print("JMCameraModule::Client_Check_Leave - player idle, timestamp " + GetGame().GetTickTime());
			player.COT_EnableBonePositionUpdate(false);
			COTCreateLocalAdminNotification(new StringLocaliser("Left freecam. In case your 3rd person camera or collision is broken, use the “Sit Crossed” emote to fix it."), "set:ccgui_enforce image:HudBuild", 5);

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(NULL, JMCameraModuleRPC.Leave_Finish, true, NULL);
		}
	}

	private void Server_Leave( PlayerIdentity sender, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Server_Leave").Add(sender).Add(target.ToString());
		#endif
Print("JMCameraModule::Server_Leave - target " + target);
		PlayerBase player;
		if ( Class.CastTo( player, target ) )
		{
			vector spectatorPosition = player.GetPosition();
			int waitForPlayerIdleTimeout;
			if (!player.m_JM_SpectatedPlayer && player.m_JM_CameraPosition != vector.Zero)
			{
				player.COTResetSpectator();

				if ( player.HasLastPosition() )
				{
					if (COT_SurfaceIsWater(player.GetLastPosition()))
						waitForPlayerIdleTimeout = 250;
					else if ( vector.DistanceSq( player.GetLastPosition(), spectatorPosition ) > 0.01 )
						waitForPlayerIdleTimeout = 5000;
				}
			}

			player.m_JM_CameraPosition = vector.Zero;

			if ( GetGame().IsMultiplayer() )
			{
				ScriptRPC rpc = new ScriptRPC();
				rpc.Write(waitForPlayerIdleTimeout);
				rpc.Send( NULL, JMCameraModuleRPC.Leave, true, sender );
			} else
			{
				Client_Leave();
			}

			GetCommunityOnlineToolsBase().Log( sender, "Left the Free Camera");
Print("JMCameraModule::Server_Leave - spectated player " + player.m_JM_SpectatedPlayer);
			if (player.m_JM_SpectatedPlayer)
				return;

			if (!waitForPlayerIdleTimeout)
				GetGame().SelectPlayer(sender, player);
		}
	}

	private void RPC_Leave( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RPC_Leave").Add(senderRPC).Add(target.ToString());
		#endif
Print("JMCameraModule::RPC_Leave - timestamp " + GetGame().GetTickTime());
		if ( IsMissionHost() )
		{
			if ( !GetPermissionsManager().HasPermission( "Camera.View", senderRPC ) )
				return;

			Server_Leave( senderRPC, target );
		} else
		{
			// RPC was sent from the server, permission would've been verified there.

			int waitForPlayerIdleTimeout;
			if ( !ctx.Read( waitForPlayerIdleTimeout ) )
				return;

			Client_Leave(waitForPlayerIdleTimeout);
		}
	}

	private void RPC_Leave_Finish( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RPC_Leave_Finish").Add(senderRPC).Add(target);
#endif
Print("JMCameraModule::RPC_Leave_Finish - timestamp " + GetGame().GetTickTime());
		if ( !GetPermissionsManager().HasPermission( "Camera.View", senderRPC ) )
			return;

		PlayerBase player;
		if (!Class.CastTo(player, senderRPC.GetPlayer()) || player.m_JM_SpectatedPlayer)
			return;

		GetGame().SelectPlayer(senderRPC, player);
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
					EnterFullmap(player);
				player.m_JM_CameraPosition = position;
				if (!player.m_JM_SpectatedPlayer)
					player.COTUpdateSpectatorPosition();
			}
		}
	}
	
	void EnterFullmap(PlayerBase player)
	{
		if (player.m_JM_CameraPosition == vector.Zero)
		{
			player.SetLastPosition();
			player.COTSetGodMode( true, false );  //! Enable godmode and remember previous state of GetAllowDamage
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

		if ( CurrentActiveCamera && CurrentActiveCamera.IsInherited(JMCinematicCamera) )
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
};
