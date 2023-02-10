modded class DayZPlayerImplement
{
	private JMSpectatorCamera m_SpectatorCamera;
	private Head_Default m_PlayerHead;

	void DayZPlayerImplement()
	{
		if ( IsMissionClient() )
			SetEventMask( EntityEvent.FRAME | EntityEvent.POSTFRAME );
	}

	void OnSpectateStart( JMSpectatorCamera camera )
	{
		m_SpectatorCamera = camera;

		//SetHeadInvisible( true );
	}

	void OnSpectateEnd()
	{
		m_SpectatorCamera = NULL;

		//SetHeadInvisible( false );
	}

	void UpdateSpecatorCamera()
	{
		if ( !m_SpectatorCamera )
			return;
		
		HumanInputController input = GetInputController();
		HumanCommandWeapons hcw = GetCommandModifier_Weapons();
		
		float lr = 0;
		float ud = 0;
		if ( hcw )
		{
			lr = hcw.GetBaseAimingAngleLR();
			ud = hcw.GetBaseAimingAngleUD();
		}

		lr += input.GetAimChange()[0] * Math.RAD2DEG;
		ud += input.GetAimChange()[1] * Math.RAD2DEG;

		//! @note this is not a controlled player since we are spectating,
		//! so any values returned by input controller or weapon cmd will be zero anyway.
		//! TODO: Workaround?
		vector ori = GetOrientation();
		ori[0] = ori[0] + lr;
		ori[1] = ori[1] + ud;

		m_SpectatorCamera.SetOrientation( ori );

		vector headPos = GetBonePositionWS( GetBoneIndexByName( "Head" ) ) + "0 0.1 0";

		m_SpectatorCamera.SetPosition( headPos - ori.AnglesToVector() * 1.33 );
	}

	override void EOnFrame( IEntity other, float timeSlice )
	{
		UpdateSpecatorCamera();
	}

	//override void EOnPostFrame( IEntity other, int extra )
	//{
		//UpdateSpecatorCamera();
	//}

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