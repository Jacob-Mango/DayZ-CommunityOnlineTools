modded class TransferValues
{
	PlayerBase m_JM_Owner;
	PlayerBase m_JM_SpectatedPlayer;

	void TransferValues(PlayerBase player)
	{
		m_JM_Owner = player;
	}

	override void CheckHealth()
	{
		COT_SubstituteSpectatedPlayer();

		super.CheckHealth();

		COT_RestoreOwner();  //! Restore in case super didn't send value
	}

	override void CheckBlood()
	{
		COT_SubstituteSpectatedPlayer();

		super.CheckBlood();

		COT_RestoreOwner();  //! Restore in case super didn't send value
	}

	override void SendValue(int value_type, float value)
	{
		COT_RestoreOwner();

		super.SendValue(value_type, value);
	}

	override void ReceiveValue(int value_type, float value)
	{
		super.ReceiveValue(value_type, value);

		//! m_JM_SpectatedPlayer will not be set on client
		if (m_JM_Owner.m_JM_SpectatedObject && m_JM_Owner.m_JM_SpectatedObject.IsInherited(PlayerBase))
		{
			if (value_type == TYPE_BLOOD)
			{
				//! Have to stop PPE here to deal with the case where owner player has bloodloss symptom but spectated entity hasn't,
				//! so the symptom's OnUpdateClient would not run
				PPERequester_BloodLoss ppeBloodloss;
				if (Class.CastTo(ppeBloodloss, PPERequesterBank.GetRequester(PPERequester_BloodLoss)))
				{
					if (m_BloodClient == 1)
						ppeBloodloss.Stop();
				}
			}
		}
	}

	void COT_SubstituteSpectatedPlayer()
	{
		if (Class.CastTo(m_JM_SpectatedPlayer, m_JM_Owner.m_JM_SpectatedObject))
			m_Player = m_JM_SpectatedPlayer;
	}

	void COT_RestoreOwner()
	{
		if (m_JM_SpectatedPlayer)
			m_Player = m_JM_Owner;
	}
}
