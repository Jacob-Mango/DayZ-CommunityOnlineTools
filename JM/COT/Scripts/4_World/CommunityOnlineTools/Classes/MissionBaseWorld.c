modded class MissionBaseWorld
{
	protected bool m_COT_DisableResetGUI;

	void COT_DisableResetGUI(bool disable = true)
	{
		m_COT_DisableResetGUI = disable;
	}

	bool COT_IsDisableResetGUI()
	{
		return m_COT_DisableResetGUI;
	}
}
