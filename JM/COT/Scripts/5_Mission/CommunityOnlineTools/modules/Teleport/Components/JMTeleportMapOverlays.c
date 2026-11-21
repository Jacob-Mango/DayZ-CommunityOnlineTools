//! The live layers the Teleport map draws over its locations: players, vehicles, custom map
//! markers, heli crashes and toxic zones, plus the vehicle hover card. The form decides WHEN
//! to refresh (RefreshOverlays); each method here draws one layer. RefreshMapMarkers stays an
//! overridable hook on the form so sub-mods can add their own markers (see Ex_FilterTeleportMenu.c).
class JMTeleportMapOverlays
{
	protected JMTeleportForm m_Form;
	protected JMVehiclesModule m_VehiclesModule;

	//! Marker id -> metadata for whatever the overlay last drew, so a map
	//! hover can look a vehicle up without re-walking the server list.
	protected ref map<string, ref JMVehicleMetaData> m_VehicleByMarkerId = new map<string, ref JMVehicleMetaData>;

	//! Same hover card the Vehicle Manager map uses - built lazily so a
	//! session that never hovers a vehicle here never pays for it.
	protected ref JMVehiclesHoverInfo m_VehicleHoverInfo;

	void JMTeleportMapOverlays( JMTeleportForm form )
	{
		m_Form = form;
	}

	JMVehiclesModule VehiclesModule()
	{
		if ( !m_VehiclesModule )
			Class.CastTo( m_VehiclesModule, GetModuleManager().GetModule( JMVehiclesModule ) );

		return m_VehiclesModule;
	}

