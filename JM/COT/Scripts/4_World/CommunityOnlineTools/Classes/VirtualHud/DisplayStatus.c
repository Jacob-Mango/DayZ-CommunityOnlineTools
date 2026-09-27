modded class VirtualHud
{
	bool m_JM_ResetLastSentArray;

	override void SendRPC()
	{
		if (!COT_SendRPC())
			super.SendRPC();
	}

	bool COT_SendRPC()
	{
		if (!g_Game.IsMultiplayer())
			return false;

		set<PlayerBase> spectators = m_Player.m_JM_Spectators;
		Object spectatedObject = m_Player.m_JM_SpectatedObject;

		if (spectatedObject && spectatedObject.IsInherited(PlayerBase))
		{
			//! If m_Player is spectating another player, skip RPC
			return true;
		}
		else if (!spectators || spectators.Count() == 0)
		{
			m_JM_ResetLastSentArray = false;
			return false;
		}

		if (!m_JM_ResetLastSentArray)
		{
			m_JM_ResetLastSentArray = true;
			m_LastSentArray = null;  //! Reset to force array send
		}

		array<int> maskArray = {};
		SerializeElements(maskArray);
		if (!m_LastSentArray || !AreArraysSame(m_LastSentArray, maskArray))
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(maskArray);

			//! If m_Player is not spectating another player, send their values to themselves
			PlayerIdentity identity = m_Player.GetIdentity();
			if (identity)
				rpc.Send(m_Player, ERPCs.RPC_SYNC_DISPLAY_STATUS, false, identity);

			foreach (PlayerBase spectator: spectators)
			{
				if (spectator)
				{
					//! If m_Player is being spectated, send their values to spectator as well
					PlayerIdentity spectatorIdentity = spectator.GetIdentity();
					if (spectatorIdentity)
						rpc.Send(spectator, ERPCs.RPC_SYNC_DISPLAY_STATUS, false, spectatorIdentity);
				}
			}

			m_LastSentArray = maskArray;
		}

		return true;
	}
}
