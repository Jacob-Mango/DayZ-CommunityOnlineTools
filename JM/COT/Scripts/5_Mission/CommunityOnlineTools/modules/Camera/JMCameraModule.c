class JMCameraModule: JMRenderableModuleBase
{
	protected float m_CurrentSmoothBlur;
	protected float m_UpdateTime;

	// UI stuff
	float m_CurrentFOV;
	float m_TargetFOV;
	bool m_DOF;
	bool m_AutoFocus = true;
	float m_FDist = 0.2;
	float m_Flength = 20.0;
	float m_FNear = 185.0;
	float m_Blur = 4.0;
	float m_BlurStrength;
	float m_FocusDistance;
	float m_FocalLength;
	float m_FocalNear;
	float m_Exposure;
	float m_ChromAbb;
	float m_Vignette;

	bool m_EnableFullmapCamera;
	bool m_HideGrass;
	int m_GrassPatchX;
	int m_GrassPatchY;

	ref TFloatArray m_Times;
	ref TBoolArray m_IsSmooth;
	ref TVectorArray m_Positions;

	void JMCameraModule()
	{
		GetPermissionsManager().RegisterPermission( "Camera.View" );

		m_Times = new TFloatArray;
		m_IsSmooth = new TBoolArray;
		m_Positions = new TVectorArray;
		
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

	override string GetIconName()
	{
		return "JM\\COT\\GUI\\textures\\modules\\Camera.paa";
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
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
			if ( !CurrentActiveCamera.IsInherited(JMSpectatorCamera) ) 
			{
				float fov;

				if (GetUApi().GetInputByID(UAZoomIn).LocalValue())
					fov = GameConstants.DZPLAYER_CAMERA_FOV_EYEZOOM;
				else
					fov = m_TargetFOV;

				m_CurrentFOV = Math.Lerp( m_CurrentFOV, fov, timeslice * CAMERA_FOV_SPEED_MODIFIER );
				CurrentActiveCamera.SetFOV( m_CurrentFOV );
			}

			if ( m_DOF )
			{
				vector from = g_Game.GetCurrentCameraPosition();

				float dist = 0.0;

				if ( CurrentActiveCamera.SelectedTarget )
				{
					dist = vector.Distance( from, CurrentActiveCamera.SelectedTarget.GetPosition() );
				} else if ( m_AutoFocus )
				{
					vector to = from + (g_Game.GetCurrentCameraDirection() * 9999);
					vector contact_pos;
					
					DayZPhysics.RaycastRV( from, to, contact_pos, NULL, NULL, NULL , NULL, NULL, false, false, ObjIntersectIFire);
					dist = vector.Distance( from, contact_pos );
				}

				if ( dist > 0 )
					m_FDist = dist;
				
				CurrentActiveCamera.SetFocus( m_FDist, m_Blur );
				PPEffects.OverrideDOF( true, m_FDist, m_Flength, m_FNear, m_Blur, CAMERA_DOFFSET );
				PPEffects.SetChromAbb( CHROMABERX );
				PPEffects.SetVignette( VIGNETTE, 0, 0, 0, 0 );
				PPEffects.SetBloom( EXPOSURE, EXPOSURE, EXPOSURE );
				// PPEffects.SetBlurOptics( 0 );
			}

			if (m_UpdateTime > 1.0)
			{
				m_UpdateTime = 0.0;

				if ((CurrentActiveCamera.IsInherited(JMCinematicCamera) && m_EnableFullmapCamera) || (COT_PreviousActiveCamera && COT_PreviousActiveCamera.IsInherited(JMSpectatorCamera)))
				{
					auto player = PlayerBase.Cast(g_Game.GetPlayer());
					if (m_EnableFullmapCamera && player.GetCommand_Vehicle())
					{
						COTCreateLocalAdminNotification(new StringLocaliser("Disabled fullmap freecam update because you are in a vehicle. Please leave the vehicle first if you want to use fullmap freecam update."));
						m_EnableFullmapCamera = false;
						JMCameraForm.Cast(GetForm()).SetEnableFullmapCamera(false);
					}
					else if (g_Game.IsClient())
					{
						ScriptRPC rpc = new ScriptRPC();
						rpc.Write(CurrentActiveCamera.GetPosition());
						rpc.Send(player, JMCameraModuleRPC.UpdatePosition, true, NULL);
					}
					else if (!player.m_JM_SpectatedObject)
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

				if ((row > 0 && row < 4) || (col > 0 && col < 4))
					g_Game.GetWorld().FlattenGrassBox(x + side * row, z + side * col, side * 1.2, 0, 0, 0.1, 1.0);

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
	
	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
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
#ifdef DEVELOPER
		return;
#endif

		GetPlayer().COT_TempDisableOnSelectPlayer();

		GetPlayer().COT_RememberVehicle();

		if ( IsMissionOffline() )
		{
			Server_Enter(NULL, g_Game.GetPlayer(), g_Game.GetCurrentCameraPosition());
		} else if ( IsMissionClient() )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(g_Game.GetCurrentCameraPosition());
			rpc.Send( g_Game.GetPlayer(), JMCameraModuleRPC.Enter, true, NULL );
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
			
			Human player = g_Game.GetPlayer();
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

	[Obsolete("Use Server_Enter(sender, target, position)")]
	private void Server_Enter( PlayerIdentity sender, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Server_Enter").Add(sender).Add(target.ToString());
		#endif

		vector transform[4];
		vector position = Vector( 0, 0, 0 );

		PlayerBase player;
		if ( Class.CastTo( player, target ) )
		{
			if (player.m_JM_SpectatedObject)
				target = player.m_JM_SpectatedObject;
		}

		if ( target )
		{
			GetCommunityOnlineToolsBase().GetHeadTransform(target, transform, true);
		}

		position = transform[3];

		Server_Enter(sender, target, position);
	}

	private void Server_Enter(PlayerIdentity sender, Object target, vector position)
	{
		PlayerBase player;
		if ( Class.CastTo( player, target ) )
		{
			player.COT_RememberVehicle();
		}

		if ( IsMissionOffline() )
		{
			CurrentActiveCamera = JMCameraBase.Cast( g_Game.CreateObject( "JMCinematicCamera", position, false ) );

			CurrentActiveCamera.SetActive( true );
			
			if ( g_Game.GetPlayer() )
				g_Game.GetPlayer().GetInputController().SetDisabled( true );
		}
		else 
		{
			PlayerBase.Cast(sender.GetPlayer()).COT_TempDisableOnSelectPlayer();

			g_Game.SelectPlayer( sender, NULL );

			g_Game.SelectSpectator( sender, "JMCinematicCamera", position );

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

			vector position;
			if (!ctx.Read(position))
				return;

			Server_Enter( senderRPC, target, position );
		} else
		{
			// RPC was sent from the server, permission would've been verified there.

			Client_Enter();
		}
	}

	void Leave()
	{
		if (GetPlayer().COT_GetOutVehicle())
			return;

		if ( IsMissionOffline() )
		{
			Server_Leave( NULL, g_Game.GetPlayer() );
		} else if ( IsMissionClient() )
		{
			SetFreezeMouse( false );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( g_Game.GetPlayer(), JMCameraModuleRPC.Leave, true, NULL );
		}
	}

	private void Client_Leave(int waitForPlayerIdleTimeout = 0)
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "Client_Leave");
		#endif
Print("JMCameraModule::Client_Leave - current cam " + CurrentActiveCamera);
		CurrentActiveCamera.SetActive( false );

		if (CurrentActiveCamera.IsInherited(JMCinematicCamera))
		{
		#ifdef DIAG_DEVELOPER
			ErrorEx("g_Game.ObjectDeleteOnClient(CurrentActiveCamera)", ErrorExSeverity.INFO);
		#endif
			g_Game.ObjectDeleteOnClient(CurrentActiveCamera);
		}

		CurrentActiveCamera = null;

Print("JMCameraModule::Client_Leave - previous cam " + COT_PreviousActiveCamera);
		if (COT_PreviousActiveCamera)
		{
			if (!COT_PreviousActiveCamera.IsInherited(JMCinematicCamera))
			{
Print("JMCameraModule::Client_Leave - switching to prev cam " + COT_PreviousActiveCamera);
				CurrentActiveCamera = COT_PreviousActiveCamera;
				CurrentActiveCamera.SetActive(true);
			}

			COT_PreviousActiveCamera = NULL;

			if (CurrentActiveCamera)
				return;
		}
		
		PPEffects.ResetDOFOverride();

Print("JMCameraModule::Client_Leave - player " + g_Game.GetPlayer());
		if ( g_Game.GetPlayer() )
		{
			g_Game.GetPlayer().GetInputController().SetDisabled( false );
		}

		PlayerBase player;
		if (waitForPlayerIdleTimeout && Class.CastTo(player, g_Game.GetPlayer()))
		{
Print("JMCameraModule::Client_Leave - waiting for player to be idle, timestamp " + g_Game.GetTickTime());
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
			g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( Client_Check_Leave, 250, false, player, waitForPlayerIdleTimeout - 250 );
		}
		else
		{
Print("JMCameraModule::Client_Check_Leave - player idle, timestamp " + g_Game.GetTickTime());
			player.COT_EnableBonePositionUpdate(false);
			COTCreateLocalAdminNotification(new StringLocaliser("Left freecam. In case your 3rd person camera or collision is broken, use the “Sit Crossed” emote to fix it."), "set:ccgui_enforce image:HudBuild", 5);

			if (g_Game.IsMultiplayer())
			{
				ScriptRPC rpc = new ScriptRPC();
				rpc.Send(NULL, JMCameraModuleRPC.Leave_Finish, true, NULL);
			}
			else
			{
				//! offline/SP
				g_Game.SelectPlayer(null, player);
			}
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
			if (!player.m_JM_SpectatedObject && player.m_JM_CameraPosition != vector.Zero)
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

			if ( g_Game.IsMultiplayer() )
			{
				ScriptRPC rpc = new ScriptRPC();
				rpc.Write(waitForPlayerIdleTimeout);
				rpc.Send( NULL, JMCameraModuleRPC.Leave, true, sender );
			} else
			{
				Client_Leave(waitForPlayerIdleTimeout);
			}

			GetCommunityOnlineToolsBase().Log( sender, "Left the Free Camera");
Print("JMCameraModule::Server_Leave - spectated object " + player.m_JM_SpectatedObject);
			if (player.m_JM_SpectatedObject)
				return;

			if (!waitForPlayerIdleTimeout)
				g_Game.SelectPlayer(sender, player);
		}
	}

	private void RPC_Leave( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "RPC_Leave").Add(senderRPC).Add(target.ToString());
		#endif
Print("JMCameraModule::RPC_Leave - timestamp " + g_Game.GetTickTime());
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
Print("JMCameraModule::RPC_Leave_Finish - timestamp " + g_Game.GetTickTime());
		if ( !GetPermissionsManager().HasPermission( "Camera.View", senderRPC ) )
			return;

		PlayerBase player;
		if (!Class.CastTo(player, senderRPC.GetPlayer()) || player.m_JM_SpectatedObject)
			return;

		g_Game.SelectPlayer(senderRPC, player);
	}

	private void RPC_UpdatePosition( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RPC_UpdatePosition").Add(senderRPC);
		#endif

		if ( g_Game.IsDedicatedServer() )
		{
			if ( !GetPermissionsManager().HasPermission( "Camera.View", senderRPC ) )
				return;

			vector position;
			if (!ctx.Read(position) || position == vector.Zero)
				return;

			PlayerBase player;
			if (Class.CastTo(player, target))
			{
				if (!player.m_JM_SpectatedObject)
					EnterFullmap(player);
				player.m_JM_CameraPosition = position;
				if (!player.m_JM_SpectatedObject)
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
		if (input.LocalValue() != 0 && CurrentActiveCamera && CurrentActiveCamera.m_JM_3rdPerson != JMCamera3rdPersonMode.DOLLY)
		{
			m_TargetFOV += input.LocalValue() * 0.01;
		}
	}

	void ZoomBackwards( UAInput input )
	{
		if (input.LocalValue() != 0 && CurrentActiveCamera && CurrentActiveCamera.m_JM_3rdPerson != JMCamera3rdPersonMode.DOLLY)
		{
			m_TargetFOV -= input.LocalValue() * 0.01;
					
			if ( m_TargetFOV < 0.01 ) 
			{
				m_TargetFOV = 0.01;
			}
		}
	}

	void LookAtSelection()
	{
		float distance = 100.0;
		vector rayStart = g_Game.GetCurrentCameraPosition();
		vector rayEnd = rayStart + ( g_Game.GetCurrentCameraDirection() * distance );

		RaycastRVParams rayInput = new RaycastRVParams( rayStart, rayEnd, g_Game.GetPlayer() );
		rayInput.flags = CollisionFlags.NEARESTCONTACT;
		rayInput.radius = 1.0;
		array< ref RaycastRVResult > results = new array< ref RaycastRVResult >;

		Object obj;
		if ( DayZPhysics.RaycastRVProxy( rayInput, results ) )
		{
			for ( int i = 0; i < results.Count(); ++i )
			{
				if ( results[i].obj == NULL )
					continue;

				if ( results[i].obj.GetType() == "" )
					continue;

				if ( results[i].obj.GetType() == "#particlesourceenf" )
					continue;

				obj = results[i].obj;
				break;
			}
		}

		if ( obj == NULL )
			return;
			
		if ( !CurrentActiveCamera )
			Enter();

		CurrentActiveCamera.SelectedTarget = obj;
		CurrentActiveCamera.LookFreeze = !CurrentActiveCamera.LookFreeze;
	}

	void GoToSelection(TVectorArray positions, TFloatArray time, TBoolArray smooth)
	{
		JMCinematicCamera cineCamera
		if ( !Class.CastTo(cineCamera, CurrentActiveCamera) )
			return;

		int count = positions.Count();
		for (int i=0; i < count; i++ )
		{
			if ( positions[i] == "0 0 0" )
			{
				time.Remove(i);
				smooth.Remove(i);
				positions.Remove(i);
			}
			else if ( time[i] == 0 )
				time[i] = 5;
		}

		if ( !CurrentActiveCamera )
			Enter();

		cineCamera.SetupTraveling(positions, time, smooth);
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
		{
			switch (CurrentActiveCamera.m_JM_3rdPerson)
			{
				case JMCamera3rdPersonMode.OFF:
					CurrentActiveCamera.m_JM_3rdPerson = JMCamera3rdPersonMode.DEFAULT;
					g_Game.GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", "Spectator camera mode: 3rd Person - Default", ""));
					break;
				case JMCamera3rdPersonMode.DEFAULT:
					CurrentActiveCamera.m_JM_3rdPerson = JMCamera3rdPersonMode.DOLLY;
					g_Game.GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", "Spectator camera mode: 3rd Person - Dolly", ""));
					break;
				case JMCamera3rdPersonMode.DOLLY:
					CurrentActiveCamera.m_JM_3rdPerson = JMCamera3rdPersonMode.AUTO;
					g_Game.GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", "Spectator camera mode: 3rd person - Automatic", ""));
					break;
				case JMCamera3rdPersonMode.AUTO:
					CurrentActiveCamera.m_JM_3rdPerson = JMCamera3rdPersonMode.OFF;
					g_Game.GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", "Spectator camera mode: 1st person", ""));
					break;
			}
		}
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
