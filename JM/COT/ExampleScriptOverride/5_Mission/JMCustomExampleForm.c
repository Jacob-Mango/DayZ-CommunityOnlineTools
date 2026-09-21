#ifdef JM_CommunityOnlineTools
// Example: the form behind JMCustomExampleModule. The layout's `scriptclass` picks this class,
// SetModule() receives the module, and the base class runs the tab lifecycle:
// OnTabCreate (once) -> OnTabFocus -> OnTabUpdate, OnTabUnfocus when another tab is picked,
// OnTabDestruct when the form goes away. The strip callback just forwards to HandleTabChange().
class JMCustomExampleForm: JMFormBase
{
	// Tab indices come from the strip (AddTab returns them), never written as numbers: another
	// mod may append tabs to a form, so no tab can assume which index it will get.
	protected int m_TabIdHome;
	protected int m_TabIdMore;
	protected JMCustomExampleModule m_Module;
	protected UIActionTabs m_Tabs;
	protected ref JMCustomExampleMoreTab m_MoreTab;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	// The one hook that makes GetActiveTabIndex(), AddTab() and HandleTabChange() work.
	protected override COT_ScriptedWidgetEventHandler GetTabStrip()
	{
		return m_Tabs;
	}

	override void OnCreate()
	{
		Widget root = layoutRoot.FindAnyWidget( "panel" );

		m_Tabs = UIActionManager.CreateTabStrip( root, this, "OnChange_Tab" );
		m_TabIdHome = m_Tabs.AddTab( "Home", "", UIActionManager.CreateGridSpacer( root, 4, 1 ) );
		m_TabIdMore = m_Tabs.AddTab( "More", "", UIActionManager.CreateGridSpacer( root, 4, 1 ) );

		// The More tab is a JMFormTab: registered against the index the strip gave it, after which it
		// reports that index itself (m_MoreTab.GetTabId()).
		m_MoreTab = new JMCustomExampleMoreTab();
		RegisterTab( m_TabIdMore, m_MoreTab );

		DeclareTabs( 2 );
		m_Tabs.SetSelection( m_TabIdHome, false );
		InitTabFocus( m_TabIdHome );
	}

	void OnChange_Tab( UIEvent eid, UIActionBase action )
	{
		if ( eid == UIEvent.CHANGE )
			HandleTabChange();
	}

	// The Home tab has no class of its own, so the form builds it; other tabs reach their controller.
	override protected void OnTabCreate( int tab, Widget panel )
	{
		if ( tab == m_TabIdHome )
			BuildHome( panel );
		else
			super.OnTabCreate( tab, panel );
	}

	protected void BuildHome( Widget parent )
	{
		Widget body = UIActionManager.CreateSection( parent, "Custom Module" );
		if ( !body )
			return;

		UIActionManager.CreateText( body, "Status:", "Built lazily on first visit." );

		UIActionButton send = UIActionManager.CreateButton( body, "Send RPC to server", null, "" );
		send.SetOnClick( this, "OnSendRPC" );
		BindPermission( send, JMConstants.PERM_RPC_EXECUTE );

		UIActionManager.CreateButton( body, "Toggle More tab", null, "" ).SetOnClick( this, "OnToggleMoreTab" );
	}

	// SetTabEnabled greys the tab out in the strip (and tells its controller).
	void OnToggleMoreTab( UIActionBase action )
	{
		SetTabEnabled( m_TabIdMore, !IsTabEnabled( m_TabIdMore ) );
	}

	// Handler of SetOnClick: fires for CLICK only, so no `if ( eid != UIEvent.CLICK )` guard.
	void OnSendRPC( UIActionBase action )
	{
		JMCustomExampleRPCModule rpc;
		if ( Class.CastTo( rpc, GetModuleManager().GetModule( JMCustomExampleRPCModule ) ) )
			rpc.SendSubModActionToServer( "target-guid", 1 );

		action.AnimateFeedback();
	}
}
#endif
