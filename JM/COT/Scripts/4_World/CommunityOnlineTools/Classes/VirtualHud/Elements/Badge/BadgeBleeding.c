modded class BadgeBleeding
{
	PlayerBase m_JM_SpectatedPlayer;

	override bool IsValueChanged()
	{
		if ( m_Player )
		{
			if ( m_Player.m_JM_SpectatedObject && Class.CastTo( m_JM_SpectatedPlayer, m_Player.m_JM_SpectatedObject ) )
			{
				m_Value = m_JM_SpectatedPlayer.GetBleedingSourceCount();
			}
			else if ( m_JM_SpectatedPlayer )
			{
				m_JM_SpectatedPlayer = null;
				m_Value = m_Player.GetBleedingSourceCount();
			}
		}

		return super.IsValueChanged();
	}
}
