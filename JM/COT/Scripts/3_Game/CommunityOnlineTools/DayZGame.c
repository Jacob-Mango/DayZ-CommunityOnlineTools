modded class DayZGame
{
	void DayZGame()
	{
		DebugPrint.OnInit();

#ifdef JM_COT_DIAG_LOGGING
		CF_Log.Level = CF_LogLevel.TRACE;
#endif
	}

	override void OnUpdate( bool doSim, float timeslice )
	{
		super.OnUpdate( doSim, timeslice );

		if ( !doSim )
			return;

		if ( JMStatics.SERVER_STATS_TICK )
			JMStatics.SERVER_STATS_TICK.Invoke( timeslice );
	}
}