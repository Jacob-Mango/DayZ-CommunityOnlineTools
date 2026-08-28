class JMVehiclesMapMarker: ScriptedWidgetEventHandler
{
	protected Widget m_Root;
	protected TextWidget m_Name;
	protected ImageWidget m_Icon;
	protected ButtonWidget m_MarkerButton;
	protected Widget m_MarkerDragging;
	protected MapWidget m_MapWidget;
	protected vector m_MarkerPos;
	protected string m_MarkerName;
	protected string m_MarkerIcon;
	protected int m_MarkerColor;
	protected int m_MarkerAlpha;

	protected float m_OffsetX;
	protected float m_OffsetY;

	// When true, the consolidated form-level ticker skips this marker's
	// position sync - used to freeze the hover-highlight state.
	protected bool m_Frozen;

	protected ref JMVehicleMetaData m_Vehicle;
	protected ref JMVehiclesForm m_COTVehicleMenu;

	void JMVehiclesMapMarker( Widget parent, MapWidget mapwidget, vector pos, int color, string icon, JMVehicleMetaData vehicle, JMVehiclesForm menu )
	{
		m_Root = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/vehicles/Vehicles_Marker.layout", parent );

		m_Name = TextWidget.Cast( m_Root.FindAnyWidget( "marker_name" ) );
		m_Icon = ImageWidget.Cast( m_Root.FindAnyWidget( "marker_icon" ) );
		m_MarkerButton = ButtonWidget.Cast( m_Root.FindAnyWidget( "marker_button" ) );
		m_MarkerDragging = ButtonWidget.Cast( m_Root.FindAnyWidget( "marker_icon_panel" ) );

		m_MapWidget = mapwidget;
		m_MarkerPos = pos;
		m_MarkerIcon = icon;
		m_MarkerColor = color;
		m_Vehicle = vehicle;
		m_COTVehicleMenu = menu;

		m_MarkerName = m_Vehicle.m_DisplayName;

		if ( m_MarkerIcon )
			m_Icon.LoadImageFile( 0, m_MarkerIcon );

		m_Icon.SetColor( m_MarkerColor );
		m_Name.SetText( m_MarkerName );
		m_Name.SetColor( m_MarkerColor );

		m_Root.SetHandler( this );
	}

	void ~JMVehiclesMapMarker()
	{
		if (g_Game && m_Root)
			m_Root.Unlink();
	}

	void ChangeColor( int color )
	{
		m_MarkerColor = color;
	}

	void SetPosition( vector position )
	{
		m_MarkerPos = position;
	}

	void ChangeIcon( string icon )
	{
		m_MarkerIcon = icon;
	}

	void ChangeName( string name )
	{
		m_MarkerName = name;
	}

	string GetMarkerName()
	{
		return m_MarkerName;
	}

	// Called from JMVehiclesForm's consolidated per-form tick. Cheap to skip
	// when frozen (hover state) or when the widget has no map anchor yet.
	void UpdatePosition()
	{
		if ( m_Frozen )
			return;
		if ( !m_MapWidget )
			return;
		if ( !m_MarkerPos )
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
		if ( m_MarkerButton && w == m_MarkerButton )
		{
			m_COTVehicleMenu.SetVehicleInfo( m_Vehicle );
			return true;
		}

		return false;
	}

	void SetPosition( float x, float y )
	{
		m_Root.SetPos( x, y, true );
		m_MarkerPos = m_MapWidget.ScreenToMap( Vector( x, y, 0 ) );
	}

	void HideMarker()
	{
		m_Root.Show( false );
	}

	void ShowMarker()
	{
		m_Root.Show( true );
	}
}
