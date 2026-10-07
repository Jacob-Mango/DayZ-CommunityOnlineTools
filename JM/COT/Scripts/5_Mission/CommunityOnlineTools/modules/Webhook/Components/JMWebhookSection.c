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
//
//  The detail pane of JMWebhookForm. Only the webhook selected in the form's
//  roster is built, as two cards:
//
//    Connection   [save] [delete]   name, URL, GUID filter, role filter
//    Event Types  (enabled / total) add row, then one row per event type
// =============================================================================

class JMWebhookSection : Managed
{
	//! Row splits, as FRACTIONS of the row - see JMObjectSpawnerForm for why a
	//! fixed-pixel control beside a fractional one is avoided.
	static const float LABEL_W    = 0.22;
	static const float FIELD_W    = 0.77;
	static const float ADD_LIST_W = 0.74;
	static const float ADD_BTN_W  = 0.25;
	static const float TYPE_ROW_W = 0.86;

	protected string             m_Name;
	protected UIActionCard       m_Card;
	protected UIActionEditableText m_NameEdit;
	protected UIActionEditableText m_URLEdit;
	//! DEPRECATED - the filters are searchable dropdowns now
	//! (m_FilterGUIDList / m_FilterRoleList). Never created; kept for sub-mods.
	protected UIActionEditableText m_FilterGUIDEdit;
	protected UIActionEditableText m_FilterRoleEdit;

	//! Searchable dropdowns: type to filter, or pick a known player / role.
	protected UIActionDropdownList m_FilterGUIDList;
	protected UIActionDropdownList m_FilterRoleList;
	protected Widget             m_TypesWrapper;
	protected UIActionDropdownList m_DropDownList;
	protected UIActionButton     m_AddTypeBtn;
	protected JMWebhookForm      m_Form;
	protected Widget             m_Parent;
	protected Widget             m_RootSpacer;

	//! The second card - event types. Its title carries the enabled count.
	protected UIActionCard       m_TypesCard;

	// ---- Type tree --------------------------------------------------------------
	//! Type names the tree was built from, in order - see HasSameTypes.
	protected ref TStringArray                          m_TypeOrder;
	//! Known types no other type is a prefix of: the module names.
	protected ref TStringArray                          m_Roots;
	protected ref map< string, UIActionToggleSwitch >   m_TypeToggles;
	protected ref map< string, UIActionButton >         m_GroupHeaders;
	protected ref map< string, UIActionFoldPanel >      m_GroupFolds;
	//! Fold state per module, kept across rebuilds of the tree. Shut by default.
	protected ref map< string, bool >                   m_Expanded;

	void JMWebhookSection( Widget parent, JMWebhookForm form, JMWebhookConnectionGroup group, array< string > allTypes )
	{
		m_Form   = form;
		m_Parent = parent;
		m_Name   = group.Name;

		m_TypeOrder    = new TStringArray;
		m_Roots        = new TStringArray;
		m_TypeToggles  = new map< string, UIActionToggleSwitch >;
		m_GroupHeaders = new map< string, UIActionButton >;
		m_GroupFolds   = new map< string, UIActionFoldPanel >;
		m_Expanded     = new map< string, bool >;

		Build( group, allTypes );
	}

	void ~JMWebhookSection()
	{
		// Widget lifetime managed by the form - do not unlink here
	}

	protected array< string > GetAvailableTypes( JMWebhookConnectionGroup group, array< string > allTypes )
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

