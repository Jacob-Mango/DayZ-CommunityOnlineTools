class JMTeleportForm: JMFormBase
{
	protected Widget m_ActionsFilterWrapper;
	protected UIActionEditableText m_Filter;

	private string m_CurrentType;
	protected UIActionDropdownList m_CategoriesList;
	protected TextListboxWidget m_LstPositionList;

	protected Widget m_ActionsWrapper;

	protected UIActionText m_PositionX;
	protected UIActionText m_PositionZ;
	protected UIActionButton m_Teleport;

	private JMTeleportModule m_Module;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_ActionsFilterWrapper = layoutRoot.FindAnyWidget( "actions_filter_wrapper" );

#ifdef COT_MODULE_TP_FILTERLIST_ENABLE
		Widget topRow = UIActionManager.CreateGridSpacer( m_ActionsFilterWrapper, 1, 2 );

		m_Filter = UIActionManager.CreateEditableText( topRow, "#STR_COT_TELEPORT_MODULE_FILTER", this, "Type_UpdateList" );
		
		m_CategoriesList = UIActionManager.CreateDropdownBox( topRow, m_ActionsFilterWrapper, "Type:", {""}, this, "Click_LocationType" );
#else
		m_Filter = UIActionManager.CreateEditableText( m_ActionsFilterWrapper, "#STR_COT_TELEPORT_MODULE_FILTER", this, "Type_UpdateList" );
#endif
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

	override void OnHide()
	{
		super.OnHide();

		//m_Module.OnSelectLocation(vector.Zero, true);
	}

	override void OnSettingsUpdated()
	{		
		UpdateList();
	}

	void Click_LocationType( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

#ifdef COT_MODULE_TP_FILTERLIST_ENABLE
		m_CurrentType = m_CategoriesList.GetText();
#endif
		UpdateList();
	}

	void Click_Teleport( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		array< string > players = JM_GetSelected().GetPlayers();
		if ( players.Count() == 0 )
		{
			PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
			if ( player.GetIdentity() )
				players.Insert(player.GetIdentity().GetId());
		}

		m_Module.Location( GetCurrentLocation(), players );
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

		m_LstPositionList.ClearItems();

		if ( !GetPermissionsManager().HasPermission( "Admin.Player.Teleport.Location" ) )
			return;
		
#ifdef COT_MODULE_TP_FILTERLIST_ENABLE
		TStringArray locationTypes = m_Module.GetLocationTypes();
		m_CategoriesList.SetItems(locationTypes);
#endif
		string filter = "" + m_Filter.GetText();
		filter.ToLower();

		array< ref JMTeleportLocation > locations = m_Module.GetLocations();
		if ( !locations )
			return;
		
		for ( int i = 0; i < locations.Count(); i++ )
		{
			string type = "" + locations[i].Type;

			if (  m_CurrentType != "" && type != m_CurrentType ) 
				continue;

			string name = "" + locations[i].Name;
			name.ToLower();

			if ( (filter != "" && (!name.Contains( filter ))) ) 
				continue;

			m_LstPositionList.AddItem( locations[i].Name, locations[i], 0 );
		}
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
			JMTeleportLocation position_name;
			m_LstPositionList.GetItemData( m_LstPositionList.GetSelectedRow(), 0, position_name );
			return position_name;
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
};
