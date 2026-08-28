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
		// Widget lifetime managed by the form - do not unlink here
	}

	private void Build( JMWebhookConnectionGroup group, array< string > allTypes )
	{
		// ----------------------------------------------------------------------
		// Webhook section layout (one block per webhook, stacked vertically)
		// ----------------------------------------------------------------------
		//   ===== Header bar with the webhook name =====
		//   Name        [editable input]   (built by BuildLabeledInput)
		//   URL         [editable input]
		//   GUID Filter [editable input]
		//   Role Filter [editable input]
		//   [delete 32] [Save (green, fills)]
		//   ----- Event Types -----
		//   [delete 32] [checkbox: EventName 1]
		//   [delete 32] [checkbox: EventName 2]
		//   [Add event type 0.75] [Add 0.25]
		//   --- 10 px divider ---
		// ----------------------------------------------------------------------

		m_RootSpacer = UIActionManager.CreateGridSpacer( m_Parent, 1, 1 );

		// ---- Section header: coloured background with webhook name + Remove -
		UIActionManager.CreatePanel( m_RootSpacer, JMTheme.DIVIDER_MEDIUM, 4 );

		// Header row: text + spacer + Remove on the same line.
		Widget headerRow = UIActionManager.CreateWrapSpacer( m_RootSpacer, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionText nameTxt = UIActionManager.CreateText( headerRow, "[ " + group.Name + " ]" );
		nameTxt.SetWidth( 1.0 - 0.12 );

		UIActionConfirmInline removeBtn = UIActionManager.CreateConfirmInline( headerRow, "", m_Form, "Action_RemoveWebhook" );
		UIActionIconGrid.ApplyDeletePreset( removeBtn );
		removeBtn.SetButton( "" );
		removeBtn.SetFixedSize( 32, 32 );
		removeBtn.CenterIcon( 32, 16 );
		removeBtn.SetConfirmLabel( "O" );
		removeBtn.SetCancelLabel( "X" );
		removeBtn.SetData( new JMWebhookTypeData( group.Name ) );
		removeBtn.SetTooltip( "Remove this webhook" );

		UIActionManager.CreatePanel( m_RootSpacer, JMTheme.DIVIDER_MEDIUM, 2 );

		// Build the four label+input rows uniformly
		m_NameEdit       = BuildLabeledInput( "Name",        group.Name );
		m_URLEdit        = BuildLabeledInput( "URL",         group.Address );
		m_FilterGUIDEdit = BuildLabeledInput( "GUID Filter", group.FilterGUID );
		m_FilterRoleEdit = BuildLabeledInput( "Role Filter", group.FilterRole );

		// ---- Save button (full-width) ---------------------------------------
		Widget saveRow = UIActionManager.CreateWrapSpacer( m_RootSpacer, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		UIActionButton saveBtn = UIActionManager.CreateButton( saveRow, "Save", m_Form, "Action_SaveWebhook" );
		saveBtn.SetWidth( 1.0 );
		saveBtn.SetColor( JMTheme.SUCCESS_FILL );
		saveBtn.SetData( new JMWebhookTypeData( group.Name ) );
		saveBtn.SetTooltip( "Save changes to this webhook" );

		// ---- Event types section --------------------------------------------
		UIActionManager.CreatePanel( m_RootSpacer, 0xFF3A3A3A, 1 );
		UIActionManager.CreateText( m_RootSpacer, "Event Types" );
		UIActionManager.CreatePanel( m_RootSpacer, 0xFF3A3A3A, 1 );

		m_TypesWrapper = UIActionManager.CreateGridSpacer( m_RootSpacer, 1, 1 );
		if ( group.Count() == 0 )
			UIActionManager.CreateText( m_TypesWrapper, "(no event types yet - pick one below)" );
		else
			RebuildTypes( group, allTypes );

		// ---- Add type row: dropdown fills, Add button right -----------------
		array< string > available = GetAvailableTypes( group, allTypes );
		Widget addRow = UIActionManager.CreateWrapSpacer( m_RootSpacer, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_DropDownList = UIActionManager.CreateDropdownBox( addRow, m_Form.GetLayoutRoot(), "Add event type", available );
		m_DropDownList.SetWidth( 0.74 );

		UIActionButton addTypeBtn = UIActionManager.CreateButton( addRow, "Add", m_Form, "Action_AddType" );
		addTypeBtn.SetWidth( 0.25 );
		addTypeBtn.SetColor( JMTheme.SUCCESS_FILL );
		addTypeBtn.SetData( new JMWebhookTypeData( group.Name ) );
		addTypeBtn.SetTooltip( "Add the selected event type to this webhook" );

		if ( available.Count() == 0 )
		{
			m_DropDownList.Disable();
			addTypeBtn.Disable();
		}

		// ---- Section divider ------------------------------------------------
		UIActionManager.CreatePanel( m_RootSpacer, 0xFF222222, 10 );
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

	// Not used - JMWebhookForm.OnSettingsUpdated rebuilds sections from scratch.
	// Kept for potential future incremental update use.
	void UpdateState( JMWebhookConnectionGroup group, array< string > allTypes )
	{
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
