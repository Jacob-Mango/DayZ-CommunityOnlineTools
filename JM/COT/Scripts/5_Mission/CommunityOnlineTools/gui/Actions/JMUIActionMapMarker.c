// =============================================================================
//  JMUIActionMapMarker
//
//  One marker's on-screen widget for UIActionMap - icon, persistent label,
//  hover glow and a selection ring. This is Vehicle Manager's marker
//  (JMVehiclesMapMarker) made generic: same layout, same "transparent button
//  sibling of the MapWidget, repositioned every tick via MapToScreen" trick,
//  but reporting back to a generic UIActionMap owner by marker id instead of
//  a JMVehicleMetaData-typed callback.
//
//  Native AddUserMark marks (what UIActionMap drew before) track pan/zoom for
//  free and can't be hovered or given a ring - a real widget can be both, at
//  the cost of needing this per-tick reprojection.
// =============================================================================
class JMUIActionMapMarker: ScriptedWidgetEventHandler
{
	protected Widget      m_Root;
	protected TextWidget  m_Name;
	protected ImageWidget m_Icon;
	protected ButtonWidget m_MarkerButton;
	protected ImageWidget m_SelectRing;

	protected UIActionMap m_Owner;
	protected string m_Id;
	protected vector m_Position;
	protected int    m_Color;

	//! True while the cursor is over this marker - held so a color set while
	//! hovered doesn't stomp the hover tint, mirroring JMVehiclesMapMarker.
	protected bool m_Frozen;

	static const int MARKER_HOVER_COLOR = JMTheme.ACCENT_HOVER;

	static const string SELECT_RING_ICON  = "circle-dashed";
	static const int    SELECT_RING_COLOR = JMTheme.ACCENT_HOVER;

	void JMUIActionMapMarker( Widget parent, UIActionMap owner, string id, vector position, string label, int color, string icon )
	{
		m_Root = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/vehicles/Vehicles_Marker.layout", parent );

		m_Name         = TextWidget.Cast( m_Root.FindAnyWidget( "marker_name" ) );
		m_Icon         = ImageWidget.Cast( m_Root.FindAnyWidget( "marker_icon" ) );
		m_MarkerButton = ButtonWidget.Cast( m_Root.FindAnyWidget( "marker_button" ) );
		m_SelectRing   = ImageWidget.Cast( m_Root.FindAnyWidget( "marker_select_ring" ) );

		m_Owner    = owner;
		m_Id       = id;
		m_Position = position;
		m_Color    = color;

		if ( m_Icon && icon != "" )
			m_Icon.LoadImageFile( 0, icon );

		if ( m_Icon )
			m_Icon.SetColor( m_Color );

		if ( m_Name )
		{
			m_Name.SetText( Widget.TranslateString( label ) );
			m_Name.SetColor( m_Color );
		}

		if ( m_SelectRing )
		{
			m_SelectRing.LoadImageFile( 0, JMConstants.Lucide( SELECT_RING_ICON ) );
			m_SelectRing.SetImage( 0 );
			m_SelectRing.SetColor( SELECT_RING_COLOR );
			m_SelectRing.Show( false );
		}

		m_Root.SetHandler( this );
	}

	void ~JMUIActionMapMarker()
	{
		if ( g_Game && m_Root )
			m_Root.Unlink();
	}

	string GetId()
	{
		return m_Id;
	}

	vector GetPosition()
	{
		return m_Position;
	}

	void SetPosition( vector position )
	{
		m_Position = position;
	}

	void SetLabel( string label )
	{
		if ( m_Name )
			m_Name.SetText( Widget.TranslateString( label ) );
	}

	void SetColor( int color )
	{
		m_Color = color;

		if ( m_Frozen )
			return;

		if ( m_Icon )
			m_Icon.SetColor( m_Color );

		if ( m_Name )
			m_Name.SetColor( m_Color );
	}

	void SetIcon( string icon )
	{
		if ( m_Icon && icon != "" )
			m_Icon.LoadImageFile( 0, icon );
	}

	//! Ring visibility. Left at whatever angle SetRingAngle last put it so a
	//! re-selection does not start mid-spin from a stale angle.
	void SetSelected( bool selected )
	{
		if ( !m_SelectRing )
			return;

		m_SelectRing.Show( selected );

		if ( !selected )
			m_SelectRing.SetRotation( 0, 0, 0 );
	}

	//! Driven from UIActionMap.TickMarkers() for whichever marker is currently
	//! selected - only one ring spins at a time, so there is no per-marker
	//! timer, just this setter.
	void SetRingAngle( float degrees )
	{
		if ( m_SelectRing && m_SelectRing.IsVisible() )
			m_SelectRing.SetRotation( 0, 0, degrees );
	}

	//! Reproject onto the map's current pan/zoom. Cheap; UIActionMap only
	//! calls this when the view has actually moved.
	void UpdateScreenPosition( MapWidget mapWidget )
	{
		if ( !mapWidget || !m_Root )
			return;

		vector screenPos = mapWidget.MapToScreen( m_Position );

		float x, y;
		m_Root.GetParent().GetScreenPos( x, y );

		m_Root.SetPos( screenPos[0] - x, screenPos[1] - y, true );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( !m_MarkerButton || w != m_MarkerButton )
			return false;

		m_Frozen = true;

		if ( m_Icon )
			m_Icon.SetColor( MARKER_HOVER_COLOR );
		if ( m_Name )
			m_Name.SetColor( MARKER_HOVER_COLOR );

		if ( m_Owner )
			m_Owner.NotifyMarkerHoverEnter( m_Id );

		return true;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( !m_MarkerButton || w != m_MarkerButton )
			return false;

		m_Frozen = false;

		if ( m_Icon )
			m_Icon.SetColor( m_Color );
		if ( m_Name )
			m_Name.SetColor( m_Color );

		if ( m_Owner )
			m_Owner.NotifyMarkerHoverLeave( m_Id );

		return true;
	}

	//! A ButtonWidget raises no OnClick for the right mouse button, so the
	//! right-click has to be read from the button-down event instead - same
	//! reason JMVehiclesMapMarker does it this way.
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( !m_MarkerButton || w != m_MarkerButton || !m_Owner )
			return false;

		if ( button != MouseState.RIGHT )
			return false;

		return m_Owner.NotifyMarkerPress( m_Id, button );
	}

	//! Left button only - the right button already fired from OnMouseButtonDown.
	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( !m_MarkerButton || w != m_MarkerButton || !m_Owner )
			return false;

		if ( button != MouseState.LEFT )
			return false;

		return m_Owner.NotifyMarkerPress( m_Id, button );
	}

	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		if ( !m_MarkerButton || w != m_MarkerButton || !m_Owner )
			return false;

		if ( button != MouseState.LEFT )
			return false;

		return m_Owner.NotifyMarkerDoubleClick( m_Id );
	}
}
