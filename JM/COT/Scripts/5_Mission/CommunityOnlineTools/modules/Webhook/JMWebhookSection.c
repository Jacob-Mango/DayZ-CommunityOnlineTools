class JMWebhookTypeData : UIActionData
{
	string Name;
	string Group;

	void JMWebhookTypeData( string name, string group = "" )
	{
		Name  = name;
		Group = group;
	}
}

// =============================================================================
//  One webhook group (name + URL + event type list)
// =============================================================================

class JMWebhookSection : Managed
{
	private string             m_Name;

	private UIActionCard       m_Card;
	private UIActionEditableText m_NameEdit;
	private UIActionEditableText m_URLEdit;
	private UIActionEditableText m_FilterGUIDEdit;
	private UIActionEditableText m_FilterRoleEdit;
	private Widget             m_TypesWrapper;
	private UIActionDropdownList m_DropDownList;
	private UIActionButton     m_AddTypeBtn;

	private JMWebhookForm      m_Form;
	private Widget             m_Parent;
	private Widget             m_RootSpacer;

	void JMWebhookSection( Widget parent, JMWebhookForm form, JMWebhookConnectionGroup group, array< string > allTypes )
	{
		m_Form   = form;
		m_Parent = parent;
		m_Name   = group.Name;

		Build( group, allTypes );
	}

	void ~JMWebhookSection()
	{
		// Widget lifetime managed by the form - do not unlink here
	}

