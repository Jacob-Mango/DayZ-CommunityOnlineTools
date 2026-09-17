// =============================================================================
//  UIActionMap
//
//  A world map any form can drop into a card, with markers it OWNS rather than
//  redraws by hand.
//
//  ---------------------------------------------------------------------------
//  WHY THIS EXISTS
//
//  Every form that wanted a map was re-implementing the same three things:
//  create the MapWidget from a layout, keep a private list of what to draw, and
//  call ClearUserMarks + AddUserMark over the whole list whenever anything
//  moved. MapWidget offers no way to move or remove ONE mark - the only API is
//  "erase everything and draw it again" - so each host also had to own every
//  mark on its map, which is exactly why two features could never share one.
//
//  This holds the marks instead. A marker has an id and a LAYER, so the player
//  module can own "players", loot analysis can own "loot", and the teleport
//  form can own "locations", each adding, moving and clearing its own without
//  touching anyone else's.
//
//  ---------------------------------------------------------------------------
//  MARKERS ARE REAL WIDGETS, NOT NATIVE MARKS
//
//  This used to draw with MapWidget's own AddUserMark/ClearUserMarks - cheap,
//  but a native mark cannot be hovered, cannot show a selection ring, and
//  resolves .paa icons only. Vehicle Manager had already solved all three with
//  JMUIActionMapMarker's ancestor (JMVehiclesMapMarker): a small transparent
//  ButtonWidget sibling of the MapWidget, positioned every tick with
//  MapToScreen. That is what a marker is now, mod-wide, via JMUIActionMapMarker
//  - the price is TickMarkers(), which the host must call once a frame (see
//  below), where a native mark tracked pan/zoom for free.
//
//  ClearLayer/refresh-heavy hosts (the teleport form's player/vehicle overlays,
//  refreshed every few seconds) would otherwise destroy and recreate every
//  widget in a layer on every refresh even when nothing moved. Cleared markers'
//  widgets are PARKED instead of destroyed immediately, and only actually
//  unlinked a frame later - so an AddMarker for the same id within that window
//  (the normal "clear then re-add the current list" pattern) reclaims the same
//  widget instead of thrashing it.
//
//  ---------------------------------------------------------------------------
//  INTERACTION
//
//  Fires UIEvent.CLICK on a press and UIEvent.DOUBLE_CLICK on a double press,
//  whether it lands on open ground or directly on a marker widget. The host
//  reads what was hit off the action the same way either way:
//
//      GetLastClickWorldPos()   - where on the ground, always set
//      GetLastClickedMarkerId() - "" unless the press landed on a marker
//      GetLastClickButton()     - MouseState.LEFT or MouseState.RIGHT
//
//  Also fires UIEvent.MOUSE_ENTER / MOUSE_LEAVE as the cursor crosses a marker
//  - GetHoveredMarkerId() reads back which one, same shape as
//  UIActionDataTable's GetHoveredRow().
//
//  Usage:
//      m_Map = UIActionManager.CreateMap( card.GetContent(), this, "OnClick_Map" );
//      m_Map.CenterOn( g_Game.GetPlayer().GetPosition() );
//      m_Map.AddMarker( "loc_nwaf", pos, "NWAF", JMTheme.ACCENT, "", "locations" );
//      ...
//      m_Map.ClearLayer( "locations" );
//
//  And, once a frame, from the host's own Update():
//      m_Map.TickMarkers();
// =============================================================================

//! One mark on the map. Held rather than drawn straight through, because
//! MapWidget cannot edit or erase an individual mark once it has it.
class JMMapMarker
{
	string Id;
	//! Which feature owns this marker. ClearLayer erases one feature's marks
	//! and leaves everyone else's alone.
	string Layer;
	string Label;
	vector Position;
	int    Color;
	//! Named IconPath, not Icon: Icon is an engine type name and Enforce
	//! refuses a member that shadows one.
	string IconPath;

	void JMMapMarker( string id, vector position, string label, int color, string icon, string layer )
	{
		Id       = id;
		Position = position;
		Label    = label;
		Color    = color;
		IconPath = icon;
		Layer    = layer;
	}
}

class UIActionMap: UIActionBase
{
	protected MapWidget m_Map;
	//! Parent for marker widgets - the panel action_map itself sits in, not
	//! the MapWidget: a widget parented to a MapWidget comes back blank.
	protected Widget m_MapHost;

