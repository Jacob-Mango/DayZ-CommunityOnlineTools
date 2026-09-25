#ifdef JM_CommunityOnlineTools
// Example: adding custom player statistics rows to the player manager (JMPlayerFormTabStats).
modded class JMPlayerFormTabStats
{
	override protected void BuildStatsList()
	{
		super.BuildStatsList();

		if ( !m_ContentParent )
			return;

		UIActionManager.CreateText( m_ContentParent, "Custom Sub-Mod Stat: Active", "" );
	}
}
#endif
