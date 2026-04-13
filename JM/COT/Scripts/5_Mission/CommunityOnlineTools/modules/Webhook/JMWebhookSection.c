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

	private UIActionEditableText m_NameEdit;
	private UIActionEditableText m_URLEdit;
	private UIActionEditableText m_FilterGUIDEdit;
	private UIActionEditableText m_FilterRoleEdit;
	private Widget             m_TypesWrapper;
	private UIActionDropdownList m_DropDownList;

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
		// Widget lifetime managed by the form — do not unlink here
	}

	private void Build( JMWebhookConnectionGroup group, array< string > allTypes )
	{
		// Label column width and input column start position
		float LABEL_W = 0.22;
		float INPUT_X = 0.22;
		float INPUT_W = 1.0 - INPUT_X;

		m_RootSpacer = UIActionManager.CreateGridSpacer( m_Parent, 1, 1 );

		// ---- Section header: coloured background with webhook name ----------
		UIActionManager.CreatePanel( m_RootSpacer, 0xFF333333, 4 );
		UIActionManager.CreateText( m_RootSpacer, "[ " + group.Name + " ]" );
		UIActionManager.CreatePanel( m_RootSpacer, 0xFF333333, 2 );

		// ---- Name row -------------------------------------------------------
		Widget nameRow = UIActionManager.CreateGridSpacer( m_RootSpacer, 1, 2 );
		UIActionText nameLbl = UIActionManager.CreateText( nameRow, "Name" );
		nameLbl.SetWidth( LABEL_W );
		m_NameEdit = UIActionManager.CreateEditableText( nameRow, "", this );
		m_NameEdit.SetText( group.Name );
		m_NameEdit.SetWidgetWidth( m_NameEdit.GetLabelWidget(), 0.0 );
		m_NameEdit.SetWidgetWidth( m_NameEdit.GetEditBoxWidget(), 1.0 );
		m_NameEdit.SetWidth( INPUT_W );
		m_NameEdit.SetPosition( INPUT_X );

		// ---- URL row --------------------------------------------------------
		Widget urlRow = UIActionManager.CreateGridSpacer( m_RootSpacer, 1, 2 );
		UIActionText urlLbl = UIActionManager.CreateText( urlRow, "URL" );
		urlLbl.SetWidth( LABEL_W );
		m_URLEdit = UIActionManager.CreateEditableText( urlRow, "", this );
		m_URLEdit.SetText( group.Address );
		m_URLEdit.SetWidgetWidth( m_URLEdit.GetLabelWidget(), 0.0 );
		m_URLEdit.SetWidgetWidth( m_URLEdit.GetEditBoxWidget(), 1.0 );
		m_URLEdit.SetWidth( INPUT_W );
		m_URLEdit.SetPosition( INPUT_X );

		// ---- GUID filter row ------------------------------------------------
		Widget guidRow = UIActionManager.CreateGridSpacer( m_RootSpacer, 1, 2 );
		UIActionText guidLbl = UIActionManager.CreateText( guidRow, "GUID Filter" );
		guidLbl.SetWidth( LABEL_W );
		m_FilterGUIDEdit = UIActionManager.CreateEditableText( guidRow, "", this );
		m_FilterGUIDEdit.SetText( group.FilterGUID );
		m_FilterGUIDEdit.SetWidgetWidth( m_FilterGUIDEdit.GetLabelWidget(), 0.0 );
		m_FilterGUIDEdit.SetWidgetWidth( m_FilterGUIDEdit.GetEditBoxWidget(), 1.0 );
		m_FilterGUIDEdit.SetWidth( INPUT_W );
		m_FilterGUIDEdit.SetPosition( INPUT_X );

		// ---- Role filter row ------------------------------------------------
		Widget roleRow = UIActionManager.CreateGridSpacer( m_RootSpacer, 1, 2 );
		UIActionText roleLbl = UIActionManager.CreateText( roleRow, "Role Filter" );
		roleLbl.SetWidth( LABEL_W );
		m_FilterRoleEdit = UIActionManager.CreateEditableText( roleRow, "", this );
		m_FilterRoleEdit.SetText( group.FilterRole );
		m_FilterRoleEdit.SetWidgetWidth( m_FilterRoleEdit.GetLabelWidget(), 0.0 );
		m_FilterRoleEdit.SetWidgetWidth( m_FilterRoleEdit.GetEditBoxWidget(), 1.0 );
		m_FilterRoleEdit.SetWidth( INPUT_W );
		m_FilterRoleEdit.SetPosition( INPUT_X );

		// ---- Save / Remove buttons ------------------------------------------
		// Save is wide (left), Remove is narrow and red (right)
		Widget actionRow = UIActionManager.CreateGridSpacer( m_RootSpacer, 1, 2 );
		UIActionButton saveBtn = UIActionManager.CreateButton( actionRow, "Save", m_Form, "Action_SaveWebhook" );
		saveBtn.SetWidth( 0.72 );
		saveBtn.SetData( new JMWebhookTypeData( group.Name ) );
		UIActionButton removeBtn = UIActionManager.CreateButton( actionRow, "Remove", m_Form, "Action_RemoveWebhook" );
		removeBtn.SetWidth( 0.26 );
		removeBtn.SetPosition( 0.74 );
		removeBtn.SetColor( COLOR_RED );
		removeBtn.SetData( new JMWebhookTypeData( group.Name ) );

		// ---- Event types section --------------------------------------------
		UIActionManager.CreatePanel( m_RootSpacer, 0xFF3A3A3A, 1 );
		UIActionText evtHeader = UIActionManager.CreateText( m_RootSpacer, "Event Types" );
		UIActionManager.CreatePanel( m_RootSpacer, 0xFF3A3A3A, 1 );

		m_TypesWrapper = UIActionManager.CreateGridSpacer( m_RootSpacer, 1, 1 );
		RebuildTypes( group, allTypes );

		// ---- Add type row: dropdown takes most width, Add button on right ---
		Widget addRow = UIActionManager.CreateGridSpacer( m_RootSpacer, 1, 2 );
		array< string > available = GetAvailableTypes( group, allTypes );
		m_DropDownList = UIActionManager.CreateDropdownBox( addRow, m_Form.GetLayoutRoot(), "Add event type", available );
		m_DropDownList.SetWidth( 0.75 );
		UIActionButton addTypeBtn = UIActionManager.CreateButton( addRow, "Add", m_Form, "Action_AddType" );
		addTypeBtn.SetWidth( 0.23 );
		addTypeBtn.SetPosition( 0.77 );
		addTypeBtn.SetData( new JMWebhookTypeData( group.Name ) );

		// ---- Section divider ------------------------------------------------
		UIActionManager.CreatePanel( m_RootSpacer, 0xFF222222, 10 );
	}

	// Not used — JMWebhookForm.OnSettingsUpdated rebuilds sections from scratch.
	// Kept for potential future incremental update use.
	void UpdateState( JMWebhookConnectionGroup group, array< string > allTypes )
	{
	}

	private void RebuildTypes( JMWebhookConnectionGroup group, array< string > allTypes )
	{
		for ( int i = 0; i < group.Count(); i++ )
		{
			JMWebhookConnection conn = group.Get( i );
			Widget typeRow = UIActionManager.CreateGridSpacer( m_TypesWrapper, 1, 2 );

			// Checkbox takes most of the row; "X" button is compact on the right
			UIActionCheckbox cb = UIActionManager.CreateCheckbox( typeRow, conn.Name, m_Form, "Action_TypeState", conn.Enabled );
			cb.SetWidth( 0.84 );
			cb.SetData( new JMWebhookTypeData( conn.Name, group.Name ) );

			UIActionButton removeTypeBtn = UIActionManager.CreateButton( typeRow, "X", m_Form, "Action_RemoveType" );
			removeTypeBtn.SetWidth( 0.14 );
			removeTypeBtn.SetPosition( 0.86 );
			removeTypeBtn.SetColor( COLOR_RED );
			removeTypeBtn.SetData( new JMWebhookTypeData( conn.Name, group.Name ) );
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
