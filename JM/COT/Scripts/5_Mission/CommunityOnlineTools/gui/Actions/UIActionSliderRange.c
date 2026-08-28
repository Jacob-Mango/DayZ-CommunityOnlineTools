// =============================================================================
//  UIActionSliderRange
//
//  A progress-bar-style range control with two draggable handles (low / high).
//  Click or drag on the track to move the nearest handle.
//  Fires UIEvent.CHANGE whenever either handle moves.
//
//  Usage:
//      m_Range = UIActionManager.CreateSliderRange( parent, "Health", 0, 100,
//          this, "OnChange_Range" );
//      m_Range.SetStep( 1 );
//      m_Range.SetRange( 20, 80 );
//
//      void OnChange_Range( UIEvent eid, UIActionBase action )
//      {
//          float lo = m_Range.GetRangeLow();
//          float hi = m_Range.GetRangeHigh();
//      }
// =============================================================================
class UIActionSliderRange: UIActionBase
{
	protected TextWidget  m_Label;
	protected TextWidget  m_RangeText;
	protected Widget      m_Track;
	protected Widget      m_Fill;
	protected Widget      m_HandleLow;
	protected Widget      m_HandleHigh;

	protected float m_Min;
	protected float m_Max;
	protected float m_Low;
	protected float m_High;
	protected float m_Step;
	protected string m_Format;

