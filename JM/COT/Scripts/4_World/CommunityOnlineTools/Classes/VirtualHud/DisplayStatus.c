modded class VirtualHud
{
	override void SendRPC()
	{
		if (!g_Game.IsMultiplayer() && m_Player == g_Game.GetPlayer())
		{
			UpdateStatus();
			return;
		}

		super.SendRPC();
	}
}
