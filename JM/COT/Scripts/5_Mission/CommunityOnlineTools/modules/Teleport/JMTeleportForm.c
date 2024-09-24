class JMTeleportForm: JMFormBase
{
	protected Widget m_ActionsFilterWrapper;
	protected UIActionEditableTextPreview m_Filter;

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
	protected UIActionEditableTextPreview m_InputCategory;

	protected UIActionButton m_InputAdd;
	protected UIActionButton m_InputRefresh;
	protected UIActionButton m_InputRemove;

	private JMTeleportModule m_Module;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_ActionsFilterWrapper = layoutRoot.FindAnyWidget( "actions_filter_wrapper" );

		Widget topRow = UIActionManager.CreateGridSpacer( m_ActionsFilterWrapper, 1, 2 );

		m_Filter = UIActionManager.CreateEditableTextPreview( topRow, "#STR_COT_TELEPORT_MODULE_FILTER", this, "Type_UpdateList" );
		
		m_CategoriesList = UIActionManager.CreateSelectionBox( topRow, "", {"ALL"}, this, "Click_LocationType" );
		m_CategoriesList.SetSelectorWidth(1.0);

		m_LstPositionList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("tls_ppp_pm_positions_list") );

		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

		Widget rows = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 2 );

		m_PositionX = UIActionManager.CreateText( rows, "X: " );
		m_PositionZ = UIActionManager.CreateText( rows, "Z: " );

		if ( GetGame().IsServer() )
		{
			m_Teleport = UIActionManager.CreateButton( m_ActionsWrapper, "#STR_COT_TELEPORT_MODULE_TELEPORT_OFFLINE", this, "Click_Teleport" );
		}
		else
		{
			m_Teleport = UIActionManager.CreateButton( m_ActionsWrapper, "#STR_COT_TELEPORT_MODULE_TELEPORT_ONLINE", this, "Click_Teleport" );
		}

		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

		Widget inputRows = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 2, 2 );

		m_InputTextLocation = UIActionManager.CreateText( inputRows, "Name" );
		m_InputTextCategory = UIActionManager.CreateText( inputRows, "Category" );

		m_InputLocation = UIActionManager.CreateEditableText( inputRows, "", this );
		m_InputLocation.SetWidgetWidth( m_InputLocation.GetLabelWidget(), 0.0 );
		m_InputLocation.SetWidgetWidth( m_InputLocation.GetEditBoxWidget(), 1.0 );

		m_InputCategory = UIActionManager.CreateEditableTextPreview( inputRows, "", this, "InputCategory_OnChange" );
		m_InputCategory.SetWidgetWidth( m_InputCategory.GetLabelWidget(), 0.0 );
		m_InputCategory.SetWidgetWidth( m_InputCategory.GetEditBoxWidget(), 1.0 );
		m_InputCategory.SetWidgetWidth( m_InputCategory.GetEditPreviewBoxWidget(), 1.0 );
		
		Widget inputBtnRows = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 1, 3 );

		m_InputAdd = UIActionManager.CreateButton( inputBtnRows, "Add", this, "Click_AddLocation" );
		m_InputRefresh = UIActionManager.CreateButton( inputBtnRows, "Refresh", this, "Click_Refresh" );
		m_InputRemove = UIActionManager.CreateButton( inputBtnRows, "Delete", this, "Click_RemoveLocation" );
		m_InputAdd.SetColor(COLOR_GREEN);
		m_InputRemove.SetColor(COLOR_RED);

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Add" ) )
		{
			m_InputTextLocation.Disable();
			m_InputTextCategory.Disable();
			m_InputLocation.Disable();
			m_InputCategory.Disable();
			m_InputAdd.Disable();
		}

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Remove" ) )
			m_InputRemove.Disable();
		
		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Refresh" ) )
			m_InputRefresh.Disable();
	}

	void Click_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Refresh" ) )
			return;

		m_Module.Reload();
	}

	void Click_RemoveLocation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location.Remove" ) )
			return;

		CreateConfirmation_Two( JMConfirmationType.INFO, "Are you sure?", "Delete "+ GetCurrentPositionName(), "#STR_COT_GENERIC_YES", "RemoveLocation_Confirmed", "#STR_COT_GENERIC_NO", "" );
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
			COTCreateLocalAdminNotification(new StringLocaliser("No name given!"));
			return;
		}

		m_Module.AddLocation(m_InputLocation.GetText(),  m_InputCategory.GetText(), GetGame().GetPlayer().GetPosition() );

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
			Teleport();
			
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

		Teleport();
	}

	void Teleport()
	{
		if (!HasTooManyPlayers("TeleportConfirm", "TeleportSelfConfirm", "TeleportSelectedConfirm"))
			TeleportConfirm(NULL);
	}

	void TeleportConfirm(JMConfirmation confirmation)
	{
		array< string > players = JM_GetSelected().GetPlayers();
		if ( players.Count() == 0 )
		{
			PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
			if ( player.GetIdentity() )
				players.Insert(player.GetIdentity().GetId());
		}

		m_Module.Location( GetCurrentLocation(), players );
	}

	void TeleportSelectedConfirm(JMConfirmation confirmation)
	{
		m_Module.Location( GetCurrentLocation(), {JM_GetSelected().GetPlayers()[0]} );
	}

	void TeleportSelfConfirm(JMConfirmation confirmation)
	{
		m_Module.Location( GetCurrentLocation(), {GetPermissionsManager().GetClientPlayer().GetGUID()} );
	}

	void InputCategory_OnChange( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		m_InputCategory.SetTextPreview("");
		
		string strSearch = m_InputCategory.GetText();
		if ( strSearch != "" )
		{
			string closestMatch;
			TStringArray suggestions = new TStringArray;
			foreach(string category: m_Categories)
			{
				if ( category == strSearch )
				{
					suggestions.Clear();
					closestMatch = category;
				}
				else if ( category.IndexOf(strSearch) == 0 )
				{
					if (!closestMatch)
						suggestions.Insert(category);
				}
			}

			if (suggestions.Count())
			{
				suggestions.Sort();
				closestMatch = suggestions[0];
			}

			m_InputCategory.SetTextPreview(closestMatch);
		}
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

		string filter = m_Filter.GetText();
		filter.ToLower();

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

			string name = locations[i].Name;
			name.ToLower();

			if (filter != "")
			{
				if (!name.Contains( filter ))
					continue;

				if ( name == filter )
				{
					suggestions.Clear();
					closestMatch = name;
				}
				else if ( name.IndexOf(filter) == 0 )
				{
					if (!closestMatch)
						suggestions.Insert(name);
				}
			}

			m_LstPositionList.AddItem( locations[i].Name, locations[i], 0 );
		}

		if (suggestions.Count())
		{
			suggestions.Sort();
			closestMatch = suggestions[0];
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

	//! TODO: Upgrade the JM Confirmation to return Result, this is a mess right now
	//! 	  I dont have time right now so this is my reminder
	bool HasTooManyPlayers(string funcName, string funcSelf, string funcOnlyName)
	{
		JMPlayerInstance inst;
		int count = JM_GetSelected().GetPlayers().Count();
		if (count > 1)
		{
			inst = GetPermissionsManager().GetPlayer( JM_GetSelected().GetPlayers()[0] );
			CreateConfirmation_Three( JMConfirmationType.INFO, "#STR_COT_WARNING_PLAYERS_MESSAGE_HEADER", string.Format(Widget.TranslateString("#STR_COT_WARNING_PLAYERS_MESSAGE_BODY"), count.ToString()), "#STR_COT_GENERIC_CANCEL", "", inst.GetName(), funcOnlyName, "#STR_COT_GENERIC_CONFIRM", funcName );
			return true;
		}
		else
		{
			inst = GetPermissionsManager().GetPlayer( JM_GetSelected().GetPlayers()[0] );
			if (inst && inst != GetPermissionsManager().GetClientPlayer() )
			{
				CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_WARNING_PLAYERS_MESSAGE_HEADER", string.Format(Widget.TranslateString("#STR_COT_WARNING_SELECTEDPLAYER_MESSAGE_BODY"), inst.GetName()), inst.GetName(), funcOnlyName, "#STR_COT_GENERIC_SELF", funcSelf );
				return true;
			}
		}

		return false;
	}
};
