enum COTCameraModuleRPC
{
	INVALID = 10160,
	Enter,
	Leave,
	COUNT
};

[CF_RegisterModule(COTCameraModule)]
class COTCameraModule : COTModule
{
	JMCameraBase m_Camera;

	float m_FOV = 1.0;
	float m_FOVTarget = 1.0;
	float m_FOVModifier = 5.0;
	string m_FOVString;

	float m_Speed = 3.0;
	float m_BoostModifier = 5.0;
	string m_SpeedString;

	float m_Friction = 0.9;
	string m_FrictionString;

	float m_MouseSensitivity = 35.0;
	float m_MouseSmoothness = 0.8;

	bool m_DOF = false;
	bool m_DOFFocusDistanceAuto = true;
	float m_DOFFocusDistance = 0.2;
	float m_DOFFocusLength = 20.0;
	float m_DOFFocusNear = 185.0;
	float m_DOFFocusDepthOffset = 10.0;
	float m_DOFBlur = 4.0;

	string m_DOFFocusDistanceString;
	string m_DOFFocusLengthString;
	string m_DOFFocusNearString;
	string m_DOFFocusDepthOffsetString;
	string m_DOFBlurString;

	float m_Blur = 0.0;
	float m_BlurTarget = 0.0;
	float m_BlurModifier = 10.0;
	string m_BlurString;

	float m_Exposure = 1.0;
	string m_ExposureString;

	override void OnInit()
	{
		super.OnInit();

		Bind("ToggleCamera", "UACameraToolToggleCamera", true);
		Bind("ZoomForwards", "UACameraToolZoomForwards", true);
		Bind("ZoomBackwards", "UACameraToolZoomBackwards", true);

		GetPermissionsManager().RegisterPermission("Camera.View");

		EnableUpdate();
		EnableRPC();
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission("Camera.View");
	}

	override string GetInputToggle()
	{
		return "UACOTToggleCamera";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/camera.layout";
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
		if (!IsMissionClient() || !m_Camera)
		{
			return;
		}

		float deltaTime = CF_EventUpdateArgs.Cast(args).DeltaTime;

		m_Blur = Math.Lerp(m_Blur, m_BlurTarget, deltaTime * m_BlurModifier);
		PPEffects.SetBlur(m_Blur);

		m_FOV = Math.Lerp(m_FOV, m_FOVTarget, deltaTime * m_FOVModifier);
		m_Camera.SetFOV(m_FOV);

		if (m_DOF)
		{
			vector from = GetGame().GetCurrentCameraPosition();

			if (m_Camera.SelectedTarget)
			{
				m_DOFFocusDistance = vector.Distance(from, m_Camera.SelectedTarget.GetPosition());
			}
			else if (m_DOFFocusDistanceAuto)
			{
				vector to = from + (GetGame().GetCurrentCameraDirection() * 10000.0);
				vector contact_pos;

				DayZPhysics.RaycastRV(from, to, contact_pos, NULL, NULL, NULL, NULL, NULL, false, false, ObjIntersectIFire);
				m_DOFFocusDistance = vector.Distance(from, contact_pos);
			}

			PPEffects.OverrideDOF(true, m_DOFFocusDistance, m_DOFFocusLength, m_DOFFocusNear, m_DOFBlur, m_DOFFocusDepthOffset);
		}

		g_Game.SetEVValue(m_Exposure);
	}

	Camera GetCamera()
	{
		return m_Camera;
	}

	override int GetRPCMin()
	{
		return COTCameraModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return COTCameraModuleRPC.COUNT;
	}

	override void OnRPC(Class sender, CF_EventArgs args)
	{
		auto rpcArgs = CF_EventRPCArgs.Cast(args);

		switch (rpcArgs.ID)
		{
		case COTCameraModuleRPC.Enter:
			RPC_Enter(rpcArgs.Context, rpcArgs.Sender, rpcArgs.Target);
			break;
		case COTCameraModuleRPC.Leave:
			RPC_Leave(rpcArgs.Context, rpcArgs.Sender, rpcArgs.Target);
			break;
		}
	}

	override void OnShow(Class sender, CF_EventArgs args)
	{
		super.OnShow(sender, args);

		OnChange(sender, args);
	}

	void OnChange(Class sender, CF_EventArgs args)
	{
		m_FOVString = m_FOVTarget.ToString() + "°";
		m_SpeedString = m_Speed.ToString();
		m_FrictionString = (m_Friction * 100.0).ToString() + "%";

		m_DOFFocusDistanceString = m_DOFFocusDistance.ToString();
		m_DOFFocusLengthString = m_DOFFocusLength.ToString();
		m_DOFFocusNearString = m_DOFFocusNear.ToString();
		m_DOFFocusDepthOffsetString = m_DOFFocusDepthOffset.ToString();
		m_DOFBlurString = m_DOFBlur.ToString();

		m_BlurString = m_BlurTarget.ToString();

		m_ExposureString = m_Exposure.ToString();
	}

