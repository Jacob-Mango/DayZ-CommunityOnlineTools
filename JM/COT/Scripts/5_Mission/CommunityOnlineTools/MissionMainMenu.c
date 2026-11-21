modded class MissionMainMenu
{
	override void OnMissionStart()
	{
	#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnMissionStart");
	#endif

		super.OnMissionStart();

		foreach (auto params: JMDeferredMessage.QueuedMessages)
		{
		#ifdef JM_COT_DIAG_LOGGING
			CF_Log.Debug("%1 %2", params.Title, params.Message);
		#endif
			g_Game.GetUIManager().ShowDialog(params.Title, params.Message, 1, DBT_OK, DBB_OK, DMT_WARNING, g_Game.GetUIManager().GetMenu());
		}

		JMDeferredMessage.QueuedMessages.Clear();
	}
}
