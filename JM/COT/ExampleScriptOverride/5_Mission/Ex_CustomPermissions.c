#ifdef JM_CommunityOnlineTools
// Example: gating a control by permission. The key is declared once in a module's
// DeclarePermissions() (see JMCustomExampleModule.c) and bound to the control here:
// BindPermission hides the control when the permission is missing and keeps it
// in step whenever the player's permissions change - no handler code needed.
// It is UI only; the server must still check (see Ex_RPCHandling.c).
modded class JMPlayerForm
{
	protected UIActionButton m_GatedButton;

	override void OnCreate()
	{
		super.OnCreate();

		AddTab( "Perms", JMConstants.Lucide( "lock" ), "BuildPermsTab" );
	}

	void BuildPermsTab( Widget parentPanel )
	{
		Widget body = UIActionManager.CreateSection( parentPanel, "Permission Gated" );
		if ( !body )
			return;

		m_GatedButton = UIActionManager.CreateButton( body, "Needs Admin.Player.InjectedPanel", null, "" );
		m_GatedButton.SetOnClick( this, "OnGatedAction" );
		BindPermission( m_GatedButton, JMConstants.PERM_PLAYER_INJECTED_PANEL );
	}

	void OnGatedAction( UIActionBase action )
	{
		action.AnimateFeedback();
	}
}
#endif
