//  JMVehiclesHoverInfo - floating vehicle-map readout (preview, health, fuel, speed, lock, occupants),
//  anchored to the window root, NOT the map panel (it clips children). Panel sort is 120: below the
//  default it never draws; at/above 151 its fill paints over the ItemPreviewWidget. Never raise it.
//  Fill is fully opaque (alpha ghosts map labels); all widgets ignorepointer so hover is not stolen.

class JMVehiclesHoverInfo
{
	protected Widget m_Anchor;
	protected Widget m_Root;
	protected TextWidget m_Title;
	protected TextWidget m_HealthLabel;
	protected TextWidget m_HealthValue;
	protected TextWidget m_FuelLabel;
	protected TextWidget m_FuelValue;
	protected TextWidget m_CoolantLabel;
	protected TextWidget m_CoolantValue;
	protected TextWidget m_SpeedLabel;
	protected TextWidget m_SpeedValue;
	protected TextWidget m_LockLabel;
	protected TextWidget m_LockValue;
	protected TextWidget m_CrewLabel;
	protected TextWidget m_CrewValue;
	protected TextWidget m_CrewNames;
	protected ItemPreviewWidget m_Preview;

	//! Local, client-only entity built purely to feed the preview widget, plus
	//! the class it was built from so re-hovering the same model is free.
	protected EntityAI m_PreviewItem;
	protected string   m_PreviewClass;
	protected ref JMVehicleMetaData m_Vehicle;
	protected bool m_Open;

	//! Card size, mirrored from the layout. The placement pass needs it before
	//! the widget has been laid out, so it cannot read it back.
	static const float PANEL_W = 300;
	static const float PANEL_H = 316;

	//! Bounded on both sides - see the header note. Neither bound is optional.
	static const int PANEL_SORT = 120;

	//! Gap between the cursor and the panel's near corner. Big enough that the
	//! panel never lands under the marker glyph it describes.
	static const float CURSOR_OFFSET_X = 18;
	static const float CURSOR_OFFSET_Y = 18;

	//! Fractions of full health / full tank below which the value turns amber
	//! and then red.
	static const float LEVEL_WARN = 0.5;
	static const float LEVEL_BAD  = 0.2;

	//! A vehicle is barely moving at this speed - not worth colouring as if it
	//! were in motion.
	static const float SPEED_IDLE = 1.0;

	void JMVehiclesHoverInfo( notnull Widget anchor )
	{
		m_Anchor = anchor;

		m_Root = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/vehicles/Vehicles_Hover_Info.layout", m_Anchor );

		if ( !m_Root )
			return;

		Class.CastTo( m_Preview, m_Root.FindAnyWidget( "vhi_preview" ) );

		m_Title        = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_title" ) );
		m_HealthLabel  = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_health_label" ) );
		m_HealthValue  = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_health_value" ) );
		m_FuelLabel    = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_fuel_label" ) );
		m_FuelValue    = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_fuel_value" ) );
		m_CoolantLabel = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_coolant_label" ) );
		m_CoolantValue = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_coolant_value" ) );
		m_SpeedLabel   = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_speed_label" ) );
		m_SpeedValue   = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_speed_value" ) );
		m_LockLabel    = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_lock_label" ) );
		m_LockValue    = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_lock_value" ) );
		m_CrewLabel    = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_crew_label" ) );
		m_CrewValue    = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_crew_value" ) );
		m_CrewNames    = TextWidget.Cast( m_Root.FindAnyWidget( "vhi_crew_names" ) );

		//! Set from script rather than left to the layout: a TextWidget with no
		//! declared colour is at the mercy of whatever the widget class defaults
		//! to, and an invisible label reads exactly like a broken panel.
		SetTextColor( m_Title, JMTheme.TEXT_PRIMARY );
		SetTextColor( m_HealthLabel, JMTheme.TEXT_MUTED );
		SetTextColor( m_FuelLabel, JMTheme.TEXT_MUTED );
		SetTextColor( m_CoolantLabel, JMTheme.TEXT_MUTED );
		SetTextColor( m_SpeedLabel, JMTheme.TEXT_MUTED );
		SetTextColor( m_LockLabel, JMTheme.TEXT_MUTED );
		SetTextColor( m_CrewLabel, JMTheme.TEXT_MUTED );
		SetTextColor( m_CrewNames, JMTheme.TEXT_MUTED );
		SetTextColor( m_HealthValue, JMTheme.TEXT_PRIMARY );
		SetTextColor( m_FuelValue, JMTheme.TEXT_PRIMARY );
		SetTextColor( m_CoolantValue, JMTheme.TEXT_PRIMARY );
		SetTextColor( m_SpeedValue, JMTheme.TEXT_PRIMARY );
		SetTextColor( m_LockValue, JMTheme.TEXT_PRIMARY );
		SetTextColor( m_CrewValue, JMTheme.TEXT_PRIMARY );

		m_Root.Show( false );
	}

