modded class DayZPlayerImplement
{
	private JMSpectatorCamera m_SpectatorCamera;
	private Head_Default m_PlayerHead;
	bool m_JM_IsHeadInvisible;

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

		vector mat[4];
		GetBoneTransformWS(GetBoneIndexByName( "Head" ), mat);
		vector headPos = mat[3];

		float cameraDistanceToHeadSq = vector.DistanceSq(m_SpectatorCamera.GetPosition(), headPos);
		if (cameraDistanceToHeadSq < 0.0625 && !m_JM_IsHeadInvisible)
			SetHeadInvisible(true);
		else if (cameraDistanceToHeadSq >= 0.0625 && m_JM_IsHeadInvisible)
			SetHeadInvisible(false);

		EntityAI hands = GetHumanInventory().GetEntityInHands();

		Weapon_Base weapon;
		ItemOptics optic;

		vector eyePos;

		vector dir;
		vector pos;

		float fov;

		if (Class.CastTo(weapon, hands))
		{
			optic = weapon.GetAttachedOptics();
			eyePos = weapon.ModelToWorld(weapon.GetSelectionPositionLS("eye"));
			m_SpectatorCamera.m_JM_IsADS = vector.DistanceSq(eyePos, headPos) < 0.04;
		}
		else
		{
			m_SpectatorCamera.m_JM_IsADS = false;
		}

		if (m_SpectatorCamera.m_JM_IsADS)
		{
			if (optic)
			{
				optic.GetCameraPoint(pos, dir);
				pos = optic.ModelToWorld(pos);

				if (optic.GetCurrentStepFOV() < GameConstants.DZPLAYER_CAMERA_FOV_IRONSIGHTS)
				{
					if (!optic.IsInOptics())
					{
						optic.EnterOptics();
						optic.OnOpticEnter();
					}
				}
				else if (optic.IsInOptics())
				{
					optic.ExitOptics();
					optic.OnOpticExit();
				}
			}
			else
			{
				weapon.GetCameraPoint(weapon.GetCurrentMuzzle(), pos, dir);
				pos = weapon.ModelToWorld(pos);
			}

			vector barrel_start = weapon.ModelToWorld(weapon.GetSelectionPositionLS("konec hlavne"));
			vector barrel_end = weapon.ModelToWorld(weapon.GetSelectionPositionLS("usti hlavne"));
			dir = vector.Direction(barrel_start, barrel_end).Normalized();

			fov = GameConstants.DZPLAYER_CAMERA_FOV_IRONSIGHTS;
		}
		else
		{
			if (optic && optic.IsInOptics())
			{
				optic.ExitOptics();
				optic.OnOpticExit();
			}

			dir = mat[1];
			pos = headPos + "0 0.1 0";

			fov = GetDayZGame().GetUserFOV();
		}

		if (m_SpectatorCamera.m_JM_3rdPerson)
		{
			pos = pos - dir * 1.33;
			vector offsetX = dir.Perpend() * 0.33;
			if (m_SpectatorCamera.m_JM_LeftShoulder)
				pos = pos + offsetX;
			else
				pos = pos - offsetX;
		}

		m_SpectatorCamera.SetDirection( vector.Lerp(m_SpectatorCamera.GetDirection(), dir, timeSlice * CAMERA_FOV_SPEED_MODIFIER) );
		m_SpectatorCamera.SetPosition( vector.Lerp(m_SpectatorCamera.GetPosition(), pos, timeSlice * CAMERA_FOV_SPEED_MODIFIER) );

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