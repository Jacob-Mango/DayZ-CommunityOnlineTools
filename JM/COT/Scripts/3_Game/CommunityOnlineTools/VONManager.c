modded class VONManagerImplementation
{
	override void OnEvent(EventType eventTypeId, Param params)
	{
		super.OnEvent(eventTypeId, params);

		ScriptRPC rpc;
		switch (eventTypeId)
		{
			case VONUserStartedTransmittingAudioEventTypeID:
				rpc = new ScriptRPC();
				rpc.Send(g_Game.GetPlayer(), JMPlayerModuleRPC.VONStartedTransmitting, true, null);
				break;
			case VONUserStoppedTransmittingAudioEventTypeID:
				rpc = new ScriptRPC();
				rpc.Send(g_Game.GetPlayer(), JMPlayerModuleRPC.VONStoppedTransmitting, true, null);
				break;
		}
	}
}

