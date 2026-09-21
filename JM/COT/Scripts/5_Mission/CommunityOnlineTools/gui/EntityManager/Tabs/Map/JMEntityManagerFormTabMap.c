//! "Map" tab of JMEntityManagerForm - the world map and one marker per entity
//! the roster is showing. Back-reference to the owning form, same shape as
//! JMPlayerRowWidget.Menu.
//!
//! The roster (and its filters) stay on the form; it decides WHICH entities are
//! on screen and tells this tab to rebuild the matching markers. A marker click
//! goes back through JMEntityManagerForm.SetEntityInfo.
class JMEntityManagerFormTabMap: JMFormTab
{
	protected JMEntityManagerForm m_Form;

	//! The MapWidget lives in the layout - it cannot be created from script.
	protected MapWidget m_MapWidget;
	protected ref array<ref JMEntityManagerMapMarker> m_MapMarkers;

	//! Consolidated 20 Hz marker reposition tick (replaces per-marker timers).
	protected ref Timer m_MarkerTickTimer;
	static const float MARKER_TICK_INTERVAL = 0.05;

	void JMEntityManagerFormTabMap( JMEntityManagerForm form )
	{
		m_Form = form;

		m_MapMarkers = new array<ref JMEntityManagerMapMarker>;
	}

	override void OnCreate( Widget panel )
	{
		super.OnCreate( panel );

		m_MapWidget = MapWidget.Cast( panel.FindAnyWidget( "entity_map" ) );
	}

	// -------------------------------------------------------------------------
	//  Markers - driven by the form's roster
	// -------------------------------------------------------------------------

	void ClearMarkers()
	{
		m_MapMarkers.Clear();
	}

	void AddEntityMarker( JMEntityMetaData entity )
	{
		JMEntityManagerMapMarker marker = new JMEntityManagerMapMarker( m_TabPanel, m_MapWidget, entity, m_Form );
		m_MapMarkers.Insert( marker );
	}

	void ShowMapMarkers()
	{
		for ( int i = 0; i < m_MapMarkers.Count(); i++ )
			m_MapMarkers.Get( i ).ShowMarker();
	}

	void HideMapMarkers()
	{
		for ( int i = 0; i < m_MapMarkers.Count(); i++ )
			m_MapMarkers.Get( i ).HideMarker();
	}

	void StartMarkerTick()
	{
		if ( !m_MarkerTickTimer )
			m_MarkerTickTimer = new Timer( CALL_CATEGORY_GUI );

		if ( !m_MarkerTickTimer.IsRunning() )
			m_MarkerTickTimer.Run( MARKER_TICK_INTERVAL, this, "TickMarkers", NULL, true );
	}

	void StopMarkerTick()
	{
		if ( m_MarkerTickTimer && m_MarkerTickTimer.IsRunning() )
			m_MarkerTickTimer.Stop();
	}

	void TickMarkers()
	{
		for ( int i = 0; i < m_MapMarkers.Count(); i++ )
		{
			JMEntityManagerMapMarker m = m_MapMarkers[i];
			if ( m )
				m.UpdatePosition();
		}
	}

	void UpdateMapPosition( bool usePlayerPosition, vector mapPosition = vector.Zero )
	{
		if ( !m_MapWidget )
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

			m_MapWidget.SetScale( scale );
		}

		m_MapWidget.SetMapPos( mapPosition );
	}
}