	private void Build( JMWebhookConnectionGroup group, array< string > allTypes )
	{
		m_Card = UIActionManager.CreateCard( m_Parent, "[ " + group.Name + " ]" );
		UIActionImageButton removeBtn = m_Card.AddDeleteButton( m_Form, "Action_RemoveWebhook", "Remove this webhook" );
		removeBtn.SetData( new JMWebhookTypeData( group.Name ) );

		m_RootSpacer = m_Card.GetContent();

		m_NameEdit       = BuildLabeledInput( "Name",        group.Name );
		m_URLEdit        = BuildLabeledInput( "URL",         group.Address );
		m_FilterGUIDEdit = BuildLabeledInput( "GUID Filter", group.FilterGUID );
		m_FilterRoleEdit = BuildLabeledInput( "Role Filter", group.FilterRole );

		Widget saveRow = UIActionManager.CreateWrapSpacer( m_RootSpacer, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionButton saveBtn = UIActionManager.CreateButton( saveRow, "#STR_COT_WEBHOOK_SAVE", m_Form, "Action_SaveWebhook" );
		saveBtn.SetWidth( 1.0 );
		saveBtn.SetColor( JMTheme.SUCCESS_FILL );
		saveBtn.SetData( new JMWebhookTypeData( group.Name ) );
		saveBtn.SetTooltip( "Save changes to this webhook" );

		UIActionManager.CreatePanel( m_RootSpacer, 0xFF3A3A3A, 1 );
		UIActionManager.CreateText( m_RootSpacer, "#STR_COT_WEBHOOK_EVENT_TYPES" );
		UIActionManager.CreatePanel( m_RootSpacer, 0xFF3A3A3A, 1 );

		m_TypesWrapper = UIActionManager.CreateGridSpacer( m_RootSpacer, 1, 1 );
		if ( group.Count() == 0 )
			UIActionManager.CreateText( m_TypesWrapper, "#STR_COT_WEBHOOK_NO_EVENT_TYPES" );
		else
			RebuildTypes( group, allTypes );

		array< string > available = GetAvailableTypes( group, allTypes );
		Widget addRow = UIActionManager.CreateWrapSpacer( m_RootSpacer, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_DropDownList = UIActionManager.CreateDropdownBox( addRow, m_Form.GetLayoutRoot(), "Add event type", available );
		m_DropDownList.SetWidth( 0.74 );
		if ( m_Form )
			m_Form.RegisterOverlay( m_DropDownList );

		m_AddTypeBtn = UIActionManager.CreateButton( addRow, "#STR_COT_WEBHOOK_ADD_TYPE", m_Form, "Action_AddType" );
		m_AddTypeBtn.SetWidth( 0.25 );
		m_AddTypeBtn.SetColor( JMTheme.SUCCESS_FILL );
		m_AddTypeBtn.SetData( new JMWebhookTypeData( group.Name ) );
		m_AddTypeBtn.SetTooltip( "Add the selected event type to this webhook" );

		//! Sections are torn down and rebuilt whenever settings change, so bind
		//! per-call instead of registering bindings to widgets that will not
		//! outlive the next rebuild. Keys match what JMWebhookCOTModule's RPC
		//! handlers enforce.
		m_Form.UpdatePermission( removeBtn,       "Webhook.Manage.URL.Remove" );
		m_Form.UpdatePermission( saveBtn,         "Webhook.Manage.URL.Edit" );
		m_Form.UpdatePermission( m_NameEdit,      "Webhook.Manage.URL.Edit" );
		m_Form.UpdatePermission( m_URLEdit,       "Webhook.Manage.URL.Edit" );
		m_Form.UpdatePermission( m_FilterGUIDEdit, "Webhook.Manage.URL.Edit" );
		m_Form.UpdatePermission( m_FilterRoleEdit, "Webhook.Manage.URL.Edit" );
		m_Form.UpdatePermission( m_AddTypeBtn,    "Webhook.Manage.Type.Add" );

		//! Left last: an empty type list disables Add regardless of permission.
		if ( available.Count() == 0 )
		{
			m_DropDownList.Disable();
			m_AddTypeBtn.Disable();
		}
	}

	private UIActionEditableText BuildLabeledInput( string label, string value )
	{
		Widget row = UIActionManager.CreateWrapSpacer( m_RootSpacer, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		UIActionText lbl = UIActionManager.CreateText( row, label );
		lbl.SetWidth( 0.22 );
		lbl.SetLabelVAlign( UIActionVAlign.CENTER );

		UIActionEditableText edit = UIActionManager.CreateEditableText( row, "", this );
		edit.SetText( value );
		edit.SetWidgetWidth( edit.GetLabelWidget(), 0.0 );
		edit.SetWidgetWidth( edit.GetEditBoxWidget(), 1.0 );
		edit.SetWidth( 0.77 );

		return edit;
	}

	//! Refreshes everything server-derived (event types, the "add type"
	//! dropdown, the card title) without touching m_NameEdit / m_URLEdit /
	//! m_FilterGUIDEdit / m_FilterRoleEdit.
	//!
	//! Those four are plain text boxes the admin edits locally and commits
	//! with the Save button - JMWebhookForm.OnSettingsUpdated() used to tear
	//! down and rebuild every section from scratch on ANY webhook settings
	//! change, so toggling one event type checkbox on webhook A wiped
	//! whatever an admin had half-typed into webhook B's Name/URL fields (or
	//! their own, if it hadn't been saved yet). Called instead of a full
	//! rebuild whenever the section already exists for this group's name.
	void UpdateState( JMWebhookConnectionGroup group, array< string > allTypes )
	{
		m_Name = group.Name;

		if ( m_Card )
			m_Card.SetLabel( "[ " + group.Name + " ]" );

		if ( !m_TypesWrapper )
			return;

		Widget child = m_TypesWrapper.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}

		if ( group.Count() == 0 )
			UIActionManager.CreateText( m_TypesWrapper, "#STR_COT_WEBHOOK_NO_EVENT_TYPES" );
		else
			RebuildTypes( group, allTypes );

		array< string > available = GetAvailableTypes( group, allTypes );

		if ( m_DropDownList )
		{
			m_DropDownList.SetItems( available );

			if ( available.Count() == 0 )
				m_DropDownList.Disable();
			else
				m_DropDownList.Enable();
		}

		if ( m_AddTypeBtn )
		{
			if ( available.Count() == 0 )
				m_AddTypeBtn.Disable();
			else
				m_AddTypeBtn.Enable();
		}
	}

	private void RebuildTypes( JMWebhookConnectionGroup group, array< string > allTypes )
	{
		for ( int i = 0; i < group.Count(); i++ )
		{
			JMWebhookConnection conn = group.Get( i );
			Widget typeRow = UIActionManager.CreateWrapSpacer( m_TypesWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

			UIActionConfirmInline removeTypeBtn = UIActionManager.CreateConfirmInline( typeRow, "", m_Form, "Action_RemoveType" );
			UIActionIconGrid.ApplyDeletePreset( removeTypeBtn );
			removeTypeBtn.SetButton( "" );
			removeTypeBtn.SetFixedSize( 32, 32 );
			removeTypeBtn.CenterIcon( 32, 16 );
			removeTypeBtn.SetConfirmLabel( "O" );
			removeTypeBtn.SetCancelLabel( "X" );
			removeTypeBtn.SetData( new JMWebhookTypeData( conn.Name, group.Name ) );
			removeTypeBtn.SetTooltip( "Remove this event type from the webhook" );

			UIActionCheckbox cb = UIActionManager.CreateCheckbox( typeRow, conn.Name, m_Form, "Action_TypeState", conn.Enabled );
			cb.SetWidth( 1.0 );

			m_Form.UpdatePermission( removeTypeBtn, "Webhook.Manage.Type.Remove" );
			m_Form.UpdatePermission( cb,            "Webhook.Manage.Type.State" );
			cb.SetData( new JMWebhookTypeData( conn.Name, group.Name ) );
		}
	}

	private array< string > GetAvailableTypes( JMWebhookConnectionGroup group, array< string > allTypes )
	{
		array< string > available = new array< string >;
		for ( int i = 0; i < allTypes.Count(); i++ )
		{
			bool already = false;
			for ( int j = 0; j < group.Count(); j++ )
			{
				if ( group.Get( j ).Name == allTypes[i] )
				{
					already = true;
					break;
				}
			}
			if ( !already )
				available.Insert( allTypes[i] );
		}
		return available;
	}

	string GetName()
	{
		return m_Name;
	}

	string GetEditedName()
	{
		if ( m_NameEdit )
			return m_NameEdit.GetText();
		return m_Name;
	}

	string GetEditedURL()
	{
		if ( m_URLEdit )
			return m_URLEdit.GetText();
		return "";
	}

	string GetSelectedType()
	{
		if ( m_DropDownList )
			return m_DropDownList.GetText();
		return "";
	}

	string GetFilterGUID()
	{
		if ( m_FilterGUIDEdit )
			return m_FilterGUIDEdit.GetText();
		return "";
	}

	string GetFilterRole()
	{
		if ( m_FilterRoleEdit )
			return m_FilterRoleEdit.GetText();
		return "";
	}
}
