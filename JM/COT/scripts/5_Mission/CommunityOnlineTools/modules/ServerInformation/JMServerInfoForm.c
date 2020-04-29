class JMServerInfoForm extends JMFormBase
{
	private Widget m_ActionsWrapper;

	private ref UIActionText m_UIServerFPS;
	private ref UIActionText m_UIOnlinePlayers;
	private ref UIActionText m_UIServerUpTime;
	private ref UIActionText m_UIMemoryUsed;
	private ref UIActionText m_UINetworkOut;
	private ref UIActionText m_UINetworkIn;
	private ref UIActionText m_UIActiveAI;
	private ref UIActionText m_UIGameTickTime;
	private ref UIActionText m_UIMissionTickTime;

	private JMServerInfoModule m_Module;

	void JMServerInfoForm()
	{
		JMServerInfoModule.DATA_UPDATED.Insert( UpdateData );
	}

	void ~JMServerInfoForm()
	{
		JMServerInfoModule.DATA_UPDATED.Remove( UpdateData );
	}

	protected override bool SetModule( ref JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_ActionsWrapper	= UIActionManager.CreateGridSpacer( layoutRoot.FindAnyWidget( "actions_wrapper" ), 4, 1 );

		m_UIServerFPS 		= UIActionManager.CreateText( m_ActionsWrapper, "Server FPS: ", "" );
		m_UIOnlinePlayers 	= UIActionManager.CreateText( m_ActionsWrapper, "Online Players: ", "" );
		m_UIServerUpTime 	= UIActionManager.CreateText( m_ActionsWrapper, "Server Up Time: ", "" );
		m_UIMemoryUsed 		= UIActionManager.CreateText( m_ActionsWrapper, "Memory Used: ", "" );
		m_UINetworkOut 		= UIActionManager.CreateText( m_ActionsWrapper, "Network Out: ", "" );
		m_UINetworkIn 		= UIActionManager.CreateText( m_ActionsWrapper, "Network In: ", "" );
		m_UIActiveAI 		= UIActionManager.CreateText( m_ActionsWrapper, "Active AI: ", "" );
		m_UIGameTickTime	= UIActionManager.CreateText( m_ActionsWrapper, "Game Tick Time: ", "" );
		m_UIMissionTickTime = UIActionManager.CreateText( m_ActionsWrapper, "Mission Tick Time: ", "" );
	}

	void UpdateData( ref JMServerInfoData data )
	{
		m_UIServerFPS.SetText( "" + data.ServerFPS );
		m_UIOnlinePlayers.SetText( "" + data.OnlinePlayers );
		m_UIServerUpTime.SetText( "" + data.ServerUpTime );
		m_UIMemoryUsed.SetText( "" + data.MemoryUsed );
		m_UINetworkOut.SetText( "" + data.NetworkOut );
		m_UINetworkIn.SetText( "" + data.NetworkIn );
		m_UIActiveAI.SetText( "" + data.ActiveAI );
		m_UIGameTickTime.SetText( "" + data.GameTickTime );
		m_UIMissionTickTime.SetText( "" + data.MissionTickTime );
	}
}