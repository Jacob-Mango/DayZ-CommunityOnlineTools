class JMVehiclesMenu: JMFormBase
{
	private JMVehiclesModule m_Module;
	
	protected Widget m_VehicleMapPanel;	
	protected Widget m_VehicleListPanel;
	protected Widget m_VehiclesListContent;
	protected ButtonWidget m_VehicleListRefreshButton;
	protected TextWidget m_VehicleListRefreshButtonLabel;
	
	protected Widget m_MapWidgetPanel;
	protected MapWidget m_MapWidget;
	
	protected ButtonWidget m_DeleteUnclaimedButton;
	protected TextWidget m_DeleteUnclaimedButtonLabel;
	protected ButtonWidget m_DeleteDestroyedButton;
	protected TextWidget m_DeleteDestroyedButtonLabel;
	protected ButtonWidget m_DeleteAllButton;
	protected TextWidget m_DeleteAllButtonLabel;
	
	protected Widget m_VehicleInfoPanel;
	protected TextWidget m_VehicleInfoNetworkID;
	protected TextWidget m_VehicleInfoPersistentABID;
	protected TextWidget m_VehicleInfoPersistentCDID;
	protected TextWidget m_VehicleInfoName;
	protected TextWidget m_VehicleInfoClassName;
	protected TextWidget m_VehicleInfoPos;
	protected TextWidget m_VehicleInfoOri;
	protected TextWidget m_VehicleInfoType;
	protected TextWidget m_VehicleInfoExploded;
	protected TextWidget m_VehicleInfoDestroyed;
	protected TextWidget m_VehicleInfoKeys;
	protected TextWidget m_VehicleInfoCover;
	protected TextWidget m_VehicleInfoOwner;
	protected TextWidget m_VehicleInfoLastDriver;	
	
	protected Widget m_VehicleOptionsPanel;
	protected ButtonWidget m_DeleteVehicleButton;
	protected TextWidget m_DeleteVehicleButtonLabel;
	protected ButtonWidget m_CancleVehicleEdit;
	protected TextWidget m_CancleVehicleEditLabel;
	protected ButtonWidget m_TeleportButton;
	protected TextWidget m_TeleportButtonLabel;
	
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
		m_VehicleListPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicles_list_panel" ) );
		m_VehiclesListContent = Widget.Cast( layoutRoot.FindAnyWidget( "vehicles_list_content" ) );
		
		//! Vehicles Map
		m_MapWidgetPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicles_map_panel" ) );
		m_MapWidget = MapWidget.Cast( layoutRoot.FindAnyWidget( "vehicles_map" ) );
		
		//! Vehicles Options
		m_VehicleListRefreshButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "vehicles_refresh_button" ) );
		m_VehicleListRefreshButtonLabel = TextWidget.Cast( layoutRoot.FindAnyWidget( "vehicles_refresh_button_label" ) );
		m_DeleteUnclaimedButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "delete_unclaimed_vehicle_button" ) );
		m_DeleteUnclaimedButtonLabel = TextWidget.Cast( layoutRoot.FindAnyWidget( "delete_unclaimed_vehicle_button_label" ) );
		#ifdef EXPANSIONMODVEHICLE
		m_DeleteUnclaimedButton.Show(true);
		#endif
		m_DeleteDestroyedButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "delete_destroyed_vehicle_button" ) );
		m_DeleteDestroyedButtonLabel = TextWidget.Cast( layoutRoot.FindAnyWidget( "delete_destroyed_vehicle_button_label" ) );
		m_DeleteAllButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "delete_all_button" ) );
		m_DeleteAllButtonLabel = TextWidget.Cast( layoutRoot.FindAnyWidget( "delete_all_button_label" ) );
		
		//! Vehicle Info
		m_VehicleInfoPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicle_info_panel" ) );
		m_VehicleInfoNetworkID = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_network_id_value" ) );
		m_VehicleInfoPersistentABID = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_persistent_ab_id_value" ) );
		m_VehicleInfoPersistentCDID = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_persistent_cd_id_value" ) );
		m_VehicleInfoName = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_name_value" ) );
		m_VehicleInfoClassName = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_classname_value" ) );
		m_VehicleInfoPos = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_pos_value" ) );
		m_VehicleInfoOri = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_ori_value" ) );
		m_VehicleInfoType = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_type_value" ) );
		m_VehicleInfoExploded = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_exploded_value" ) );
		m_VehicleInfoDestroyed = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_destroyed_value" ) );
		m_VehicleInfoKeys = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_keys_value" ) );
		m_VehicleInfoCover = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_cover_value" ) );
		m_VehicleInfoOwner = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_owner_value" ) );
		m_VehicleInfoLastDriver = TextWidget.Cast( layoutRoot.FindAnyWidget( "info_last_driver_value" ) );
		#ifdef EXPANSIONMODVEHICLE
		m_VehicleInfoType.GetParent().Show(true);
		m_VehicleInfoExploded.GetParent().Show(true);
		m_VehicleInfoKeys.GetParent().Show(true);
		m_VehicleInfoCover.GetParent().Show(true);
		m_VehicleInfoOwner.GetParent().Show(true);
		#endif
		#ifdef EXPANSIONMODCORE
		m_VehicleInfoLastDriver.GetParent().Show(true);
		#endif
		
		//! Vehicle Options
		m_VehicleOptionsPanel = Widget.Cast( layoutRoot.FindAnyWidget( "vehicle_info_buttons_panel" ) );
		m_DeleteVehicleButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "delete_vehicle_button" ) );
		m_DeleteVehicleButtonLabel = TextWidget.Cast( layoutRoot.FindAnyWidget( "delete_vehicle_button_label" ) );
		m_TeleportButton = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "teleport_to_button" ) );
		m_TeleportButtonLabel = TextWidget.Cast( layoutRoot.FindAnyWidget( "teleport_to_button_label" ) );
		m_CancleVehicleEdit = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "cancel_edit_button" ) );
		m_CancleVehicleEditLabel = TextWidget.Cast( layoutRoot.FindAnyWidget( "cancel_edit_button_label" ) );
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
		color = ARGB( 255, 243, 156, 18 );
	}

	void SetVehicleInfo(JMVehicleMetaData vehicle)
	{		
		m_IsInVehicleInfo = true;
		
		m_MapWidgetPanel.Show( false );
		
		HideMapMarkers();
		
		m_VehicleInfoPanel.Show( true );
		m_VehicleOptionsPanel.Show( true );
		
		m_VehicleInfoNetworkID.SetText( vehicle.m_NetworkIDHigh.ToString() + "    " + vehicle.m_NetworkIDLow.ToString() );
		m_VehicleInfoPersistentABID.SetText( vehicle.m_PersistentIDA.ToString() + "    " + vehicle.m_PersistentIDB.ToString() );
		m_VehicleInfoPersistentCDID.SetText( vehicle.m_PersistentIDC.ToString() + "    " + vehicle.m_PersistentIDD.ToString() );

		m_VehicleInfoName.SetText( vehicle.m_DisplayName );

		m_VehicleInfoClassName.SetText( vehicle.m_ClassName );
		m_VehicleInfoPos.SetText( "X: " + vehicle.m_Position[0] + " Z: " + vehicle.m_Position[1] + " Y: " + vehicle.m_Position[2] );
		m_VehicleInfoOri.SetText( "Yaw: " + vehicle.m_Orientation[0] + " Pitch: " + vehicle.m_Orientation[1] + " Roll: " + vehicle.m_Orientation[2] );
		#ifdef EXPANSIONMODVEHICLE
		m_VehicleInfoType.SetText( vehicle.GetVehicleType() );
		m_VehicleInfoExploded.SetText( vehicle.IsExploded().ToString() );
		#endif
		m_VehicleInfoDestroyed.SetText( vehicle.IsDestroyed().ToString() );
		#ifdef EXPANSIONMODVEHICLE
		m_VehicleInfoKeys.SetText( vehicle.m_HasKeys.ToString() );
		m_VehicleInfoCover.SetText( vehicle.m_IsCover.ToString() );
		if (vehicle.m_OwnerName)
			m_VehicleInfoOwner.SetText( string.Format("%1 | %2", vehicle.m_OwnerName, vehicle.m_OwnerUID) );
		else
			m_VehicleInfoOwner.SetText( vehicle.m_OwnerUID );
		#endif
		#ifdef EXPANSIONMODCORE
		string lastDriverName;
		#ifdef EXPANSIONMODVEHICLE
		if (vehicle.m_LastDriverUID == vehicle.m_OwnerUID)
			lastDriverName = vehicle.m_OwnerName;
		#endif
		if (!lastDriverName)
		{
			PlayerBase player = PlayerBase.GetPlayerByUID(vehicle.m_LastDriverUID);
			if (player)
				lastDriverName = player.GetIdentityName();
		}
		if (lastDriverName)
			m_VehicleInfoLastDriver.SetText( string.Format("%1 | %2", lastDriverName, vehicle.m_LastDriverUID) );
		else
			m_VehicleInfoLastDriver.SetText( vehicle.m_LastDriverUID );
		#endif
		
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
		
		m_VehicleInfoNetworkID.SetText( "" );
		m_VehicleInfoPersistentABID.SetText( "" );
		m_VehicleInfoPersistentCDID.SetText( "" );
		m_VehicleInfoName.SetText( "" );
		m_VehicleInfoClassName.SetText( "" );
		m_VehicleInfoPos.SetText( "" );
		m_VehicleInfoOri.SetText( "" );
		m_VehicleInfoType.SetText( "" );
		m_VehicleInfoExploded.SetText( "" );
		m_VehicleInfoDestroyed.SetText( "" );
		m_VehicleInfoKeys.SetText( "" );
		m_VehicleInfoCover.SetText( "" );
		m_VehicleInfoLastDriver.SetText( "" );
		
		m_CurrentVehicle = NULL;
	}
		
	override bool OnClick( Widget w, int x, int y, int button )
	{		
		if ( w == m_VehicleListRefreshButton )
		{
			SyncAndRefreshVehicles();
		}
		
		if ( w == m_CancleVehicleEdit )
		{
			BackToList();
		}

		if ( w == m_DeleteAllButton )
		{
			CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_VEHICLE_LIST_DELETE_ALL", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "DeleteVehicleAll" );
		}

		if ( w == m_DeleteDestroyedButton )
		{
			CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_VEHICLE_LIST_DELETE_DESTROYED", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "DeleteVehicleDestroyed" );
		}

		if ( w == m_DeleteUnclaimedButton )
		{
			CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_VEHICLE_LIST_DELETE_UNCLAIMED", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "DeleteVehicleUnclaimed" );
		}

		if ( w == m_DeleteVehicleButton )
		{
			if ( m_CurrentVehicle )
			{
				CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_VEHICLE_DELETE", "", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "DeleteVehicle" );
			}
		}

		if ( w == m_TeleportButton )
		{
			if ( m_CurrentVehicle )
			{
				TeleportToVehicle();	
			}	
		}
		
		return false;
	}

	void DeleteVehicle(JMConfirmation confirmation)
	{
		m_Module.DeleteVehicle(m_CurrentVehicle);
		BackToList();
	}

	void DeleteVehicleUnclaimed(JMConfirmation confirmation)
	{
		m_Module.DeleteVehicleUnclaimed();
	}

	void DeleteVehicleDestroyed(JMConfirmation confirmation)
	{
		m_Module.DeleteVehicleDestroyed();
	}

	void DeleteVehicleAll(JMConfirmation confirmation)
	{
		m_Module.DeleteVehicleAll();
	}

	override void OnShow()
	{
		super.OnShow();
		
		SyncAndRefreshVehicles();

		GetGame().GetCallQueue( CALL_CATEGORY_GUI ).CallLater( UpdateMapPosition, 34, false, true, vector.Zero );
	}

	void TeleportToVehicle()
	{
		m_Module.RequestTeleportToVehicle(m_CurrentVehicle);
	}

	void TeleportToVehicle(int netLow, int netHigh)
	{
		m_Module.RequestTeleportToVehicle(netLow, netHigh);
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (w == m_DeleteUnclaimedButton)
		{
			m_DeleteUnclaimedButtonLabel.SetColor( ARGB( 255,0,0,0 ) );		
		} else if (w == m_DeleteDestroyedButton)
		{
			m_DeleteDestroyedButtonLabel.SetColor( ARGB( 255,0,0,0 ) );
		} else if (w == m_DeleteAllButton)
		{
			m_DeleteAllButtonLabel.SetColor( ARGB( 255,0,0,0 ) );
		} else if (w == m_DeleteVehicleButton)
		{
			m_DeleteVehicleButtonLabel.SetColor( ARGB( 255,0,0,0 ) );
		} else if (w == m_CancleVehicleEdit)
		{
			m_CancleVehicleEditLabel.SetColor( ARGB( 255,0,0,0 ) );
		} else if (w == m_TeleportButton)
		{
			m_TeleportButtonLabel.SetColor( ARGB( 255,0,0,0 ) );
		} else if (w == m_VehicleListRefreshButton)
		{
			m_VehicleListRefreshButtonLabel.SetColor( ARGB( 255,0,0,0 ) );
		}
		
		return false;
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (w == m_DeleteUnclaimedButton)
		{
			m_DeleteUnclaimedButtonLabel.SetColor( ARGB( 255,255,255,255 ) );		
		} else if (w == m_DeleteDestroyedButton)
		{
			m_DeleteDestroyedButtonLabel.SetColor( ARGB( 255,255,255,255 ) );
		} else if (w == m_DeleteAllButton)
		{
			m_DeleteAllButtonLabel.SetColor( ARGB( 255,255,255,255 ) );
		} else if (w == m_DeleteVehicleButton)
		{
			m_DeleteVehicleButtonLabel.SetColor( ARGB( 255,255,255,255 ) );
		} else if (w == m_CancleVehicleEdit)
		{
			m_CancleVehicleEditLabel.SetColor( ARGB( 255,255,255,255 ) );
		} else if (w == m_TeleportButton)
		{
			m_TeleportButtonLabel.SetColor( ARGB( 255,255,255,255 ) );
		} else if (w == m_VehicleListRefreshButton)
		{
			m_VehicleListRefreshButtonLabel.SetColor( ARGB( 255,255,255,255 ) );
		}
		
		return false;
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
