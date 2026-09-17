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
	protected Widget      m_GradientHost;
	protected Widget      m_CapLow;
	protected Widget      m_CapHigh;

	//! Arrows drifting along the filled span toward the end the value is
	//! heading for. A range that means "here is where it is, here is where it
	//! is going" cannot say WHICH end is the target from its geometry alone -
	//! the bar looks the same either way round - so the motion says it.
	protected Widget m_Flow;
	protected ref array<ImageWidget> m_Arrows;
	//! 0 = no arrows, 1 = drifting right, -1 = drifting left.
	protected int   m_FlowDir;
	protected float m_FlowPhase;
	protected float m_FlowLeft;
	protected float m_FlowWidth;
	protected Widget      m_HandleLow;
	protected Widget      m_HandleHigh;

	protected float m_Min;
	protected float m_Max;
	protected float m_Low;
	protected float m_High;
	protected float m_Step;
	protected string m_Format;
	protected int   m_FillColor;

	//! Colour stops across the slider's whole min..max span, evenly spaced. The
	//! filled bar is drawn as a strip of segments sampled from them, because a
	//! PanelWidget has one colour and no way to shade across itself.
	protected ref TIntArray m_Gradient;
	protected ref array<Widget> m_Segments;

	// 0 = none, 1 = dragging low handle, 2 = dragging high handle
	protected int m_Dragging;

	//! Single-value mode: one handle, and the bar filled from the minimum the
	//! way a plain slider fills.
	//!
	//! The low bound is pinned onto the high one rather than the control
	//! growing a second value store, so a caller reading GetRangeLow() and
	//! GetRangeHigh() gets one exact value out of a control it never has to ask
	//! the mode of - which is the whole reason this is a mode rather than a
	//! second widget class.
	protected bool m_Single;

	//! How many arrows the layout declares. Must match the number of flow_N
	//! images in UIActionSliderRange.layout.
	static const int   FLOW_ARROWS      = 12;
	//! Closest two arrows are allowed to sit, in screen pixels. This is a
	//! FLOOR, not the spacing: a wide bar spreads the arrows it has across its
	//! whole length instead of bunching them at the left edge.
	static const float FLOW_SPACING_PX  = 30.0;
	//! Cycles per second - one cycle moves each arrow into its neighbour's
	//! place. Slow enough to read as a direction, not as a barber pole.
	static const float FLOW_SPEED       = 0.5;
	//! Below this the span is too short to show motion in, and cramming
	//! arrows into it reads as noise.
	static const float FLOW_MIN_PX      = 36.0;

	//! How many slices the shaded bar is built from. Enough that the steps are
	//! not readable at the width one of these rows gets.
	//! Must match the number of grad_N panels in UIActionSliderRange.layout.
	static const int GRADIENT_SEGMENTS = 16;

	override void OnInit()
	{
		super.OnInit();

		m_Gradient = new TIntArray;
		m_Segments = new array<Widget>;
		m_Arrows   = new array<ImageWidget>;

		Class.CastTo( m_Label,      layoutRoot.FindAnyWidget( "action_label"       ) );
		Class.CastTo( m_RangeText,  layoutRoot.FindAnyWidget( "action"             ) );
		Class.CastTo( m_Track,      layoutRoot.FindAnyWidget( "action_track"       ) );
		Class.CastTo( m_Fill,       layoutRoot.FindAnyWidget( "action_fill"        ) );
		Class.CastTo( m_GradientHost, layoutRoot.FindAnyWidget( "action_gradient"   ) );

		//! The strip's slices are declared in the layout, not built here, and
		//! each one carries a style. A PanelWidget draws its Color by TINTING
		//! the nine-slice art its style names, so a panel with no style has
		//! nothing to tint and paints nothing at all - which is why every
		//! earlier version of this strip was invisible no matter what parent it
		//! was given or what colour it was set to.
		for ( int seg = 0; seg < GRADIENT_SEGMENTS; seg++ )
		{
			Widget slice = layoutRoot.FindAnyWidget( "grad_" + seg );

			if ( slice )
				m_Segments.Insert( slice );
		}
		m_Flow = layoutRoot.FindAnyWidget( "action_flow" );

		for ( int arrow = 0; arrow < FLOW_ARROWS; arrow++ )
		{
			ImageWidget image = ImageWidget.Cast( layoutRoot.FindAnyWidget( "flow_" + arrow ) );

			if ( image )
				m_Arrows.Insert( image );
		}

		Class.CastTo( m_CapLow,     layoutRoot.FindAnyWidget( "action_cap_low"     ) );
		Class.CastTo( m_CapHigh,    layoutRoot.FindAnyWidget( "action_cap_high"    ) );
		Class.CastTo( m_HandleLow,  layoutRoot.FindAnyWidget( "action_handle_low"  ) );
		Class.CastTo( m_HandleHigh, layoutRoot.FindAnyWidget( "action_handle_high" ) );

		m_Min      = 0;
		m_Max      = 1;
		m_Low      = 0;
		m_High     = 1;
		m_Step     = 0.01;
		m_Format   = "%1";
		m_Dragging = 0;
		m_Single   = false;



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

		//! The step decides whether the readout rounds, so changing it changes
		//! what the text should say.
		UpdateVisuals();
	}

	void SetFormat( string fmt )
	{
		m_Format = fmt;
		UpdateVisuals();
	}

	//! Collapse the control to one handle, or open it back up to two.
	//!
	//! The low handle is HIDDEN rather than the row being rebuilt as a
	//! UIActionSlider: the two read identically apart from that second handle,
	//! and swapping the widget out would cost the caller its reference, its
	//! gradient and its colours every time the mode changed.
	//!
	//! Collapsing loses where the low handle was. A caller that wants the span
	//! back as it left it keeps that value itself and re-applies it with
	//! SetRange after switching back - see JMObjectSpawnerForm.ApplyRangeMode.
	void SetSingle( bool single )
	{
		if ( m_Single == single )
			return;

		m_Single = single;

		if ( m_HandleLow )
			m_HandleLow.Show( !m_Single );

		UpdateVisuals();
	}

	bool IsSingle()
	{
		return m_Single;
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

	float GetMin() { return m_Min; }
	float GetMax() { return m_Max; }

	//! Tint the filled span and both handles, the same way UIActionSlider tints
	//! its bar - a caller that colours a value by how good it is (health, for
	//! one) expects the same control to answer.
	override void SetColor( int color )
	{
		m_FillColor = color;

		if ( m_Fill )
			m_Fill.SetColor( color );

		if ( m_HandleLow )
			m_HandleLow.SetColor( color );

		if ( m_HandleHigh )
			m_HandleHigh.SetColor( color );
	}

	//! Shade the filled bar across its own span.
	//!
	//! `stops` are evenly spaced over the slider's min..max, and every point in
	//! between is mixed from the two it falls between - so a health slider can
	//! run ruined-red through to pristine-green and each part of the bar shows
	//! the condition of the values under it. Pass an empty array to go back to
	//! one flat colour.
	//!
	//! The bar is not one widget shaded but a row of small ones each painted
	//! its own colour: nothing in the engine gradients a panel.
	void SetGradient( notnull TIntArray stops )
	{
		m_Gradient.Clear();

		for ( int i = 0; i < stops.Count(); i++ )
			m_Gradient.Insert( stops[i] );

		UpdateVisuals();
	}

	//! Colour at `t`, where 0 is the slider's min and 1 its max.
	protected int SampleGradient( float t )
	{
		int count = m_Gradient.Count();

		if ( count == 0 )
			return m_FillColor;

		if ( count == 1 )
			return m_Gradient[0];

		float pos = Math.Clamp( t, 0, 1 ) * ( count - 1 );
		int index = ( int ) Math.Floor( pos );

		if ( index >= count - 1 )
			return m_Gradient[count - 1];

		return JMTheme.Mix( m_Gradient[index], m_Gradient[index + 1], pos - index );
	}

	//! One cap's width, as a fraction of the track - the same measurement
	//! HandleHalfWidth takes, and for the same reason: the caps are sized in
	//! pixels but positioned in fractions of the track.
	protected float CapWidth()
	{
		if ( !m_Track || !m_CapLow )
			return 0;

		float trackW, trackH;
		m_Track.GetScreenSize( trackW, trackH );

		if ( trackW <= 0 )
			return 0;

		float capW, capH;
		m_CapLow.GetScreenSize( capW, capH );

		return capW / trackW;
	}

	//! Hide the caps by making them transparent rather than by hiding them.
	//!
	//! A hidden widget measures zero wide, and CapWidth has to be able to
	//! measure one to know how far to inset the strip - so they stay visible
	//! for their whole life and the colour carries whether they show.
	protected void ClearCaps()
	{
		if ( m_CapLow )
			m_CapLow.SetColor( 0x00000000 );

		if ( m_CapHigh )
			m_CapHigh.SetColor( 0x00000000 );
	}

	//! Paint the segment strip across the filled span, in slider space.
	//! The strip lives in action_gradient - a plain panel declared in the
	//! layout, holding a fixed row of slices declared alongside it.
	//!
	//! Nothing here is created at runtime, and every slice carries a style -
	//! rover_sim_colorable, the vanilla all-WhitePixel one, so the colour set
	//! below has flat art to tint and no corner rounding to seam between
	//! neighbours. All script does is size the host to the filled span and
	//! colour what the layout already declared.
	//!
	//! Flat art leaves the strip SQUARE, though, and the track behind it is a
	//! 6px pill - so at full range the strip's corners overhang it. The two ends
	//! are therefore drawn by rounded caps instead, with the strip inset by half
	//! a cap at each end so only the caps' outer halves are left showing.
	//!
	//! The caps and the host are declared BEFORE the handles, so the handles
	//! draw over them without anyone having to sort anything.
	protected void PaintGradient( float fillLeft, float fillRight )
	{
		if ( !m_GradientHost )
			return;

		float span = fillRight - fillLeft;

		if ( m_Gradient.Count() == 0 || span <= 0 )
		{
			m_GradientHost.Show( false );
			ClearCaps();

			if ( m_Fill )
				m_Fill.Show( true );

			//! A collapsed range has no strip to draw, and with it hidden the
			//! two handles are all the colour there is.
			if ( m_Gradient.Count() > 0 )
				PaintHandles( fillLeft, fillRight );

			return;
		}

		//! The strip IS the bar now, so the plain one underneath would only show
		//! through the seams.
		if ( m_Fill )
			m_Fill.Show( false );

		float capSpan = CapWidth();

		if ( m_CapLow )
		{
			m_CapLow.SetColor( SampleGradient( fillLeft ) );
			m_CapLow.SetPos( fillLeft, 0 );
		}

		if ( m_CapHigh )
		{
			m_CapHigh.SetColor( SampleGradient( fillRight ) );
			m_CapHigh.SetPos( fillRight - capSpan, 0 );
		}

		float stripLeft = fillLeft + capSpan * 0.5;
		float stripSpan = span - capSpan;

		//! Narrower than the two caps: they already cover the whole span, and
		//! overlapping each other they still read as one short rounded bar.
		if ( stripSpan <= 0 )
		{
			m_GradientHost.Show( false );
			PaintHandles( fillLeft, fillRight );
			return;
		}

		m_GradientHost.Show( true );
		m_GradientHost.SetPos( stripLeft, 0 );
		m_GradientHost.SetSize( stripSpan, 1 );
		m_GradientHost.Update();

		for ( int i = 0; i < m_Segments.Count(); i++ )
		{
			//! Sampled at the value under the slice rather than across the
			//! strip, so a narrow range shows the slice of the ramp its own
			//! values fall in instead of the whole ramp squeezed into it.
			m_Segments[i].SetColor( SampleGradient( stripLeft + ( ( i + 0.5 ) / GRADIENT_SEGMENTS ) * stripSpan ) );
		}

		PaintHandles( fillLeft, fillRight );
	}

	//! Point the arrows at the end the value is moving toward.
	//!
	//! 1 drifts them right, -1 left, 0 turns them off - which is the default,
	//! so a range slider that is only a range shows none of this.
	void SetFlowDirection( int direction )
	{
		if ( direction == m_FlowDir )
			return;

		m_FlowDir = direction;

		if ( direction == 0 )
			return;

		//! Loaded only when the direction changes, not every frame.
		string icon = JMConstants.ICON_CHEVRON_RIGHT;

		if ( direction < 0 )
			icon = JMConstants.ICON_CHEVRON_LEFT;

		for ( int i = 0; i < m_Arrows.Count(); i++ )
		{
			m_Arrows[i].LoadImageFile( 0, icon );
			m_Arrows[i].SetImage( 0 );
		}
	}

	int GetFlowDirection()
	{
		return m_FlowDir;
	}

	//! Slide the arrows one frame along the filled span.
	protected void UpdateFlow( float timeSlice )
	{
		if ( !m_Flow || !m_Track )
			return;

		float trackW, trackH;
		m_Track.GetScreenSize( trackW, trackH );

		float hostW = trackW * m_FlowWidth;

		if ( m_FlowDir == 0 || m_FlowWidth <= 0 || hostW < FLOW_MIN_PX )
		{
			m_Flow.Show( false );
			return;
		}

		m_Flow.Show( true );
		m_Flow.SetPos( m_FlowLeft, 0 );
		m_Flow.SetSize( m_FlowWidth, 1 );

		m_FlowPhase += timeSlice * FLOW_SPEED;

		while ( m_FlowPhase >= 1.0 )
			m_FlowPhase -= 1.0;

		//! Use every arrow the layout has, and divide the span between them,
		//! so the line of arrows reaches the far end of the fill.
		//!
		//! This used to fix the gap at FLOW_SPACING_PX and then cap the count
		//! at the number of arrows that exist, which meant a wide bar got the
		//! same 6 arrows crowded into its first ~180px with the rest of the
		//! fill empty - the motion read as a detail in the corner rather than
		//! as the bar flowing.
		int count = m_Arrows.Count();

		//! On a short span that many arrows would sit on top of each other, so
		//! the pixel floor decides how many are used instead.
		int fits = ( int ) Math.Floor( hostW / FLOW_SPACING_PX );

		if ( count > fits )
			count = fits;

		if ( count < 1 )
			count = 1;

		float step  = 1.0 / count;
		float cycle = 1.0;

		for ( int i = 0; i < m_Arrows.Count(); i++ )
		{
			if ( i >= count )
			{
				m_Arrows[i].Show( false );
				continue;
			}

			m_Arrows[i].Show( true );

			//! Every arrow slides by the same amount and wraps a whole cycle
			//! back, so the line of them looks continuous rather than each one
			//! restarting on its own.
			float t = ( i + m_FlowPhase * m_FlowDir ) * step;

			if ( t < 0 )
				t += cycle;

			if ( t >= cycle )
				t -= cycle;

			m_Arrows[i].SetPos( t, 0 );
		}
	}

	protected void PaintHandles( float fillLeft, float fillRight )
	{
		if ( m_HandleLow )
			m_HandleLow.SetColor( SampleGradient( fillLeft ) );

		if ( m_HandleHigh )
			m_HandleHigh.SetColor( SampleGradient( fillRight ) );
	}

	//! Colour the two ends independently.
	//!
	//! A range whose meaning changes across it - health running ruined to
	//! pristine - says more when each handle carries the colour of the value it
	//! is sitting on, and the bar between them the blend. One flat colour for
	//! the whole control can only describe one end of it.
	void SetRangeColors( int lowColor, int highColor )
	{
		m_FillColor = JMTheme.Mix( lowColor, highColor, 0.5 );

		if ( m_Fill )
			m_Fill.SetColor( m_FillColor );

		if ( m_HandleLow )
			m_HandleLow.SetColor( lowColor );

		if ( m_HandleHigh )
			m_HandleHigh.SetColor( highColor );
	}

	void SetAlpha( float alpha )
	{
		if ( m_Fill )
			m_Fill.SetAlpha( alpha );
	}

	//! Move the value while single, clamped to the control's own bounds.
	//!
	//! NOT through the high handle's usual clamp: that floors at m_Low, which is
	//! sitting exactly on the value here, so the number could be dragged up and
	//! never back down again.
	protected void SetSingleValue( float value )
	{
		m_High = SnapToStep( Math.Clamp( value, m_Min, m_Max ) );
		m_Low  = m_High;
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( button != MouseState.LEFT )
			return false;

		if ( w != m_Track && w != m_Fill && w != m_HandleLow && w != m_HandleHigh )
			return false;

		float val = XToValue( x );

		if ( m_Single )
		{
			m_Dragging = 2;
			SetSingleValue( val );
		}
		else if ( PickHandle( val ) == 1 )
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

		if ( m_Single )
		{
			SetSingleValue( m_High + wheel * m_Step );
			UpdateVisuals();
			CallEvent( UIEvent.CHANGE );
			return true;
		}

		int handle = PickHandle( val );

		//! A collapsed range has no nearer handle, so the scroll direction says
		//! which one is meant: up opens it from the top, down from the bottom.
		//! Without this the tie always resolved to the low handle, which cannot
		//! move up past the high one - so the wheel did nothing at all.
		if ( m_Low == m_High )
		{
			if ( wheel > 0 )
				handle = 2;
			else
				handle = 1;
		}

		if ( handle == 1 )
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
		UpdateFlow( timeSlice );

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

		//! A drag that has run the handles together hands over to the other one
		//! rather than pushing against it. Clamping alone would leave the pair
		//! stuck wherever they met - grabbing the low handle and dragging right
		//! past the high one could never open the range again.
		//! Single mode is exempt: the pair is pinned together there on purpose,
		//! and there is only ever the one handle to drag.
		if ( m_Low == m_High && !m_Single )
		{
			if ( m_Dragging == 1 && val > m_High )
				m_Dragging = 2;
			else if ( m_Dragging == 2 && val < m_Low )
				m_Dragging = 1;
		}

		if ( m_Single )
			SetSingleValue( val );
		else if ( m_Dragging == 1 )
			m_Low  = SnapToStep( Math.Clamp( val, m_Min, m_High ) );
		else
			m_High = SnapToStep( Math.Clamp( val, m_Low, m_Max ) );

		UpdateVisuals();
		CallEvent( UIEvent.CHANGE );
	}

	//! Which handle a click at `value` is asking for: 1 = low, 2 = high.
	//!
	//! Nearest wins, but the two handles can sit on the SAME value, and then
	//! every click is equidistant. Resolving that tie the same way every time
	//! picks the low handle, which is clamped by the high one and so cannot
	//! move up - a collapsed range would be frozen. The tie is broken by which
	//! side of the pair was clicked instead, so it can always be opened.
	private int PickHandle( float value )
	{
		//! Only one handle to pick from.
		if ( m_Single )
			return 2;

		float distLow  = Math.AbsFloat( value - m_Low  );
		float distHigh = Math.AbsFloat( value - m_High );

		if ( distLow == distHigh )
		{
			if ( value > m_High )
				return 2;

			return 1;
		}

		if ( distLow <= distHigh )
			return 1;

		return 2;
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
	//! Measured off the HIGH handle, the one that is always visible.
	//!
	//! A hidden widget measures zero wide, so taking this from the low handle
	//! returned 0 the moment single mode hid it - and the remaining handle was
	//! then drawn with its LEFT edge on the value instead of its centre.
	private float HandleHalfWidth()
	{
		if ( !m_Track || !m_HandleHigh )
			return 0;

		float trackW, trackH;
		m_Track.GetScreenSize( trackW, trackH );
		if ( trackW <= 0 )
			return 0;

		float handleW, handleH;
		m_HandleHigh.GetScreenSize( handleW, handleH );

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

		//! Every mutator lands here, so pinning the pair in this one place is
		//! what keeps SetRange, SetMinMax and SetRangeLow honest in single mode
		//! without each of them having to know the mode exists.
		if ( m_Single )
			m_Low = m_High;

		float fillLeft  = ( m_Low  - m_Min ) / range;
		float fillRight = ( m_High - m_Min ) / range;

		//! A plain slider fills from its own minimum. With both bounds
		//! collapsed onto the value there is no span between them to draw, so
		//! the bar would read empty at every setting.
		if ( m_Single )
			fillLeft = 0;

		float fillWidth = fillRight - fillLeft;

		if ( m_Fill )
		{
			m_Fill.SetPos( fillLeft, 0 );
			m_Fill.SetSize( fillWidth, 1 );
		}

		//! The arrows ride the filled span, and they move every frame while
		//! this only runs on a change - so the span is kept for UpdateFlow to
		//! read rather than the host being placed here.
		m_FlowLeft  = fillLeft;
		m_FlowWidth = fillWidth;

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

		PaintGradient( fillLeft, fillRight );

		if ( m_RangeText )
			m_RangeText.SetText( FormatRange() );
	}

	//! Formats both ends through the value format.
	//!
	//! The format has to be TRANSLATED first. Callers pass a stringtable key
	//! ("#STR_COT_FORMAT_PERCENTAGE"), the same as UIActionSlider takes, and
	//! this used to hand that key straight to string.Format - so a percentage
	//! range rendered as the literal "%1% - %1%" instead of "20% - 80%".
	//!
	//! Whole-number steps round, matching UIActionSlider, so a 1-step range
	//! does not read as "19.999999%".
	protected string FormatRange()
	{
		string fmt = Widget.TranslateString( m_Format );

		//! One number in single mode. "30 - 30" is not a range, and reading it
		//! as one is exactly the confusion the mode exists to remove.
		if ( m_Single && m_Step >= 1.0 )
			return string.Format( fmt, Math.Round( m_High ) );

		if ( m_Single )
			return string.Format( fmt, m_High );

		if ( m_Step >= 1.0 )
			return string.Format( fmt, Math.Round( m_Low ) ) + " - " + string.Format( fmt, Math.Round( m_High ) );

		return string.Format( fmt, m_Low ) + " - " + string.Format( fmt, m_High );
	}
}
