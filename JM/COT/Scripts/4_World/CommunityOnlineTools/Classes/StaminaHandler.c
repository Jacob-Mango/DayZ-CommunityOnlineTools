modded class StaminaHandler
{
    override void Update(float deltaT, int pCurrentCommandID)
	{
        if (m_Player && m_Player.COTHasUnlimitedStamina())
        {
		    m_Stamina = GameConstants.STAMINA_MAX; 
		    m_StaminaCap = GameConstants.STAMINA_MAX;
            m_IsInCooldown = false;
            
			SetStamina(m_Stamina);
            return;
        }

        super.Update(deltaT, pCurrentCommandID);
    }
};
