#ifdef JM_CommunityOnlineTools
// Example: injecting custom action shortcut buttons into the COT sidebar footer (JMCOTSideBarFooter).
modded class JMCOTSideBarFooter
{
	override void OnInit()
	{
		super.OnInit();

		if ( !layoutRoot )
			return;

		#ifdef COT_DEBUGLOGS
		Print( "[COT_DBG] Ex_InjectedFooterButton: Initialized" );
		#endif
	}
}
#endif
