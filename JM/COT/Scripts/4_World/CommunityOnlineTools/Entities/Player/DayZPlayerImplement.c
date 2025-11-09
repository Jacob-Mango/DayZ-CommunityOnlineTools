modded class DayZPlayerImplement
{
	private JMSpectatorCamera m_SpectatorCamera;
	private Head_Default m_PlayerHead;
	bool m_JM_IsHeadInvisible;

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

		m_SpectatorCamera.m_JM_CameraPosMS = vector.Zero;
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

		m_SpectatorCamera.OnUpdateEx(this, timeSlice);
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

	//! Prevent calling vanilla EntityAI::OnDebugSpawn
	//! Logic will use COT to spawn attachments, so it'll work like vanilla EXCEPT not have possibility of crashes due to bad items
	override void OnDebugSpawn()
	{
		GetCommunityOnlineToolsBase().SpawnCompatibleAttachments(this, null, 0);
	}

	void COT_OnDebugSpawn(PlayerBase player)
	{
		OnDebugSpawnEx(DebugSpawnParams.WithPlayer(player));
	}
}