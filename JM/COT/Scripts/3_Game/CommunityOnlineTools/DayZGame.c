modded class DayZGame
{
	void DayZGame()
	{
		DebugPrint.OnInit();

#ifdef JM_COT_DIAG_LOGGING
		CF_Log.Level = CF_LogLevel.TRACE;
#endif
	}
};