	protected ref array<ref JMMapMarker>       m_Markers;
	protected ref map<string, ref JMMapMarker> m_ById;

	//! One live widget per marker id, plus whatever ClearLayer parked this
	//! frame pending FlushParkedWidgets.
	protected ref map<string, ref JMUIActionMapMarker> m_Widgets;
	protected ref map<string, ref JMUIActionMapMarker> m_ParkedWidgets;
	protected bool m_FlushScheduled;

	protected string m_SelectedId;
	protected string m_HoveredMarkerId;

	//! Reprojection dirty-check, same two-probe trick JMVehiclesForm used: a
	//! single point cannot tell a zoom centred on that point from no change
	//! at all.
	protected vector m_TickProbeA;
	protected vector m_TickProbeB;

	//! Taken straight off the clock rather than accumulated, so the ring
	//! cannot drift, stall on a dropped frame, or double up if two ticks land
	//! in the same one. Matches JMVehiclesForm's own selection ring.
	static const int RING_PERIOD_MS = 7000;

	protected string m_LastClickedMarkerId;
	protected vector m_LastClickWorld;
	protected int    m_LastClickButton;

	//! Target of a CenterOnDeferred still waiting for its second attempt.
	protected vector m_DeferredCenter;

	//! Zoom the map opens at. Matches what the player form's position map used,
	//! which is close enough to read a town on and wide enough to place one.
	static const float DEFAULT_SCALE = 0.15;

	//! Screen-pixel radius a press has to land within to count as hitting a
	//! marker rather than the ground under it. Only matters as a fallback now
	//! that a marker's own widget usually catches the press first - see
	//! MarkerAtScreen.
	static const float MARKER_HIT_PX = 14.0;

	//! Drawn for a marker that named no colour of its own.
	static const int COLOR_DEFAULT  = JMTheme.ACCENT;
	//! The one marker the host has called selected, so the map agrees with
	//! whatever list is beside it.
	static const int COLOR_SELECTED = 0xFFFFFF00;

	//! Marker glyphs - all JMConstants.Lucide() .edds icons, loaded the same
	//! way Vehicle Manager's markers always have.
	//!
	//! These used to be DayZ's own native navigation .paa set (map_camp_ca and
	//! the like) plus a couple of hand-authored COT .paa files, back when
	//! AddUserMark drew them - AddUserMark's mark renderer is the only thing
	//! that ever knew how to read that particular packing. Loaded through a
	//! plain ImageWidget the way every marker is now, they come out as a flat
	//! colour square instead of a glyph, so they are mapped to the nearest
	//! Lucide equivalent here instead.
	static const string ICON_DOT          = "JM/COT/GUI/textures/icons/lucide/dot.edds";
	static const string ICON_DOT_SELECTED = "JM/COT/GUI/textures/icons/lucide/circle-dot.edds";
	static const string ICON_CAR          = "JM/COT/GUI/textures/icons/lucide/car.edds";

