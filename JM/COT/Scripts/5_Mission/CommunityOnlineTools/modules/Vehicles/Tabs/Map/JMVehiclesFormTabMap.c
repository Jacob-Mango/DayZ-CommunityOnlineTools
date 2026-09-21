//! "Map" tab of JMVehiclesForm - the world map, one marker per vehicle the
//! roster is showing, the hover readout, the selection ring and the right-click
//! marker menu. Back-reference to the owning form, same shape as
//! JMPlayerRowWidget.Menu.
//!
//! The roster (and its filters) stay on the form; it decides WHICH vehicles are
//! on screen and tells this tab to add, patch or drop the matching markers.
//! Picking a vehicle - from a marker here or a row there - goes back through
//! JMVehiclesForm.SetVehicleInfo.
class JMVehiclesFormTabMap: JMFormTab
{
	protected JMVehiclesForm m_Form;

	//! The map itself - UIActionMap, the shared component this form's own
	//! markers/hover/selection-ring were the model for. See UIActionMap and
	//! JMUIActionMapMarker.
	protected UIActionMap m_Map;

	//! Layer this form's own vehicle markers live under on the shared map -
	//! nothing else currently shares JMVehiclesForm's UIActionMap, but a name
	//! is cheap and ClearLayer needs one.
	static const string MAP_LAYER = "vehicles";

	//! Vehicle behind each live marker id, by id ("veh_" + netLow + "_" +
	//! netHigh) - UIActionMap owns the marker widgets themselves; this is only
	//! what a hover/click callback needs to get back to the JMVehicleMetaData
	//! it was raised for.
	protected ref map<string, ref JMVehicleMetaData> m_VehicleByMarkerId;

	//! Hover readout for map markers. Built on first hover, then reused - it
	//! keeps a local preview entity alive between hovers of the same class.
	protected ref JMVehiclesHoverInfo m_HoverInfo;

	//! The marker the cursor is on, and when its live state is next re-fetched.
	//! Health, fuel, speed and crew are only as fresh as the last sync, and the
	//! panel exists to show exactly those, so the hovered vehicle - and only
	//! the hovered vehicle - is polled while the panel is up.
	protected bool m_HoverActive;
	protected int  m_HoverNetLow;
	protected int  m_HoverNetHigh;
	protected int  m_HoverNextPollAt;
	static const int HOVER_POLL_MS = 1000;

	//! Right-click popup for map markers, plus the vehicle the open menu was
	//! raised on. Built on first use - most sessions never open it.
	protected UIActionContextMenu m_MarkerMenu;
	protected ref JMVehicleMetaData m_MarkerMenuVehicle;
	static const string MK_MENU_SELECT  = "select";
	static const string MK_MENU_TP_TO   = "tp_to";
	static const string MK_MENU_TP_HERE = "tp_here";
	static const string MK_MENU_TP_UNDO = "tp_undo";
	static const string MK_MENU_TP_REDO = "tp_redo";
	static const string MK_MENU_COPYPOS = "copy_pos";
	static const string MK_MENU_FOCUS   = "focus";
	static const string MK_MENU_CARGO   = "clear_cargo";
	static const string MK_MENU_KEY     = "spawn_key";
	static const string MK_MENU_REPAIR  = "repair";
	static const string MK_MENU_REFUEL  = "refuel";
	static const string MK_MENU_UNSTUCK = "unstuck";
	static const string MK_MENU_COVER   = "cover";
	static const string MK_MENU_LOCK    = "lock";
	static const string MK_MENU_UNPAIR  = "unpair";
	static const string MK_MENU_DELETE  = "delete";

	void JMVehiclesFormTabMap( JMVehiclesForm form )
	{
		m_Form = form;

		m_VehicleByMarkerId = new map<string, ref JMVehicleMetaData>;
	}

