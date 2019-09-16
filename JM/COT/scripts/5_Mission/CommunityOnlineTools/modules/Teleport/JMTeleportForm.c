class JMTeleportForm extends JMFormBase
{
	protected Widget m_ActionsFilterWrapper;
	protected UIActionEditableText m_Filter;

	protected TextListboxWidget m_LstPositionList;

	protected Widget m_ActionsWrapper;

	protected UIActionText m_PositionX;
	protected UIActionText m_PositionZ;
	protected UIActionButton m_Teleport;

	void JMTeleportForm()
	{
	}

	void ~JMTeleportForm()
	{
	}

	override string GetTitle()
	{
		return "Teleport";
	}
	
	override string GetIconName()
	{
		return "T";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnInit( bool fromMenu )
	{
		m_ActionsFilterWrapper = layoutRoot.FindAnyWidget( "actions_filter_wrapper" );

		m_Filter = UIActionManager.CreateEditableText( m_ActionsFilterWrapper, "Filter: ", this, "Type_UpdateList" );

		m_LstPositionList = TextListboxWidget.Cast( layoutRoot.FindAnyWidget("tls_ppp_pm_positions_list") );

		m_ActionsWrapper = layoutRoot.FindAnyWidget( "actions_wrapper" );

		ref Widget rows = UIActionManager.CreateGridSpacer( m_ActionsWrapper, 2, 1 );

		ref Widget coords = UIActionManager.CreateGridSpacer( rows, 1, 2 );

		m_PositionX = UIActionManager.CreateText( coords, "X: " );
		m_PositionZ = UIActionManager.CreateText( coords, "Z: " );

		if ( GetGame().IsServer() )
		{
			m_Teleport = UIActionManager.CreateButton( rows, "Teleport", this, "Click_Teleport" );
		} else
		{
			m_Teleport = UIActionManager.CreateButton( rows, "Teleport Selected Player(s)", this, "Click_Teleport" );
		}
	}

	override void OnShow()
	{
		super.OnShow();

		m_PositionX.SetText( "N/A" );
		m_PositionZ.SetText( "N/A" );

		UpdateList();
	}

	override void OnHide()
	{
		super.OnHide();
	}

	void Click_Teleport( UIEvent eid, ref UIActionBase action )
	{
		JMTeleportModule mod;
		if ( Class.CastTo( mod, GetModuleManager().GetModule( JMTeleportModule ) ) )
		{
			mod.Location( GetCurrentLocation(), GetSelectedPlayers() );
		}
	}

	void Type_UpdateList( UIEvent eid, ref UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		UpdateList();
	}

	void UpdateList()
	{
		JMTeleportModule tm;
		if ( !Class.CastTo( tm, GetModuleManager().GetModule( JMTeleportModule ) ) )
		{
			return;
		}

		m_LstPositionList.ClearItems();

		string filter = "" + m_Filter.GetText();
		filter.ToLower();

		array< ref JMTeleportLocation > locations = tm.GetLocations();
		if ( !locations )
			return;

		for ( int i = 0; i < locations.Count(); i++ )
		{
			string name = "" + locations[i].Name;
			name.ToLower();

			if ( (filter != "" && (!name.Contains( filter ))) ) 
			{
				continue;
			}

			if ( !GetPermissionsManager().HasPermission( "Teleport.Location." + locations[i].Permission ) )
			{
				continue;
			}

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
		} else 
		{
			m_PositionX.SetText( location.Position[0].ToString() );
			m_PositionZ.SetText( location.Position[2].ToString() );
		}

		return true;
	}

	JMTeleportLocation GetCurrentLocation()
	{
		if ( m_LstPositionList.GetSelectedRow() != -1 )
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
}