modded class DayZGame
{
	void DayZGame()
	{
		DebugPrint.OnInit();

#ifdef DIAG
		CF_Log.Level = CF_LogLevel.TRACE;
#endif
	}
};