	void Enter()
	{
		if (IsMissionOffline())
		{
			Server_Enter(NULL, GetGame().GetPlayer());
		}
		else if (IsMissionClient())
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(GetGame().GetPlayer(), COTCameraModuleRPC.Enter, true, NULL);
		}
	}

	private void Client_Enter()
	{
		if (Class.CastTo(m_Camera, Camera.GetCurrentCamera()))
		{
			m_Camera.SetActive(true);

			if (GetGame().GetPlayer())
			{
				GetGame().GetPlayer().GetInputController().SetDisabled(true);
			}
		}
	}

	private void Server_Enter(PlayerIdentity sender, Object target)
	{
		vector position = Vector(0, 0, 0);

		if (target)
		{
			position = target.GetPosition();
		}

		PlayerBase player;
		if (Class.CastTo(player, target))
		{
			position = player.GetBonePositionWS(player.GetBoneIndexByName("Head"));
			//player.GetInputController().SetDisabled( true );
		}

		if (IsMissionOffline())
		{
			m_Camera = JMCameraBase.Cast(GetGame().CreateObject("JMCinematicCamera", position, false));

			m_Camera.SetActive(true);

			if (GetGame().GetPlayer())
			{
				GetGame().GetPlayer().GetInputController().SetDisabled(true);
			}
		}
		else
		{
			GetGame().SelectPlayer(sender, NULL);

			GetGame().SelectSpectator(sender, "JMCinematicCamera", position);

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(NULL, COTCameraModuleRPC.Enter, true, sender);
		}

		GetCommunityOnlineToolsBase().Log(sender, "Entered the Free Camera");
	}

	private void RPC_Enter(ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (IsMissionHost())
		{
			if (!GetPermissionsManager().HasPermission("Camera.View", senderRPC))
				return;

			Server_Enter(senderRPC, target);
		}
		else
		{
			// RPC was sent from the server, permission would've been verified there.

			Client_Enter();
		}
	}

	void Leave()
	{
		if (IsMissionOffline())
		{
			Server_Leave(NULL, GetGame().GetPlayer());
		}
		else if (IsMissionClient())
		{
			if (m_Camera) m_Camera.LookFreeze = false;

			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(GetGame().GetPlayer(), COTCameraModuleRPC.Leave, true, NULL);
		}
	}

	private void Client_Leave()
	{
		m_Camera.SetActive(false);
		m_Camera = NULL;

		PPEffects.ResetDOFOverride();

		if (GetGame().GetPlayer())
		{
			GetGame().GetPlayer().GetInputController().SetDisabled(false);
		}
	}

	private void Server_Leave(PlayerIdentity sender, Object target)
	{
		PlayerBase player;
		if (Class.CastTo(player, target))
		{
			GetGame().SelectPlayer(sender, player);

			//player.GetInputController().SetDisabled( false );

			if (GetGame().IsMultiplayer())
			{
				ScriptRPC rpc = new ScriptRPC();
				rpc.Send(NULL, COTCameraModuleRPC.Leave, true, sender);
			}
			else
			{
				Client_Leave();
			}

			GetCommunityOnlineToolsBase().Log(sender, "Left the Free Camera");
		}
	}

	private void RPC_Leave(ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target)
	{
		if (IsMissionHost())
		{
			if (!GetPermissionsManager().HasPermission("Camera.View", senderRPC))
				return;

			Server_Leave(senderRPC, target);
		}
		else
		{
			// RPC was sent from the server, permission would've been verified there.

			Client_Leave();
		}
	}

	void ToggleCamera(UAInput input)
	{
		if (!(input.LocalPress()))
		{
			return;
		}

		if (!GetCommunityOnlineToolsBase().IsActive())
		{
			COTCreateLocalAdminNotification(new StringLocaliser("STR_COT_NOTIFICATION_WARNING_TOGGLED_OFF"));
			return;
		}

		if (m_Camera)
		{
			if (!COTPlayerIsRemoved)
			{
				Leave();
			}
		}
		else
		{
			Enter();
		}
	}

	void ZoomForwards(UAInput input)
	{
		if (input.LocalValue() == 0) return;
		
		m_FOVTarget += input.LocalValue() * 0.01;
	}

	void ZoomBackwards(UAInput input)
	{
		if (input.LocalValue() == 0) return;
		
		m_FOVTarget -= input.LocalValue() * 0.01;

		if (m_FOVTarget <= 0.01) m_FOVTarget = 0.01;
	}

	Object GetTargetObject()
	{
		if (!m_Camera)
			return NULL;

		return m_Camera.SelectedTarget;
	}

	vector GetTargetPos()
	{
		if (!m_Camera)
			return "0 0 0";

		return m_Camera.TargetPosition;
	}
};
