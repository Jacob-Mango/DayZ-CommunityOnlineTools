// =============================================================================
//  JMEntityManagerMapMarker
//
//  Generic map marker. Identical behaviour to JMVehiclesMapMarker but takes
//  a JMEntityMetaData instead of a vehicle-specific struct.
//
//  Positioning is driven by the owning form's consolidated 20 Hz tick via
//  UpdatePosition(). No per-marker Timer.
// =============================================================================
class JMEntityManagerMapMarker: ScriptedWidgetEventHandler
{
	protected Widget       m_Root;
	protected TextWidget   m_Name;
	protected ImageWidget  m_Icon;
	protected ButtonWidget m_MarkerButton;
	protected Widget       m_MarkerDragging;
	protected MapWidget    m_MapWidget;
	protected vector       m_MarkerPos;
	protected string       m_MarkerName;
	protected string       m_MarkerIcon;
	protected int          m_MarkerColor;

	// Skip the shared form ticker while hovered, so the highlight colour
	// stays solid white without constant repositioning fighting it.
	protected bool         m_Frozen;

	protected ref JMEntityMetaData    m_Entity;
	protected ref JMEntityManagerForm m_Form;

	void JMEntityManagerMapMarker( Widget parent, MapWidget mapwidget, JMEntityMetaData entity, JMEntityManagerForm form )
	{
		m_Root          = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/entitymanager/EntityManager_Marker.layout", parent );
		m_Name          = TextWidget.Cast(   m_Root.FindAnyWidget( "marker_name" ) );
		m_Icon          = ImageWidget.Cast(  m_Root.FindAnyWidget( "marker_icon" ) );
		m_MarkerButton  = ButtonWidget.Cast( m_Root.FindAnyWidget( "marker_button" ) );
		m_MarkerDragging = ButtonWidget.Cast( m_Root.FindAnyWidget( "marker_icon_panel" ) );

		m_MapWidget   = mapwidget;
		m_Entity      = entity;
		m_Form        = form;

		m_MarkerPos   = entity.m_Position;
		m_MarkerIcon  = entity.m_MarkerIcon;
		m_MarkerColor = entity.m_MarkerColor;
		m_MarkerName  = entity.m_DisplayName;

		if ( m_MarkerIcon != "" && m_Icon )
			m_Icon.LoadImageFile( 0, m_MarkerIcon );

		if ( m_Icon ) m_Icon.SetColor( m_MarkerColor );
		if ( m_Name )
		{
			m_Name.SetText( m_MarkerName );
			m_Name.SetColor( m_MarkerColor );
		}

		m_Root.SetHandler( this );
	}

	void ~JMEntityManagerMapMarker()
	{
		if ( g_Game && m_Root )
			m_Root.Unlink();
	}

	// Called by the form's consolidated ticker. Cheap no-op when frozen or
	// before the marker has been placed on a map.
	void UpdatePosition()
	{
		if ( m_Frozen )
			return;
		if ( !m_MapWidget || !m_MarkerPos )
			return;

		vector mapPos = m_MapWidget.MapToScreen( m_MarkerPos );

		float x;
		float y;
		m_Root.GetParent().GetScreenPos( x, y );

		m_Root.SetPos( mapPos[0] - x, mapPos[1] - y, true );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( m_MarkerButton && w == m_MarkerButton )
		{
			m_Frozen = true;
			m_Icon.SetColor( ARGB( 255, 255, 255, 255 ) );
			m_Name.SetColor( ARGB( 255, 255, 255, 255 ) );
			return true;
		}
		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( m_MarkerButton && w == m_MarkerButton )
		{
			m_Frozen = false;
			m_Icon.SetColor( m_MarkerColor );
			m_Name.SetColor( m_MarkerColor );
			return true;
		}
		return false;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( m_MarkerButton && w == m_MarkerButton && m_Form )
		{
			m_Form.SetEntityInfo( m_Entity );
			return true;
		}
		return false;
	}

	void HideMarker() { m_Root.Show( false ); }
	void ShowMarker() { m_Root.Show( true  ); }
}
