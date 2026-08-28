modded class DayZGame
{
	void DayZGame()
	{
		DebugPrint.OnInit();

#ifdef JM_COT_DIAG_LOGGING
		CF_Log.Level = CF_LogLevel.TRACE;
#endif
	}

	//! COT's imageset and style table are registered here, at game creation,
	//! rather than at mission init. Swapping the active imageset after a
	//! mission exists is what kills every MapWidget and ItemPreviewWidget for
	//! the rest of the session - see JMWidgetStyles.
	override void OnAfterCreate()
	{
		super.OnAfterCreate();

		JMWidgetStyles.Load();
	}
}