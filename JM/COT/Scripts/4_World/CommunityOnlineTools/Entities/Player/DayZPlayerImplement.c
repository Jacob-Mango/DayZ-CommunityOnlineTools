modded class DayZPlayerImplement
{
	private JMSpectatorCamera m_SpectatorCamera;
	private Head_Default m_PlayerHead;
	bool m_JM_IsHeadInvisible;
	vector m_JM_CameraPosMS;

	vector m_COT_HeadBonePositionWS;
	float m_COT_HeadBoneIdleTime;
	vector m_COT_LHandBonePositionWS;
	float m_COT_LHandBoneIdleTime;
	vector m_COT_RHandBonePositionWS;
	float m_COT_RHandBoneIdleTime;
	vector m_COT_LFootBonePositionWS;
	float m_COT_LFootBoneIdleTime;
	vector m_COT_RFootBonePositionWS;
	float m_COT_RFootBoneIdleTime;

	bool m_COT_EnableBonePositionUpdate;
	
	private float m_SmoothVelDir0[1];
	private float m_SmoothVelDir1[1];
	private float m_SmoothVelDir2[1];
	
	private float m_SmoothVelPos0[1];
	private float m_SmoothVelPos1[1];
	private float m_SmoothVelPos2[1];

	protected vector m_COT_LastPlayerPos;

	void DayZPlayerImplement()
	{
		if ( IsMissionClient() )
			SetEventMask( EntityEvent.FRAME | EntityEvent.POSTFRAME );

		m_SmoothVelDir0[0] = 0.0;
		m_SmoothVelDir1[0] = 0.0;
		m_SmoothVelDir2[0] = 0.0;

		m_SmoothVelPos0[0] = 0.0;
		m_SmoothVelPos1[0] = 0.0;
		m_SmoothVelPos2[0] = 0.0;
	}

	override void CommandHandler( float pDt, int pCurrentCommandID, bool pCurrentCommandFinished )	
	{
		super.CommandHandler( pDt, pCurrentCommandID, pCurrentCommandFinished );

		if (m_COT_EnableBonePositionUpdate)
			COT_UpdateBonePositionTimes(pDt);
	}

	void OnSpectateStart( JMSpectatorCamera camera )
	{
		m_SpectatorCamera = camera;

		if (m_SpectatorCamera && !m_SpectatorCamera.m_JM_3rdPerson)
			SetHeadInvisible( true );

		m_JM_CameraPosMS = vector.Zero;
	}

	void OnSpectateEnd()
	{
		m_SpectatorCamera = NULL;

		if (m_JM_IsHeadInvisible)
			SetHeadInvisible( false );
	}

	void UpdateSpecatorCamera(float timeSlice)
	{
		if ( !m_SpectatorCamera )
			return;

		vector playerTransform[4];
		GetTransform(playerTransform);
		vector playerPos = playerTransform[3];

		vector headTransform[4];
		GetBoneTransformWS(GetBoneIndexByName( "Head" ), headTransform);
		vector headPos = headTransform[3];

		float offsetY = headPos[1] - playerPos[1];

		EntityAI hands = GetHumanInventory().GetEntityInHands();

		Weapon_Base weapon;
		vector weaponTransform[4];
		//ItemOptics optic;
		//vector opticTransform[4];

		vector eyePos;

		vector dir;
		vector pos;

		float fov;

		bool weaponRaised;

		if (Class.CastTo(weapon, hands))
		{
			//optic = weapon.GetAttachedOptics();
			weapon.GetTransform(weaponTransform);
			eyePos = weapon.GetSelectionPositionLS("eye").Multiply4(weaponTransform);
			vector rHandPos = GetBonePositionWS(GetBoneIndexByName("RightHand"));
			if (vector.DistanceSq(headPos, rHandPos) < 0.09)
				weaponRaised = true;
			m_SpectatorCamera.m_JM_IsADS = m_SpectatorCamera.IsActive() && weaponRaised && vector.DistanceSq(eyePos, headPos) < 0.04;
		}
		else
		{
			m_SpectatorCamera.m_JM_IsADS = false;
		}

		if ( !m_SpectatorCamera.IsActive() )
		{
			if (m_JM_IsHeadInvisible)
				SetHeadInvisible(false);

			return;
		}

		bool dollyCam;
		float speed;
		switch (m_SpectatorCamera.m_JM_3rdPerson)
		{
			case JMCamera3rdPersonMode.DOLLY:
				if (!m_SpectatorCamera.m_COT_IsInFreeLook)
				{
					dollyCam = true;
					speed = 0.9;  //! Slow interpolation, resulting in movement like camera on a dolly
					break;
				}
			default:
				speed = 0.3;
				break;
		}

		if (m_SpectatorCamera.m_JM_IsADS)
		{
			//if (optic)
			//{
				//optic.GetCameraPoint(pos, dir);
				//optic.GetTransform(opticTransform);
				//pos = pos.Multiply4(opticTransform);

				//if (optic.GetCurrentStepFOV() < GameConstants.DZPLAYER_CAMERA_FOV_IRONSIGHTS)
				//{
					//if (!optic.IsInOptics())
					//{
						//optic.EnterOptics();
						//optic.OnOpticEnter();
					//}
				//}
				//else if (optic.IsInOptics())
				//{
					//optic.ExitOptics();
					//optic.OnOpticExit();
				//}
			//}
			//else
			//{
				//weapon.GetCameraPoint(weapon.GetCurrentMuzzle(), pos, dir);
				//pos = pos.Multiply4(weaponTransform);
			//}

			vector barrel_start = weapon.GetSelectionPositionLS("konec hlavne").Multiply4(weaponTransform);
			vector barrel_end = weapon.GetSelectionPositionLS("usti hlavne").Multiply4(weaponTransform);
			dir = vector.Direction(barrel_start, barrel_end).Normalized();
			pos = eyePos - dir * 0.06;

			if (!m_SpectatorCamera.m_JM_3rdPerson)
				fov = GameConstants.DZPLAYER_CAMERA_FOV_IRONSIGHTS;
			else if (GetUApi().GetInputByID(UAZoomIn).LocalValue())
				fov = GameConstants.DZPLAYER_CAMERA_FOV_EYEZOOM;
			else
				fov = GetDayZGame().GetUserFOV();
		}
		else
		{
			//if (optic)
			//{
				//if (optic.IsInOptics())
				//{
					//optic.ExitOptics();
					//optic.OnOpticExit();
				//}
			//}

			vector wl;
			EWaterLevels waterLevel = DayZPlayerUtils.CheckWaterLevel(this, wl);

			float waterDepth = wl[0];
			float characterDepth = wl[1];

			if (waterDepth > 1.5 && characterDepth > 0.0)
			{
				//! Swimming
				dir = playerTransform[2];
				if (m_SpectatorCamera.m_JM_3rdPerson)
					pos = headPos + "0 0.5 0";
				else
					pos = headPos + "0 0.1 0";
			}
			else
			{
				dir = headTransform[1];
				pos = headPos + "0 0.1 0";
			}

			if (GetUApi().GetInputByID(UAZoomIn).LocalValue())
				fov = GameConstants.DZPLAYER_CAMERA_FOV_EYEZOOM;
			else
				fov = GetDayZGame().GetUserFOV();
		}

		vector fromDir = m_SpectatorCamera.GetDirection();
		if (m_SpectatorCamera.m_JM_3rdPerson)
		{
			dir[0] = Math.SmoothCD(fromDir[0], dir[0], m_SmoothVelDir0, speed, 1000, timeSlice);
			dir[1] = Math.SmoothCD(fromDir[1], dir[1], m_SmoothVelDir1, speed, 1000, timeSlice);
			dir[2] = Math.SmoothCD(fromDir[2], dir[2], m_SmoothVelDir2, speed, 1000, timeSlice);
		}
		else
		{
			dir = vector.Lerp(fromDir, dir, timeSlice * CAMERA_FOV_SPEED_MODIFIER);
		}

		float dist;
		if (m_SpectatorCamera.m_JM_3rdPerson)
		{
			dist = 1.33;
			if (dollyCam)
			{
				vector vel = (playerPos - m_COT_LastPlayerPos) * (1.0 / timeSlice);  //! GetVelocity(this) returns 0 on client
				dist *= Math.Max(1.0 - vel.Length() / 6.756, 0.0);
			}
			pos = pos - dir * dist;
			vector offsetX = dir.Perpend() * 0.33;
			if (m_SpectatorCamera.m_JM_LeftShoulder)
				pos = pos + offsetX;
			else
				pos = pos - offsetX;
		}
		else if (!m_SpectatorCamera.m_JM_IsADS)
		{
			dist = 0.06;
			pos = pos + playerTransform[2] * dist;
		}

		m_COT_LastPlayerPos = playerPos;

		m_SpectatorCamera.SetDirection( dir );

		vector cameraPos;

		if (m_JM_CameraPosMS == vector.Zero)
		{
			cameraPos = pos;
			m_JM_CameraPosMS = pos.InvMultiply4(playerTransform);
		}
		else
		{
			cameraPos = m_SpectatorCamera.GetPosition();

			if (dollyCam)
			{
				//! Interpolate in world space to give cinematic look

				cameraPos[0] = Math.SmoothCD(cameraPos[0], pos[0], m_SmoothVelPos0, 0.4, 1000, timeSlice);
				cameraPos[1] = Math.SmoothCD(cameraPos[1], pos[1], m_SmoothVelPos1, 0.3, 1000, timeSlice);
				cameraPos[2] = Math.SmoothCD(cameraPos[2], pos[2], m_SmoothVelPos2, 0.4, 1000, timeSlice);

				m_JM_CameraPosMS = cameraPos.InvMultiply4(playerTransform);
			}
			else
			{
				//! Interpolate in model space so camera sticks to character
				pos = pos.InvMultiply4(playerTransform);

				m_JM_CameraPosMS = vector.Lerp(m_JM_CameraPosMS, pos, timeSlice * CAMERA_FOV_SPEED_MODIFIER);

				//m_JM_CameraPosMS[0] = Math.SmoothCD(m_JM_CameraPosMS[0], pos[0], m_SmoothVelPos0, 0.3, 1000, timeSlice);
				//m_JM_CameraPosMS[1] = Math.SmoothCD(m_JM_CameraPosMS[1], pos[1], m_SmoothVelPos1, 0.3, 1000, timeSlice);
				//m_JM_CameraPosMS[2] = Math.SmoothCD(m_JM_CameraPosMS[2], pos[2], m_SmoothVelPos2, 0.3, 1000, timeSlice);

				cameraPos = m_JM_CameraPosMS.Multiply4(playerTransform);
			}
		}

		float surfaceY = GetGame().SurfaceRoadY3D(cameraPos[0], cameraPos[1] - offsetY, cameraPos[2], RoadSurfaceDetection.LEGACY) + 0.1;
		if (surfaceY > cameraPos[1])
			cameraPos[1] = surfaceY;

		m_SpectatorCamera.SetPosition( cameraPos );

		float cameraDistanceToHeadSq = vector.DistanceSq(m_SpectatorCamera.GetPosition(), headPos);
		if (cameraDistanceToHeadSq < 0.0625 && !m_JM_IsHeadInvisible)
			SetHeadInvisible(true);
		else if (m_SpectatorCamera.m_JM_3rdPerson && cameraDistanceToHeadSq >= 0.0625 && m_JM_IsHeadInvisible)
			SetHeadInvisible(false);

		m_SpectatorCamera.SetFOV( Math.Lerp(m_SpectatorCamera.GetCurrentFOV(), fov, timeSlice * CAMERA_FOV_SPEED_MODIFIER) );
	}

#ifndef SERVER
	override void EOnFrame( IEntity other, float timeSlice )
	{
		UpdateSpecatorCamera(timeSlice);
	}

	override void EOnPostFrame( IEntity other, int extra )
	{
		if (m_SpectatorCamera)
		{
			EntityAI hands = GetHumanInventory().GetEntityInHands();

			Weapon_Base weapon;

			if (Class.CastTo(weapon, hands))
			{
				ItemOptics optic = weapon.GetAttachedOptics();

				if (optic)
				{
					if (m_SpectatorCamera.IsActive() && m_SpectatorCamera.m_JM_IsADS && !m_SpectatorCamera.m_JM_3rdPerson)
						COT_SetEntityInvisibleRecursive(optic, m_SpectatorCamera.m_JM_1stPersonADS_HideScope);
					else
						COT_SetEntityInvisibleRecursive(optic, false);
				}
			}
		}
	}
#endif

	override void EEItemAttached(EntityAI item, string slot_name)
	{
		super.EEItemAttached(item, slot_name);

		switch (slot_name)
		{
			case "Headgear":
			case "Mask":
			case "Eyewear":
				if (m_JM_IsHeadInvisible)
					COT_SetEntityInvisibleRecursive(item, true);
				break;
		}
	}

	void SetHeadInvisible( bool invisible )
	{
		if ( !m_PlayerHead )
		{
			int slot_id = InventorySlots.GetSlotIdFromString( "Head" );
			m_PlayerHead = Head_Default.Cast( GetInventory().FindPlaceholderForSlot( slot_id ) );
		}
		
		m_PlayerHead.SetInvisible( invisible );

		SetAttachmentInvisible( "Head", invisible );
		SetAttachmentInvisible( "Headgear", invisible );
		SetAttachmentInvisible( "Mask", invisible );
		SetAttachmentInvisible( "Eyewear", invisible );

		m_JM_IsHeadInvisible = invisible;
	}

	void SetAttachmentInvisible( string slot, bool invisible )
	{
		int slot_id = InventorySlots.GetSlotIdFromString( slot );
		EntityAI ent = GetInventory().FindAttachment( slot_id );
		if ( ent )
		{
			COT_SetEntityInvisibleRecursive( ent, invisible );
		}
	}

	void COT_SetEntityInvisibleRecursive(EntityAI ent, bool invisible)
	{
		ent.SetInvisible(invisible);

		for (int i = 0; i < ent.GetInventory().AttachmentCount(); i++)
		{
			COT_SetEntityInvisibleRecursive(ent.GetInventory().GetAttachmentFromIndex(i), invisible);
		}
	}

#ifndef CF_MODULE_PERMISSIONS
	string FormatSteamWebhook()
	{
		return GetDisplayName();
	}
#endif

	void COT_EnableBonePositionUpdate(bool state)
	{
		m_COT_HeadBoneIdleTime = 0;
		m_COT_LHandBoneIdleTime = 0;
		m_COT_RHandBoneIdleTime = 0;
		m_COT_LFootBoneIdleTime = 0;
		m_COT_RFootBoneIdleTime = 0;

		m_COT_EnableBonePositionUpdate = state;
	}

	void COT_UpdateBonePositionTimes(float pDt)
	{
		vector headPosition = GetBonePositionWS(GetBoneIndexByName("Head"));
		if (vector.DistanceSq(headPosition, m_COT_HeadBonePositionWS) < 0.0004)
		{
			m_COT_HeadBoneIdleTime += pDt;
		}
		else
		{
			m_COT_HeadBonePositionWS = headPosition;
			m_COT_HeadBoneIdleTime = 0;
		}
		vector lHandPosition = GetBonePositionWS(GetBoneIndexByName("LeftHand"));
		if (vector.DistanceSq(lHandPosition, m_COT_LHandBonePositionWS) < 0.01)
		{
			m_COT_LHandBoneIdleTime += pDt;
		}
		else
		{
			m_COT_LHandBonePositionWS = lHandPosition;
			m_COT_LHandBoneIdleTime = 0;
		}
		vector rHandPosition = GetBonePositionWS(GetBoneIndexByName("RightHand"));
		if (vector.DistanceSq(rHandPosition, m_COT_RHandBonePositionWS) < 0.01)
		{
			m_COT_RHandBoneIdleTime += pDt;
		}
		else
		{
			m_COT_RHandBonePositionWS = rHandPosition;
			m_COT_RHandBoneIdleTime = 0;
		}
		vector lFootPosition = GetBonePositionWS(GetBoneIndexByName("LeftFoot"));
		if (vector.DistanceSq(lFootPosition, m_COT_LFootBonePositionWS) < 0.0004)
		{
			m_COT_LFootBoneIdleTime += pDt;
		}
		else
		{
			m_COT_LFootBonePositionWS = lFootPosition;
			m_COT_LFootBoneIdleTime = 0;
		}
		vector rFootPosition = GetBonePositionWS(GetBoneIndexByName("RightFoot"));
		if (vector.DistanceSq(rFootPosition, m_COT_RFootBonePositionWS) < 0.0004)
		{
			m_COT_RFootBoneIdleTime += pDt;
		}
		else
		{
			m_COT_RFootBonePositionWS = rFootPosition;
			m_COT_RFootBoneIdleTime = 0;
		}
	}

	bool COT_IsAnimationIdle()
	{
		if (m_COT_HeadBoneIdleTime < 0.25)
			return false;
		if (m_COT_LHandBoneIdleTime < 0.25)
			return false;
		if (m_COT_RHandBoneIdleTime < 0.25)
			return false;
		if (m_COT_LFootBoneIdleTime < 0.25)
			return false;
		if (m_COT_RFootBoneIdleTime < 0.25)
			return false;
		return true;
	}
};