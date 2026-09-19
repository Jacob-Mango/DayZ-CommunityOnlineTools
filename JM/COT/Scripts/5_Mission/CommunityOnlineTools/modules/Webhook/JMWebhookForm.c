class JMWebhookForm: JMFormBase
{
	protected static const float           HEADER_HEIGHT = 35;

	protected Widget                       m_Panel;
	protected UIActionScroller             m_Scroller;
	protected Widget                       m_ActionsWrapper;
	protected UIActionText                 m_HeaderTitle;

	protected ref map< string, ref JMWebhookSection > m_Sections;
	protected ref array< string >          m_Types;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMWebhookCOTModule         m_Module;

	protected string                       m_PendingName;

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
		// ----------------------------------------------------------------------
		// Webhook form layout map (400 x 350 px)
		// ----------------------------------------------------------------------
		// HEADER (35 px):   "Webhooks (N)" ????????????? [+ Add Webhook]
		// PANEL  (315 px):  scrollable list of JMWebhookSection blocks.
		// ----------------------------------------------------------------------

		Widget header = layoutRoot.FindAnyWidget( "header_panel" );
		Widget headerRow = UIActionManager.CreateWrapSpacer( header, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_HeaderTitle = UIActionManager.CreateText( headerRow, "#STR_COT_WEBHOOK_HEADER_TITLE" );
		m_HeaderTitle.SetWidth( 0.59 );
		m_HeaderTitle.SetLabelVAlign( UIActionVAlign.CENTER );

		UIActionButton addBtn = UIActionManager.CreateButton( headerRow, "#STR_COT_WEBHOOK_ADD_WEBHOOK", this, "Action_AddWebhook" );
		addBtn.SetWidth( 0.4 );
		addBtn.SetColor( JMTheme.SUCCESS_FILL );
		addBtn.SetTooltip( "Create a new Discord webhook configuration" );

		//! Matches the key RPC_AddConnectionGroup enforces server-side. The
		//! per-webhook controls are gated in JMWebhookSection, which rebuilds
		//! them whenever the settings change.
		BindPermission( addBtn, "Webhook.Manage.URL.Add" );

		// Scrollable content area
		m_Panel         = layoutRoot.FindAnyWidget( "panel" );
		m_Scroller      = UIActionManager.CreateScroller( m_Panel );
		m_ActionsWrapper = m_Scroller.GetContentWidget();

		OnSettingsUpdated();

		m_Scroller.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		// Panel width is relative to the form root, height is in pixels, so only
		// the height has to follow the window: fill everything below the header.
		if ( m_Panel )
			m_Panel.SetSize( 1.0, Math.Max( 0, h - HEADER_HEIGHT ) );

		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}

	//! Every webhook section carries its own unsaved Name/URL/filter edits
	//! until its Save button is clicked. This fires on ANY webhook settings
	//! change - including one admin flipping an event-type checkbox on webhook
	//! A - so tearing down and rebuilding every section from scratch here used
	//! to wipe whatever another admin (or this one) had half-typed into an
	//! unrelated, unsaved section.
	//!
	//! A webhook add/remove/rename changes the set of names m_Sections is
	//! keyed by, and there is no safe "in place" edit for that - those still
	//! fall back to the original full rebuild. Everything else (toggling,
	//! adding or removing an event type; saving a URL/filter edit without
	//! renaming) keeps the same set of names, and is refreshed in place via
	//! JMWebhookSection.UpdateState() instead, leaving every other section's
	//! widgets - and any unsaved edits sitting in them - untouched.
	override void OnSettingsUpdated()
	{
		if ( !m_Module || !m_ActionsWrapper )
			return;

		array< ref JMWebhookConnectionGroup > groups = m_Module.GetConnections();

		if ( m_HeaderTitle )
			m_HeaderTitle.SetLabel( "Webhooks (" + groups.Count() + ")" );

		if ( groups.Count() != m_Sections.Count() )
		{
			RebuildAllSections( groups );
			return;
		}

		for ( int i = 0; i < groups.Count(); i++ )
		{
			JMWebhookConnectionGroup group = groups[i];
			JMWebhookSection section = m_Sections.Get( group.Name );

			if ( !section )
			{
				//! Count matched but this name did not, e.g. a rename - the
				//! set of keys changed even though the count didn't, which
				//! the in-place path cannot express safely.
				RebuildAllSections( groups );
				return;
			}

			section.UpdateState( group, m_Types );
		}
	}

	private void RebuildAllSections( array< ref JMWebhookConnectionGroup > groups )
	{
		m_Sections.Clear();

		Widget child = m_ActionsWrapper.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		if ( groups.Count() == 0 )
		{
			UIActionManager.CreateText( m_ActionsWrapper, "No webhooks configured. Click 'Add Webhook' to create one." );
		}
		else
		{
			for ( int i = 0; i < groups.Count(); i++ )
			{
				JMWebhookConnectionGroup group = groups[i];
				JMWebhookSection section = new JMWebhookSection( m_ActionsWrapper, this, group, m_Types );
				m_Sections.Insert( group.Name, section );
			}
		}

		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}

	// -------------------------------------------------------------------------
	//  Add Webhook - two-step: name then URL
	// -------------------------------------------------------------------------

	void Action_AddWebhook( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Two( JMConfirmationType.EDIT, "Add Webhook", "Enter a name for this webhook:", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "Action_AddWebhook_GotName" );
	}

	void Action_AddWebhook_GotName( JMConfirmation confirmation )
	{
		string name = confirmation.GetEditBoxValue();
		name.Trim();
		if ( name == "" )
			return;

		m_PendingName = name;

		CreateConfirmation_Two( JMConfirmationType.EDIT, "Add Webhook", "Enter the Discord webhook URL for '" + name + "':", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", "Action_AddWebhook_GotURL" );
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

	// ConfirmInline button: fires UIEvent.CHANGE only after the user has
	// confirmed the action through its built-in two-step interaction, so
	// no extra confirmation popup is needed here.
	void Action_RemoveWebhook( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		JMWebhookTypeData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		m_Module.RemoveConnectionGroup( data.Name );
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
		if ( eid != UIEvent.CHANGE )
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
