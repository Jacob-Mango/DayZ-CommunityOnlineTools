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

	void DayZPlayerImplement()
	{
		if ( IsMissionClient() )
			SetEventMask( EntityEvent.FRAME | EntityEvent.POSTFRAME );
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

		vector headTransform[4];
		GetBoneTransformWS(GetBoneIndexByName( "Head" ), headTransform);
		vector headPos = headTransform[3];

		EntityAI hands = GetHumanInventory().GetEntityInHands();

		Weapon_Base weapon;
		vector weaponTransform[4];
		ItemOptics optic;
		//vector opticTransform[4];

		vector eyePos;

		vector dir;
		vector pos;

		float fov;

		if (Class.CastTo(weapon, hands))
		{
			optic = weapon.GetAttachedOptics();
			weapon.GetTransform(weaponTransform);
			eyePos = weapon.GetSelectionPositionLS("eye").Multiply4(weaponTransform);
			m_SpectatorCamera.m_JM_IsADS = m_SpectatorCamera.IsActive() && vector.DistanceSq(eyePos, headPos) < 0.04;
		}
		else
		{
			m_SpectatorCamera.m_JM_IsADS = false;
		}

		if ( !m_SpectatorCamera.IsActive() )
		{
			if (m_JM_IsHeadInvisible)
				SetHeadInvisible(false);

			if (optic)
				optic.SetInvisible(false);

			return;
		}

		if (m_SpectatorCamera.m_JM_IsADS)
		{
			if (optic)
			{
				//optic.GetCameraPoint(pos, dir);
				//optic.GetTransform(opticTransform);
				//pos = pos.Multiply4(opticTransform).InvMultiply4(playerTransform);

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

				optic.SetInvisible(m_SpectatorCamera.m_JM_1stPersonADS_HideScope);
			}
			//else
			//{
				//weapon.GetCameraPoint(weapon.GetCurrentMuzzle(), pos, dir);
				//pos = pos.Multiply4(weaponTransform).InvMultiply4(playerTransform);
			//}

			vector barrel_start = weapon.GetSelectionPositionLS("konec hlavne").Multiply4(weaponTransform);
			vector barrel_end = weapon.GetSelectionPositionLS("usti hlavne").Multiply4(weaponTransform);
			dir = vector.Direction(barrel_start, barrel_end).Normalized();
			pos = (eyePos - dir * 0.06).InvMultiply4(playerTransform);

			fov = GameConstants.DZPLAYER_CAMERA_FOV_IRONSIGHTS;
		}
		else
		{
			if (optic)
			{
				//if (optic.IsInOptics())
				//{
					//optic.ExitOptics();
					//optic.OnOpticExit();
				//}

				optic.SetInvisible(false);
			}

			dir = headTransform[1];
			pos = headPos.InvMultiply4(playerTransform) + "0 0.1 0";

			fov = GetDayZGame().GetUserFOV();
		}

		dir = vector.Lerp(m_SpectatorCamera.GetDirection(), dir, timeSlice * CAMERA_FOV_SPEED_MODIFIER);

		if (m_SpectatorCamera.m_JM_3rdPerson)
		{
			pos = pos.Multiply4(playerTransform);
			pos = pos - dir * 1.33;
			vector offsetX = dir.Perpend() * 0.33;
			if (m_SpectatorCamera.m_JM_LeftShoulder)
				pos = pos + offsetX;
			else
				pos = pos - offsetX;
			pos = pos.InvMultiply4(playerTransform);
		}

		m_SpectatorCamera.SetDirection( dir );

		if (m_JM_CameraPosMS == vector.Zero)
		{
			m_JM_CameraPosMS = pos;
		}
		else
		{
			m_JM_CameraPosMS = vector.Lerp(m_JM_CameraPosMS, pos, timeSlice * CAMERA_FOV_SPEED_MODIFIER);
		}

		m_SpectatorCamera.SetPosition( m_JM_CameraPosMS.Multiply4(playerTransform) );

		float cameraDistanceToHeadSq = vector.DistanceSq(m_SpectatorCamera.GetPosition(), headPos);
		if (cameraDistanceToHeadSq < 0.0625 && !m_JM_IsHeadInvisible)
			SetHeadInvisible(true);
		else if (cameraDistanceToHeadSq >= 0.0625 && m_JM_IsHeadInvisible)
			SetHeadInvisible(false);

		if (m_SpectatorCamera.m_JM_IsADS && !m_SpectatorCamera.m_JM_3rdPerson)
			m_SpectatorCamera.SetFOV( Math.Lerp(m_SpectatorCamera.GetCurrentFOV(), fov, timeSlice * CAMERA_FOV_SPEED_MODIFIER) );
	}

	override void EOnFrame( IEntity other, float timeSlice )
	{
		UpdateSpecatorCamera(timeSlice);
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