	//! Marker glyph per vehicle class. Every entry is a real texture path: the
	//! old table handed LoadImageFile bare words ("Boat", "Helicopter", ...),
	//! which resolve to nothing and draw as the engine's magenta placeholder.
	protected void GetVehicleTypeInfo( int type, out int color, out string marker )
	{
		switch ( type )
		{
			default:
			case JMVT_CAR:
				color  = ARGB( 255, 243, 156, 18 );
				marker = JMConstants.Lucide( "car" );
			break;
			case JMVT_BOAT:
				color  = ARGB( 255, 243, 18, 156 );
				marker = JMConstants.Lucide( "sailboat" );
			break;
			case JMVT_HELICOPTER:
				color  = ARGB( 255, 18, 156, 243 );
				marker = JMConstants.Lucide( "helicopter" );
			break;
			case JMVT_PLANE:
				color  = ARGB( 255, 18, 243, 156 );
				marker = JMConstants.Lucide( "plane" );
			break;
			case JMVT_BIKE:
				color  = ARGB( 255, 243, 156, 100 );
				marker = JMConstants.Lucide( "bike" );
			break;
		}
	}

	protected bool HasTeleportRedo( JMVehicleMetaData vehicle )
	{
		if ( !vehicle )
			return false;

		return JMTeleportHistory.HasRedo( JMTeleportHistory.ObjectKey( vehicle.m_NetworkIDLow, vehicle.m_NetworkIDHigh ) );
	}

	//! Whether this vehicle has a move of its own this session that can still
	//! be taken back.
	protected bool HasTeleportUndo( JMVehicleMetaData vehicle )
	{
		if ( !vehicle )
			return false;

		return JMTeleportHistory.Has( JMTeleportHistory.ObjectKey( vehicle.m_NetworkIDLow, vehicle.m_NetworkIDHigh ) );
	}

