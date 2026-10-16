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

		if ( JMStatics.SERVER_STATS_TICK && g_cotGame && g_cotGame.ActiveCount() > 0 )
			JMStatics.SERVER_STATS_TICK.Invoke( timeslice );
	}

	TStringArray COT_GetInventorySlots( string path )
	{
		TStringArray inventorySlots = {};
		string inventorySlotsPath = string.Format( "%1 %2", path, "inventorySlot" );
		string slotName;
		int type = ConfigGetType( inventorySlotsPath );

		if ( type == CT_ARRAY )
			ConfigGetTextArray( inventorySlotsPath, inventorySlots );
		else if ( type == CT_STRING && ConfigGetText( inventorySlotsPath, slotName ) )
			inventorySlots.Insert( slotName );

		return inventorySlots;
	}

	TStringArray COT_GetInventorySlots( EntityAI entity )
	{
		TStringArray inventorySlots = {};

		entity.ConfigGetTextArray( "inventorySlot", inventorySlots );

		if ( inventorySlots.Count() == 0 )
		{
			string slotName = entity.ConfigGetString( "inventorySlot" );
			if ( slotName )
				inventorySlots.Insert( slotName );
		}

		return inventorySlots;
	}
}