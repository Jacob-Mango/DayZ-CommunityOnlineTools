class JMTeleportForm: JMFormBase
{
	protected Widget m_ActionsFilterWrapper;
	protected UIActionSearchBox m_Filter;

	protected string m_CurrentCategory = "ALL";
	protected ref TStringArray m_Categories;
	protected UIActionSelectBox m_CategoriesList;
	
	protected TextListboxWidget m_LstPositionList;

	protected Widget m_ActionsWrapper;

	protected UIActionText m_PositionX;
	protected UIActionText m_PositionZ;
	protected UIActionButton m_Teleport;

	protected UIActionText m_InputTextLocation;
	protected UIActionText m_InputTextCategory;

	protected UIActionEditableText m_InputLocation;
	protected UIActionSearchBox m_InputCategory;

	protected UIActionButton m_InputAdd;
	protected UIActionImageButton m_InputRefresh;
	protected UIActionConfirmInline m_InputRemove;

	//! protected, not private: sub-mods reach for the module through the form.
	protected JMTeleportModule m_Module;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		// ---- Filter bar (archetype B header) -----------------------------------
		m_ActionsFilterWrapper = layoutRoot.FindAnyWidget( "panel_top" );

		m_Filter = UIActionManager.CreateSearchBox( m_ActionsFilterWrapper, this, "Type_UpdateList", "#STR_COT_TELEPORT_MODULE_FILTER" );

		m_CategoriesList = UIActionManager.CreateSelectionBox( m_ActionsFilterWrapper, "", {"ALL"}, this, "Click_LocationType" );
		m_CategoriesList.SetSelectorWidth(1.0);

		// ---- Position list (middle, from layout) --------------------------------
		m_LstPositionList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("tls_ppp_pm_positions_list") );

		// ---- Bottom action area (5 rows in actions_wrapper) --------------------
		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

		// Row 1 - selected position coordinates
		Widget coordRow = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 2 );
		m_PositionX = UIActionManager.CreateText( coordRow, "X: " );
		m_PositionZ = UIActionManager.CreateText( coordRow, "Z: " );

		// Row 2 - teleport button (full width)
		m_Teleport = UIActionManager.CreateButton( m_ActionsWrapper, "Teleport", this, "Click_Teleport" );
		m_Teleport.SetTooltip( "Teleport yourself to the selected saved location" );

		// Row 3 - name label (25%) + editable input (75%).
		// Fractional widths must sum to slightly less than 1.0 in a WrapSpacer;
		// at exactly 1.0 the engine wraps the second child to a new line.
		Widget nameRow = UIActionManager.CreateWrapSpacer( m_ActionsWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		m_InputTextLocation = UIActionManager.CreateText( nameRow, "#STR_COT_GENERIC_NAME" );
		m_InputTextLocation.SetWidth( 0.24 );
		m_InputTextLocation.SetLabelVAlign( UIActionVAlign.CENTER );
		m_InputLocation = UIActionManager.CreateEditableText( nameRow, "", this );
		m_InputLocation.SetWidth( 0.74 );
		m_InputLocation.SetWidgetWidth( m_InputLocation.GetLabelWidget(), 0.0 );
		m_InputLocation.SetWidgetWidth( m_InputLocation.GetEditBoxWidget(), 1.0 );

		// Row 4 - category label (25%) + editable input with autocomplete (75%)
		Widget catRow = UIActionManager.CreateWrapSpacer( m_ActionsWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
		m_InputTextCategory = UIActionManager.CreateText( catRow, "#STR_COT_GENERIC_CATEGORY" );
		m_InputTextCategory.SetWidth( 0.24 );
		m_InputTextCategory.SetLabelVAlign( UIActionVAlign.CENTER );
		m_InputCategory = UIActionManager.CreateSearchBox( catRow, this, "InputCategory_OnChange" );
		m_InputCategory.SetWidth( 0.74 );
		m_InputCategory.SetWidgetWidth( m_InputCategory.GetEditBoxWidget(), 1.0 );
		m_InputCategory.SetWidgetWidth( m_InputCategory.GetEditPreviewBoxWidget(), 1.0 );

		// Row 5 - Delete (icon) | Refresh (icon) | Add (label, fills rest)
		Widget btnRow = UIActionManager.CreateWrapSpacer( m_ActionsWrapper, WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

		m_InputRemove = UIActionManager.CreateConfirmInline( btnRow, "", this, "Click_RemoveLocation" );
		UIActionIconGrid.ApplyDeletePreset( m_InputRemove );
		m_InputRemove.SetButton( "" );
		m_InputRemove.SetFixedSize( ICON_BUTTON_PX, ICON_BUTTON_PX );
		m_InputRemove.CenterIcon( ICON_BUTTON_PX, 16 );
		m_InputRemove.SetConfirmLabel( "O" );
		m_InputRemove.SetCancelLabel( "X" );
		m_InputRemove.SetTooltip( "Delete the selected saved location" );

		m_InputRefresh = UIActionManager.CreateRefreshButton( btnRow, this, "Click_Refresh", "#STR_COT_GENERIC_REFRESH" );
		m_InputRefresh.SetFixedSize( ICON_BUTTON_PX, ICON_BUTTON_PX );

		m_InputAdd = UIActionManager.CreateButton( btnRow, "Add", this, "Click_AddLocation" );
		m_InputAdd.SetWidth( 1.0 );
		m_InputAdd.SetColor( JMTheme.SUCCESS_FILL );
		m_InputAdd.SetTooltip( "Save your current position as a new location" );

		// ---- Permission gating -------------------------------------------------
		// Bound rather than evaluated once: this used to run only here, so a
		// permission granted while the form was open left the controls dead
		// until it was closed and reopened.
		RegisterPermission( m_InputTextLocation, "Admin.Player.Teleport.Location.Add" );
		RegisterPermission( m_InputTextCategory, "Admin.Player.Teleport.Location.Add" );
		RegisterPermission( m_InputLocation,     "Admin.Player.Teleport.Location.Add" );
		RegisterPermission( m_InputCategory,     "Admin.Player.Teleport.Location.Add" );
		RegisterPermission( m_InputAdd,          "Admin.Player.Teleport.Location.Add" );
		RegisterPermission( m_InputRemove,       "Admin.Player.Teleport.Location.Remove" );
		RegisterPermission( m_InputRefresh,      "Admin.Player.Teleport.Location.Refresh" );
		RegisterPermission( m_Teleport,          "Admin.Player.Teleport.Location" );
	}

	void Click_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Refresh" ) )
			return;

		m_InputRefresh.TriggerSpin( 2 );
		m_Module.Reload();
	}

	void Click_RemoveLocation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Remove" ) )
			return;

		RemoveLocation_Confirmed();
	}

	void RemoveLocation_Confirmed()
	{
		COTCreateLocalAdminNotification(new StringLocaliser("Removed " + GetCurrentPositionName()));

		m_Module.RemoveLocation(GetCurrentLocation());		

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Refresh" ) )
			return;

		m_Module.Reload();
	}

	void Click_AddLocation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Add" ) )
			return;

		if ( !m_InputLocation.GetText() )
		{
			COTCreateLocalAdminNotification(new StringLocaliser("#STR_COT_TELEPORT_MODULE_TELEPORT_ERROR_NONAME"));
			return;
		}

		m_Module.AddLocation(m_InputLocation.GetText(),  m_InputCategory.GetText(), g_Game.GetPlayer().GetPosition() );

		COTCreateLocalAdminNotification(new StringLocaliser("Added "+ m_InputLocation.GetText()+" to Category "+ m_InputCategory.GetText()));

		m_InputLocation.SetText("");
		m_InputCategory.SetText("");
		m_InputCategory.SetTextPreview("");

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Refresh" ) )
			return;

		m_Module.Reload();
	}

	override void OnShow()
	{
		super.OnShow();

		m_PositionX.SetText( "N/A" );
		m_PositionZ.SetText( "N/A" );

		if (m_Module.IsLoaded())
		{
			UpdateList();
		}
		else
		{
			m_Module.Load();
		}
	}

	override void OnSettingsUpdated()
	{
		UpdateList();
	}

	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		super.OnDoubleClick( w, x, y, button );

		if ( w == m_LstPositionList && button == MouseState.LEFT )
		{
			TeleportSelf();
			
			return true;
		}

		return false;
	}

	void Click_LocationType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_CurrentCategory = m_Categories[m_CategoriesList.GetSelection()];
		UpdateList();
	}

	void Click_Teleport( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateAdvancedPlayerConfirm("#STR_COT_PLAYER_MODULE_RIGHT_PLAYER_POSITION_TELEPORT_ME_TO", "TeleportMulti", "TeleportSingle", "TeleportSelf", false);
	}

	void TeleportMulti(JMConfirmation confirmation = NULL)
	{
		m_Module.Location( GetCurrentLocation(), JM_GetSelected().GetPlayers() );
	}

	void TeleportSingle(JMConfirmation confirmation = NULL)
	{
		m_Module.Location( GetCurrentLocation(), {JM_GetSelected().GetPlayers()[0]} );
	}

	void TeleportSelf(JMConfirmation confirmation = NULL)
	{
		m_Module.Location( GetCurrentLocation(), {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void InputCategory_OnChange( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		string closestMatch;
		
		string strSearch = m_InputCategory.GetText();
		if ( strSearch != "" )
		{
			foreach(string category: m_Categories)
			{
				if ( category == strSearch )
				{
					closestMatch = category;
					break;  //! We can end the search here because we got a perfect match
				}
				else if ( category.IndexOf(strSearch) == 0 )
				{
					if (!closestMatch || category.Length() < closestMatch.Length())
						closestMatch = category;
				}
			}
		}

		m_InputCategory.SetTextPreview(closestMatch);
	}

	void Type_UpdateList( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateList();
	}

	void UpdateList()
	{
		if (!m_Module.IsLoaded())
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location" ) )
			return;

		if(!m_Categories)
		{
			m_Categories = new TStringArray;
			m_Categories = m_Module.GetLocationTypes();
			if(m_Categories)
				m_CategoriesList.SetSelections(m_Categories);
		}

		m_Filter.SetTextPreview("");
		m_LstPositionList.ClearItems();

		COT_String filter = m_Filter.GetText();
		bool requireAllKeywords;
		TStringArray keywords = filter.KeywordSearch_Prepare(requireAllKeywords);

		array< ref JMTeleportLocation > locations = m_Module.GetLocations();
		if ( !locations )
			return;
		
		string closestMatch;
		TStringArray suggestions = new TStringArray;
		for ( int i = 0; i < locations.Count(); i++ )
		{
			string type = locations[i].Type;

			if (  m_CurrentCategory != "ALL" && type != m_CurrentCategory ) 
				continue;

			COT_String name = locations[i].Name;
			name.ToLower();

			if (filter != "")
			{
				if (!name.KeywordSearchImplEx(filter, keywords, requireAllKeywords, closestMatch))
					continue;
			}

			m_LstPositionList.AddItem( locations[i].Name, locations[i], 0 );
		}

		m_Filter.SetTextPreview(closestMatch);
	}

	override bool OnItemSelected( Widget w, int x, int y, int row, int column, int oldRow, int oldColumn )
	{
		JMTeleportLocation location = GetCurrentLocation();

		if ( location == NULL )
		{
			m_PositionX.SetText( "N/A" );
			m_PositionZ.SetText( "N/A" );
		} else {
			m_PositionX.SetText( location.Position[0].ToString() );
			m_PositionZ.SetText( location.Position[2].ToString() );

			m_Module.OnSelectLocation(location.Position);
		}

		return true;
	}

	JMTeleportLocation GetCurrentLocation()
	{
		if ( m_LstPositionList && m_LstPositionList.GetSelectedRow() != -1 )
		{
			JMTeleportLocation location;
			m_LstPositionList.GetItemData( m_LstPositionList.GetSelectedRow(), 0, location );
			return location;
		}

		return NULL;
	}

	string GetCurrentPositionName()
	{
		if ( m_LstPositionList.GetSelectedRow() != -1 )
		{
			string position_name;
			m_LstPositionList.GetItemText( m_LstPositionList.GetSelectedRow(), 0, position_name );
			return position_name;
		}

		return "";
	}
}
