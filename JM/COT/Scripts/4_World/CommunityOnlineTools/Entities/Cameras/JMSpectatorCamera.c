class JMDollyCamLight: ScriptedLightBase
{
	void JMDollyCamLight()
	{
		SetLightType(LightSourceType.SpotLight);
		SetVisibleDuringDaylight(true);
		SetRadiusTo(3);
		if (g_Game.GetMission().GetWorldData().GetDaytime() == WorldDataDaytime.DAY)
			SetBrightnessTo(1000);
		else
			SetBrightnessTo(10);
		SetCastShadow(false);
		SetDiffuseColor(1.0, 0.75, 0.25);
		SetFlareVisible(false);
		SetSpotLightAngle(45);
		SetFadeOutTime(3);
	}
}

class JMSpectatorCamera: JMCameraBase
{
	static const int DOLLY_CAM_PATH_LIMIT = 200;

	static JMSpectatorCamera s_COT_SpectatorCamera;
	static bool s_DbgDraw;

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

	protected vector m_COT_LastObjectPos;

	vector m_COT_DollyCamPath[200];
	int m_COT_DollyCamPathNextIdx;
	int m_COT_DollyCamPathIdx;
	float m_COT_DollyCamPathUpdateDT;
	float m_COT_DollyCamSpeedMult = 0.433333;
	float m_COT_DollyCamSpeedMultTgt = 0.433333;
	float m_COT_DollyCamDistThreshMult = 0.433333;
	float m_COT_SpectatedObjectSpeed;
	float m_COT_DollyCamJumpClimbTimeout;
	bool m_COT_DollyCamReversing;
	Object m_COT_LookAtTarget;
	float m_COT_LookAtTarget_Time;
	ParticleSource m_COT_TargetMarker;
	Object m_COT_TargetMarker_Object;
	ScriptedLightBase m_COT_TargetLight;
	bool m_COT_RemoveMarker;

#ifdef DIAG_DEVELOPER
	float m_COT_TempFloat01;
	float m_COT_TempFloat02;
	float m_COT_TempFloat03;
	int m_COT_TempInt01;
	int m_COT_TempInt02;
	int m_COT_TempInt03;
	bool m_COT_TempBool01;
	bool m_COT_TempBool02;
	bool m_COT_TempBool03;
	vector m_COT_TempVec01;
	vector m_COT_TempVec02;
	vector m_COT_TempVec03;
#endif

	void JMSpectatorCamera()
	{
		if (s_COT_SpectatorCamera)
			g_Game.ObjectDeleteOnClient(s_COT_SpectatorCamera);

		s_COT_SpectatorCamera = this;
	}

	void ~JMSpectatorCamera()
	{
		if (g_Game)
			COT_RemoveMarker();
	}
	
	override void OnTargetSelected( Object target )
	{
		m_JM_CameraPosMS = vector.Zero;

		DayZPlayerImplement impl;
		if ( !Class.CastTo( impl, target ) )
			return;

		impl.OnSpectateStart( this );

		//m_COT_CrosshairEnabled = g_Game.GetProfileOptionBool(EDayZProfilesOptions.CROSSHAIR);
		//g_Game.SetProfileOptionBool(EDayZProfilesOptions.CROSSHAIR, false);
	}

	override void OnTargetDeselected( Object target )
	{
		DayZPlayerImplement impl;
		if ( !Class.CastTo( impl, target ) )
			return;

		impl.OnSpectateEnd();

		//g_Game.SetProfileOptionBool(EDayZProfilesOptions.CROSSHAIR, m_COT_CrosshairEnabled);
	}

