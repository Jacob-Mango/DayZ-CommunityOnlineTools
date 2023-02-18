modded class DayZPlayerImplement
{
	private JMSpectatorCamera m_SpectatorCamera;
	private Head_Default m_PlayerHead;
	bool m_JM_IsHeadInvisible;
	vector m_JM_CameraPosMS;

	void DayZPlayerImplement()
	{
		if ( IsMissionClient() )
			SetEventMask( EntityEvent.FRAME | EntityEvent.POSTFRAME );
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

		m_SpectatorCamera.SetDirection( vector.Lerp(m_SpectatorCamera.GetDirection(), dir, timeSlice * CAMERA_FOV_SPEED_MODIFIER) );

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
			ent.SetInvisible( invisible );
		}
	}

#ifndef CF_MODULE_PERMISSIONS
	string FormatSteamWebhook()
	{
		return GetDisplayName();
	}
#endif
};