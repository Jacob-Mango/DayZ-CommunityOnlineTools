#ifdef JM_CommunityOnlineTools
// Example: registering custom automated server events in JMEventsModule.
modded class JMEventsModule
{
	override void RegisterEvents()
	{
		super.RegisterEvents();

		#ifdef COT_DEBUGLOGS
		Print( "[COT_DBG] Ex_EventRegistration: Registered sub-mod event" );
		#endif
	}
}
#endif
