class JMWebhookForm: JMFormBase
{
	private UIActionScroller             m_Scroller;
	private Widget                       m_ActionsWrapper;

	private ref map< string, ref JMWebhookSection > m_Sections;
	private ref array< string >          m_Types;

	private JMWebhookCOTModule           m_Module;

	private string                       m_PendingName;

	void JMWebhookForm()
	{
		m_Types    = new array< string >();
		m_Sections = new map< string, ref JMWebhookSection >();

		JMWebhookConstructor.Generate( m_Types );
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		// Header: Add Webhook button — right-aligned, moderate width
		Widget header = layoutRoot.FindAnyWidget( "header_panel" );
		Widget headerSpacer = UIActionManager.CreateGridSpacer( header, 1, 2 );
		// Empty left cell pushes the button to the right side
		UIActionManager.CreateText( headerSpacer, "" );
		UIActionButton addBtn = UIActionManager.CreateButton( headerSpacer, "+ Add Webhook", this, "Action_AddWebhook" );
		addBtn.SetColor( COLOR_GREEN );

		// Scrollable content area
		m_Scroller      = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		m_ActionsWrapper = m_Scroller.GetContentWidget();

		OnSettingsUpdated();

		m_Scroller.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}

	override void OnSettingsUpdated()
	{
		if ( !m_Module || !m_ActionsWrapper )
			return;

		array< ref JMWebhookConnectionGroup > groups = m_Module.GetConnections();

		// Rebuild all sections from scratch to stay in sync with server state
		m_Sections.Clear();

		// Remove existing children by relinking
		Widget child = m_ActionsWrapper.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		for ( int i = 0; i < groups.Count(); i++ )
		{
			JMWebhookConnectionGroup group = groups[i];
			JMWebhookSection section = new JMWebhookSection( m_ActionsWrapper, this, group, m_Types );
			m_Sections.Insert( group.Name, section );
		}

		if ( groups.Count() == 0 )
			UIActionManager.CreateText( m_ActionsWrapper, "No webhooks configured. Click 'Add Webhook' to create one." );

		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  Add Webhook — two-step: name then URL
	// -------------------------------------------------------------------------

	void Action_AddWebhook( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Two( JMConfirmationType.EDIT, "Add Webhook",
			"Enter a name for this webhook:",
			"#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "Action_AddWebhook_GotName" );
	}

	void Action_AddWebhook_GotName( JMConfirmation confirmation )
	{
		string name = confirmation.GetEditBoxValue();
		name.Trim();
		if ( name == "" )
			return;

		m_PendingName = name;

		CreateConfirmation_Two( JMConfirmationType.EDIT, "Add Webhook",
			"Enter the Discord webhook URL for '" + name + "':",
			"#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "Action_AddWebhook_GotURL" );
	}

	void Action_AddWebhook_GotURL( JMConfirmation confirmation )
	{
		string url = confirmation.GetEditBoxValue();
		url.Trim();
		if ( url == "" || m_PendingName == "" )
		{
			m_PendingName = "";
			return;
		}

		m_Module.AddConnectionGroup( m_PendingName, url );
		m_PendingName = "";
	}

	// -------------------------------------------------------------------------
	//  Save (edit name + URL of existing webhook)
	// -------------------------------------------------------------------------

	void Action_SaveWebhook( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMWebhookTypeData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		JMWebhookSection section = m_Sections.Get( data.Name );
		if ( !section )
			return;

		string newName = section.GetEditedName();
		string newURL  = section.GetEditedURL();

		newName.Trim();
		newURL.Trim();

		if ( newName == "" || newURL == "" )
			return;

		string filterGUID = section.GetFilterGUID();
		string filterRole = section.GetFilterRole();
		filterGUID.Trim();
		filterRole.Trim();

		m_Module.EditConnectionGroup( data.Name, newName, newURL, filterGUID, filterRole );
	}

	// -------------------------------------------------------------------------
	//  Remove Webhook
	// -------------------------------------------------------------------------

	void Action_RemoveWebhook( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMWebhookTypeData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_PendingName = data.Name;

		CreateConfirmation_Two( JMConfirmationType.INFO, "Remove Webhook",
			"Remove webhook '" + data.Name + "'? This cannot be undone.",
			"#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_REMOVE", "Action_RemoveWebhook_Confirm" );
	}

	void Action_RemoveWebhook_Confirm( JMConfirmation confirmation )
	{
		if ( m_PendingName == "" )
			return;

		m_Module.RemoveConnectionGroup( m_PendingName );
		m_PendingName = "";
	}

	// -------------------------------------------------------------------------
	//  Add event type to a webhook
	// -------------------------------------------------------------------------

	void Action_AddType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMWebhookTypeData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		JMWebhookSection section = m_Sections.Get( data.Name );
		if ( !section )
			return;

		string typeName = section.GetSelectedType();
		typeName.Trim();

		if ( typeName == "" )
			return;

		m_Module.AddType( typeName, data.Name, true );
	}

	// -------------------------------------------------------------------------
	//  Remove event type
	// -------------------------------------------------------------------------

	void Action_RemoveType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMWebhookTypeData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Module.RemoveType( data.Name, data.Group );
	}

	// -------------------------------------------------------------------------
	//  Toggle event type enabled/disabled
	// -------------------------------------------------------------------------

	void Action_TypeState( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMWebhookTypeData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		UIActionCheckbox cb;
		if ( !Class.CastTo( cb, action ) )
			return;

		m_Module.TypeState( data.Name, data.Group, cb.IsChecked() );
	}

}
