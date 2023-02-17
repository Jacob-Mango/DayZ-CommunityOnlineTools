modded class DayZPlayerImplement
{
	private JMSpectatorCamera m_SpectatorCamera;
	private Head_Default m_PlayerHead;
	bool m_JM_IsHeadInvisible;
	float m_JM_3rdPersonInterpolate;

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

		if (m_SpectatorCamera.m_JM_3rdPerson && m_JM_3rdPersonInterpolate < 1)
		{
			m_JM_3rdPersonInterpolate += timeSlice * 4;
			if (m_JM_3rdPersonInterpolate > 0.25 && m_JM_IsHeadInvisible)
				SetHeadInvisible(false);
			if (m_JM_3rdPersonInterpolate > 1)
				m_JM_3rdPersonInterpolate = 1;
		}
		else if (!m_SpectatorCamera.m_JM_3rdPerson && m_JM_3rdPersonInterpolate > 0)
		{
			m_JM_3rdPersonInterpolate -= timeSlice * 4;
			if (m_JM_3rdPersonInterpolate < 0.25 && !m_JM_IsHeadInvisible)
				SetHeadInvisible(true);
			if (m_JM_3rdPersonInterpolate < 0)
				m_JM_3rdPersonInterpolate = 0;
		}

		vector mat[4];
		GetBoneTransformWS(GetBoneIndexByName( "Head" ), mat);
		vector headPos = mat[3] + "0 0.1 0" - mat[1] * 1.33 * m_JM_3rdPersonInterpolate - mat[1].Perpend() * m_JM_3rdPersonInterpolate * 0.33;

		m_SpectatorCamera.SetDirection( mat[1] );
		m_SpectatorCamera.SetPosition( headPos );
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