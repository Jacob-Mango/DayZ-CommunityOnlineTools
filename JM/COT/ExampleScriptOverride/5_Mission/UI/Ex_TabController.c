#ifdef JM_CommunityOnlineTools
// Example: a tab as its own class. Extend JMFormTab and hand it to the form - the form assigns
// the tab its index at runtime (tab.GetTabId()), so the class never names a number and cannot
// clash with tabs other mods add to the same form. The form then drives its lifecycle:
// OnCreate once, then OnFocus / OnUpdate / OnUnfocus as the user moves around,
// OnResize on resize and OnDestruct when the form goes away.
class JMCustomExampleMoreTab: JMFormTab
{
	protected UIActionText m_Info;
	protected int m_Updates;

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		Widget body = UIActionManager.CreateSection( panel, "More" );
		if ( !body )
			return;

		m_Info = UIActionManager.CreateText( body, "Tab 2:", "Created once, on first visit." );
		UIActionManager.CreateButton( body, "Update focused tab", null, "" ).SetOnClick( this, "OnUpdateClicked" );
	}

	// UpdateActiveTab() reaches OnUpdate of whichever tab is focused - a timer or an RPC answer can
	// call it without checking which tab is showing. UpdateTab( TAB_HOME ) from here would do nothing.
	void OnUpdateClicked( UIActionBase action )
	{
		m_TabForm.UpdateActiveTab();
	}

	override void OnFocus()
	{
		// Start whatever should only run while this tab is on screen.
	}

	// Only ever called while this tab is focused - a form timer can call
	// UpdateActiveTab() without checking which tab is showing.
	override void OnUpdate()
	{
		m_Updates++;

		if ( m_Info )
			m_Info.SetText( "Updated " + m_Updates + " time(s) while focused." );
	}

	override void OnUnfocus()
	{
		// Hide hover panels, stop timers.
	}

	override void OnDestruct()
	{
		// Release anything the widgets do not own.
	}
}

// A mod adding a controller tab to an EXISTING form: AddTab( ..., controller ) appends it to the strip and
// registers it. Nothing here knows (or needs to know) which index it ends up with.
modded class JMPlayerForm
{
	protected ref JMCustomExampleMoreTab m_ExControllerTab;

	override void OnCreate()
	{
		super.OnCreate();

		m_ExControllerTab = new JMCustomExampleMoreTab();
		AddTab( "Controller", JMConstants.Lucide( "layers" ), "", null, m_ExControllerTab );
	}
}
#endif