	//! "Event Types (enabled/total)" for the second card's title.
	static string GetTypesTitle( JMWebhookConnectionGroup group )
	{
		int enabled = 0;
		for ( int i = 0; i < group.Count(); i++ )
		{
			if ( group.Get( i ).Enabled )
				enabled++;
		}

		return Widget.TranslateString( "#STR_COT_WEBHOOK_EVENT_TYPES" ) + " (" + enabled + "/" + group.Count() + ")";
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

	//! The GUID to filter on. A picked suggestion reads "Name | SteamID | GUID",
	//! so the GUID is its last field; typed-in text is taken as a GUID as is.
	string GetFilterGUID()
	{
		if ( m_FilterGUIDList )
			return ParseGUID( m_FilterGUIDList.GetText() );
		if ( m_FilterGUIDEdit )
			return m_FilterGUIDEdit.GetText();
		return "";
	}

	string GetFilterRole()
	{
		if ( m_FilterRoleList )
			return m_FilterRoleList.GetText();
		if ( m_FilterRoleEdit )
			return m_FilterRoleEdit.GetText();
		return "";
	}

	//! Separator between the fields of a player suggestion.
	static const string PLAYER_FIELD_SEP = " | ";

	static string FormatPlayer( string name, string steamID, string guid )
	{
		return name + PLAYER_FIELD_SEP + steamID + PLAYER_FIELD_SEP + guid;
	}

	static string ParseGUID( string text )
	{
		text.Trim();

		string fieldSep = PLAYER_FIELD_SEP;
		int sep = text.LastIndexOf( fieldSep );
		if ( sep < 0 )
			return text;

		int start = sep + fieldSep.Length();
		string guid = text.Substring( start, text.Length() - start );
		guid.Trim();
		return guid;
	}

	//! Every player this client knows of - online players, from the
	//! permission manager - as "Name | SteamID | GUID", so the dropdown's
	//! filter matches any of the three. `currentGUID` is resolved to its
	//! entry (or left as the bare GUID for a player not online).
	static array< string > GetPlayerSuggestions( string currentGUID, out string currentText )
	{
		array< string > items = new array< string >;
		currentText = currentGUID;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();
		foreach ( JMPlayerInstance player : players )
		{
			if ( !player )
				continue;

			string entry = FormatPlayer( player.GetName(), player.GetSteam64ID(), player.GetGUID() );
			items.Insert( entry );

			if ( currentGUID != "" && player.GetGUID() == currentGUID )
				currentText = entry;
		}

		return items;
	}

	//! Every role this client knows of: the permission manager's role list,
	//! plus any role an online player holds that is not in it.
	static array< string > GetRoleSuggestions()
	{
		array< string > items = new array< string >;

		map< string, ref JMRole > roles = GetPermissionsManager().Roles;
		foreach ( string roleName, JMRole role : roles )
		{
			if ( items.Find( roleName ) < 0 )
				items.Insert( roleName );
		}

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();
		foreach ( JMPlayerInstance player : players )
		{
			if ( !player )
				continue;

			array< string > playerRoles = player.GetRoles();
			foreach ( string playerRole : playerRoles )
			{
				if ( items.Find( playerRole ) < 0 )
					items.Insert( playerRole );
			}
		}

		items.Sort();
		return items;
	}

	string GetName()
	{
		return m_Name;
	}

	string GetSelectedType()
	{
		if ( m_DropDownList )
			return m_DropDownList.GetText();
		return "";
	}

	protected void Build( JMWebhookConnectionGroup group, array< string > allTypes )
	{
		// ---- Connection card ------------------------------------------------
		//! Save and delete live in the card's title bar rather than in a row of
		//! their own under the fields. Delete is a confirm icon: it raises
		//! CHANGE only once confirmed, which is what Action_RemoveWebhook waits
		//! for - a plain image button raises CLICK and never got through.
		m_Card = UIActionManager.CreateCard( m_Parent, group.Name );

		UIActionImageButton saveBtn = m_Card.AddSaveButton( m_Form, "Action_SaveWebhook", "#STR_COT_WEBHOOK_SAVE_CHANGES_TO_THIS_WEBHOOK" );
		saveBtn.SetData( new JMWebhookTypeData( group.Name ) );

		UIActionConfirmInline removeBtn = UIActionManager.CreateDeleteConfirmIcon( m_Card.GetHeaderActions(), m_Form, "Action_RemoveWebhook" );
		removeBtn.SetFixedSize( JMFormBase.HEADER_ACTION_PX, JMFormBase.HEADER_ACTION_PX );
		removeBtn.CenterIcon( JMFormBase.HEADER_ACTION_PX, 16 );
		removeBtn.SetData( new JMWebhookTypeData( group.Name ) );
		removeBtn.SetTooltip( "Remove this webhook" );

		m_RootSpacer = m_Card.GetContent();

		m_NameEdit       = BuildLabeledInput( "Name",        group.Name );
		m_URLEdit        = BuildLabeledInput( "URL",         group.Address );

		string guidText;
		array< string > players = GetPlayerSuggestions( group.FilterGUID, guidText );
		m_FilterGUIDList = BuildLabeledDropdown( "GUID Filter", guidText, players, "Search name / SteamID / GUID..." );

		array< string > roles = GetRoleSuggestions();
		m_FilterRoleList = BuildLabeledDropdown( "Role Filter", group.FilterRole, roles, "Search role..." );

		// ---- Event types card -----------------------------------------------
		m_TypesCard = UIActionManager.CreateCard( m_Parent, GetTypesTitle( group ) );
		Widget typesBody = m_TypesCard.GetContent();

		//! The add row sits ABOVE the list: it stays in the same place however
		//! many types the webhook already has.
		array< string > available = GetAvailableTypes( group, allTypes );
		Widget addRow = UIActionManager.CreateWrapSpacerCompact( typesBody, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_DropDownList = UIActionManager.CreateDropdownBox( addRow, m_Form.GetLayoutRoot(), "", available );
		m_DropDownList.SetPlaceholder( "Add event type..." );
		m_DropDownList.SetWidth( ADD_LIST_W );
		if ( m_Form )
			m_Form.AddOverlay( m_DropDownList );

		m_AddTypeBtn = UIActionManager.CreateButton( addRow, "#STR_COT_WEBHOOK_ADD_TYPE", m_Form, "Action_AddType" );
		m_AddTypeBtn.SetWidth( ADD_BTN_W );
		m_AddTypeBtn.SetColor( JMTheme.SUCCESS_FILL );
		m_AddTypeBtn.SetData( new JMWebhookTypeData( group.Name ) );
		m_AddTypeBtn.SetTooltip( "#STR_COT_WEBHOOK_ADD_THE_SELECTED_EVENT_TYPE_TO" );

		UIActionManager.CreateDivider( typesBody, JMTheme.DIVIDER_DARK, 1 );

		m_TypesWrapper = UIActionManager.CreateGridSpacer( typesBody, 1, 1 );
		if ( group.Count() == 0 )
			UIActionManager.CreateText( m_TypesWrapper, "#STR_COT_WEBHOOK_NO_EVENT_TYPES" );
		else
			RebuildTypes( group, allTypes );

		//! Sections are torn down and rebuilt whenever settings change, so bind
		//! per-call instead of registering bindings to widgets that will not
		//! outlive the next rebuild. Keys match what JMWebhookCOTModule's RPC
		//! handlers enforce.
		m_Form.UpdatePermission( removeBtn,       JMConstants.PERM_WEBHOOK_MANAGE_URL_REMOVE );
		m_Form.UpdatePermission( saveBtn,         JMConstants.PERM_WEBHOOK_MANAGE_URL_EDIT );
		m_Form.UpdatePermission( m_NameEdit,      JMConstants.PERM_WEBHOOK_MANAGE_URL_EDIT );
		m_Form.UpdatePermission( m_URLEdit,       JMConstants.PERM_WEBHOOK_MANAGE_URL_EDIT );
		m_Form.UpdatePermission( m_FilterGUIDList, JMConstants.PERM_WEBHOOK_MANAGE_URL_EDIT );
		m_Form.UpdatePermission( m_FilterRoleList, JMConstants.PERM_WEBHOOK_MANAGE_URL_EDIT );
		m_Form.UpdatePermission( m_AddTypeBtn,    JMConstants.PERM_WEBHOOK_MANAGE_TYPE_ADD );

		//! Left last: an empty type list disables Add regardless of permission.
		if ( available.Count() == 0 )
		{
			m_DropDownList.Disable();
			m_AddTypeBtn.Disable();
		}
	}

	protected UIActionEditableText BuildLabeledInput( string label, string value )
	{
		Widget row = UIActionManager.CreateWrapSpacerCompact( m_RootSpacer, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		UIActionText lbl = UIActionManager.CreateText( row, label );
		lbl.SetWidth( LABEL_W );
		lbl.SetLabelVAlign( UIActionVAlign.CENTER );

		UIActionEditableText edit = UIActionManager.CreateEditableText( row, "", this );
		edit.SetText( value );
		edit.SetWidgetWidth( edit.GetLabelWidget(), 0.0 );
		edit.SetWidgetWidth( edit.GetEditBoxWidget(), 1.0 );
		edit.SetWidth( FIELD_W );

		return edit;
	}

	//! A label and a searchable dropdown, same split as BuildLabeledInput.
	//! The text stays free: a value nobody is online with can still be typed.
	protected UIActionDropdownList BuildLabeledDropdown( string label, string value, array< string > items, string placeholder )
	{
		Widget row = UIActionManager.CreateWrapSpacerCompact( m_RootSpacer, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		UIActionText lbl = UIActionManager.CreateText( row, label );
		lbl.SetWidth( LABEL_W );
		lbl.SetLabelVAlign( UIActionVAlign.CENTER );

		UIActionDropdownList list = UIActionManager.CreateDropdownBox( row, m_Form.GetLayoutRoot(), "", items );
		list.SetPlaceholder( placeholder );
		list.SetText( value );
		list.SetWidth( FIELD_W );

		if ( m_Form )
			m_Form.AddOverlay( list );

		return list;
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
	//!
	//! The type tree itself is only rebuilt when the SET of types changed
	//! (added or removed). A toggle flipped - the common case - is pushed into
	//! the existing switches, so the rows are not torn down and recreated
	//! under the cursor, which is what made the delete buttons flicker.
	void UpdateState( JMWebhookConnectionGroup group, array< string > allTypes )
	{
		m_Name = group.Name;

		if ( m_Card )
			m_Card.SetLabel( group.Name );

		if ( m_TypesCard )
			m_TypesCard.SetLabel( GetTypesTitle( group ) );

		if ( !m_TypesWrapper )
			return;

		if ( HasSameTypes( group ) )
		{
			RefreshTypeStates( group );
		}
		else
		{
			UIActionManager.ClearChildren( m_TypesWrapper );

			if ( group.Count() == 0 )
			{
				m_TypeOrder.Clear();
				UIActionManager.CreateText( m_TypesWrapper, "#STR_COT_WEBHOOK_NO_EVENT_TYPES" );
			}
			else
			{
				RebuildTypes( group, allTypes );
			}
		}

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

	// -------------------------------------------------------------------------
	//  Type tree - one fold per module, one row per action under it
	// -------------------------------------------------------------------------

	//! True when `group` holds exactly the types the tree was built from, in
	//! the same order.
	protected bool HasSameTypes( JMWebhookConnectionGroup group )
	{
		if ( group.Count() != m_TypeOrder.Count() )
			return false;

		for ( int i = 0; i < group.Count(); i++ )
		{
			if ( group.Get( i ).Name != m_TypeOrder[i] )
				return false;
		}

		return true;
	}

	//! Push enabled states into the existing switches and recount the folds.
	protected void RefreshTypeStates( JMWebhookConnectionGroup group )
	{
		for ( int i = 0; i < group.Count(); i++ )
		{
			JMWebhookConnection conn = group.Get( i );

			UIActionToggleSwitch sw = m_TypeToggles.Get( conn.Name );
			if ( sw && sw.IsChecked() != conn.Enabled )
				sw.SetChecked( conn.Enabled );
		}

		RefreshGroupLabels( group );
	}

	//! Event types are module names, or a module name followed by an action
	//! ("JMPlayerModule", "JMPlayerModuleKick"). The module is the longest
	//! ROOT - a known type with no shorter known type in front of it - that
	//! starts `typeName`.
	protected string GetModuleOf( string typeName )
	{
		string best = "";

		foreach ( string root : m_Roots )
		{
			if ( root.Length() <= best.Length() )
				continue;

			if ( typeName.IndexOf( root ) == 0 )
				best = root;
		}

		if ( best == "" )
			return typeName;

		return best;
	}

	//! Every known type that no other known type is a prefix of.
	protected void BuildRoots( array< string > allTypes )
	{
		m_Roots.Clear();

		foreach ( string candidate : allTypes )
		{
			bool isRoot = true;

			foreach ( string other : allTypes )
			{
				if ( other != candidate && other.Length() < candidate.Length() && candidate.IndexOf( other ) == 0 )
				{
					isRoot = false;
					break;
				}
			}

			if ( isRoot )
				m_Roots.Insert( candidate );
		}
	}

	//! The row text: the action part of the name, the module itself reads
	//! "General".
	protected string GetActionLabel( string typeName, string moduleName )
	{
		if ( typeName == moduleName )
			return "General";

		int start = moduleName.Length();
		return typeName.Substring( start, typeName.Length() - start );
	}

	//! "Module (enabled/total)" for one fold's header button.
	protected string GetGroupLabel( JMWebhookConnectionGroup group, string moduleName )
	{
		int enabled = 0;
		int total   = 0;

		for ( int i = 0; i < group.Count(); i++ )
		{
			JMWebhookConnection conn = group.Get( i );
			string connName = conn.Name;

			if ( GetModuleOf( connName ) != moduleName )
				continue;

			total++;
			if ( conn.Enabled )
				enabled++;
		}

		return moduleName + " (" + enabled + "/" + total + ")";
	}

	protected void RefreshGroupLabels( JMWebhookConnectionGroup group )
	{
		foreach ( string moduleName, UIActionButton header : m_GroupHeaders )
		{
			if ( header )
				header.SetButton( GetGroupLabel( group, moduleName ) );
		}
	}

	//! The tree: modules in the order their first type appears, each a flat
	//! chevron header over a fold of rows - switch, action name, remove icon
	//! anchored to the right edge (webhook_type_row.layout).
	protected void RebuildTypes( JMWebhookConnectionGroup group, array< string > allTypes )
	{
		BuildRoots( allTypes );

		m_TypeOrder.Clear();
		m_TypeToggles.Clear();
		m_GroupHeaders.Clear();
		m_GroupFolds.Clear();

		array< string > modules = new array< string >;
		int i;

		for ( i = 0; i < group.Count(); i++ )
		{
			JMWebhookConnection conn = group.Get( i );
			string connName = conn.Name;

			m_TypeOrder.Insert( connName );

			string moduleName = GetModuleOf( connName );
			if ( modules.Find( moduleName ) < 0 )
				modules.Insert( moduleName );
		}

		foreach ( string moduleKey : modules )
		{
			bool expanded = false;
			if ( m_Expanded.Contains( moduleKey ) )
				expanded = m_Expanded.Get( moduleKey );

			UIActionButton header = UIActionManager.CreateButton( m_TypesWrapper, GetGroupLabel( group, moduleKey ), this, "" );
			if ( header ) header.SetOnClick( this, "OnClick_ToggleGroup" );
			header.SetFlat( true );
			header.SetIcon( JMConstants.ICON_CHEVRON_DOWN );
			header.SetIconRotation( ChevronAngle( expanded ), false );
			header.SetData( new JMWebhookTypeData( moduleKey, group.Name ) );

			UIActionFoldPanel fold = UIActionManager.CreateFoldPanel( m_TypesWrapper, m_Form, "OnChange_TypeFold", expanded );

			m_GroupHeaders.Insert( moduleKey, header );
			m_GroupFolds.Insert( moduleKey, fold );

			for ( i = 0; i < group.Count(); i++ )
			{
				JMWebhookConnection child = group.Get( i );
				string childName = child.Name;

				if ( GetModuleOf( childName ) != moduleKey )
					continue;

				BuildTypeRow( fold.GetContent(), child, moduleKey, group.Name );
			}
		}
	}

	protected void BuildTypeRow( Widget parent, JMWebhookConnection conn, string moduleName, string groupName )
	{
		Widget row = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/webhook_type_row.layout", parent );
		if ( !row )
			return;

		Widget toggleSlot = row.FindAnyWidget( "type_toggle_slot" );
		Widget deleteSlot = row.FindAnyWidget( "type_delete_slot" );

		TextWidget nameText;
		if ( Class.CastTo( nameText, row.FindAnyWidget( "type_name" ) ) )
			nameText.SetText( GetActionLabel( conn.Name, moduleName ) );

		UIActionToggleSwitch sw = UIActionManager.CreateToggleSwitch( toggleSlot, "", m_Form, "Action_TypeState", conn.Enabled );
		sw.SetData( new JMWebhookTypeData( conn.Name, groupName ) );
		sw.SetTooltip( conn.Name );

		UIActionConfirmInline removeTypeBtn = UIActionManager.CreateDeleteConfirmIcon( deleteSlot, m_Form, "Action_RemoveType" );
		removeTypeBtn.SetData( new JMWebhookTypeData( conn.Name, groupName ) );
		removeTypeBtn.SetTooltip( "#STR_COT_WEBHOOK_REMOVE_THIS_EVENT_TYPE_FROM_THE" );

		m_Form.UpdatePermission( removeTypeBtn, JMConstants.PERM_WEBHOOK_MANAGE_TYPE_REMOVE );
		m_Form.UpdatePermission( sw,            JMConstants.PERM_WEBHOOK_MANAGE_TYPE_STATE );

		m_TypeToggles.Insert( conn.Name, sw );
	}

	//! Where the chevron rests: down over an open fold, right over a shut one
	//! - the convention the role manager's permission tree uses.
	protected float ChevronAngle( bool expanded )
	{
		if ( expanded )
			return 0;

		return -90;
	}

	void OnClick_ToggleGroup( UIActionBase action )
	{
		JMWebhookTypeData data;
		if ( !Class.CastTo( data, action.GetData() ) )
			return;

		UIActionFoldPanel fold = m_GroupFolds.Get( data.Name );
		if ( !fold )
			return;

		bool expand = !fold.IsExpanded();
		fold.SetExpanded( expand );

		UIActionButton header = m_GroupHeaders.Get( data.Name );
		if ( header )
			header.SetIconRotation( ChevronAngle( expand ) );

		m_Expanded.Set( data.Name, expand );
	}
}