	static const string ICON_BORDER_CROSS = "JM/COT/GUI/textures/icons/lucide/milestone.edds";
	static const string ICON_BROADLEAF    = "JM/COT/GUI/textures/icons/lucide/tree-deciduous.edds";
	static const string ICON_CAMP         = "JM/COT/GUI/textures/icons/lucide/tent.edds";
	static const string ICON_FACTORY      = "JM/COT/GUI/textures/icons/lucide/factory.edds";
	static const string ICON_FIR          = "JM/COT/GUI/textures/icons/lucide/tree-pine.edds";
	static const string ICON_FIREDEP      = "JM/COT/GUI/textures/icons/lucide/flame.edds";
	static const string ICON_GOVOFFICE    = "JM/COT/GUI/textures/icons/lucide/landmark.edds";
	static const string ICON_HILL         = "JM/COT/GUI/textures/icons/lucide/mountain.edds";
	static const string ICON_MONUMENT     = "JM/COT/GUI/textures/icons/lucide/castle.edds";
	static const string ICON_PALM         = "JM/COT/GUI/textures/icons/lucide/palmtree.edds";
	static const string ICON_POLICE       = "JM/COT/GUI/textures/icons/lucide/shield.edds";
	static const string ICON_STATION      = "JM/COT/GUI/textures/icons/lucide/train-front.edds";
	static const string ICON_STORE        = "JM/COT/GUI/textures/icons/lucide/store.edds";
	static const string ICON_TOURISM      = "JM/COT/GUI/textures/icons/lucide/umbrella.edds";
	static const string ICON_TRANSMITTER  = "JM/COT/GUI/textures/icons/lucide/radio-tower.edds";
	static const string ICON_TSHELTER     = "JM/COT/GUI/textures/icons/lucide/flag.edds";
	static const string ICON_TSIGN        = "JM/COT/GUI/textures/icons/lucide/signpost.edds";
	static const string ICON_VIEWPOINT    = "JM/COT/GUI/textures/icons/lucide/eye.edds";
	static const string ICON_VINEYARD     = "JM/COT/GUI/textures/icons/lucide/leafy-green.edds";
	static const string ICON_WATERPUMP    = "JM/COT/GUI/textures/icons/lucide/droplet.edds";

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Map, layoutRoot.FindAnyWidget( "action_map" ) );
		m_MapHost = layoutRoot.FindAnyWidget( "action_map_host" );

		m_Markers       = new array<ref JMMapMarker>;
		m_ById          = new map<string, ref JMMapMarker>;
		m_Widgets       = new map<string, ref JMUIActionMapMarker>;
		m_ParkedWidgets = new map<string, ref JMUIActionMapMarker>;

		m_LastClickWorld  = vector.Zero;
		m_LastClickButton = MouseState.LEFT;

		if ( m_Map )
			m_Map.SetScale( DEFAULT_SCALE );
	}

	override void OnShow() {}
	override void OnHide() {}

	//! The raw widget, for the rare caller that needs something this wrapper
	//! does not cover. Prefer the methods below.
	MapWidget GetMapWidget()
	{
		return m_Map;
	}

	// -------------------------------------------------------------------------
	//  View
	// -------------------------------------------------------------------------

	//! Height comes from UIActionBase.SetHeight, and the root is already
	//! VEXACTSIZE - CreateMap sets that when it applies the height it was
	//! asked for - so a host that wants the map to fill a band just calls it
	//! with the band's height in layout pixels. A MapWidget has no content
	//! height of its own, so somebody always has to.

	void SetScale( float scale )
	{
		if ( m_Map )
			m_Map.SetScale( scale );
	}

	float GetScale()
	{
		if ( !m_Map )
			return DEFAULT_SCALE;

		return m_Map.GetScale();
	}

	void SetCenter( vector worldPos )
	{
		if ( m_Map )
			m_Map.SetMapPos( worldPos );
	}

	vector GetCenter()
	{
		if ( !m_Map )
			return vector.Zero;

		return m_Map.GetMapPos();
	}

	//! Put `worldPos` in the middle, optionally changing zoom. A negative scale
	//! means "leave the zoom where the user left it", which is what a "show me
	//! this" button should do to someone who has already zoomed in.
	void CenterOn( vector worldPos, float scale = -1 )
	{
		if ( !m_Map )
			return;

		if ( scale > 0 )
			m_Map.SetScale( scale );

		m_Map.SetMapPos( worldPos );
	}

	//! Centre on something that may not have been laid out yet.
	//!
	//! SetMapPos on a map the engine has not sized is discarded, so a form that
	//! centres in the same pass that builds the map comes up on the world
	//! origin. Re-issuing it a frame later is the only reliable fix, and it is
	//! wanted often enough that every host should not have to rediscover it.
	void CenterOnDeferred( vector worldPos )
	{
		CenterOn( worldPos );

		m_DeferredCenter = worldPos;
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( ApplyDeferredCenter, 1, false );
	}

	protected void ApplyDeferredCenter()
	{
		CenterOn( m_DeferredCenter );
	}

	// -------------------------------------------------------------------------
	//  Markers
	// -------------------------------------------------------------------------

	//! Add a marker, or replace the one already under this id. Ids are the
	//! host's to choose and only have to be unique across the whole map, so a
	//! layer prefix is the usual shape: "loot_142", "player_" + guid.
	void AddMarker( string id, vector position, string label, int color = 0, string icon = "", string layer = "" )
	{
		if ( id == "" )
			return;

		if ( color == 0 )
			color = COLOR_DEFAULT;

		if ( icon == "" )
			icon = ICON_DOT;

		JMMapMarker existing = GetMarker( id );
		if ( existing )
		{
			existing.Position = position;
			existing.Label    = label;
			existing.Color    = color;
			existing.IconPath = icon;
			existing.Layer    = layer;

			JMUIActionMapMarker widget = GetWidgetFor( id );
			if ( widget )
			{
				widget.SetPosition( position );
				widget.SetLabel( label );

				if ( id != m_SelectedId )
					widget.SetColor( color );

				widget.SetIcon( icon );

				if ( m_Map )
					widget.UpdateScreenPosition( m_Map );
			}

			return;
		}

		JMMapMarker marker = new JMMapMarker( id, position, label, color, icon, layer );

		m_Markers.Insert( marker );
		m_ById.Insert( id, marker );

		CreateOrReuseMarkerWidget( marker );
	}

	//! Move one marker. This is the call MapWidget does not have, and the whole
	//! reason the markers live here.
	bool MoveMarker( string id, vector position )
	{
		JMMapMarker marker = GetMarker( id );

		if ( !marker )
			return false;

		marker.Position = position;

		JMUIActionMapMarker widget = GetWidgetFor( id );
		if ( widget )
		{
			widget.SetPosition( position );

			if ( m_Map )
				widget.UpdateScreenPosition( m_Map );
		}

		return true;
	}

	bool SetMarkerLabel( string id, string label )
	{
		JMMapMarker marker = GetMarker( id );

		if ( !marker )
			return false;

		marker.Label = label;

		JMUIActionMapMarker widget = GetWidgetFor( id );
		if ( widget )
			widget.SetLabel( label );

		return true;
	}

	bool SetMarkerColor( string id, int color )
	{
		JMMapMarker marker = GetMarker( id );

		if ( !marker )
			return false;

		marker.Color = color;

		JMUIActionMapMarker widget = GetWidgetFor( id );
		if ( widget && id != m_SelectedId )
			widget.SetColor( color );

		return true;
	}

	//! `redraw` is kept for source compatibility with existing callers -
	//! there is nothing left to redraw, a marker's widget is the only copy of
	//! it there ever was.
	bool RemoveMarker( string id, bool redraw = true )
	{
		if ( !m_ById.Contains( id ) )
			return false;

		JMMapMarker marker = m_ById.Get( id );

		m_ById.Remove( id );

		int index = m_Markers.Find( marker );
		if ( index >= 0 )
			m_Markers.Remove( index );

		if ( m_SelectedId == id )
			m_SelectedId = "";

		if ( m_HoveredMarkerId == id )
			m_HoveredMarkerId = "";

		//! An explicit removal, unlike ClearLayer, is not "about to be
		//! re-added this frame" - destroy it now rather than parking it.
		m_Widgets.Remove( id );

		return true;
	}

	//! Erase one feature's marks. The point of layers: loot analysis dropping
	//! its results must not take the player dots with it.
	//!
	//! Widgets are parked rather than destroyed here - the normal caller shape
	//! is ClearLayer followed immediately by re-adding the current list, and
	//! parking lets AddMarker reclaim the same widget for an id that survives
	//! the refresh instead of destroying and recreating it.
	void ClearLayer( string layer )
	{
		for ( int i = m_Markers.Count() - 1; i >= 0; i-- )
		{
			if ( m_Markers[i].Layer != layer )
				continue;

			string id = m_Markers[i].Id;

			m_ById.Remove( id );

			if ( m_SelectedId == id )
				m_SelectedId = "";

			if ( m_HoveredMarkerId == id )
				m_HoveredMarkerId = "";

			m_Markers.Remove( i );

			ParkWidget( id );
		}
	}

	void ClearMarkers()
	{
		m_Markers.Clear();
		m_ById.Clear();
		m_SelectedId     = "";
		m_HoveredMarkerId = "";

		m_Widgets.Clear();
		m_ParkedWidgets.Clear();
	}

	JMMapMarker GetMarker( string id )
	{
		if ( !m_ById.Contains( id ) )
			return NULL;

		return m_ById.Get( id );
	}

	int GetMarkerCount()
	{
		return m_Markers.Count();
	}

	//! Highlight one marker, so the map and a list beside it agree on what is
	//! being looked at. "" clears the highlight.
	void SetSelectedMarker( string id )
	{
		if ( m_SelectedId == id )
			return;

		if ( m_SelectedId != "" )
		{
			JMUIActionMapMarker prevWidget = GetWidgetFor( m_SelectedId );
			JMMapMarker         prevData   = GetMarker( m_SelectedId );

			if ( prevWidget )
			{
				prevWidget.SetSelected( false );

				if ( prevData )
					prevWidget.SetColor( prevData.Color );
			}
		}

		m_SelectedId = id;

		if ( m_SelectedId == "" )
			return;

		JMUIActionMapMarker widget = GetWidgetFor( m_SelectedId );
		if ( widget )
		{
			widget.SetSelected( true );
			widget.SetColor( COLOR_SELECTED );
		}
	}

	string GetSelectedMarker()
	{
		return m_SelectedId;
	}

	//! Kept for source compatibility - there is no longer a mass redraw to
	//! bracket, every marker call already only touches its own widget.
	void BeginBatch() {}
	void EndBatch()   {}

	// -------------------------------------------------------------------------
	//  Widget lifecycle
	// -------------------------------------------------------------------------

	protected JMUIActionMapMarker GetWidgetFor( string id )
	{
		if ( !m_Widgets.Contains( id ) )
			return null;

		return m_Widgets.Get( id );
	}

	protected void CreateOrReuseMarkerWidget( JMMapMarker marker )
	{
		if ( !m_MapHost )
			return;

		JMUIActionMapMarker widget;

		if ( m_ParkedWidgets.Contains( marker.Id ) )
		{
			widget = m_ParkedWidgets.Get( marker.Id );
			m_ParkedWidgets.Remove( marker.Id );

			widget.SetPosition( marker.Position );
			widget.SetLabel( marker.Label );
			widget.SetColor( marker.Color );
			widget.SetIcon( marker.IconPath );
		}
		else
		{
			widget = new JMUIActionMapMarker( m_MapHost, this, marker.Id, marker.Position, marker.Label, marker.Color, marker.IconPath );
		}

		m_Widgets.Insert( marker.Id, widget );

		widget.SetSelected( marker.Id == m_SelectedId && m_SelectedId != "" );

		if ( m_Map )
			widget.UpdateScreenPosition( m_Map );
	}

	protected void ParkWidget( string id )
	{
		if ( !m_Widgets.Contains( id ) )
			return;

		JMUIActionMapMarker widget = m_Widgets.Get( id );
		m_Widgets.Remove( id );

		m_ParkedWidgets.Insert( id, widget );

		if ( m_FlushScheduled )
			return;

		m_FlushScheduled = true;
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( FlushParkedWidgets, 1, false );
	}

	//! One frame after the last ClearLayer, anything still parked really is
	//! gone (nothing re-added it under the same id) - drop it for real.
	protected void FlushParkedWidgets()
	{
		m_FlushScheduled = false;
		m_ParkedWidgets.Clear();
	}

	//! Reposition every marker widget to match the map's current pan/zoom, and
	//! spin whichever ring is selected. The host calls this once a frame -
	//! skipped whenever the view has not moved, so it stays cheap at any
	//! marker count. See JMVehiclesForm.Update() for the pattern this is
	//! lifted from.
	void TickMarkers()
	{
		if ( !m_Map )
			return;

		if ( m_Widgets.Count() > 0 )
		{
			vector probeA = m_Map.MapToScreen( vector.Zero );
			vector probeB = m_Map.MapToScreen( Vector( 1000, 0, 1000 ) );

			if ( probeA != m_TickProbeA || probeB != m_TickProbeB )
			{
				m_TickProbeA = probeA;
				m_TickProbeB = probeB;

				array<string> ids = m_Widgets.GetKeyArray();

				foreach ( string id: ids )
					m_Widgets.Get( id ).UpdateScreenPosition( m_Map );
			}
		}

		if ( m_SelectedId == "" )
			return;

		JMUIActionMapMarker selected = GetWidgetFor( m_SelectedId );
		if ( !selected )
			return;

		int elapsed = g_Game.GetTime();
		int cycle   = elapsed - ( ( elapsed / RING_PERIOD_MS ) * RING_PERIOD_MS );

		selected.SetRingAngle( ( cycle / RING_PERIOD_MS ) * 360.0 );
	}

	//! Marker under the last hover, or "" if none. Same shape as
	//! UIActionDataTable.GetHoveredRow().
	string GetHoveredMarkerId()
	{
		return m_HoveredMarkerId;
	}

	// -------------------------------------------------------------------------
	//  Callbacks from JMUIActionMapMarker
	// -------------------------------------------------------------------------

	//! A press landed directly on a marker's own widget. Reported through the
	//! same state and event a ground press uses, so a host reads
	//! GetLastClickedMarkerId()/GetLastClickButton() the same way regardless
	//! of which path fired.
	bool NotifyMarkerPress( string id, int button )
	{
		JMMapMarker data = GetMarker( id );
		if ( !data )
			return false;

		m_LastClickButton     = button;
		m_LastClickWorld      = data.Position;
		m_LastClickedMarkerId = id;

		CallEvent( UIEvent.CLICK );

		return true;
	}

	bool NotifyMarkerDoubleClick( string id )
	{
		JMMapMarker data = GetMarker( id );
		if ( !data )
			return false;

		m_LastClickButton     = MouseState.LEFT;
		m_LastClickWorld      = data.Position;
		m_LastClickedMarkerId = id;

		CallEvent( UIEvent.DOUBLE_CLICK );

		return true;
	}

	void NotifyMarkerHoverEnter( string id )
	{
		m_HoveredMarkerId = id;
		CallEvent( UIEvent.MOUSE_ENTER );
	}

	void NotifyMarkerHoverLeave( string id )
	{
		if ( m_HoveredMarkerId != id )
			return;

		m_HoveredMarkerId = "";
		CallEvent( UIEvent.MOUSE_LEAVE );
	}

	// -------------------------------------------------------------------------
	//  Interaction
	// -------------------------------------------------------------------------

	//! Marker under the last press, or "" if it landed on open ground.
	string GetLastClickedMarkerId()
	{
		return m_LastClickedMarkerId;
	}

	//! Where the last press landed, snapped to the ground.
	vector GetLastClickWorldPos()
	{
		return m_LastClickWorld;
	}

	int GetLastClickButton()
	{
		return m_LastClickButton;
	}

	//! Screen point -> world point, snapped to the ground.
	//!
	//! ScreenToMap answers a position at map height, which is not a place a
	//! player can stand; every caller wanted the ground under it, so the snap
	//! is done here rather than in four places.
	vector ScreenToWorld( int x, int y )
	{
		if ( !m_Map )
			return vector.Zero;

		return SnapToGround( m_Map.ScreenToMap( Vector( x, y, 0 ) ) );
	}

	//! Nearest marker to a screen point within MARKER_HIT_PX, or "". Fallback
	//! only now - a press landing on a marker is normally reported straight
	//! from its own widget via NotifyMarkerPress, which does not go through
	//! this at all.
	protected string MarkerAtScreen( int x, int y )
	{
		if ( !m_Map )
			return "";

		string best     = "";
		float  bestDist = MARKER_HIT_PX;

		for ( int i = 0; i < m_Markers.Count(); i++ )
		{
			vector screen = m_Map.MapToScreen( m_Markers[i].Position );

			float dx = screen[0] - x;
			float dy = screen[1] - y;

			float dist = Math.Sqrt( ( dx * dx ) + ( dy * dy ) );

			if ( dist > bestDist )
				continue;

			bestDist = dist;
			best     = m_Markers[i].Id;
		}

		return best;
	}

	//! Presses, not clicks.
	//!
	//! MapWidget is not a ButtonWidget and never raises OnClick, so the press is
	//! the only event there is. It is also what lets a right press be reported
	//! at all, which OnClick has no way to express.
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( w != m_Map )
			return false;

		if ( button != MouseState.LEFT && button != MouseState.RIGHT )
			return false;

		m_LastClickButton     = button;
		m_LastClickWorld      = ScreenToWorld( x, y );
		m_LastClickedMarkerId = MarkerAtScreen( x, y );

		CallEvent( UIEvent.CLICK );

		//! LEFT is not consumed: the engine's own drag-to-pan runs off it, and
		//! swallowing the press here would freeze the map in place.
		return ( button == MouseState.RIGHT );
	}

	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		if ( w != m_Map || button != MouseState.LEFT )
			return false;

		m_LastClickButton     = button;
		m_LastClickWorld      = ScreenToWorld( x, y );
		m_LastClickedMarkerId = MarkerAtScreen( x, y );

		CallEvent( UIEvent.DOUBLE_CLICK );

		return true;
	}
}
