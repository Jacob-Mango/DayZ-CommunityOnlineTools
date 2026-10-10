#ifdef JM_CommunityOnlineTools
// Example: injecting custom action shortcut buttons into the COT sidebar footer (JMCOTSideBarFooter).
modded class JMCOTSideBarFooter
{
	override void Init( Widget footer )
	{
		super.Init( footer );

		if ( !m_Root )
			return;

		#ifdef COT_DEBUGLOGS
		Print( "[COT_DBG] Ex_InjectedFooterButton: Initialized" );
		#endif
	}
}
#endif
