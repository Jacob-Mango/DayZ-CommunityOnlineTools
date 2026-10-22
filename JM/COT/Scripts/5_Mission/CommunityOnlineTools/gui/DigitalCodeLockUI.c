modded class DigitalCodeLockUI
{
	override void Update(float timeslice)
	{
		EntityAI targetLock;

		//! Disable distance check while COT sidebar is open
		if ( m_TargetLock && g_cotBase.IsOpen() )
		{
			targetLock = m_TargetLock;
			m_TargetLock = null;
		}

		super.Update(timeslice);

		if (targetLock)
			m_TargetLock = targetLock;
	}
}
