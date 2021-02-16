modded class StaminaHandler
{
    override void Update(float deltaT, int pCurrentCommandID)
	{
        if (m_Player && m_Player.COTHasUnlimitedStamina()) return;
        
        super.Update(deltaT, pCurrentCommandID);
    }
};