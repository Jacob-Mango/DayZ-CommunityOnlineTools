modded class NotifierBase
{
	PlayerBase m_JM_Owner;
	PlayerBase m_JM_SpectatedPlayer;

	void NotifierBase(NotifiersManager manager)
	{
		m_JM_Owner = manager.GetPlayer();
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