	void ~JMVehiclesHoverInfo()
	{
		if (!g_Game)
			return;

		DestroyPreviewItem();

		if ( m_Root && m_Root.ToString() != "INVALID" )
			m_Root.Unlink();
	}

	JMVehicleMetaData GetVehicle()
	{
		return m_Vehicle;
	}

	bool IsOpen()
	{
		return m_Open;
	}

	//! Is the panel currently describing this network id?
	bool IsShowing( int netLow, int netHigh )
	{
		if ( !m_Open || !m_Vehicle )
			return false;

		return m_Vehicle.m_NetworkIDLow == netLow && m_Vehicle.m_NetworkIDHigh == netHigh;
	}

	protected void SetCrewField( JMVehicleMetaData vehicle )
	{
		if ( m_CrewValue )
		{
			m_CrewValue.SetText( vehicle.m_CrewCount.ToString() );

			if ( vehicle.m_CrewCount > 0 )
				m_CrewValue.SetColor( JMTheme.ACCENT_HOVER );
			else
				m_CrewValue.SetColor( JMTheme.TEXT_SECONDARY );
		}

		if ( m_CrewNames )
			m_CrewNames.SetText( vehicle.m_CrewNames );
	}

	// -------------------------------------------------------------------------
	//  Fields
	// -------------------------------------------------------------------------

	protected void SetFields( JMVehicleMetaData vehicle )
	{
		if ( m_Title )
			m_Title.SetText( vehicle.m_DisplayName );

		SetLevelField( m_HealthValue, vehicle.m_HealthPct );
		SetLevelField( m_FuelValue, vehicle.m_FuelPct );
		SetLevelField( m_CoolantValue, vehicle.m_CoolantPct );

		SetSpeedField( vehicle );
		SetLockField( vehicle );
		SetCrewField( vehicle );
	}

	//! Shared by health and fuel: a 0..1 fraction as a percentage, coloured by
	//! how low it is. A negative fraction means the vehicle has no such tank.
	protected void SetLevelField( TextWidget widget, float fraction )
	{
		if ( !widget )
			return;

		if ( fraction < 0 )
		{
			widget.SetText( "#STR_COT_GENERIC_NA" );
			widget.SetColor( JMTheme.TEXT_DISABLED );
			return;
		}

		int percent = Math.Round( fraction * 100 );
		widget.SetText( percent.ToString() + "%" );

		if ( fraction <= LEVEL_BAD )
			widget.SetColor( JMTheme.DANGER );
		else if ( fraction <= LEVEL_WARN )
			widget.SetColor( JMTheme.WARNING );
		else
			widget.SetColor( JMTheme.SUCCESS );
	}

	//! Only Expansion gives a vehicle a lock state. Saying "Unlocked" on a
	//! vanilla server would read as a fact about the vehicle rather than about
	//! what the build can see.
	protected void SetLockField( JMVehicleMetaData vehicle )
	{
		if ( !m_LockValue )
			return;

	#ifdef EXPANSIONMODVEHICLE
		if ( vehicle.m_IsLocked )
		{
			m_LockValue.SetText( "#STR_COT_VEHICLE_LOCKED" );
			m_LockValue.SetColor( JMTheme.WARNING );
		}
		else
		{
			m_LockValue.SetText( "#STR_COT_VEHICLE_UNLOCKED" );
			m_LockValue.SetColor( JMTheme.SUCCESS );
		}
	#else
		m_LockValue.SetText( "#STR_COT_GENERIC_NA" );
		m_LockValue.SetColor( JMTheme.TEXT_DISABLED );
	#endif
	}

	// -------------------------------------------------------------------------
	//  Model preview
	// -------------------------------------------------------------------------

	//! Build (or reuse) a local entity for `className` and frame it. Rebuilding
	//! only when the class actually changes matters: hovering along a row of
	//! the same vehicle type is the common case, and spawning a car per hover
	//! is not free.
	protected void SetPreview( string className )
	{
		if ( !m_Preview )
			return;

		if ( className == "" )
		{
			ClearPreview();
			return;
		}

		if ( m_PreviewItem && m_PreviewClass == className )
		{
			m_Preview.Show( true );
			return;
		}

		DestroyPreviewItem();

		m_PreviewItem  = EntityAI.Cast( g_Game.CreateObject( className, vector.Zero, true, false, false ) );
		m_PreviewClass = className;

		if ( !m_PreviewItem )
		{
			ClearPreview();
			return;
		}

		//! The same quieting the object spawner's preview entity gets: it is a
		//! prop, not something the world should simulate.
		dBodyActive( m_PreviewItem, ActiveState.INACTIVE );
		dBodyDynamic( m_PreviewItem, false );
		m_PreviewItem.DisableSimulation( true );

		m_Preview.SetItem( m_PreviewItem );
		m_Preview.SetView( m_PreviewItem.GetViewIndex() );
		m_Preview.SetModelOrientation( vector.Zero );
		m_Preview.SetModelPosition( PreviewCameraPosition( m_PreviewItem ) );
		m_Preview.Show( true );
	}

