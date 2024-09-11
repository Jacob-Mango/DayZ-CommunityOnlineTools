class JMVehiclesMenu: JMFormBase
{
	private JMVehiclesModule m_Module;
	
	protected Widget m_VehicleMapPanel;
	protected Widget m_VehicleListPanel;
	protected Widget m_VehiclesListContent;

	protected UIActionButton m_RefreshButton;
	protected UIActionButton m_DeleteAllButton;
	protected UIActionButton m_DeleteDestroyedButton;
	protected UIActionButton m_DeleteUnclaimedButton;
	
	protected Widget m_MapWidgetPanel;
	protected MapWidget m_MapWidget;
	
	protected Widget m_VehicleInfoPanel;
	protected UIActionText m_VehicleName;
	protected UIActionText m_VehicleClassName;

	protected UIActionText m_VehicleStatus; // Exploded - Destroyed - Functional
	protected UIActionText m_VehicleType;

	protected UIActionText m_VehicleID;
	protected UIActionText m_VehiclePersistentIDAB;
	protected UIActionText m_VehiclePersistentIDCD;

	protected UIActionText m_VehiclePosition;
	protected UIActionText m_VehicleRotation;
	
	protected UIActionText m_VehicleKeys;
	protected UIActionText m_VehicleLastDriverUID;

	protected UIActionText m_VehicleCovered;
	
	protected Widget m_VehicleOptionsPanel;
	
	protected UIActionButton m_DeleteVehicleButton;
	protected UIActionButton m_RepairVehicleButton;
	protected UIActionButton m_UnstuckVehicleButton;
	protected UIActionButton m_RefuelVehicleButton;
	protected UIActionButton m_CoverVehicleButton;
	protected UIActionButton m_LockVehicleButton;
	protected UIActionButton m_UnPairVehicleButton;
	protected UIActionButton m_CopyPositionButton;
	protected UIActionButton m_CopyRotationButton;
	protected UIActionButton m_CopyLastDriverUIDButton;
	protected UIActionButton m_TeleportVehicleButton;
	protected UIActionButton m_TeleportMeButton;
	protected UIActionButton m_ReturnButton;
	
	protected ref array<ref JMVehiclesMapMarker> m_MapMarkers;
	protected ref array<ref JMVehiclesListEntry> m_VehicleEntries;
	
	protected JMVehicleMetaData m_CurrentVehicle;
	protected bool m_IsInVehicleInfo = false;

	void JMVehiclesMenu()
	{
		m_MapMarkers = new array<ref JMVehiclesMapMarker>;
		m_VehicleEntries = new array<ref JMVehiclesListEntry>;
	}

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		return Class.CastTo( m_Module, mdl );
	}

	override void OnInit()
	{
		m_VehicleMapPanel = Widget.Cast( layoutRoot.FindAnyWidget( "map_window_panel" ) );
		
		//! Vehicles List
		m_VehicleListPanel 		= Widget.Cast( layoutRoot.FindAnyWidget( "vehicles_list_panel" ) );
		m_VehiclesListContent 	= Widget.Cast( layoutRoot.FindAnyWidget( "vehicles_list_content" ) );

		Widget vehiclesListButtons = Widget.Cast( layoutRoot.FindAnyWidget( "vehicles_list_buttons" ) );
			Widget gridlistbtns = UIActionManager.CreateGridSpacer( vehiclesListButtons, 4, 1 );
				UIActionManager.CreateEditableTextPreview( gridlistbtns, "Search", this );
				m_RefreshButton = UIActionManager.CreateButton( gridlistbtns, "Refresh", this, "OnClick_Refresh" );
				m_DeleteAllButton = UIActionManager.CreateButton( gridlistbtns, "Delete All", this, "OnClick_DeleteAll" );
				m_DeleteDestroyedButton = UIActionManager.CreateButton( gridlistbtns, "Delete Destroyed", this, "OnClick_DeleteDestroyed" );
				m_DeleteUnclaimedButton = UIActionManager.CreateButton( gridlistbtns, "Delete Unclaimed", this, "OnClick_DeleteUnclaimed" );
		
		//! Vehicles Map
		m_MapWidgetPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicles_map_panel" ) );
		m_MapWidget = MapWidget.Cast( layoutRoot.FindAnyWidget( "vehicles_map" ) );
		
		//! Vehicle Info
		m_VehicleInfoPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicle_info_panel" ) );
			Widget gridinfos = UIActionManager.CreateGridSpacer( m_VehicleInfoPanel, 12, 1 );
				m_VehicleName = UIActionManager.CreateText( gridinfos, "Name:", "Value" );
				m_VehicleClassName = UIActionManager.CreateText( gridinfos, "ClassName:", "Value" );

				m_VehicleStatus = UIActionManager.CreateText( gridinfos, "Status:", "Value" );
				m_VehicleType = UIActionManager.CreateText( gridinfos, "Type:", "Value" );

				m_VehicleID = UIActionManager.CreateText( gridinfos, "ID:", "Value" );
				m_VehiclePersistentIDAB = UIActionManager.CreateText( gridinfos, "ID AB:", "Value" );
				m_VehiclePersistentIDCD = UIActionManager.CreateText( gridinfos, "ID CD:", "Value" );

				m_VehiclePosition = UIActionManager.CreateText( gridinfos, "Position:", "Value" );
					Widget positionGrid = UIActionManager.CreateGridSpacer( gridinfos, 1, 3 );
						UIActionManager.CreateText( positionGrid, "Copy" );
						UIActionManager.CreateText( positionGrid, "Paste" );

				m_VehicleRotation = UIActionManager.CreateText( gridinfos, "Rotation:", "Value" );
					Widget rotationGrid = UIActionManager.CreateGridSpacer( gridinfos, 1, 3 );
						UIActionManager.CreateText( rotationGrid, "Copy" );
						UIActionManager.CreateText( rotationGrid, "Paste" );

				m_VehicleKeys = UIActionManager.CreateText( gridinfos, "Keys:", "Value" );
				m_VehicleLastDriverUID = UIActionManager.CreateText( gridinfos, "Last Driver" );
					Widget driverUIDGrid = UIActionManager.CreateGridSpacer( gridinfos, 1, 3 );
						UIActionManager.CreateText( driverUIDGrid, "Name:", "Value" );
						UIActionManager.CreateText( driverUIDGrid, "SteamID:", "Value" );
						UIActionManager.CreateText( driverUIDGrid, "GUID:", "Value" );
				
				m_VehicleCovered = UIActionManager.CreateText( gridinfos, "MyText", "Value" );

		//! Vehicle Options
		m_VehicleOptionsPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicle_info_buttons_panel" ) );
			Widget gridoptions = UIActionManager.CreateGridSpacer( m_VehicleOptionsPanel, 13, 1 );
				m_DeleteVehicleButton = UIActionManager.CreateButton( gridoptions, "Delete", this, "OnClick_DeleteVehicle" );

				m_RepairVehicleButton = UIActionManager.CreateButton( gridoptions, "Repair", this, "OnClick_RepairVehicle" );
				m_UnstuckVehicleButton = UIActionManager.CreateButton( gridoptions, "Unstuck", this, "OnClick_UnstuckVehicle" );
				m_RefuelVehicleButton = UIActionManager.CreateButton( gridoptions, "Refuel", this, "OnClick_RefuelVehicle" );

				m_CoverVehicleButton = UIActionManager.CreateButton( gridoptions, "Cover", this, "OnClick_CoverVehicle" );
				m_LockVehicleButton = UIActionManager.CreateButton( gridoptions, "Lock", this, "OnClick_LockVehicle" );
				m_UnPairVehicleButton = UIActionManager.CreateButton( gridoptions, "UnPair", this, "OnClick_UnPairVehicle" );

				m_TeleportVehicleButton = UIActionManager.CreateButton( gridoptions, "TeleportToVehicle", this, "OnClick_TeleportToVehicle" );
				m_TeleportMeButton = UIActionManager.CreateButton( gridoptions, "TeleportToMe", this, "OnClick_TeleportVehicleToMe" );

				m_ReturnButton = UIActionManager.CreateButton( gridoptions, "Return", this, "OnClick_Return" );
		
		BackToList();
	}

	void LoadVehicles()
	{
		m_MapMarkers.Clear();
		m_VehicleEntries.Clear();
		
		if ( m_Module )
		{
			auto vehicles =  m_Module.GetServerVehicles();
			TStringArray vehicleEntryLabels = {};

			foreach (JMVehicleMetaData currentVehicle: vehicles)
			{
				int color;
				string marker;

				GetVehicleTypeInfo( currentVehicle.m_VehicleType, color, marker );
				
				//! Create map marker for vehicle
				if (!m_IsInVehicleInfo)
				{
					JMVehiclesMapMarker vehicleMapMarker = new JMVehiclesMapMarker( m_VehicleMapPanel, m_MapWidget, currentVehicle.m_Position, color, marker, currentVehicle, this );
					m_MapMarkers.Insert( vehicleMapMarker );
				}
				
				//! Create list entry for vehicle
				JMVehiclesListEntry vehicleListEntry = new JMVehiclesListEntry(m_VehiclesListContent, this, currentVehicle);
				m_VehicleEntries.Insert( vehicleListEntry );

				int idx = 0;
				foreach (string vehicleEntryLabel: vehicleEntryLabels)
				{
					if (JMStatics.StrCaseCmp(vehicleListEntry.GetLabel(), vehicleEntryLabel) < 0)
						break;
					idx++;
				}

				vehicleEntryLabels.InsertAt(vehicleListEntry.GetLabel(), idx);
			}

			int sort;
			foreach (auto listEntry: m_VehicleEntries)
			{
				sort = vehicleEntryLabels.Find(listEntry.GetLabel());
				if (sort > -1)
					listEntry.SetSort(sort, false);
			}
		}
	}

	private void GetVehicleTypeInfo( int type, out int color, out string marker )
	{
		switch (type)
		{
			default:
			case JMVT_CAR:
				color = ARGB( 255, 243, 156, 18 );
				marker = "Car";
			break;
			case JMVT_BOAT:
				color = ARGB( 255, 243, 18, 156 );
				marker = "Boat";
			break;
			case JMVT_HELICOPTER:
				color = ARGB( 255, 18, 156, 243 );
				marker = "Helicopter";
			break;
			case JMVT_PLANE:
				color = ARGB( 255, 18, 243, 156 );
				marker = "Plane";
			break;
			case JMVT_BIKE:
				color = ARGB( 255, 243, 156, 100 );
				marker = "Bike";
			break;
		}
	}

	void SetVehicleInfo(JMVehicleMetaData vehicle)
	{		
		m_IsInVehicleInfo = true;
		
		m_MapWidgetPanel.Show( false );
		
		HideMapMarkers();
		
		m_VehicleInfoPanel.Show( true );
		m_VehicleOptionsPanel.Show( true );
		
		m_CurrentVehicle = vehicle;

		UpdateMapPosition( false, vehicle.m_Position );
	}

	void SyncAndRefreshVehicles()
	{		
		m_Module.RequestServerVehicles();
	}

	void ShowMapMarkers()
	{
		for ( int i = 0; i < m_MapMarkers.Count(); i++ )
		{
			JMVehiclesMapMarker marker = m_MapMarkers.Get( i );
			marker.ShowMarker();
		}
	}

	void HideMapMarkers()
	{
		for ( int i = 0; i < m_MapMarkers.Count(); i++ )
		{
			JMVehiclesMapMarker marker = m_MapMarkers.Get( i );
			marker.HideMarker();
		}
	}

	void BackToList()
	{
		m_IsInVehicleInfo = false;
		
		ShowMapMarkers();
		
		m_VehicleListPanel.Show( true );
		m_MapWidgetPanel.Show( true );
		
		m_VehicleInfoPanel.Show( false );
		m_VehicleOptionsPanel.Show( false );
		
		m_CurrentVehicle = NULL;
	}

	void OnClick_CopyPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_CopyRotation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_PastePosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_PasteRotation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_CopyDriverGUID( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_CopyDriverSteamID( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_CopyDriverName( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_CoverVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_LockVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_UnpairVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_RepairVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_RefuelVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_UnstuckVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		if ( m_CurrentVehicle )
		{
			//! TODO:
		}
	}

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		SyncAndRefreshVehicles();
	}

	void OnClick_Return( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		BackToList();
	}

	void OnClick_TeleportToVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			m_Module.RequestTeleportToVehicle(m_CurrentVehicle);
	}

	void OnClick_TeleportVehicleToMe( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			m_Module.RequestTeleportVehicleToMe(m_CurrentVehicle);
	}

	void OnClick_DeleteVehicle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_CurrentVehicle )
			CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_VEHICLE_DELETE", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "OnConfirmation_DeleteVehicle" );
	}

	void OnClick_DeleteVehicleAll( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_VEHICLE_LIST_DELETE_ALL", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "OnConfirmation_DeleteVehicleAll" );
	}

	void OnClick_DeleteVehicleDestroyed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		
		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_VEHICLE_LIST_DELETE_DESTROYED", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "OnConfirmation_DeleteVehicleDestroyed" );
	}

	void OnClick_DeleteVehicleUnclaimed( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_VEHICLE_LIST_DELETE_UNCLAIMED", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "OnConfirmation_DeleteVehicleUnclaimed" );
	}

	void OnConfirmation_DeleteVehicle()
	{
		m_Module.DeleteVehicle( m_CurrentVehicle.m_NetworkIDLow, m_CurrentVehicle.m_NetworkIDHigh );
		BackToList();
	}

	void OnConfirmation_DeleteVehicleAll()
	{
		m_Module.DeleteVehicleAll();
	}

	void OnConfirmation_DeleteVehicleDestroyed()
	{
		m_Module.DeleteVehicleDestroyed();
	}

	void OnConfirmation_DeleteVehicleUnclaimed()
	{
		m_Module.DeleteVehicleUnclaimed();
	}

	override void OnShow()
	{
		super.OnShow();
		
		SyncAndRefreshVehicles();

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapPosition, 34, false, true, vector.Zero );
	}

	void UpdateMapPosition( bool usePlayerPosition, vector mapPosition = vector.Zero )
	{
		if ( usePlayerPosition )
		{
			PlayerBase player;
			float scale;
			if ( Class.CastTo( player, GetGame().GetPlayer() ) && !player.GetLastMapInfo( scale, mapPosition ) )
			{
				scale = 0.33;
				mapPosition = player.GetWorldPosition();
			}

			m_MapWidget.SetScale( scale );
		}

		m_MapWidget.SetMapPos( mapPosition );
	}
};