	// 0 = none, 1 = dragging low handle, 2 = dragging high handle
	protected int m_Dragging;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,      layoutRoot.FindAnyWidget( "action_label"       ) );
		Class.CastTo( m_RangeText,  layoutRoot.FindAnyWidget( "action"             ) );
		Class.CastTo( m_Track,      layoutRoot.FindAnyWidget( "action_track"       ) );
		Class.CastTo( m_Fill,       layoutRoot.FindAnyWidget( "action_fill"        ) );
		Class.CastTo( m_HandleLow,  layoutRoot.FindAnyWidget( "action_handle_low"  ) );
		Class.CastTo( m_HandleHigh, layoutRoot.FindAnyWidget( "action_handle_high" ) );

		m_Min      = 0;
		m_Max      = 1;
		m_Low      = 0;
		m_High     = 1;
		m_Step     = 0.01;
		m_Format   = "%1";
		m_Dragging = 0;

		UpdateVisuals();
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_Label )
			m_Label.SetText( text );
	}

	void SetMinMax( float min, float max )
	{
		m_Min  = min;
		m_Max  = max;
		m_Low  = Math.Clamp( m_Low,  m_Min, m_Max );
		m_High = Math.Clamp( m_High, m_Min, m_Max );
		UpdateVisuals();
	}

	void SetStep( float step )
	{
		m_Step = Math.Max( 0.001, step );
	}

	void SetFormat( string fmt )
	{
		m_Format = fmt;
		UpdateVisuals();
	}

	void SetRangeLow( float value )
	{
		m_Low = SnapToStep( Math.Clamp( value, m_Min, m_High ) );
		UpdateVisuals();
	}

	void SetRangeHigh( float value )
	{
		m_High = SnapToStep( Math.Clamp( value, m_Low, m_Max ) );
		UpdateVisuals();
	}

	void SetRange( float low, float high )
	{
		m_Low  = SnapToStep( Math.Clamp( low,  m_Min, m_Max ) );
		m_High = SnapToStep( Math.Clamp( high, m_Low, m_Max ) );
		UpdateVisuals();
	}

	float GetRangeLow()  { return m_Low;  }
	float GetRangeHigh() { return m_High; }

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( button != MouseState.LEFT )
			return false;

		if ( w != m_Track && w != m_Fill && w != m_HandleLow && w != m_HandleHigh )
			return false;

		float val = XToValue( x );
		float distLow  = Math.AbsFloat( val - m_Low  );
		float distHigh = Math.AbsFloat( val - m_High );

		if ( distLow <= distHigh )
		{
			m_Dragging = 1;
			m_Low = SnapToStep( Math.Clamp( val, m_Min, m_High ) );
		}
		else
		{
			m_Dragging = 2;
			m_High = SnapToStep( Math.Clamp( val, m_Low, m_Max ) );
		}

		UpdateVisuals();
		CallEvent( UIEvent.CHANGE );
		return true;
	}

	override bool OnMouseButtonUp( Widget w, int x, int y, int button )
	{
		if ( button == MouseState.LEFT && m_Dragging != 0 )
		{
			m_Dragging = 0;
			return true;
		}
		return false;
	}

	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		if ( w != m_Track && w != m_Fill && w != m_HandleLow && w != m_HandleHigh )
			return false;

		// Pick the handle closest to the mouse X position
		float val = XToValue( x );
		float distLow  = Math.AbsFloat( val - m_Low  );
		float distHigh = Math.AbsFloat( val - m_High );

		if ( distLow <= distHigh )
			m_Low  = SnapToStep( Math.Clamp( m_Low  + wheel * m_Step, m_Min, m_High ) );
		else
			m_High = SnapToStep( Math.Clamp( m_High + wheel * m_Step, m_Low,  m_Max ) );

		UpdateVisuals();
		CallEvent( UIEvent.CHANGE );
		return true;
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		RefreshOnTrackResize();

		if ( m_Dragging == 0 )
			return;

		if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) == 0 )
		{
			m_Dragging = 0;
			return;
		}

		int mx, my;
		GetMousePos( mx, my );
		float val = XToValue( mx );

		if ( m_Dragging == 1 )
			m_Low  = SnapToStep( Math.Clamp( val, m_Min, m_High ) );
		else
			m_High = SnapToStep( Math.Clamp( val, m_Low, m_Max ) );

		UpdateVisuals();
		CallEvent( UIEvent.CHANGE );
	}

	private float SnapToStep( float value )
	{
		if ( m_Step <= 0 )
			return value;
		float steps = Math.Round( ( value - m_Min ) / m_Step );
		return m_Min + steps * m_Step;
	}

	private float XToValue( int screenX )
	{
		if ( !m_Track )
			return m_Min;
		float tx, ty, tw, th;
		m_Track.GetScreenPos( tx, ty );
		m_Track.GetScreenSize( tw, th );
		if ( tw <= 0 )
			return m_Min;
		float t = Math.Clamp( ( screenX - tx ) / tw, 0, 1 );
		return m_Min + t * ( m_Max - m_Min );
	}

	//! Track width the handles were last placed against - see
	//! UIActionSlider.RefreshOnTrackResize for why this is needed.
	protected float m_LastTrackWidth;

	protected void RefreshOnTrackResize()
	{
		if ( !m_Track )
			return;

		float trackW, trackH;
		m_Track.GetScreenSize( trackW, trackH );

		if ( trackW == m_LastTrackWidth )
			return;

		m_LastTrackWidth = trackW;
		UpdateVisuals();
	}

	//! Half a handle's width, as a fraction of the track width.
	private float HandleHalfWidth()
	{
		if ( !m_Track || !m_HandleLow )
			return 0;

		float trackW, trackH;
		m_Track.GetScreenSize( trackW, trackH );
		if ( trackW <= 0 )
			return 0;

		float handleW, handleH;
		m_HandleLow.GetScreenSize( handleW, handleH );

		return ( handleW * 0.5 ) / trackW;
	}

	private float ClampHandlePos( float t, float half )
	{
		return Math.Clamp( t - half, 0, Math.Max( 0, 1 - half - half ) );
	}

	private void UpdateVisuals()
	{
		if ( !m_Track )
			return;

		float range = m_Max - m_Min;
		if ( range <= 0 )
			return;

		float fillLeft  = ( m_Low  - m_Min ) / range;
		float fillRight = ( m_High - m_Min ) / range;
		float fillWidth = fillRight - fillLeft;

		if ( m_Fill )
		{
			m_Fill.SetPos( fillLeft, 0 );
			m_Fill.SetSize( fillWidth, 1 );
		}

		// Handle positions are relative to the track but their size is in pixels,
		// so SetPos( t, 0 ) puts the LEFT edge at the value and the handle ends
		// up sitting past the fill - visible as a gap on the high end. Shift each
		// one back by half its own width so it is centred on its value, and keep
		// it inside the track at the extremes.
		float half = HandleHalfWidth();

		if ( m_HandleLow )
			m_HandleLow.SetPos( ClampHandlePos( fillLeft, half ), 0 );

		if ( m_HandleHigh )
			m_HandleHigh.SetPos( ClampHandlePos( fillRight, half ), 0 );

		if ( m_RangeText )
			m_RangeText.SetText( string.Format( m_Format + " - " + m_Format, m_Low, m_High ) );
	}
}
