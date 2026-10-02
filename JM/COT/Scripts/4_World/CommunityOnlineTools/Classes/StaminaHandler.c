modded class StaminaHandler
{
	bool m_JM_IsSpectatingPlayer;
	bool m_JM_HasUnlimitedStamina;
	float m_JM_StaminaBackup = -1;
	float m_JM_StaminaCapBackup;

    override void Update(float deltaT, int pCurrentCommandID)
	{
        if (m_Player)
        {
			float stamina = m_Stamina;
			float staminaCap = m_StaminaCap;

			PlayerBase spectatedPlayer;
			if (m_Player.m_JM_SpectatedObject && Class.CastTo(spectatedPlayer, m_Player.m_JM_SpectatedObject))
			{
				m_JM_IsSpectatingPlayer = true;
				m_JM_HasUnlimitedStamina = false;

				if (m_JM_StaminaBackup == -1)
				{
					m_JM_StaminaBackup = m_Stamina;
					m_JM_StaminaCapBackup = m_StaminaCap;
				}

				StaminaHandler staminaHandler = spectatedPlayer.GetStaminaHandler();
				if (staminaHandler)
				{
					stamina = staminaHandler.GetStamina();
					staminaCap = staminaHandler.GetStaminaCap();
				}
			}
			else if (m_JM_IsSpectatingPlayer)
			{
				m_JM_IsSpectatingPlayer = false;

				stamina = m_JM_StaminaBackup;
				staminaCap = m_JM_StaminaCapBackup;

				m_JM_StaminaBackup = -1;
			}
			else if (m_Player.COTHasUnlimitedStamina())
			{
				m_JM_HasUnlimitedStamina = true;

				if (m_JM_StaminaBackup == -1)
				{
					m_JM_StaminaBackup = m_Stamina;
					m_JM_StaminaCapBackup = m_StaminaCap;
				}

				stamina = GameConstants.STAMINA_MAX; 
				staminaCap = GameConstants.STAMINA_MAX;
			}
			else if (m_JM_HasUnlimitedStamina)
			{
				m_JM_HasUnlimitedStamina = false;

				stamina = m_JM_StaminaBackup;
				staminaCap = m_JM_StaminaCapBackup;

				m_JM_StaminaBackup = -1;
			}

			if (stamina != m_Stamina || staminaCap != m_StaminaCap)
			{
				m_Stamina = stamina;
				m_StaminaCap = staminaCap;
				m_IsInCooldown = false;

				SetStamina(m_Stamina);
				return;
			}
        }

        super.Update(deltaT, pCurrentCommandID);
    }
}
