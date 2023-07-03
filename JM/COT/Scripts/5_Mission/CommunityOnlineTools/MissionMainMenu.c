modded class MissionMainMenu
{
	override void OnMissionStart()
	{
		auto trace = CF_Trace_0(this, "OnMissionStart");

		super.OnMissionStart();

		foreach (auto params: JMDeferredMessage.QueuedMessages)
		{
			CF_Log.Debug("%1 %2", params.Title, params.Message);
			GetGame().GetUIManager().ShowDialog(params.Title, params.Message, 1, DBT_OK, DBB_OK, DMT_WARNING, g_Game.GetUIManager().GetMenu());
		}

		JMDeferredMessage.QueuedMessages.Clear();
	}
};
