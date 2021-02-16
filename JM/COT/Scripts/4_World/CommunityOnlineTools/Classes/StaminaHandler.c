modded class StaminaHandler
{
    override void Update(float deltaT, int pCurrentCommandID)
	{
        if (m_Player && m_Player.COTHasUnlimitedStamina())
        {
			m_Player.SetStamina(m_Stamina, m_StaminaCap);
            return;
        }

        super.Update(deltaT, pCurrentCommandID);
    }
};