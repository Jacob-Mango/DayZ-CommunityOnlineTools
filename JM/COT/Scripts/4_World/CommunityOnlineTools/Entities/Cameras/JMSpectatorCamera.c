class JMSpectatorCamera: JMCameraBase
{
	static const int DOLLY_CAM_PATH_COUNT = 100;

	vector linearVelocity;
	vector angularVelocity;

	vector orientation;
	bool m_COT_IsInFreeLook;

	vector m_JM_CameraPosMS;
	
	private float m_COT_SmoothVelDir0[1];
	private float m_COT_SmoothVelDir1[1];
	private float m_COT_SmoothVelDir2[1];
	
	private float m_COT_SmoothVelPos0[1];
	private float m_COT_SmoothVelPos1[1];
	private float m_COT_SmoothVelPos2[1];

	protected vector m_COT_LastPlayerPos;

	vector m_COT_DollyCamPath[100];
	vector m_COT_DollyCamOri[100];
	int m_COT_DollyCamPathIdx;
	float m_COT_DollyCamPathUpdateDT;
	float m_COT_DollyCamSpeedMult = 0.433333;
	float m_COT_DollyCamSpeedMultTgt = 0.433333;
	
	override void OnTargetSelected( Object target )
	{
		DayZPlayerImplement impl;
		if ( !Class.CastTo( impl, target ) )
			return;

		impl.OnSpectateStart( this );
	}

	override void OnTargetDeselected( Object target )
	{
		DayZPlayerImplement impl;
		if ( !Class.CastTo( impl, target ) )
			return;

		impl.OnSpectateEnd();
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		Input input = GetGame().GetInput();
		bool freelook = input.LocalValue( "UALookAround" );

		if (m_COT_IsInFreeLook != freelook)
		{
			m_COT_IsInFreeLook = freelook;
			if (freelook)
				orientation = GetOrientation();
		}
		
		if ( !LookFreeze || freelook )
		{
			float yawDiff = input.LocalValue( "UAAimLeft" ) - input.LocalValue( "UAAimRight" );
			float pitchDiff = input.LocalValue( "UAAimDown" ) - input.LocalValue( "UAAimUp" );
		
			angularVelocity = angularVelocity * CAMERA_SMOOTH;

			angularVelocity[0] = angularVelocity[0] + ( yawDiff * CAMERA_MSENS );
			angularVelocity[1] = angularVelocity[1] + ( pitchDiff * CAMERA_MSENS );

			orientation[0] = orientation[0] - ( angularVelocity[0] * timeslice );
			orientation[1] = orientation[1] - ( angularVelocity[1] * timeslice );
			orientation[2] = orientation[2] - ( angularVelocity[2] * timeslice );

			if ( orientation[1] <= -90 )
			{
				angularVelocity[1] = Math.Min( angularVelocity[1], 0 );
				orientation[1] = -90;
			} else if ( orientation[1] >= 90 )
			{
				orientation[1] = 90;
				angularVelocity[1] = Math.Max( angularVelocity[1], 0 );
			}

			orientation[0] = Math.NormalizeAngle( orientation[0] );
			orientation[2] = Math.NormalizeAngle( orientation[2] );

			SetOrientation( orientation );
		}
	}

	void OnUpdateEx(DayZPlayerImplement spectatedPlayer, float timeSlice)
	{
		vector playerTransform[4];
		spectatedPlayer.GetTransform(playerTransform);
		vector playerPos = playerTransform[3];

		vector headTransform[4];
		spectatedPlayer.GetBoneTransformWS(spectatedPlayer.GetBoneIndexByName( "Head" ), headTransform);
		vector headPos = headTransform[3];

		float offsetY = headPos[1] - playerPos[1];

		EntityAI hands = spectatedPlayer.GetHumanInventory().GetEntityInHands();

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
			vector rHandPos = spectatedPlayer.GetBonePositionWS(spectatedPlayer.GetBoneIndexByName("RightHand"));
			if (vector.DistanceSq(headPos, rHandPos) < 0.09)
				weaponRaised = true;
			m_JM_IsADS = IsActive() && weaponRaised && vector.DistanceSq(eyePos, headPos) < 0.04;
		}
		else
		{
			m_JM_IsADS = false;
		}

		if ( !IsActive() )
		{
			if (spectatedPlayer.m_JM_IsHeadInvisible)
				spectatedPlayer.SetHeadInvisible(false);

			return;
		}

		bool dollyCam;
		float speed;
		switch (m_JM_3rdPerson)
		{
			case JMCamera3rdPersonMode.DOLLY:
				if (!m_COT_IsInFreeLook)
				{
					dollyCam = true;
					speed = 0.9;  //! Slow interpolation, resulting in movement like camera on a dolly
					break;
				}
			default:
				speed = 0.3;
				break;
		}

		if (m_JM_IsADS)
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

			if (!m_JM_3rdPerson)
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
			EWaterLevels waterLevel = DayZPlayerUtils.CheckWaterLevel(spectatedPlayer, wl);

			//float waterDepth = wl[0];
			//float characterDepth = wl[1];

			//if (waterDepth > 1.5 && characterDepth > 0.0)
			if (waterLevel == EWaterLevels.LEVEL_SWIM_START)
			{
				//! Swimming
				dir = playerTransform[2];
				if (m_JM_3rdPerson)
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

		vector targetPos = pos;

		vector fromOri = GetOrientation();
		vector targetOri = Math.COT_DirToOri(dir);

		float yawDiff;
		float pitchDiff;

		float dollyCamSpeedMultMin = 0.333333;  //! Fastest
		float dollyCamSpeedMultMax = 1.0;  //! Slowest

		bool isUnderRoofBuilding = IsUnderRoofBuilding(spectatedPlayer);

		if (isUnderRoofBuilding)
		{
			m_COT_DollyCamSpeedMult = Math.Lerp(m_COT_DollyCamSpeedMult, Math.Min(m_COT_DollyCamSpeedMultTgt, 0.666666), timeSlice);
		}
		else
		{
			if (GetUApi().GetInputByID(UACameraToolSpeedIncrease).LocalValue())
				m_COT_DollyCamSpeedMultTgt = Math.Clamp(m_COT_DollyCamSpeedMultTgt - 0.1, dollyCamSpeedMultMin, dollyCamSpeedMultMax);
			else if (GetUApi().GetInputByID(UACameraToolSpeedDecrease).LocalValue())
				m_COT_DollyCamSpeedMultTgt = Math.Clamp(m_COT_DollyCamSpeedMultTgt + 0.1, dollyCamSpeedMultMin, dollyCamSpeedMultMax);

			m_COT_DollyCamSpeedMult = Math.Lerp(m_COT_DollyCamSpeedMult, m_COT_DollyCamSpeedMultTgt, timeSlice * 4);
		}

		vector ori;

		yawDiff = Math.COT_AngleDiff2(fromOri[0], targetOri[0]);
		pitchDiff = Math.COT_AngleDiff2(fromOri[1], targetOri[1]);

		targetOri[0] = fromOri[0] + yawDiff;
		targetOri[1] = fromOri[1] + pitchDiff;

		if (m_JM_3rdPerson)
		{
			ori[0] = Math.SmoothCD(fromOri[0], targetOri[0], m_COT_SmoothVelDir0, speed, 1000, timeSlice);
			ori[1] = Math.SmoothCD(fromOri[1], targetOri[1], m_COT_SmoothVelDir1, speed, 1000, timeSlice);
		}
		else
		{
			ori = vector.Lerp(fromOri, targetOri, timeSlice * CAMERA_FOV_SPEED_MODIFIER);
		}

		m_COT_DollyCamPathUpdateDT += timeSlice;

		if (m_COT_DollyCamPathUpdateDT > 0.1 * m_COT_DollyCamSpeedMult)
		{
			m_COT_DollyCamPathUpdateDT = 0;

			if (m_COT_DollyCamPathIdx < Math.Clamp(9 * m_COT_DollyCamSpeedMult, 1, DOLLY_CAM_PATH_COUNT - 1))
			{
				m_COT_DollyCamPath[m_COT_DollyCamPathIdx] = pos;
				m_COT_DollyCamOri[m_COT_DollyCamPathIdx] = targetOri;

				++m_COT_DollyCamPathIdx;
			}
		}

		if (dollyCam)
		{
			if (m_COT_DollyCamPathIdx > 0)
			{
				pos = m_COT_DollyCamPath[0];

				targetOri = m_COT_DollyCamOri[0];
			}

			if (m_COT_DollyCamSpeedMult > dollyCamSpeedMultMin)
			{
				yawDiff = Math.COT_AngleDiff2(fromOri[0], targetOri[0]);
				pitchDiff = Math.COT_AngleDiff2(fromOri[1], targetOri[1]);

				targetOri[0] = fromOri[0] + yawDiff;
				targetOri[1] = fromOri[1] + pitchDiff;

				targetOri[0] = Math.SmoothCD(fromOri[0], targetOri[0], m_COT_SmoothVelDir0, speed, 1000, timeSlice);
				targetOri[1] = Math.SmoothCD(fromOri[1], targetOri[1], m_COT_SmoothVelDir1, speed, 1000, timeSlice);

				float t = Math.InverseLerp(dollyCamSpeedMultMin, dollyCamSpeedMultMax, m_COT_DollyCamSpeedMult);
				ori[0] = Math.Lerp(ori[0], targetOri[0], t);
				ori[1] = Math.Lerp(ori[1], targetOri[1], t);
			}
		}

		ori = ori.GetRelAngles();
		dir = ori.AnglesToVector();

		vector cameraPos = GetPosition();
		float cameraDistToTargetSq = vector.DistanceSq(cameraPos, targetPos);

		float targetDist;
		if (m_JM_3rdPerson)
		{
			float offsetFactor = 1.0;

			targetDist = 1.33;

			if (dollyCam)
			{
				vector vel = (playerPos - m_COT_LastPlayerPos) * (1.0 / timeSlice);  //! GetVelocity(spectatedPlayer) returns 0 on client
				targetDist *= Math.Max(1.0 - vel.Length() / 6.756, 0.0);

				if (cameraDistToTargetSq > targetDist)
					offsetFactor = targetDist / cameraDistToTargetSq;
			}
			else
			{
				pos = pos - dir * targetDist;
			}

			vector offsetX = dir.Perpend() * 0.33 * offsetFactor;

			if (m_JM_LeftShoulder)
				pos = pos + offsetX;
			else
				pos = pos - offsetX;
		}
		else if (!m_JM_IsADS)
		{
			targetDist = 0.06;
			pos = pos + playerTransform[2] * targetDist;
		}

		m_COT_LastPlayerPos = playerPos;

		SetOrientation(ori);

		if (m_COT_DollyCamPathIdx > 1 && m_COT_DollyCamPathUpdateDT == 0 && cameraDistToTargetSq > 0.25)
		{
			for (int i = 0; i < m_COT_DollyCamPathIdx - 1; ++i)
			{
				m_COT_DollyCamPath[i] = m_COT_DollyCamPath[i + 1];
				m_COT_DollyCamOri[i] = m_COT_DollyCamOri[i + 1];
			}

			m_COT_DollyCamPathIdx -= 1;
		}

		if (m_JM_CameraPosMS == vector.Zero)
		{
			cameraPos = pos;
			m_JM_CameraPosMS = pos.InvMultiply4(playerTransform);
		}
		else
		{
			if (dollyCam)
			{
				//! Interpolate in world space to give cinematic look

				cameraPos[0] = Math.SmoothCD(cameraPos[0], pos[0], m_COT_SmoothVelPos0, 0.9 * m_COT_DollyCamSpeedMult, 1000, timeSlice);
				cameraPos[1] = Math.SmoothCD(cameraPos[1], pos[1], m_COT_SmoothVelPos1, 0.8 * m_COT_DollyCamSpeedMult, 1000, timeSlice);
				cameraPos[2] = Math.SmoothCD(cameraPos[2], pos[2], m_COT_SmoothVelPos2, 0.9 * m_COT_DollyCamSpeedMult, 1000, timeSlice);

				m_JM_CameraPosMS = cameraPos.InvMultiply4(playerTransform);
			}
			else
			{
				//! Interpolate in model space so camera sticks to character
				pos = pos.InvMultiply4(playerTransform);

				m_JM_CameraPosMS = vector.Lerp(m_JM_CameraPosMS, pos, timeSlice * CAMERA_FOV_SPEED_MODIFIER);

				//m_JM_CameraPosMS[0] = Math.SmoothCD(m_JM_CameraPosMS[0], pos[0], m_COT_SmoothVelPos0, 0.3, 1000, timeSlice);
				//m_JM_CameraPosMS[1] = Math.SmoothCD(m_JM_CameraPosMS[1], pos[1], m_COT_SmoothVelPos1, 0.3, 1000, timeSlice);
				//m_JM_CameraPosMS[2] = Math.SmoothCD(m_JM_CameraPosMS[2], pos[2], m_COT_SmoothVelPos2, 0.3, 1000, timeSlice);

				cameraPos = m_JM_CameraPosMS.Multiply4(playerTransform);
			}
		}

		float surfaceY = GetGame().SurfaceRoadY3D(cameraPos[0], cameraPos[1] - offsetY, cameraPos[2], RoadSurfaceDetection.LEGACY) + 0.1;
		if (surfaceY > cameraPos[1])
			cameraPos[1] = surfaceY;

		SetPosition( cameraPos );

		float cameraDistanceToHeadSq = vector.DistanceSq(GetPosition(), headPos);
		if (cameraDistanceToHeadSq < 0.0625 && !spectatedPlayer.m_JM_IsHeadInvisible)
			spectatedPlayer.SetHeadInvisible(true);
		else if (m_JM_3rdPerson && cameraDistanceToHeadSq >= 0.0625 && spectatedPlayer.m_JM_IsHeadInvisible)
			spectatedPlayer.SetHeadInvisible(false);

		SetFOV( Math.Lerp(GetCurrentFOV(), fov, timeSlice * CAMERA_FOV_SPEED_MODIFIER) );
	}

	bool IsUnderRoofBuilding(DayZPlayerImplement player)
	{
		//! if inside vehicle return immediately
		if (player.IsInVehicle())
			return false;
		
		float hitFraction;
		vector hitPosition, hitNormal;
		vector from = player.GetPosition();
		vector to = from + "0 25 0";
		Object hitObject;
		PhxInteractionLayers collisionLayerMask = PhxInteractionLayers.ITEM_LARGE|PhxInteractionLayers.BUILDING|PhxInteractionLayers.VEHICLE;
		
		DayZPhysics.RayCastBullet(from, to, collisionLayerMask, null, hitObject, hitPosition, hitNormal, hitFraction);

		return hitObject && hitObject.IsInherited(House);
	}
};
