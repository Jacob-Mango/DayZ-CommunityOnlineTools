#ifdef JM_CommunityOnlineTools
// Example: Registering custom permissions and binding them to UI controls
modded class JMPlayerForm
{
	protected UIActionButton m_GatedButton;

	override void OnInit()
	{
		super.OnInit();

		// Register custom permission using JMConstants constant string
		GetPermissionsManager().RegisterPermission( JMConstants.PERM_PLAYER_INJECTED_PANEL );
	}

	protected void InitCustomGatedControl( Widget parent )
	{
		m_GatedButton = UIActionManager.CreateButton( parent, "Permission Gated Action", this, "OnClick_GatedAction" );

		// Declarative permission binding: automates enabled/disabled state updates when permissions refresh
		RegisterPermission( m_GatedButton, JMConstants.PERM_PLAYER_INJECTED_PANEL );
	}

	void OnClick_GatedAction( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		// Always guard action execution with permission check
		if ( !GetPermissionsManager().HasPermission( JMConstants.PERM_PLAYER_INJECTED_PANEL ) )
			return;

		action.AnimateFeedback();
	}
}
#endif
