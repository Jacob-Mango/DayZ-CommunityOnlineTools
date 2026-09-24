class UIActionSlider: UIActionBase
{
	protected TextWidget  m_Label;
	protected Widget      m_Track;
	protected Widget      m_Fill;
	protected Widget      m_Handle;
	protected TextWidget  m_Value;
	protected int         m_FillColor;

	//! Height of the fill bar as a fraction of the track, 1.0 being the full
	//! track. A slider whose value IS a thickness can grow the bar with the
	//! value so it previews what it sets; everything else leaves it at 1.
	protected float       m_FillThickness;
	protected float m_Min;
	protected float m_Max;
	protected float m_Current;
	protected float m_Step;
	protected string m_Format;
	protected bool m_Dragging;

	//! Optional numeric box coupled to this slider. Owned here so it lives and
	//! dies with the slider; see UIActionSliderSync.
	protected ref UIActionSliderSync m_Sync;

	//! Track width the handle position was last computed against. The handle is
	//! centred on its value using the track's SCREEN width, which is still 0
	//! while OnInit runs - the widget has not been laid out yet - so the first
	//! placement is uncentred and stayed that way until something moved the
	//! slider. Re-running once the real width appears (and whenever it changes,
	//! e.g. the window is resized) fixes it without polling every frame.
	protected float m_LastTrackWidth;

	TextWidget GetLabelWidget()  { return m_Label;  }

	float GetMax() { return m_Max; }

	float GetMin() { return m_Min; }

	Widget     GetSliderWidget() { return m_Track;  }

	float GetStepValue() { return m_Step; }

	UIActionSliderSync GetSync() { return m_Sync; }

	TextWidget GetValueWidget()  { return m_Value;  }

	void SetAlpha( float alpha )
	{
		if ( m_Fill )
			m_Fill.SetAlpha( alpha );
	}

	//! Draw the fill as a bar `frac` of the track's height. Used by a slider
	//! that sets a thickness, so the bar is a preview of the value rather than
	//! just a position.
	void SetFillThickness( float frac )
	{
		m_FillThickness = Math.Clamp( frac, 0.1, 1.0 );
		UpdateVisuals();
	}

	void SetFormat( string format )
	{
		m_Format = format;
		UpdateVisuals();
	}

	void SetMinMax( float min, float max )
	{
		if ( max <= min )
			return;
		m_Min = min;
		m_Max = max;
		m_Current = Math.Clamp( m_Current, m_Min, m_Max );
		UpdateVisuals();
	}

	//! The slider's own width, bypassing the pair handling above.
	void SetOwnWidth( float width )
	{
		super.SetWidth( width );
	}

	void SetSliderWidth( float width )
	{
		if ( m_Track )
			SetWidgetWidth( m_Track, width );
	}

	void SetStepValue( float step )
	{
		if ( step > 0 )
			m_Step = step;
	}

	void SetSync( UIActionSliderSync sync )
	{
		m_Sync = sync;
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,  layoutRoot.FindAnyWidget( "action_label"  ) );
		Class.CastTo( m_Track,  layoutRoot.FindAnyWidget( "action_track"  ) );
		Class.CastTo( m_Fill,   layoutRoot.FindAnyWidget( "action_fill"   ) );
		Class.CastTo( m_Handle, layoutRoot.FindAnyWidget( "action_handle" ) );
		Class.CastTo( m_Value,  layoutRoot.FindAnyWidget( "action_value"  ) );

		m_Min       = 0;
		m_Max       = 1;
		m_Current   = 0;
		m_Step      = 0.01;
		m_Format    = "%1";
		m_Dragging  = false;
		m_FillColor = JMTheme.ACCENT;
		m_FillThickness = 1.0;

		UpdateVisuals();
		UpdateHandleColor();
	}

	override void SetLabel( string text )
	{
		if ( m_Label )
			m_Label.SetText( text );

		if ( m_Track )
		{
			if ( text.Length() > 0 )
				SetWidgetWidth( m_Track, 0.7 );
			else
				SetWidgetWidth( m_Track, 1.0 );
		}
	}

	//! With a numeric box attached, `width` is the width of the whole
	//! slider + box pair, so callers that size a slider to fill its row
	//! (SetWidth( 1.0 )) keep the box beside it instead of wrapping it away.
	override void SetWidth( float width )
	{
		if ( m_Sync )
		{
			m_Sync.ApplyWidth( width );
			return;
		}

		super.SetWidth( width );
	}

	override void Show()
	{
		super.Show();

		if ( m_Sync )
			m_Sync.OnSliderShown( true );
	}

	override void Hide()
	{
		super.Hide();

		if ( m_Sync )
			m_Sync.OnSliderShown( false );
	}

	override void Enable()
	{
		super.Enable();

		if ( m_Sync )
			m_Sync.OnSliderEnabled( true );
	}

	override void Disable()
	{
		super.Disable();

		if ( m_Sync )
			m_Sync.OnSliderEnabled( false );
	}

	override float GetCurrent() { return m_Current; }

	override void SetCurrent( float value )
	{
		m_Current = SnapToStep( Math.Clamp( value, m_Min, m_Max ) );
		UpdateVisuals();
	}

	override void SetText( string text )
	{
		if ( m_Value )
			m_Value.SetText( text );
	}

	override void SetColor( int color )
	{
		m_FillColor = color;
		if ( m_Fill )
			m_Fill.SetColor( color );

		UpdateHandleColor();
	}

	protected void UpdateHandleColor()
	{
		if ( !m_Handle )
			return;

		m_Handle.SetColor( m_FillColor );
	}

	override bool IsFocusWidget( Widget widget )
	{
		return widget == m_Track || widget == m_Handle || widget == m_Fill;
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( button != MouseState.LEFT )
			return false;
		if ( w != m_Track && w != m_Fill && w != m_Handle )
			return false;

		m_Dragging = true;
		m_Current  = SnapToStep( XToValue( x ) );
		UpdateVisuals();
		CallEvent( UIEvent.CHANGE );
		return true;
	}

	override bool OnMouseButtonUp( Widget w, int x, int y, int button )
	{
		if ( button == MouseState.LEFT && m_Dragging )
		{
			m_Dragging = false;
			return true;
		}
		return false;
	}

	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		if ( w != m_Track && w != m_Fill && w != m_Handle )
			return false;
		m_Current = SnapToStep( Math.Clamp( m_Current + wheel * m_Step, m_Min, m_Max ) );
		UpdateVisuals();
		CallEvent( UIEvent.CHANGE );
		return true;
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		RefreshOnTrackResize();

		if ( !m_Dragging )
			return;

		if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) == 0 )
		{
			m_Dragging = false;
			return;
		}

		int mx, my;
		GetMousePos( mx, my );
		float val = SnapToStep( XToValue( mx ) );
		if ( val != m_Current )
		{
			m_Current = val;
			UpdateVisuals();
			CallEvent( UIEvent.CHANGE );
		}
	}

	void UpdateValue()
	{
		if ( m_Sync )
			m_Sync.Refresh();

		if ( !m_Value )
			return;

		if ( m_Step >= 1.0 )
			m_Value.SetText( TranslateStringEx( m_Format, Math.Round( m_Current ).ToString() ) );
		else
			m_Value.SetText( TranslateStringEx( m_Format, m_Current.ToString() ) );
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		return false;
	}

	protected float SnapToStep( float value )
	{
		if ( m_Step <= 0 || m_Max <= m_Min )
			return value;
		float steps = Math.Round( ( value - m_Min ) / m_Step );
		return m_Min + steps * m_Step;
	}

	protected float XToValue( int screenX )
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

	protected void UpdateVisuals()
	{
		float range = m_Max - m_Min;
		if ( range <= 0 )
			return;

		float t = ( m_Current - m_Min ) / range;

		if ( m_Fill )
		{
			//! Centred in the track rather than pinned to its top, so a bar
			//! thinner than the track grows outward from the middle.
			m_Fill.SetPos( 0, ( 1.0 - m_FillThickness ) * 0.5 );
			m_Fill.SetSize( t, m_FillThickness );
		}

		// The handle's position is relative to the track, but its size is in
		// pixels, so SetPos( t, 0 ) puts its LEFT edge at the value and the
		// whole handle ends up sitting after the fill. Shift it back by half
		// its own width so it is centred on the value, then keep it inside the
		// track at both ends.
		if ( m_Handle && m_Track )
		{
			float trackW, trackH;
			m_Track.GetScreenSize( trackW, trackH );

			float handleW, handleH;
			m_Handle.GetScreenSize( handleW, handleH );

			float half = 0;
			if ( trackW > 0 )
				half = ( handleW * 0.5 ) / trackW;

			float pos = t - half;
			pos = Math.Clamp( pos, 0, Math.Max( 0, 1 - half - half ) );

			m_Handle.SetPos( pos, 0 );
		}

		UpdateValue();
	}
}