	//! IMPORTANT: This NEEDS to be called from EOnPostFrame, not EOnFrame, else 1st and 3rd person jitter like crazy under 1.29
	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );

		Input input = g_Game.GetInput();
		bool freelook = input.LocalValue( "UALookAround" );

		if (m_COT_IsInFreeLook != freelook)
		{
			m_COT_IsInFreeLook = freelook;
			if (freelook)
				orientation = GetOrientation();
		}
		
		float yawDiff;
		float pitchDiff;
		vector dir;

		if ( !LookFreeze || freelook )
		{
			yawDiff = input.LocalValue( "UAAimLeft" ) - input.LocalValue( "UAAimRight" );
			pitchDiff = input.LocalValue( "UAAimDown" ) - input.LocalValue( "UAAimUp" );
		
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
		}

		if (!SelectedTarget)
		{
			SetOrientation( orientation );
			return;
		}

		vector objectTransform[4];
		SelectedTarget.GetTransform(objectTransform);
		vector objectPos = objectTransform[3];

		vector headTransform[4];
		GetCommunityOnlineToolsBase().GetHeadTransform(SelectedTarget, headTransform);
		vector headPos = headTransform[3];

		float offsetY = headPos[1] - objectPos[1];

		DayZPlayerImplement spectatedPlayer;
		EntityAI hands;
		if (Class.CastTo(spectatedPlayer, SelectedTarget))
			hands = spectatedPlayer.GetHumanInventory().GetEntityInHands();

		Weapon_Base weapon;
		vector weaponTransform[4];
		//ItemOptics optic;
		//vector opticTransform[4];

		vector eyePos;

		vector begPos;

		float fov;

		bool weaponRaised;
		bool isSwimming;
		bool isOnLadder;

		int i;
		int j;

		vector barrel_start;
		vector barrel_end;
		vector aimDir;

		if (Class.CastTo(weapon, hands))
		{
			//optic = weapon.GetAttachedOptics();
			weapon.GetTransform(weaponTransform);
			eyePos = weapon.GetSelectionPositionLS("eye").Multiply4(weaponTransform);
			barrel_start = weapon.GetSelectionPositionLS("konec hlavne").Multiply4(weaponTransform);
			barrel_end = weapon.GetSelectionPositionLS("usti hlavne").Multiply4(weaponTransform);
			aimDir = (barrel_end - barrel_start).Normalized();
			//! Can't use spectatedPlayer::IsRaised() since it won't work for player-based AI
			//! (GetMovementState doesn't work on client for INSTANCETYPE_AI_REMOTE).
			//! Use look and aim direction in relation to head and shoulder to determine if raised.
			if (!spectatedPlayer.GetItemAccessor().IsItemInHandsHidden())
			{
				vector lookDir = headTransform[1];
				vector rHandPos = spectatedPlayer.GetBonePositionWS(spectatedPlayer.GetBoneIndexByName("RightHandIndex4"));
				vector headToRHand = (rHandPos - headPos);
				vector rightShoulder = spectatedPlayer.GetBonePositionWS(spectatedPlayer.GetBoneIndexByName("RightArm"));
				vector rightShoulderToBarrelEnd = (barrel_end - rightShoulder);
				vector lHandPos = spectatedPlayer.GetBonePositionWS(spectatedPlayer.GetBoneIndexByName("LeftHandIndex4"));
				vector rightShoulderToLHand = (lHandPos - rightShoulder);
				vector rightShoulderToRHand = (rHandPos - rightShoulder);

			#ifdef DIAG_DEVELOPER
				int lookDirColor = COLOR_RED;
				int headToRHandColor = COLOR_RED;
				int aimDirColor = COLOR_RED;
				int rightShoulderToBarrelEndColor = COLOR_RED;
				int rightShoulderToRHandColor = COLOR_RED;
				int rightShoulderToLHandColor = COLOR_BLUE;
			#endif

				float lookDot = vector.Dot(lookDir, headToRHand.Normalized());
				if (lookDot > 0.82)
				{
				#ifdef DIAG_DEVELOPER
					lookDirColor = Colors.ORANGE | 0xFF000000;
					headToRHandColor = Colors.ORANGE | 0xFF000000;
				#endif

					float aimDot = vector.Dot(aimDir, rightShoulderToBarrelEnd.Normalized());
					if (aimDot > 0.98)
					{
					#ifdef DIAG_DEVELOPER
						aimDirColor = Colors.ORANGE | 0xFF000000;
						rightShoulderToBarrelEndColor = Colors.ORANGE | 0xFF000000;
					#endif

						if (vector.Dot(aimDir, rightShoulderToRHand.Normalized()) > 0.9)
						{
						#ifdef DIAG_DEVELOPER
							lookDirColor = COLOR_GREEN;
							headToRHandColor = COLOR_GREEN;
							aimDirColor = COLOR_GREEN;
							rightShoulderToBarrelEndColor = COLOR_GREEN;
							rightShoulderToRHandColor = COLOR_GREEN;
						#endif

							//if (vector.Dot(aimDir, rightShoulderToLHand.Normalized()) > 0.9)
								weaponRaised = true;
						}
					}
				}

			#ifdef DIAG_DEVELOPER
				if (s_DbgDraw)
				{
					Debug.DrawArrow(headPos, headPos + lookDir * 0.5, 0.1, lookDirColor, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);
					Debug.DrawArrow(headPos, headPos + headToRHand, 0.1, headToRHandColor, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);

				/*
					vector neckTransform[4];
					spectatedPlayer.GetBoneTransformWS(spectatedPlayer.GetBoneIndexByName("Neck"), neckTransform);
					vector neckPos = neckTransform[3];
					vector neckDir = neckTransform[1];
					Debug.DrawArrow(neckPos, neckPos + neckDir * 0.5, 0.1, COLOR_WHITE, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);

					vector spineTransform[4];
					spectatedPlayer.GetBoneTransformWS(spectatedPlayer.GetBoneIndexByName("Spine3"), spineTransform);
					vector spinePos = spineTransform[3];
					vector spineDir = spineTransform[1];
					Debug.DrawArrow(spinePos, spinePos + spineDir * 0.5, 0.1, COLOR_WHITE, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);

					vector pelvisTransform[4];
					spectatedPlayer.GetBoneTransformWS(spectatedPlayer.GetBoneIndexByName("Pelvis"), pelvisTransform);
					vector pelvisPos = pelvisTransform[3];
					vector pelvisDir = pelvisTransform[1];
					Debug.DrawArrow(pelvisPos, pelvisPos + pelvisDir * 0.5, 0.1, COLOR_WHITE, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);
					vector objectDir = objectTransform[2];

					Debug.DrawArrow(objectPos, objectPos + objectDir * 0.5, 0.1, COLOR_WHITE, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);
				*/

					Debug.DrawArrow(barrel_start, barrel_end + aimDir * 0.5, 0.1, aimDirColor, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);

					Debug.DrawArrow(rightShoulder, rightShoulder + rightShoulderToBarrelEnd, 0.1, rightShoulderToBarrelEndColor, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);

					Debug.DrawArrow(rightShoulder, rightShoulder + rightShoulderToRHand, 0.1, rightShoulderToRHandColor, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);

					//Debug.DrawArrow(rightShoulder, rightShoulder + rightShoulderToLHand, 0.1, rightShoulderToLHandColor, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);
				}
			#endif
			}
			m_JM_IsADS = IsActive() && weaponRaised && vector.DistanceSq(eyePos, headPos) < 0.04;
		}
		else
		{
			m_JM_IsADS = false;
		}

		if ( !IsActive() )
		{
			if (spectatedPlayer && spectatedPlayer.m_JM_IsHeadInvisible)
				spectatedPlayer.SetHeadInvisible(false);

			//return;
		}

		vector pos = headPos;

		if (spectatedPlayer || SelectedTarget.IsInherited(DayZInfected))
			pos = pos + "0 0.1 0";
		else
			pos = pos + "0 0.3 0";

		bool dollyCam;
		float speed;
		switch (m_JM_3rdPerson)
		{
			case JMCamera3rdPersonMode.DOLLY:
			case JMCamera3rdPersonMode.AUTO:
				dollyCam = true;
				//if (m_JM_3rdPerson != JMCamera3rdPersonMode.AUTO || !weaponRaised)
				//{
					speed = 0.45;  //! Slow interpolation, resulting in movement like camera on a dolly
					break;
				//}
			default:
				speed = 0.3;
				break;
		}

		if (GetUApi().GetInputByID(UAZoomIn).LocalValue())
			fov = GameConstants.DZPLAYER_CAMERA_FOV_EYEZOOM;
		else
			fov = GetDayZGame().GetUserFOV();

		if (m_JM_IsADS || weaponRaised)
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

			begPos = barrel_end;
			dir = aimDir;

			if (!m_JM_3rdPerson)
			{
				pos = eyePos - dir * 0.06;
				fov = GameConstants.DZPLAYER_CAMERA_FOV_IRONSIGHTS;
			}
		}
		else if (spectatedPlayer)
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

			float waterDepth = wl[0];
			float characterDepth = wl[1];

			begPos = headPos;

			if ((waterDepth > 1.5 && characterDepth > 0) || waterLevel == EWaterLevels.LEVEL_SWIM_START)
			{
				//! Swimming
				isSwimming = true;

				dir = objectTransform[2];

				if (dollyCam)
					pos = objectPos + "0 2.1 0";
				else if (m_JM_3rdPerson)
					pos = headPos + "0 0.5 0";
			}
			else
			{
				dir = headTransform[1];
			}

			//! This is a bit of a dodgy way to detect if spectated player is climbing a ladder, but I see no better way
			IEntity parent = spectatedPlayer.GetParent();
			if (parent && parent.IsInherited(BuildingBase))
				isOnLadder = true;
		}
		else
		{
			begPos = headPos;

			DayZCreature spectatedCreature;

			if (SelectedTarget.IsInherited(DayZInfected))
			{
				dir = headTransform[1];
			}
			else if (Class.CastTo(spectatedCreature, SelectedTarget))
			{
				//dir = headTransform[0];

				vector chestPos = spectatedCreature.GetBonePositionWS(spectatedCreature.GetBoneIndexByName("Chest"));
				chestPos[1] = chestPos[1] + 0.1;
				dir = (headPos - chestPos).Normalized();

				if (m_JM_3rdPerson)
					pos = headPos + "0 0.5 0";
			}
			else
			{
				dir = objectTransform[2];
			}
		}

		vector cameraPos = GetPosition();

	#ifdef DIAG_DEVELOPER
		if (s_DbgDraw && !IsActive())
			DrawCube(cameraPos, 0.1);
	#endif

		float cameraDistToSpectatedObjSq = vector.DistanceSq(cameraPos, pos);

		if (dollyCam)
		{
			if (!isSwimming)
			{
				//! Eliminate head bob
				float stanceHeight = pos[1] - objectPos[1];

				if (stanceHeight < 0.6)
					stanceHeight = 0.5;  //! Prone
				else if (stanceHeight < 1.3)
					stanceHeight = 1.2;  //! Crouched
				else
					stanceHeight = 1.6;  //! Standing

				pos = objectPos;
				pos[1] = pos[1] + stanceHeight;
			}

			if (cameraDistToSpectatedObjSq > 0.04)
			{
				Object target;
				vector cameraToTargetDir;

				vector hitPos;
				vector hitDir;
				int component;
				set<Object> results = new set<Object>;

				//! Raycast in spectated entity look direction to check for a suitable target to look at
				if (DayZPhysics.RaycastRV(begPos, begPos + dir * 1000, hitPos, hitDir, component, results,
										  null, SelectedTarget, false, false, ObjIntersectView, 0.2))
				{
					foreach (Object result: results)
					{
						if ((result.IsMan() || result.IsDayZCreature()) && !result.IsDamageDestroyed())
						{
						#ifdef DZ_Expansion_AI
							if (spectatedPlayer)
							{
								DayZPlayerImplement targetPlayer;
								if (Class.CastTo(targetPlayer, result) && targetPlayer.GetGroup() == spectatedPlayer.GetGroup())
									continue;
							}
						#endif

							target = result;
							if (target != m_COT_LookAtTarget)
								m_COT_LookAtTarget_Time = 0;
							break;
						}
					}
				}

				if (!target)
					target = m_COT_LookAtTarget;

				if (target)
				{
					bool checkTarget = true;

					vector targetPos = target.GetPosition();
					targetPos[1] = targetPos[1] + 1.6;
					cameraToTargetDir = vector.Direction(cameraPos, targetPos);

					float cameraDistToTargetSq = cameraToTargetDir.LengthSq();

					vector toTargetDir = vector.Direction(begPos, targetPos);
					toTargetDir.Normalize();

					float angle1 = dir.VectorToAngles()[0];
					float angle2 = toTargetDir.VectorToAngles()[0];
					float angleDiff = Math.AbsFloat(angle2 - angle1);

					//! Raycast from camera to target to check whether view is obstructed or not
					if (DayZPhysics.RaycastRV(cameraPos, targetPos, hitPos, hitDir, component, results,
											  null, SelectedTarget, false, false, ObjIntersectView))
					{
						if (vector.DistanceSq(cameraPos, hitPos) < cameraDistToTargetSq - 0.04)
						{
							//! View to target currently obstructed
							if (target == m_COT_LookAtTarget)
							{
								//! View to target wasn't obstructed before, tick up time before we discard target
								m_COT_LookAtTarget_Time += timeslice;
								if (m_COT_LookAtTarget_Time > Math.Lerp(1.5, 3, angleDiff / 360))
									target = null;
							}
							else
							{
								//! View to target was always obstructed
								target = null;
							}

							checkTarget = false;
						}
					}

					if (checkTarget)
					{
						float angleThresh;
						if (weaponRaised)
							angleThresh = 30;
						else
							angleThresh = 180;

						DayZCreatureAI creature;
						vector cameraDir = GetDirection();
						vector cameraToTargetDirNorm = cameraToTargetDir.Normalized();
						if (target.IsDamageDestroyed() && (vector.Dot(cameraDir, cameraToTargetDirNorm) > 0.866 || angleDiff > angleThresh))
						{
							m_COT_LookAtTarget_Time += timeslice;
							if (m_COT_LookAtTarget_Time > Math.RandomFloat(0.5, 0.8))
								target = null;
						}
						else if (angleDiff > angleThresh || target.IsDamageDestroyed() || (Class.CastTo(creature, target) && (!creature.IsDanger() || cameraDistToTargetSq > 900)))
						{
							m_COT_LookAtTarget_Time += timeslice;
							if (m_COT_LookAtTarget_Time > Math.Lerp(3, 5, angleDiff / 360))
								target = null;
						}
					}

					m_COT_LookAtTarget = target;

					speed = 0.9;
				}

				if (target)
				{
					dir = cameraToTargetDir.Normalized();  //! Look at what spectated entity is looking at

					if (!m_COT_TargetMarker)
					{
					#ifdef DIAG_DEVELOPER
						g_Game.Chat("Creating particle", "colorFriendly");
					#endif

						ParticleSource particle = ParticleManager.GetInstance().PlayOnObject(ParticleList.COT_TARGET_MARKER, target, "0 0 0", "0 0 0", true);

						float scale = 3.0;

						float size;
						particle.GetParameter(0, EmitorParam.SIZE, size);
						particle.SetParameter(-1, EmitorParam.SIZE, size * scale * target.GetScale());

						float velocity;
						particle.GetParameter(0, EmitorParam.VELOCITY, velocity);
						particle.SetParameter(-1, EmitorParam.VELOCITY, velocity / target.GetScale());

						particle.SetParameter(0, EmitorParam.BIRTH_RATE, 1.0);

						SetParticleParm(particle, -1, EmitorParam.EMITOFFSET, Vector(0.0, 2.2 / target.GetScale(), 0.0));

						m_COT_TargetMarker = particle;
					}
					else
					{
						if (m_COT_TargetMarker.GetParent() != target)
						{
						#ifdef DIAG_DEVELOPER
							g_Game.Chat("Reparenting particle", "colorFriendly");
						#endif
							m_COT_TargetMarker.AddAsChild(null);
							m_COT_TargetMarker.AddAsChild(target, "0 0 0", "0 0 0", true);
						}

						if (!m_COT_TargetMarker.IsParticlePlaying())
						{
						#ifdef DIAG_DEVELOPER
							g_Game.Chat("Playing particle", "colorFriendly");
						#endif
							m_COT_TargetMarker.PlayParticle();
						}
					}

					if (!m_COT_TargetLight)
					{
						m_COT_TargetLight = ScriptedLightBase.CreateLight(JMDollyCamLight, vector.Zero);
						m_COT_TargetLight.AttachOnObject(target, "0 2.2 0", "0 -90 0");
					}
					else
					{
						if (m_COT_TargetLight.GetAttachmentParent() != target)
							m_COT_TargetLight.AttachOnObject(target, "0 2.2 0", "0 -90 0");
					}

					m_COT_RemoveMarker = false;
				}
				else if (dollyCam && (m_JM_3rdPerson != JMCamera3rdPersonMode.AUTO || !weaponRaised))
				{
					dir = vector.Direction(cameraPos, pos);  //! Look at spectated entity

					//! Remove the vertical offset used for camera pos while swimming (point camera at spectated entity's head)
					if (isSwimming)
						dir[1] = dir[1] - 0.5;

					m_COT_RemoveMarker = true;

					dir.Normalize();
				}
				else
				{
					m_COT_RemoveMarker = true;
				}
			}
		}
		else
		{
			m_COT_LookAtTarget = null;

			m_COT_RemoveMarker = true;
		}

		if (m_COT_RemoveMarker)
		{
			COT_RemoveMarker();

			m_COT_RemoveMarker = false;
		}

		vector fromOri = GetOrientation();
		vector targetOri = Math.COT_DirToOri(dir);
		targetOri[1] = Math.Clamp(targetOri[1], -85, 85);

		float dollyCamSpeedMultMin = 0.3;  //! Fastest
		float dollyCamSpeedMultMax = 0.5;  //! Slowest
		float dollyCamSpeedMult;
		float distSqThreshMult = m_COT_DollyCamDistThreshMult;

		bool isUnderRoofBuilding = IsUnderRoofBuilding(spectatedPlayer);

		vector movementDir = objectPos - m_COT_LastObjectPos;
		vector spectatedObjectVelocity = movementDir * (1.0 / timeslice);  //! GetVelocity(spectatedPlayer) returns 0 on client
		if (!isOnLadder)
			movementDir[1] = 0;  //! ignore vertical velocity so we don't speed up during climbing
		movementDir.Normalize();
		m_COT_SpectatedObjectSpeed = Math.Lerp(m_COT_SpectatedObjectSpeed, spectatedObjectVelocity.Length(), timeslice * 2);
		float spectatedObjectSpeedInverse01 = Math.Max(1 - m_COT_SpectatedObjectSpeed / 6.565, 0.0);  //! 0 = sprint, 1 = not moving

		if (m_JM_3rdPerson == JMCamera3rdPersonMode.AUTO && weaponRaised)
		{
			m_COT_DollyCamSpeedMult = 0.433333;
			dollyCamSpeedMult = m_COT_DollyCamSpeedMult;
			distSqThreshMult =  Math.Lerp(distSqThreshMult, 0, timeslice);
		}
		else if (isUnderRoofBuilding)
		{
		#ifdef DIAG_DEVELOPER
			if (s_DbgDraw)
				Debug.DrawBoxEx(pos + "-0.25 0.5 -0.25", pos + "0.25 0.5 0.25", COLOR_GREEN, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);
		#endif

			m_COT_DollyCamSpeedMult = Math.Lerp(m_COT_DollyCamSpeedMult, dollyCamSpeedMultMin, timeslice);
			dollyCamSpeedMult = m_COT_DollyCamSpeedMult;
			distSqThreshMult =  Math.Lerp(distSqThreshMult, dollyCamSpeedMult, timeslice);
		}
		else
		{
			if (GetUApi().GetInputByName("UACameraToolSpeedIncrease").LocalValue())
				m_COT_DollyCamSpeedMultTgt = Math.Clamp(m_COT_DollyCamSpeedMultTgt - 0.01, dollyCamSpeedMultMin, dollyCamSpeedMultMax);
			else if (GetUApi().GetInputByName("UACameraToolSpeedDecrease").LocalValue())
				m_COT_DollyCamSpeedMultTgt = Math.Clamp(m_COT_DollyCamSpeedMultTgt + 0.01, dollyCamSpeedMultMin, dollyCamSpeedMultMax);

			m_COT_DollyCamSpeedMult = Math.Lerp(m_COT_DollyCamSpeedMult, m_COT_DollyCamSpeedMultTgt, timeslice * 4);

			dollyCamSpeedMult = m_COT_DollyCamSpeedMult;
			distSqThreshMult =  Math.Lerp(distSqThreshMult, dollyCamSpeedMult, timeslice);

			float speedAdjustment = Math.Lerp(distSqThreshMult, dollyCamSpeedMultMin, spectatedObjectSpeedInverse01);
			distSqThreshMult = Math.Lerp(distSqThreshMult, speedAdjustment, timeslice * 4);
		}

		m_COT_DollyCamDistThreshMult = distSqThreshMult;

		float distSqThresh = 1.0 * distSqThreshMult;

		vector ori;

		yawDiff = Math.COT_AngleDiff2(fromOri[0], targetOri[0]);
		pitchDiff = Math.COT_AngleDiff2(fromOri[1], targetOri[1]);

		targetOri[0] = fromOri[0] + yawDiff;
		targetOri[1] = fromOri[1] + pitchDiff;

		if (m_COT_IsInFreeLook)
		{
			ori = orientation;
		}
		else if (m_JM_3rdPerson)
		{
			ori[0] = Math.SmoothCD(fromOri[0], targetOri[0], m_COT_SmoothVelDir0, speed, 1000, timeslice);
			ori[1] = Math.SmoothCD(fromOri[1], targetOri[1], m_COT_SmoothVelDir1, speed, 1000, timeslice);
		}
		else
		{
			ori = vector.Lerp(fromOri, targetOri, timeslice * CAMERA_FOV_SPEED_MODIFIER);
		}

		m_COT_DollyCamPathUpdateDT += timeslice;

		float dollyCamPathUpdateDTThresh = 0.033333;

		if (m_COT_DollyCamPathUpdateDT > dollyCamPathUpdateDTThresh)
		{
			string errorMsg;

			if (m_COT_DollyCamPathNextIdx < 0)
			{
				errorMsg = "COT dollycam: ERROR: " + m_COT_DollyCamPathNextIdx;
				g_Game.Chat(errorMsg, "colorImportant");
				Error(errorMsg);
				m_COT_DollyCamPathNextIdx = 0;
			}
			else if (m_COT_DollyCamPathNextIdx >= DOLLY_CAM_PATH_LIMIT)
			{
				errorMsg = "COT dollycam: ERROR: " + m_COT_DollyCamPathNextIdx;
				g_Game.Chat(errorMsg, "colorImportant");
				Error(errorMsg);
				m_COT_DollyCamPathNextIdx = DOLLY_CAM_PATH_LIMIT - 1;
			}

			if (m_COT_DollyCamJumpClimbTimeout > 0.0)
			{
				if (m_COT_DollyCamReversing)
				{
					m_COT_DollyCamReversing = false;

				#ifdef DIAG_DEVELOPER
					g_Game.Chat("COT dollycam: No longer reversing " + m_COT_DollyCamPathIdx + " " + m_COT_DollyCamPathNextIdx + ": Jump/climb detected", "colorAction");
				#endif
				}

				m_COT_DollyCamJumpClimbTimeout = Math.Max(m_COT_DollyCamJumpClimbTimeout - m_COT_DollyCamPathUpdateDT, 0);

			#ifdef DIAG_DEVELOPER
				if (m_COT_DollyCamJumpClimbTimeout == 0)
					g_Game.Chat("COT dollycam: Depleted jump/climb timeout", "colorAction");
			#endif
			}
			else if (dollyCam && !isSwimming && !isOnLadder && !m_COT_DollyCamReversing && m_COT_DollyCamPathNextIdx > 1 && movementDir.LengthSq() > 0.0001)
			{
				vector prevPoint = m_COT_DollyCamPath[m_COT_DollyCamPathNextIdx - 2];
				vector lastSegDir = m_COT_DollyCamPath[m_COT_DollyCamPathNextIdx - 1] - prevPoint;
				float yDiffSeg = Math.Max(lastSegDir[1], 0.0);
				lastSegDir[1] = 0;

				float segDist2DSq = lastSegDir.LengthSq();
				float segPitch = Math.Atan2(yDiffSeg, segDist2DSq) * Math.RAD2DEG;
				if (yDiffSeg > 0.3 && Math.AbsFloat(segPitch) > 70)
				{
					if (m_COT_DollyCamJumpClimbTimeout == 0)
					{
						m_COT_DollyCamJumpClimbTimeout = 5;

					#ifdef DIAG_DEVELOPER
						g_Game.Chat("COT dollycam: Jump/climb detected (pitch " + segPitch.ToString() + "°), not reversing", "colorAction");
					#endif
					}
				}
				else if (segDist2DSq > 0.0001)
				{
					lastSegDir.Normalize();

					float dot = vector.Dot(movementDir, lastSegDir);

					bool reversing = (dot < 0.0);

					if (reversing)
					{
						--m_COT_DollyCamPathNextIdx;

					#ifdef DIAG_DEVELOPER
						g_Game.Chat("COT dollycam: Reversing " + m_COT_DollyCamPathIdx, "colorAction");
					#endif
					}

					m_COT_DollyCamReversing = reversing;
				}
			}
			else if (m_COT_DollyCamReversing)
			{
				float smallestDistSq = float.MAX;
				float distSq;
				int smallestDistIdx;
				vector point;

				//! Find closest point on path to spectated object current pos
				for (i = m_COT_DollyCamPathNextIdx; i >= 0; --i)
				{
					point = m_COT_DollyCamPath[i];
					distSq = vector.DistanceSq(pos, point);
					if (distSq < smallestDistSq)
					{
						smallestDistSq = distSq;
						smallestDistIdx = i;
					}
				}

				m_COT_DollyCamPathNextIdx = smallestDistIdx;

				bool tooFar = smallestDistSq > 1.0;
				bool depleted = m_COT_DollyCamPathNextIdx < 2;

				if (tooFar || depleted)
				{
					m_COT_DollyCamReversing = false;

					if (tooFar)
						dollyCamSpeedMult *= 2;  //! Slow down to bridge the gap

				#ifdef DIAG_DEVELOPER
					string reason;
					if (tooFar)
						reason = "too far";
					else
						reason = "depleted";
					g_Game.Chat("COT dollycam: No longer reversing " + m_COT_DollyCamPathIdx + " " + m_COT_DollyCamPathNextIdx + ": " + reason, "colorAction");
				#endif
				}
			}

			if (cameraDistToSpectatedObjSq > 0.04 && m_COT_SpectatedObjectSpeed > 0.2)
			{
				if (!m_COT_DollyCamReversing)
				{
					m_COT_DollyCamPath[m_COT_DollyCamPathNextIdx] = pos;

					++m_COT_DollyCamPathNextIdx;
				}
			}
		}

		float accumulatedDistSq;

		if (dollyCam)
		{
			if (m_COT_DollyCamPathNextIdx > 0)
			{
				float stepDistSq;
				vector lastPos = pos;

				for (i = m_COT_DollyCamPathNextIdx - 1; i >= 0; --i)
				{
					pos = m_COT_DollyCamPath[i];

				#ifdef DIAG_DEVELOPER
					if (s_DbgDraw)
					{
						//Debug.DrawSphere(pos, 0.01, COLOR_GREEN, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER);
						Debug.DrawArrow(pos, lastPos, 0.02, COLOR_GREEN, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);
					}
				#endif

					float yDiff = Math.Max(pos[1] - lastPos[1], 0);
					if (!isOnLadder)
						lastPos[1] = pos[1];
					stepDistSq = vector.DistanceSq(pos, lastPos);
					accumulatedDistSq += stepDistSq;
					float pitch = Math.Atan2(yDiff, Math.Sqrt(stepDistSq)) * Math.RAD2DEG;

					if (!isSwimming && !isOnLadder && yDiff > 0.3 && Math.AbsFloat(pitch) > 70)
					{
					#ifdef DIAG_DEVELOPER
						if (m_COT_DollyCamJumpClimbTimeout == 0)
						{
							g_Game.Chat("COT dollycam: Jump/climb detected (pitch " + pitch.ToString() + "°)", "colorAction");
						}
					#endif

						m_COT_DollyCamJumpClimbTimeout = 5.0;

						//! Set all previous values Y to current value Y if previous Y lower than current Y
						for (j = 0; j < i; ++j)
						{
							float vj = m_COT_DollyCamPath[j][1];
							float vi = pos[1]
							if (vj < vi)
								m_COT_DollyCamPath[j][1] = vi;
						}
					}

				#ifdef DIAG_DEVELOPER
					if (s_DbgDraw && i > 0)
					{
						//Debug.DrawSphere(pos, 0.01, COLOR_GREEN, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER);
						Debug.DrawArrow(m_COT_DollyCamPath[i - 1], m_COT_DollyCamPath[i], 0.02, COLOR_RED, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER);
					}
				#endif

					if (accumulatedDistSq >= distSqThresh)
					{
						m_COT_DollyCamPathIdx = i;
						break;
					}

					lastPos = pos;
				}

				if (accumulatedDistSq < distSqThresh)
				{
					m_COT_DollyCamPathIdx = 0;
				}
			}
		}

		if (m_COT_DollyCamPathUpdateDT > dollyCamPathUpdateDTThresh)
			m_COT_DollyCamPathUpdateDT = 0;

		ori = ori.GetRelAngles();
		dir = ori.AnglesToVector();

		float targetDist;
		if (m_JM_3rdPerson)
		{
			float offsetFactor;

			targetDist = 1.6;

			if (dollyCam && (m_JM_3rdPerson != JMCamera3rdPersonMode.AUTO || !weaponRaised))
			{
			/*
				if (weaponRaised)
				{
					targetDist *= spectatedObjectSpeedInverse01;

					float targetDistSq = targetDist * targetDist;

					if (cameraDistToSpectatedObjSq > targetDistSq)
						offsetFactor = targetDistSq / cameraDistToSpectatedObjSq;
					else
						offsetFactor = 1.0;
				}
			*/
			}
			else if (!dollyCam || accumulatedDistSq < Math.Pow(targetDist * 1.875, 2.0))
			{
				offsetFactor = 1.0;
				begPos = pos;
				pos = pos - dir * targetDist;

				ResolveCollision(begPos, cameraPos, pos, offsetFactor);
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
			pos = pos + objectTransform[2] * targetDist;
		}

		m_COT_LastObjectPos = objectPos;

		SetOrientation(ori);

		if (!m_COT_DollyCamReversing && m_COT_DollyCamPathNextIdx >= DOLLY_CAM_PATH_LIMIT)
		{
			for (i = 0; i < DOLLY_CAM_PATH_LIMIT; ++i)
			{
				m_COT_DollyCamPath[i] = m_COT_DollyCamPath[i + 1];
			}

			m_COT_DollyCamPathNextIdx = DOLLY_CAM_PATH_LIMIT - 1;
		}

		if (m_JM_CameraPosMS == vector.Zero)
		{
			cameraPos = pos;
			m_JM_CameraPosMS = pos.InvMultiply4(objectTransform);
		}
		else
		{
			if (dollyCam)
			{
				//! Interpolate in world space to give cinematic look

				//float camToTargetPosDist = vector.Distance(cameraPos, pos);
				//if (camToTargetPosDist > 0)
					//dollyCamSpeedMult /= camToTargetPosDist;

				cameraPos[0] = Math.SmoothCD(cameraPos[0], pos[0], m_COT_SmoothVelPos0, 0.9 * dollyCamSpeedMult, 10000, timeslice);
				cameraPos[1] = Math.SmoothCD(cameraPos[1], pos[1], m_COT_SmoothVelPos1, 0.8 * dollyCamSpeedMult, 10000, timeslice);
				cameraPos[2] = Math.SmoothCD(cameraPos[2], pos[2], m_COT_SmoothVelPos2, 0.9 * dollyCamSpeedMult, 10000, timeslice);

				m_JM_CameraPosMS = cameraPos.InvMultiply4(objectTransform);
			}
			else
			{
				//! Interpolate in model space so camera sticks to character
				pos = pos.InvMultiply4(objectTransform);

				m_JM_CameraPosMS = vector.Lerp(m_JM_CameraPosMS, pos, timeslice * CAMERA_FOV_SPEED_MODIFIER);

				//m_JM_CameraPosMS[0] = Math.SmoothCD(m_JM_CameraPosMS[0], pos[0], m_COT_SmoothVelPos0, 0.3, 1000, timeslice);
				//m_JM_CameraPosMS[1] = Math.SmoothCD(m_JM_CameraPosMS[1], pos[1], m_COT_SmoothVelPos1, 0.3, 1000, timeslice);
				//m_JM_CameraPosMS[2] = Math.SmoothCD(m_JM_CameraPosMS[2], pos[2], m_COT_SmoothVelPos2, 0.3, 1000, timeslice);

				cameraPos = m_JM_CameraPosMS.Multiply4(objectTransform);
			}
		}

		float surfaceY = g_Game.SurfaceRoadY3D(cameraPos[0], cameraPos[1] - offsetY, cameraPos[2], RoadSurfaceDetection.LEGACY) + 0.1;
		if (surfaceY > cameraPos[1])
			cameraPos[1] = surfaceY;

		SetPosition( cameraPos );

		if (spectatedPlayer && IsActive())
		{
			float cameraDistanceToHeadSq = vector.DistanceSq(cameraPos, headPos);
			if (cameraDistanceToHeadSq < 0.0625 && !spectatedPlayer.m_JM_IsHeadInvisible)
				spectatedPlayer.SetHeadInvisible(true);
			else if (m_JM_3rdPerson && cameraDistanceToHeadSq >= 0.0625 && spectatedPlayer.m_JM_IsHeadInvisible)
				spectatedPlayer.SetHeadInvisible(false);
		}

		SetFOV( Math.Lerp(GetCurrentFOV(), fov, timeslice * CAMERA_FOV_SPEED_MODIFIER) );
	}

	void ResolveCollision(vector origin, vector cameraPos, inout vector pos, inout float offsetFactor)
	{
		vector toCameraDir = (cameraPos - origin).Normalized();
		float r = toCameraDir.Length();

		if (r <= 0)
			return;

		float rayRadius = r * 0.2;
		float rayLength = r - rayRadius;
		float minDist = r;
		int segments = 8;

		for (int i = 1; i < segments; ++i)
		{
			float theta = ((float)i / (float)segments) * Math.PI2;
			vector endPos = Vector(origin[0] + rayLength * Math.Cos(theta), origin[1], origin[2] + rayLength * Math.Sin(theta));
			vector rayDir = (endPos - origin).Normalized();
			vector begPos = origin + rayDir * rayRadius;
			vector hitPosition;
			vector hitNormal;
			int hitComponent;
			set<Object> hitObjs = new set<Object>;

			if (DayZPhysics.RaycastRV(begPos, endPos, hitPosition, hitNormal, hitComponent, hitObjs,
									  null, SelectedTarget, false, false, ObjIntersectGeom, rayRadius))
			{
				if (!hitObjs.Count() || !hitObjs[0].IsBush())
				{
					vector hitDirection = (hitPosition - origin);
					float dist = hitDirection.Length();

					if (dist < minDist)
					{
						minDist = dist;
						//! Pull camera toward player to avoid clipping
						pos = origin + toCameraDir * Math.Max(dist - 0.05, 0);
						offsetFactor = (dist / r) * 0.54;
					}
				}
			}
		}
	}

	bool IsUnderRoofBuilding(DayZPlayerImplement player)
	{
		float hitFraction;
		vector hitPosition, hitNormal;
		vector from;
		vector to;
		Object hitObject;
		PhxInteractionLayers collisionLayerMask = PhxInteractionLayers.ITEM_LARGE|PhxInteractionLayers.BUILDING|PhxInteractionLayers.VEHICLE;
		
		if (player)
		{
			//! if inside vehicle return immediately
			if (player.IsInVehicle())
				return false;

			//! XXX: Somehow this doesn't work reliably?
			IEntity floorEntity = player.PhysicsGetFloorEntity();
			if (floorEntity && floorEntity.IsInherited(House))
				return true;

			//! Player position raycast
			from = player.GetPosition();
			to = from + "0 25 0";

			DayZPhysics.RayCastBullet(from, to, collisionLayerMask, null, hitObject, hitPosition, hitNormal, hitFraction);

			if (hitObject && hitObject.IsInherited(House))
				return true;
		}

		//! Camera position raycast
		from = GetPosition();
		to = from + "0 25 0";
		
		DayZPhysics.RayCastBullet(from, to, collisionLayerMask, null, hitObject, hitPosition, hitNormal, hitFraction);

		return hitObject && hitObject.IsInherited(House);
	}

	void COT_RemoveMarker()
	{
		if (m_COT_TargetMarker)
		{
		#ifdef DIAG_DEVELOPER
			g_Game.Chat("Stopping particle", "colorFriendly");
		#endif
			if (m_COT_TargetMarker.IsParticlePlaying())
				m_COT_TargetMarker.StopParticle();
			m_COT_TargetMarker = null;
		}

		if (m_COT_TargetLight)
		{
			m_COT_TargetLight.FadeOut();
			m_COT_TargetLight = null;
		}
	}
	
	static Shape DrawCube(vector pos, float size = 1, int color = 0x1fff7f7f)
	{
		vector min = pos;
		vector max = pos;
		
		float size_h = size * 0.5;
	
		min[0] = min[0] - size_h;
		min[1] = min[1] - size_h;
		min[2] = min[2] - size_h;
		
		max[0] = max[0] + size_h;
		max[1] = max[1] + size_h;
		max[2] = max[2] + size_h;
		
		Shape shape = Shape.Create(ShapeType.DIAMOND, color, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.ADDITIVE | ShapeFlags.NOZBUFFER, min, max);
		return shape;
	}
};
