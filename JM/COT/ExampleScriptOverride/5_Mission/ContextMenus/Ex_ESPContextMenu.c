#ifdef JM_CommunityOnlineTools
// Example: adding entries to the 3D world / tracked object context menu (JMESPActionMenu).
// Uses JMContextMenuRegistry helper for simplified 1-line context menu item registration.
modded class JMESPActionMenu
{
	override void BuildMain()
	{
		super.BuildMain();

		//! Register sub-mod 3D world action item using helper
		JMContextMenuRegistry.Register3DWorldAction( "ex_3d_tp_here", "Sub-Mod Teleport To Me", JMConstants.Lucide( "move-down-left" ), this, "OnExTeleportToMe" );
	}

	void OnExTeleportToMe( string itemId )
	{
		if ( !m_Meta || !m_Meta.target )
			return;

		Man self = g_Game.GetPlayer();

		if ( !self )
			return;

		vector pos = self.GetPosition();
		JMPlayerInstance targetPlayer = TargetPlayer();

		if ( targetPlayer )
		{
			if ( !Perm( JMConstants.PERM_PLAYER_TELEPORT_POSITION ) )
				return;

			JMTeleportModule teleportModule = CF_Modules<JMTeleportModule>.Get();

			if ( teleportModule )
				teleportModule.Position( pos, { targetPlayer.GetGUID() } );

			return;
		}

		if ( !Perm( JMConstants.PERM_ESP_OBJECT_SETPOSITION ) )
			return;

		JMTeleportHistory.PushObject( m_Meta.target );
		m_Meta.module.SetPosition( pos, m_Meta.target );
	}
}
#endif
