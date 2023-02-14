modded class Environment
{
	override protected void ProcessItemsWetness(array<int> pSlotIds)
	{
		if (m_Player.COTHasGodMode())
			return;

		super.ProcessItemsWetness(pSlotIds);
	}
};
