modded class MissionBaseWorld
{
	protected bool m_COT_TempDisableOnSelectPlayer;

	void COT_TempDisableOnSelectPlayer(bool disable = true)
	{
		m_COT_TempDisableOnSelectPlayer = disable;
	}

	bool COT_IsTempDisableOnSelectPlayer()
	{
		return m_COT_TempDisableOnSelectPlayer;
	}

	void COT_LeaveFreeCam()
	{
	}
}
