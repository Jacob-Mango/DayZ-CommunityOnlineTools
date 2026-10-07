class JMWebhookForm: JMFormBase
{
	//! Height of the roster's header band (search + add), pinned from OnResize.
	protected static const float           HEADER_HEIGHT = 34;

	//! Header row splits, as FRACTIONS of the row.
	static const float SEARCH_ROW_W = 0.84;
	static const float ADD_BTN_W    = 0.15;

	protected Widget                       m_Panel;
	protected UIActionScroller             m_Scroller;
	protected Widget                       m_ActionsWrapper;

	//! "Webhooks (N)" - shown on the right pane's placeholder while nothing
	//! is selected.
	protected UIActionText                 m_HeaderTitle;

	//! The detail section of the selected webhook, keyed by its name. Holds at
	//! most one entry: only the selected webhook is built.
	protected ref map< string, ref JMWebhookSection > m_Sections;
	protected ref array< string >          m_Types;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMWebhookCOTModule         m_Module;
	protected string                       m_PendingName;

	// ---- Roster -------------------------------------------------------------
	protected Widget                       m_HeaderWrapper;
	protected Widget                       m_ListWrapper;
	protected UIActionSearchBox            m_SearchBox;
	protected UIActionItemList             m_List;

	//! Webhook name for every row currently in the list, in row order.
	protected ref TStringArray             m_ListNames;

	//! Name of the webhook shown in the detail pane, "" for none.
	protected string                       m_SelectedName;

	//! A webhook to select as soon as the server confirms it exists - the one
	//! just added, or the new name of the one just renamed.
	protected string                       m_PendingSelect;

	void JMWebhookForm()
	{
		m_Types     = new array< string >();
		m_Sections  = new map< string, ref JMWebhookSection >();
		m_ListNames = new TStringArray();

		JMWebhookConstructor.Generate( m_Types );
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnCreate()
	{
		// ----------------------------------------------------------------------
		// Webhook form layout map (760 x 480 px) - split pane, roster + detail
		// ----------------------------------------------------------------------
		// LEFT  header (34 px): [search .................] [+]
		// LEFT  list:           one row per webhook - name, event count
		// RIGHT detail:         the selected webhook as JMWebhookSection cards
		//                       (Connection, Event Types) in a scroller.
		// ----------------------------------------------------------------------

		m_LeftPanel         = layoutRoot.FindAnyWidget( "panel_left" );
		m_RightPanel        = layoutRoot.FindAnyWidget( "panel_right" );
		m_RightPanelDisable = layoutRoot.FindAnyWidget( "panel_right_disable" );

		m_HeaderWrapper = layoutRoot.FindAnyWidget( "header_panel" );
		m_ListWrapper   = layoutRoot.FindAnyWidget( "wh_list_wrapper" );

		Widget headerRow = UIActionManager.CreateWrapSpacerCompact( m_HeaderWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_SearchBox = UIActionManager.CreateSearchBox( headerRow, this, "OnChange_Search", "#STR_COT_GENERIC_SEARCH" );
		m_SearchBox.SetWidth( SEARCH_ROW_W );

		UIActionImageButton addBtn = UIActionManager.CreateIconButton( headerRow, JMConstants.Lucide( "plus" ), this, "" );
		if ( addBtn ) addBtn.SetOnClick( this, "Action_AddWebhook" );
		addBtn.SetWidth( ADD_BTN_W );
		addBtn.SetTooltip( "#STR_COT_WEBHOOK_CREATE_A_NEW_DISCORD_WEBHOOK_CONFIGURATI" );

		//! Matches the key RPC_AddConnectionGroup enforces server-side. The
		//! per-webhook controls are gated in JMWebhookSection, which rebuilds
		//! them whenever the settings change.
		BindPermission( addBtn, JMConstants.PERM_WEBHOOK_MANAGE_URL_ADD );

		//! COT's pooled list - name on the left, event count on the right.
		m_List = UIActionManager.CreateItemList( m_ListWrapper, this, "OnClick_List" );
		m_List.SetEmptyText( "#STR_COT_WEBHOOK_NO_WEBHOOKS_CONFIGURED_CLICK_ADD_WEBHOOK" );

		// Scrollable content area
		m_Panel         = layoutRoot.FindAnyWidget( "panel" );
		m_Scroller      = UIActionManager.CreateScroller( m_Panel );
		m_ActionsWrapper = m_Scroller.GetContentWidget();

		//! What the right pane says while nothing is selected.
		if ( m_RightPanelDisable )
		{
			Widget placeholder = UIActionManager.CreateGridSpacer( m_RightPanelDisable, 1, 1 );
			m_HeaderTitle = UIActionManager.CreateText( placeholder, "#STR_COT_WEBHOOK_HEADER_TITLE", "Select a webhook on the left" );
		}

		SetPanelEnabled( false );

		OnSettingsUpdated();

		m_Scroller.UpdateScroller();
	}

	override void OnResize( float w, float h )
	{
		super.OnResize( w, h );

		//! The roster's two bands are pinned here: the header is fixed, the
		//! list takes the rest of the column and is told that height rather
		//! than measuring it.
		PinBand( m_HeaderWrapper, 0, HEADER_HEIGHT );

		if ( m_ListWrapper && h > HEADER_HEIGHT )
		{
			float listH = h - HEADER_HEIGHT;

			PinBand( m_ListWrapper, HEADER_HEIGHT, listH );

			if ( m_List )
				m_List.SetViewportHeight( listH );
		}

		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}

	//! Place one band of the left column: exact y, exact height, full width.
	protected void PinBand( Widget band, float y, float height )
	{
		if ( !band )
			return;

		band.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		band.SetPos( 0, y );
		band.SetSize( 1, height );
	}

	//! The selected webhook carries its own unsaved Name/URL/filter edits
	//! until its Save button is clicked. This fires on ANY webhook settings
	//! change - including one admin flipping an event-type checkbox - so
	//! tearing down and rebuilding the detail from scratch here used to wipe
	//! whatever another admin (or this one) had half-typed into it.
	//!
	//! The roster is cheap and always refreshed. The detail is refreshed in
	//! place via JMWebhookSection.UpdateState() while the selected name still
	//! exists, leaving any unsaved edits sitting in it untouched; only a
	//! selection change (add, rename, remove, click) rebuilds it.
	override void OnSettingsUpdated()
	{
		if ( !m_Module || !m_ActionsWrapper )
			return;

		array< ref JMWebhookConnectionGroup > groups = m_Module.GetConnections();

		if ( m_HeaderTitle )
			m_HeaderTitle.SetLabel( "Webhooks (" + groups.Count() + ")" );

		if ( m_PendingSelect != "" && FindGroup( groups, m_PendingSelect ) )
		{
			m_SelectedName  = m_PendingSelect;
			m_PendingSelect = "";
		}

		RefreshList( groups );

		JMWebhookConnectionGroup selected = FindGroup( groups, m_SelectedName );
		if ( !selected )
		{
			m_SelectedName = "";
			RebuildAllSections( groups );
			return;
		}

		JMWebhookSection section = m_Sections.Get( selected.Name );
		if ( section )
		{
			section.UpdateState( selected, m_Types );
			DeferCall( "UpdateDetailScroller", 250 );
		}
		else
		{
			RebuildAllSections( groups );
		}
	}

	protected JMWebhookConnectionGroup FindGroup( array< ref JMWebhookConnectionGroup > groups, string name )
	{
		if ( name == "" )
			return null;

		foreach ( JMWebhookConnectionGroup group : groups )
		{
			if ( group.Name == name )
				return group;
		}

		return null;
	}

	//! Rebuild the roster rows from `groups`, filtered by the search box, and
	//! keep the selected webhook highlighted.
	protected void RefreshList( array< ref JMWebhookConnectionGroup > groups )
	{
		if ( !m_List )
			return;

		m_ListNames.Clear();

		array< string > labels = new array< string >;
		array< string > subs   = new array< string >;

		string filter;
		if ( m_SearchBox )
			filter = m_SearchBox.GetText();

		JMSearchMatcher matcher = new JMSearchMatcher( filter );

		foreach ( JMWebhookConnectionGroup group : groups )
		{
			if ( !matcher.Matches( group.Name ) && !matcher.Matches( group.Address ) )
				continue;

			int enabled = 0;
			for ( int i = 0; i < group.Count(); i++ )
			{
				if ( group.Get( i ).Enabled )
					enabled++;
			}

			labels.Insert( group.Name );
			subs.Insert( enabled.ToString() + "/" + group.Count().ToString() );
			m_ListNames.Insert( group.Name );
		}

		m_List.SetItems( labels, subs );
		m_List.SetSelectedIndex( m_ListNames.Find( m_SelectedName ), false );
	}

	//! Rebuild the detail pane for m_SelectedName. Kept under its old name:
	//! it used to rebuild one section per webhook, and still drops them all.
	protected void RebuildAllSections( array< ref JMWebhookConnectionGroup > groups )
	{
		m_Sections.Clear();

		UIActionManager.ClearChildren( m_ActionsWrapper );

		bool hasSelection = false;

		JMWebhookConnectionGroup group = FindGroup( groups, m_SelectedName );
		if ( group )
		{
			JMWebhookSection section = new JMWebhookSection( m_ActionsWrapper, this, group, m_Types );
			m_Sections.Insert( group.Name, section );
			hasSelection = true;
		}

		SetPanelEnabled( hasSelection );

		UpdateDetailScroller();

		//! The cards size themselves to their rows over the next frames, so
		//! the scroller measured now sees an empty pane and shows no bar.
		//! Measure again once they have settled.
		DeferCall( "UpdateDetailScroller", 250 );
	}

	void UpdateDetailScroller()
	{
		if ( m_Scroller )
			m_Scroller.UpdateScroller();
	}

	//! A module fold in the event-type tree reports every frame its height
	//! moves; the pane grows and shrinks with it, so re-measure for the whole
	//! slide rather than once at the end.
	void OnChange_TypeFold( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateDetailScroller();
	}

	// -------------------------------------------------------------------------
	//  Roster
	// -------------------------------------------------------------------------

	void OnClick_List( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK || !m_Module )
			return;

		int row = m_List.GetSelectedIndex();
		if ( row < 0 || row >= m_ListNames.Count() )
			return;

		if ( m_ListNames[row] == m_SelectedName )
			return;

		m_SelectedName = m_ListNames[row];

		RebuildAllSections( m_Module.GetConnections() );
	}

	void OnChange_Search( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE || !m_Module )
			return;

		RefreshList( m_Module.GetConnections() );
	}

	// -------------------------------------------------------------------------
	//  Add Webhook - two-step: name then URL
	// -------------------------------------------------------------------------

	void Action_AddWebhook( UIActionBase action )
	{
		PromptInput( "#STR_COT_WEBHOOK_ADD_WEBHOOK_2", "#STR_COT_WEBHOOK_ENTER_A_NAME_FOR_THIS_WEBHOOK", "Action_AddWebhook_GotName" );
	}

	void Action_AddWebhook_GotName( JMConfirmation confirmation )
	{
		string name = confirmation.GetEditBoxValue();
		name.Trim();
		if ( name == "" )
			return;

		m_PendingName = name;

		PromptInput( "#STR_COT_WEBHOOK_ADD_WEBHOOK_2", "Enter the Discord webhook URL for '" + name + "':", "Action_AddWebhook_GotURL" );
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

		//! Open the new webhook once the server has added it.
		m_PendingSelect = m_PendingName;

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

		//! A rename moves the selection with it.
		if ( newName != data.Name )
			m_PendingSelect = newName;

		action.AnimateFeedback();

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

		//! A toggle switch now, not a checkbox - IsChecked() is on the base.
		m_Module.TypeState( data.Name, data.Group, action.IsChecked() );
	}

}