	protected void SetSpeedField( JMVehicleMetaData vehicle )
	{
		if ( !m_SpeedValue )
			return;

		int kph = Math.Round( vehicle.m_SpeedKph );
		m_SpeedValue.SetText( kph.ToString() + " " + "#STR_COT_VEHICLE_KMH" );

		if ( vehicle.m_SpeedKph >= SPEED_IDLE )
			m_SpeedValue.SetColor( JMTheme.ACCENT_HOVER );
		else
			m_SpeedValue.SetColor( JMTheme.TEXT_SECONDARY );
	}

	protected void SetTextColor( TextWidget widget, int color )
	{
		if ( widget )
			widget.SetColor( color );
	}

	//! Open the panel for `vehicle` at a screen-space cursor position.
	void ShowAt( JMVehicleMetaData vehicle, float screenX, float screenY )
	{
		if ( !m_Root || !vehicle )
			return;

		m_Vehicle = vehicle;
		m_Open    = true;

		SetFields( vehicle );
		SetPreview( vehicle.m_ClassName );

		m_Root.Show( true );
		m_Root.SetSort( PANEL_SORT, true );

		Place( screenX, screenY );
	}

	//! Repaint the text for a vehicle whose metadata was just refreshed. Does
	//! nothing when the panel has moved on to a different marker, so a late
	//! reply from the server cannot overwrite the one under the cursor.
	void Refresh( JMVehicleMetaData vehicle )
	{
		if ( !m_Open || !m_Root || !vehicle )
			return;

		if ( !IsShowing( vehicle.m_NetworkIDLow, vehicle.m_NetworkIDHigh ) )
			return;

		m_Vehicle = vehicle;

		SetFields( vehicle );
		SetPreview( vehicle.m_ClassName );
	}

	void Hide()
	{
		m_Open    = false;
		m_Vehicle = NULL;

		if ( m_Root )
			m_Root.Show( false );
	}

	// -------------------------------------------------------------------------
	//  Placement
	// -------------------------------------------------------------------------

	//! Flip rather than clamp when the panel would run off the anchor: a
	//! clamped panel slides over the marker it belongs to, a flipped one keeps
	//! the marker in view on the other side of the cursor.
	protected void Place( float screenX, float screenY )
	{
		if ( !m_Root || !m_Anchor )
			return;

		float ax, ay, aw, ah;
		m_Anchor.GetScreenPos( ax, ay );
		m_Anchor.GetScreenSize( aw, ah );

		float offsetX = JMUILayout.Px( CURSOR_OFFSET_X );
		float offsetY = JMUILayout.Px( CURSOR_OFFSET_Y );
		float panelW  = JMUILayout.Px( PANEL_W );
		float panelH  = JMUILayout.Px( PANEL_H );

		float px = screenX + offsetX - ax;
		float py = screenY + offsetY - ay;

		if ( screenX + offsetX + panelW > ax + aw )
			px = screenX - offsetX - panelW - ax;

		if ( screenY + offsetY + panelH > ay + ah )
			py = screenY - offsetY - panelH - ay;

		if ( px < 0 )
			px = 0;
		if ( py < 0 )
			py = 0;

		m_Root.SetPos( px, py );
	}

	//! Pull the camera back by the model's own diagonal so a truck and a bike
	//! both fill the frame. The expression matches the object spawner's zoom
	//! slider - only the distance is computed here instead of dragged.
	protected vector PreviewCameraPosition( EntityAI item )
	{
		vector minMax[2];
		float  span = 1.0;

		if ( item.GetCollisionBox( minMax ) )
			span = vector.Distance( minMax[0], minMax[1] );

		float distance = span * 0.45;

		if ( distance < 0.5 )
			distance = 0.5;

		return Vector( distance, 0, 0.5 + distance );
	}

	protected void ClearPreview()
	{
		if ( m_Preview && m_Preview.ToString() != "INVALID" )
			m_Preview.Show( false );
	}

	protected void DestroyPreviewItem()
	{
		ClearPreview();

		if ( g_Game && m_PreviewItem )
			g_Game.ObjectDelete( m_PreviewItem );

		m_PreviewItem  = NULL;
		m_PreviewClass = "";
	}
}
