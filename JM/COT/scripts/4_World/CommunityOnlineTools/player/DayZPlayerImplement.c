modded class DayZPlayerImplement
{
	private JMSpectatorCamera m_SpectatorCamera;
	private Head_Default m_PlayerHead;

	void DayZPlayerImplement()
	{
		if ( IsMissionClient() )
			SetEventMask( EntityEvent.FRAME | EntityEvent.SIMULATE );
	}

	void OnSpectateStart( JMSpectatorCamera camera )
	{
		m_SpectatorCamera = camera;

		SetHeadInvisible( true );
	}

	void OnSpectateEnd()
	{
		m_SpectatorCamera = NULL;

		SetHeadInvisible( false );
	}

	void UpdateSpecatorCamera()
	{
		if ( !m_SpectatorCamera )
			return;
		
		HumanInputController input = GetInputController();
		float heading = GetOrientation()[0];
		
		HumanCommandWeapons hcw = GetCommandModifier_Weapons();
		
		float lr = 0;
		float ud = 0;
		if ( hcw )
		{
			lr = hcw.GetBaseAimingAngleLR();
			ud = hcw.GetBaseAimingAngleUD();
		}

		vector localMat[4];
		vector worldMat[4];
		vector cameraMat[4];
		Math3D.YawPitchRollMatrix( Vector( heading + lr, ud, 0 ), localMat );
		localMat[3] = GetBonePositionMS( GetBoneIndexByName( "Head" ) ) + "0.04 0.04 0";

		GetTransformWS( worldMat );
		Math3D.MatrixMultiply4( localMat, worldMat, cameraMat );

		m_SpectatorCamera.SetTransform( cameraMat );
	}

	override void EOnFrame( IEntity other, float timeSlice )
	{
		UpdateSpecatorCamera();
	}

	override void EOnSimulate( IEntity other, float timeSlice )
	{
		UpdateSpecatorCamera();
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
	}

	void SetAttachmentInvisible( string slot, bool invisible )
	{
		EntityAI ent = GetInventory().FindAttachment( InventorySlots.GetSlotIdFromString( slot ) );
		if ( ent )
		{
			ent.SetInvisible( invisible );
		}
	}
};