	//! The map lives in the layout because a MapWidget cannot be created from
	//! script; `panel` is that layout panel.
	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_Map = UIActionManager.CreateMapFill( panel, this, "OnClick_Map" );
	}

	override void OnUnfocus()
	{
		//! The hover panel is anchored to the window root, not to the map tab,
		//! so switching tabs does not hide it on its own.
		HideHoverInfo();
	}

	//! Marker sync runs off the window's per-frame Update() rather than a
	//! Timer, same reasoning UIActionMap.TickMarkers() itself documents: a
	//! 20 Hz Timer meant a marker only caught up with the map three frames
	//! out of four, which reads as the markers lagging behind the terrain
	//! while the map is being dragged. Per-frame is the only rate that
	//! tracks a drag, and it stays cheap because TickMarkers() skips the
	//! work whenever the view has not actually moved.
	void Tick()
	{
		//! Ahead of the marker-placement early-out below: the hover poll is
		//! what keeps the panel's speed and crew readouts honest, and a still
		//! map is exactly when a vehicle is most likely to be the thing moving.
		if ( m_HoverActive && g_Game.GetTime() >= m_HoverNextPollAt )
			PollHoveredVehicle();

		if ( !m_Map )
			return;

		//! Markers only exist on the map tab; off it there is nothing to place.
		if ( m_Form.GetActiveTabIndex() != GetTabId() )
			return;

		m_Map.TickMarkers();
	}

	// -------------------------------------------------------------------------
	//  Markers - driven by the form's roster
	// -------------------------------------------------------------------------

	static string MarkerIdFor( JMVehicleMetaData vehicle )
	{
		if ( !vehicle )
			return "";

		return "veh_" + vehicle.m_NetworkIDLow + "_" + vehicle.m_NetworkIDHigh;
	}

	//! Every marker is about to be rebuilt.
	void ClearMarkers()
	{
		//! Every marker is about to be cleared, including whichever one the
		//! cursor is on - and a cleared marker never raises the hover-leave
		//! that would take the hover panel down.
		HideHoverInfo();

		if ( m_Map )
			m_Map.ClearLayer( MAP_LAYER );

		m_VehicleByMarkerId.Clear();
	}

	void AddVehicleMarker( JMVehicleMetaData vehicle )
	{
		int color;
		string marker;
		GetVehicleTypeInfo( vehicle.m_VehicleType, color, marker );

		string markerId = MarkerIdFor( vehicle );
		m_VehicleByMarkerId.Insert( markerId, vehicle );

		if ( m_Map )
			m_Map.AddMarker( markerId, vehicle.m_Position, vehicle.m_DisplayName, color, marker, MAP_LAYER );
	}

	//! Repoint the existing marker at `fresh`. False means the vehicle has no
	//! marker here, so the caller has to rebuild.
	bool PatchVehicleMarker( JMVehicleMetaData fresh )
	{
		string markerId = MarkerIdFor( fresh );
		if ( !m_VehicleByMarkerId.Contains( markerId ) )
			return false;

		m_VehicleByMarkerId.Set( markerId, fresh );

		if ( m_Map )
		{
			int color;
			string marker;
			GetVehicleTypeInfo( fresh.m_VehicleType, color, marker );

			//! AddMarker replaces the marker already under this id in place -
			//! this is the delta-refresh path, so the id always already exists.
			m_Map.AddMarker( markerId, fresh.m_Position, fresh.m_DisplayName, color, marker, MAP_LAYER );
		}

		return true;
	}

	//! Drop the one marker for a vehicle that left the server list. False means
	//! it was never on screen to begin with.
	bool RemoveVehicleMarker( int netLow, int netHigh )
	{
		string markerId = "veh_" + netLow + "_" + netHigh;

		if ( !m_VehicleByMarkerId.Contains( markerId ) )
			return false;

		m_VehicleByMarkerId.Remove( markerId );

		if ( m_Map )
			m_Map.RemoveMarker( markerId );

		return true;
	}

	// -------------------------------------------------------------------------
	//  Selection ring / view
	// -------------------------------------------------------------------------

	//! Point UIActionMap's selection ring at whatever the form has selected.
	//! UIActionMap owns the ring widget and spins it itself from TickMarkers()
	//! (see Tick()) - this only has to say which marker it belongs on.
	void ApplySelectionRing()
	{
		if ( !m_Map )
			return;

		m_Map.SetSelectedMarker( MarkerIdFor( m_Form.GetCurrentVehicle() ) );
	}

	void UpdateMapPosition( bool usePlayerPosition, vector mapPosition = vector.Zero )
	{
		if ( !m_Map )
			return;

		if ( usePlayerPosition )
		{
			PlayerBase player;
			float scale;
			if ( Class.CastTo( player, GetGame().GetPlayer() ) && !player.GetLastMapInfo( scale, mapPosition ) )
			{
				scale = 0.33;
				mapPosition = player.GetWorldPosition();
			}

			m_Map.SetScale( scale );
		}

		m_Map.SetCenter( mapPosition );
	}

	// -------------------------------------------------------------------------
	//  Map events - hover, click, right-click, all funnelled through here from
	//  UIActionMap's single callback.
	// -------------------------------------------------------------------------

	void OnClick_Map( UIEvent eid, UIActionBase action )
	{
		if ( !m_Map )
			return;

		if ( eid == UIEvent.MOUSE_ENTER )
		{
			string hoveredId = m_Map.GetHoveredMarkerId();
			if ( m_VehicleByMarkerId.Contains( hoveredId ) )
				OnMarker_HoverEnter( m_VehicleByMarkerId.Get( hoveredId ) );
			return;
		}

		if ( eid == UIEvent.MOUSE_LEAVE )
		{
			//! UIActionMap only fires a leave for the marker it currently has
			//! hovered - a leave for one already superseded by a newer enter
			//! is swallowed there, so there is no stale-vs-fresh race to sort
			//! out here the way there was chasing raw widget events by hand.
			HideHoverInfo();
			return;
		}

		if ( eid != UIEvent.CLICK )
			return;

		string markerId = m_Map.GetLastClickedMarkerId();
		if ( !m_VehicleByMarkerId.Contains( markerId ) )
			return;

		JMVehicleMetaData vehicle = m_VehicleByMarkerId.Get( markerId );

		if ( m_Map.GetLastClickButton() == MouseState.RIGHT )
		{
			OnMarker_RightClick( vehicle );
			return;
		}

		m_Form.SetVehicleInfo( vehicle );
	}

	//! Raised via UIActionMap's UIEvent.MOUSE_ENTER (see OnClick_Map). Opens
	//! the info panel and starts polling that one vehicle's live state.
	void OnMarker_HoverEnter( JMVehicleMetaData vehicle )
	{
		CF_Window window = m_Form.GetWindow();

		if ( !vehicle || !window )
			return;

		if ( !m_HoverInfo )
			m_HoverInfo = new JMVehiclesHoverInfo( window.GetWidgetRoot() );

		int x, y;
		GetMousePos( x, y );

		m_HoverInfo.ShowAt( vehicle, x, y );

		m_HoverActive  = true;
		m_HoverNetLow  = vehicle.m_NetworkIDLow;
		m_HoverNetHigh = vehicle.m_NetworkIDHigh;

		//! The panel is up with whatever the last sync knew; ask for the real
		//! figures straight away rather than waiting out the first interval.
		PollHoveredVehicle();
	}

	void HideHoverInfo()
	{
		m_HoverActive = false;

		if ( m_HoverInfo )
			m_HoverInfo.Hide();
	}

	//! The hover panel is the reason most upserts arrive at all, so it gets
	//! the new figures whatever else a delta turns out to touch.
	void RefreshHoverInfo( JMVehicleMetaData fresh )
	{
		if ( m_HoverInfo )
			m_HoverInfo.Refresh( fresh );
	}

	//! One vehicle, once per HOVER_POLL_MS. The server answers with a normal
	//! Upsert, which lands in OnDeltaUpsert and repaints the panel.
	protected void PollHoveredVehicle()
	{
		if ( !m_Form.GetModule() )
			return;

		m_HoverNextPollAt = g_Game.GetTime() + HOVER_POLL_MS;

		m_Form.GetModule().RequestVehicleUpsert( m_HoverNetLow, m_HoverNetHigh );
	}

	// -------------------------------------------------------------------------
	//  Map marker context menu
	// -------------------------------------------------------------------------

	//! Raised via UIActionMap's right-click CLICK event (see OnClick_Map).
	//! Offers the same actions as the Actions tab against the marker under the
	//! cursor, so an admin does not have to select the vehicle and change tab
	//! first.
	void OnMarker_RightClick( JMVehicleMetaData vehicle )
	{
		if ( !vehicle )
			return;

		if ( !m_MarkerMenu )
		{
			if ( !m_Form.GetWindow() )
				return;

			//! Anchored to the window root, not to the map panel: a menu
			//! parented to the map would be clipped by it near the edges.
			m_MarkerMenu = UIActionManager.CreateOverlayMenu( m_Form, this, "OnClick_MarkerMenu" );

			if ( !m_MarkerMenu )
				return;
		}

		m_MarkerMenuVehicle = vehicle;

		m_MarkerMenu.ClearItems();

		JMVehicleMetaData current = m_Form.GetCurrentVehicle();

		bool isAlreadySelected = ( current && current.m_NetworkIDLow == vehicle.m_NetworkIDLow && current.m_NetworkIDHigh == vehicle.m_NetworkIDHigh );
		if ( !isAlreadySelected )
			m_MarkerMenu.AddItem( MK_MENU_SELECT,  "Select",                        JMConstants.Lucide( "mouse-pointer-click" ) );

		m_MarkerMenu.AddItem( MK_MENU_TP_TO,   "Teleport To Vehicle",           JMConstants.Lucide( "footprints" ) );
		m_MarkerMenu.AddItem( MK_MENU_TP_HERE, "Teleport Vehicle To Me",        JMConstants.Lucide( "move-down-left" ) );
		m_MarkerMenu.AddItem( MK_MENU_TP_UNDO, "Undo Teleport",                 JMConstants.Lucide( "undo-2" ) );
		m_MarkerMenu.AddItem( MK_MENU_TP_REDO, "Redo Teleport",                 JMConstants.Lucide( "redo-2" ) );
		m_MarkerMenu.AddItem( MK_MENU_COPYPOS, "Copy Position",                 JMConstants.Lucide( "copy" ) );
		m_MarkerMenu.AddItem( MK_MENU_FOCUS,   "Center Map Here",               JMConstants.Lucide( "locate-fixed" ) );
		m_MarkerMenu.AddItem( MK_MENU_REPAIR,  "Repair",                        JMConstants.Lucide( "wrench" ) );
		m_MarkerMenu.AddItem( MK_MENU_REFUEL,  "Refuel",                        JMConstants.Lucide( "fuel" ) );
		m_MarkerMenu.AddItem( MK_MENU_UNSTUCK, "Unstuck",                       JMConstants.Lucide( "arrow-up-from-line" ) );
		m_MarkerMenu.AddItem( MK_MENU_CARGO,   "Clear Cargo",                   JMConstants.Lucide( "package-x" ), JMTheme.DANGER );

	#ifdef EXPANSIONMODVEHICLE
		m_MarkerMenu.AddItem( MK_MENU_COVER,  "Cover/Uncover", JMConstants.Lucide( "tent" ) );
		m_MarkerMenu.AddItem( MK_MENU_LOCK,   "Lock/Unlock",   JMConstants.Lucide( "lock" ) );
		m_MarkerMenu.AddItem( MK_MENU_UNPAIR, "UnPair Keys",   JMConstants.Lucide( "key-round" ) );
		m_MarkerMenu.AddItem( MK_MENU_KEY,    "Spawn Key",     JMConstants.Lucide( "key" ) );
	#endif

		m_MarkerMenu.AddItem( MK_MENU_DELETE, "Delete", JMConstants.ICON_TRASH_CAN, JMTheme.DANGER );

		//! SetItemEnabled on an id the menu is not carrying is a no-op, so the
		//! Expansion-only entries need no special case here.
		m_MarkerMenu.SetItemEnabled( MK_MENU_TP_TO,   JMPermissions.Has( JMConstants.PERM_VEHICLES_TELEPORT ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_TP_HERE, JMPermissions.Has( JMConstants.PERM_VEHICLES_TELEPORT ) );

		//! Greyed rather than dropped when there is nothing recorded: an entry
		//! that comes and goes moves every row under it between one opening of
		//! the menu and the next.
		m_MarkerMenu.SetItemEnabled( MK_MENU_TP_UNDO, JMPermissions.Has( JMConstants.PERM_VEHICLES_TELEPORT ) && HasTeleportUndo( vehicle ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_TP_REDO, JMPermissions.Has( JMConstants.PERM_VEHICLES_TELEPORT ) && HasTeleportRedo( vehicle ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_CARGO,   JMPermissions.Has( JMConstants.PERM_VEHICLES_CLEARCARGO ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_KEY,     JMPermissions.Has( JMConstants.PERM_VEHICLES_SPAWNKEY ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_REPAIR,  JMPermissions.Has( JMConstants.PERM_VEHICLES_REPAIR ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_REFUEL,  JMPermissions.Has( JMConstants.PERM_VEHICLES_REFUEL ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_UNSTUCK, JMPermissions.Has( JMConstants.PERM_VEHICLES_UNSTUCK ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_COVER,   JMPermissions.Has( JMConstants.PERM_VEHICLES_COVER ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_LOCK,    JMPermissions.Has( JMConstants.PERM_VEHICLES_LOCK ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_UNPAIR,  JMPermissions.Has( JMConstants.PERM_VEHICLES_UNPAIR ) );
		m_MarkerMenu.SetItemEnabled( MK_MENU_DELETE,  JMPermissions.Has( JMConstants.PERM_VEHICLES_DELETE ) );

		int mx, my;
		GetMousePos( mx, my );

		m_MarkerMenu.OpenAt( mx, my );
	}

	void OnClick_MarkerMenu( UIEvent eid, UIActionBase action )
	{
		JMVehiclesModule module = m_Form.GetModule();

		if ( eid != UIEvent.CLICK || !m_MarkerMenu || !module )
			return;

		JMVehicleMetaData vehicle = m_MarkerMenuVehicle;

		if ( !vehicle )
			return;

		string id = m_MarkerMenu.GetLastClickedId();

		if ( id == MK_MENU_SELECT )
		{
			m_Form.SetVehicleInfo( vehicle );
			return;
		}

		if ( id == MK_MENU_TP_TO )
		{
			module.RequestTeleportToVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_TP_HERE )
		{
			module.RequestTeleportVehicleToMe( vehicle );
			return;
		}

		if ( id == MK_MENU_TP_UNDO )
		{
			UndoTeleport( vehicle );
			return;
		}

		if ( id == MK_MENU_TP_REDO )
		{
			RedoTeleport( vehicle );
			return;
		}

		if ( id == MK_MENU_COPYPOS )
		{
			//! Same "<x, y, z>" shape every other copy in COT writes, so it
			//! pastes into the player manager's coordinate box or an ESP paste.
			COTFeedback.Copy( "<" + vehicle.m_Position[0] + ", " + vehicle.m_Position[1] + ", " + vehicle.m_Position[2] + ">" );
			return;
		}

		if ( id == MK_MENU_FOCUS )
		{
			UpdateMapPosition( false, vehicle.m_Position );
			return;
		}

		if ( id == MK_MENU_CARGO )
		{
			if ( JMPermissions.Has( JMConstants.PERM_VEHICLES_CLEARCARGO ) )
				module.RequestClearVehicleCargo( vehicle );
			return;
		}

		if ( id == MK_MENU_KEY )
		{
			if ( JMPermissions.Has( JMConstants.PERM_VEHICLES_SPAWNKEY ) )
				module.RequestSpawnVehicleKey( vehicle );
			return;
		}

		if ( id == MK_MENU_REPAIR )
		{
			module.RequestRepairVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_REFUEL )
		{
			module.RequestRefuelVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_UNSTUCK )
		{
			module.RequestUnstuckVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_COVER )
		{
			module.RequestCoverVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_LOCK )
		{
			module.RequestLockVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_UNPAIR )
		{
			module.RequestUnPairVehicle( vehicle );
			return;
		}

		if ( id == MK_MENU_DELETE )
		{
			if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_DELETE ) )
				return;

			m_Form.RequestDeleteVehicle( vehicle );
			return;
		}
	}

	//! Put back a move an undo took away.
	//!
	//! Goes out through the same RPC the undo does - the two differ only in
	//! which stack they read, which is what keeps them exact mirrors.
	protected void RedoTeleport( JMVehicleMetaData vehicle )
	{
		JMVehiclesModule module = m_Form.GetModule();

		if ( !vehicle || !module )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_TELEPORT ) )
			return;

		vector forward = JMTeleportHistory.PopRedo( JMTeleportHistory.ObjectKey( vehicle.m_NetworkIDLow, vehicle.m_NetworkIDHigh ), 0, vehicle.m_Position );

		if ( forward == vector.Zero )
			return;

		JMTeleportHistory.BeginApply();
		module.RequestTeleportVehicleTo( vehicle, forward );
		JMTeleportHistory.EndApply();
	}

	//! Put the vehicle back where it was before its last move. Each use walks
	//! one more step back, up to JMTeleportHistory.MAX_ENTRIES.
	protected void UndoTeleport( JMVehicleMetaData vehicle )
	{
		JMVehiclesModule module = m_Form.GetModule();

		if ( !vehicle || !module )
			return;

		if ( !JMPermissions.Has( JMConstants.PERM_VEHICLES_TELEPORT ) )
			return;

		vector back = JMTeleportHistory.Pop( JMTeleportHistory.ObjectKey( vehicle.m_NetworkIDLow, vehicle.m_NetworkIDHigh ), 0, vehicle.m_Position );

		if ( back == vector.Zero )
			return;

		JMTeleportHistory.BeginApply();
		module.RequestTeleportVehicleTo( vehicle, back );
		JMTeleportHistory.EndApply();
	}
}
