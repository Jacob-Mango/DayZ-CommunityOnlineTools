modded class PlayerAgentPool
{
    override void AddAgent(int agent_id, float count)
    {
        if ( !m_Player.COTHasGodMode() )
            super.AddAgent(agent_id, count);
    }
};