	void RequestVehicles()
	{
		if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW ) )
			return;

		JMVehiclesModule vehicles = VehiclesModule();

		if ( !vehicles )
			return;

		vehicles.RequestServerVehicles();
	}

	void RefreshPlayerMarkers()
	{
		if ( !JMPermissions.Has( JMConstants.PERM_MAP_PLAYERS ) )
			return;

		array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

		if ( !players )
			return;

		//! One redraw for the whole layer instead of one per player.
		m_Form.GetMap().BeginBatch();
		m_Form.GetMap().ClearLayer( JMTeleportForm.MAP_LAYER_PLAYERS );

		for ( int i = 0; i < players.Count(); i++ )
		{
			JMPlayerInstance player = players[i];

			if ( !player )
				continue;

			if ( !m_Form.GetFilters().IsPlayerVisibleByRole( player ) )
				continue;

			int color = JMTeleportForm.COLOR_PLAYER;

			if ( JM_GetSelected().IsSelected( player.GetGUID() ) )
				color = JMTeleportForm.COLOR_PLAYER_SELECTED;
			else if ( player.HasPermission( "COT" ) )
				color = JMTeleportForm.COLOR_PLAYER_ADMIN;

			m_Form.GetMap().AddMarker( "tp_player_" + player.GetGUID(), player.GetPosition(), player.GetName(), color, UIActionMap.ICON_DOT, JMTeleportForm.MAP_LAYER_PLAYERS );
		}

		m_Form.GetMap().EndBatch();

		//! Positions only arrive because something asked for them.
		GetCommunityOnlineTools().RefreshClientPositions();
	}

	void RefreshCustomMarkers()
	{
		if ( !m_Form.GetMap() )
			return;

		m_Form.GetMap().BeginBatch();
		m_Form.GetMap().ClearLayer( JMTeleportForm.MAP_LAYER_MARKERS );

		array< ref JMTeleportMapEntry > entries;
		m_Form.CollectMapMarkerEntries( entries );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			JMTeleportMapEntry entry = entries[i];
			m_Form.GetMap().AddMarker( "tp_" + entry.m_Id, entry.m_Position, entry.m_Label, entry.m_Color, entry.m_Icon, JMTeleportForm.MAP_LAYER_MARKERS );
		}

		m_Form.GetMap().EndBatch();
	}

	void RefreshHeliCrashMarkers()
	{
		if ( !m_Form.GetMap() )
			return;

		m_Form.GetMap().BeginBatch();
		m_Form.GetMap().ClearLayer( JMTeleportForm.MAP_LAYER_HELICRASH );

		array< ref JMTeleportMapEntry > entries;
		m_Form.CollectHeliCrashEntries( entries );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			JMTeleportMapEntry entry = entries[i];
			m_Form.GetMap().AddMarker( "tp_" + entry.m_Id, entry.m_Position, entry.m_Label, entry.m_Color, entry.m_Icon, JMTeleportForm.MAP_LAYER_HELICRASH );
		}

		m_Form.GetMap().EndBatch();
	}

	void RefreshToxicZoneMarkers()
	{
		if ( !m_Form.GetMap() )
			return;

		m_Form.GetMap().BeginBatch();
		m_Form.GetMap().ClearLayer( JMTeleportForm.MAP_LAYER_TOXICZONES );

		array< ref JMTeleportMapEntry > entries;
		m_Form.CollectToxicZoneEntries( entries );

		for ( int i = 0; i < entries.Count(); i++ )
		{
			JMTeleportMapEntry entry = entries[i];
			m_Form.GetMap().AddMarker( "tp_" + entry.m_Id, entry.m_Position, entry.m_Label, entry.m_Color, entry.m_Icon, JMTeleportForm.MAP_LAYER_TOXICZONES );
		}

		m_Form.GetMap().EndBatch();
	}

	//! Raised via UIActionMap's UIEvent.MOUSE_ENTER (see OnClick_Map). Static
	//! snapshot only, unlike the Vehicle Manager's own hover which polls the
	//! server for live figures - this overlay is a teleport aid, not a status
	//! panel, so the metadata already on hand is enough.
	void ShowVehicleHoverInfo( JMVehicleMetaData vehicle )
	{
		if ( !vehicle || !m_Form.GetWindow() )
			return;

		if ( !m_VehicleHoverInfo )
			m_VehicleHoverInfo = new JMVehiclesHoverInfo( m_Form.GetWindow().GetWidgetRoot() );

		int x, y;
		GetMousePos( x, y );

		m_VehicleHoverInfo.ShowAt( vehicle, x, y );
	}

	void HideVehicleHoverInfo()
	{
		if ( m_VehicleHoverInfo )
			m_VehicleHoverInfo.Hide();
	}

	void RefreshVehicleMarkers()
	{
		if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_VIEW ) )
			return;

		JMVehiclesModule vehicles = VehiclesModule();

		if ( !vehicles )
			return;

		array< ref JMVehicleMetaData > list = vehicles.GetServerVehicles();

		if ( !list )
			return;

		m_Form.GetMap().BeginBatch();
		m_Form.GetMap().ClearLayer( JMTeleportForm.MAP_LAYER_VEHICLES );
		m_VehicleByMarkerId.Clear();

		for ( int i = 0; i < list.Count(); i++ )
		{
			JMVehicleMetaData meta = list[i];

			if ( !meta )
				continue;

			//! A recognized type answers to its own checkbox; a vehicle
			//! carrying none of the known bits (modded, unclassified) shows
			//! whenever the vehicle layer is on at all - same as before this
			//! had per-type rows.
			if ( meta.m_VehicleType != 0 && ( meta.m_VehicleType & m_Form.GetFilters().GetVehicleTypeFilter() ) == 0 )
				continue;

			//! Indexed rather than keyed on the network id: the ids are a pair
			//! of ints, and the layer is replaced whole on every refresh, so
			//! nothing outside this loop ever asks for one of these by name.
			string vehMarkerId = "tp_veh_" + i;
			m_Form.GetMap().AddMarker( vehMarkerId, meta.m_Position, meta.m_DisplayName, VehicleColorFor( meta.m_VehicleType ), VehicleIconFor( meta.m_VehicleType ), JMTeleportForm.MAP_LAYER_VEHICLES );
			m_VehicleByMarkerId.Insert( vehMarkerId, meta );
		}

		m_Form.GetMap().EndBatch();
	}

	//! Same colours the vehicle form's own map uses, so a helicopter is the
	//! same blue wherever it is drawn.
	protected int VehicleColorFor( int vehicleType )
	{
		if ( vehicleType == JMVT_BOAT )
			return ARGB( 255, 243, 18, 156 );

		if ( vehicleType == JMVT_HELICOPTER )
			return ARGB( 255, 18, 156, 243 );

		if ( vehicleType == JMVT_PLANE )
			return ARGB( 255, 18, 243, 156 );

		if ( vehicleType == JMVT_BIKE )
			return ARGB( 255, 243, 156, 100 );

		return ARGB( 255, 243, 156, 18 );
	}

	//! Same glyphs the vehicle form's map uses, so a boat is a boat on both maps.
	protected string VehicleIconFor( int vehicleType )
	{
		if ( vehicleType == JMVT_BOAT )
			return JMConstants.Lucide( "sailboat" );

		if ( vehicleType == JMVT_HELICOPTER )
			return JMConstants.Lucide( "helicopter" );

		if ( vehicleType == JMVT_PLANE )
			return JMConstants.Lucide( "plane" );

		if ( vehicleType == JMVT_BIKE )
			return JMConstants.Lucide( "bike" );

		return UIActionMap.ICON_CAR;
	}

	//! Show the vehicle hover card for a map marker id, if that marker is a vehicle.
	void ShowHoverFor( string markerId )
	{
		if ( m_VehicleByMarkerId.Contains( markerId ) )
			ShowVehicleHoverInfo( m_VehicleByMarkerId.Get( markerId ) );
	}
}
