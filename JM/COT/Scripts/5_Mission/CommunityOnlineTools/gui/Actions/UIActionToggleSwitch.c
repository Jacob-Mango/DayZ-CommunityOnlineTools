// =============================================================================
//  UIActionToggleSwitch
//
//  A visual on/off switch with an animated thumb that slides across the track
//  (track + thumb colour also cross-fade).  Fires UIEvent.CLICK on every toggle.
// =============================================================================
class UIActionToggleSwitch: UIActionBase
{
	protected TextWidget   m_Label;
	protected ButtonWidget m_Button;
	protected Widget       m_Track;
	protected Widget       m_Thumb;

	protected bool m_Checked;

	protected ref JMAnimFloat  m_AnimPos;     // thumb X
	protected ref JMAnimColor  m_AnimTrack;   // track colour
	protected ref JMAnimColor  m_AnimThumb;   // thumb colour

	static const int COLOR_TRACK_ON  = JMTheme.ACCENT;
	static const int COLOR_TRACK_OFF = JMTheme.INK_500;
	static const int COLOR_THUMB_ON  = JMTheme.TEXT_ON_ACCENT;
	static const int COLOR_THUMB_OFF = JMTheme.INK_200;

	// Thumb travel fallbacks, in layout pixels, matching UIActionToggleSwitch.layout:
	// track 44 wide, thumb 18 across, 2px inset at each end.
	//   OFF = inset            = 2
	//   ON  = 44 - 18 - inset  = 24   (flush with the right end)
	// These are only fallbacks. The button does not always render at the
	// authored 44 units, so a travel hardcoded from that number leaves the thumb
	// short of the right end. Measure() replaces both values with the button's
	// real size.
	static const float THUMB_X_ON  = 24;
	static const float THUMB_X_OFF =  2;

	protected float m_ThumbXOn  = THUMB_X_ON;
	protected float m_ThumbXOff = THUMB_X_OFF;
	protected float m_MeasuredWidth;

	static const float ANIM_SPEED = 16.0;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,  layoutRoot.FindAnyWidget( "action_label"  ) );
		Class.CastTo( m_Button, layoutRoot.FindAnyWidget( "action_button" ) );
		Class.CastTo( m_Track,  layoutRoot.FindAnyWidget( "action_track"  ) );
		Class.CastTo( m_Thumb,  layoutRoot.FindAnyWidget( "action_thumb"  ) );

		m_Checked    = false;
		m_AnimPos    = new JMAnimFloat();   m_AnimPos.Set( m_ThumbXOff );
		m_AnimTrack  = new JMAnimColor();   m_AnimTrack.Set( COLOR_TRACK_OFF );
		m_AnimThumb  = new JMAnimColor();   m_AnimThumb.Set( COLOR_THUMB_OFF );

		ApplyImmediate();
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_Label )
			m_Label.SetText( text );
	}

	override void SetChecked( bool checked )
	{
		m_Checked = checked;
		RetargetAnim();
	}

	override bool IsChecked()
	{
		return m_Checked;
	}

	void Toggle()
	{
		SetChecked( !m_Checked );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_Button )
		{
			Toggle();
			CallEvent( UIEvent.CLICK );
			return true;
		}
		return false;
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		Measure();

		bool tickedPos   = m_AnimPos.Step( timeSlice );
		bool tickedTrack = m_AnimTrack.Step( timeSlice );
		bool tickedThumb = m_AnimThumb.Step( timeSlice );

		if ( tickedPos && m_Thumb )
		{
			float dummy, h;
			m_Thumb.GetPos( dummy, h );
			m_Thumb.SetPos( m_AnimPos.Value, h );
		}
		if ( tickedTrack && m_Track )
			m_Track.SetColor( m_AnimTrack.Value );
		if ( tickedThumb && m_Thumb )
			m_Thumb.SetColor( m_AnimThumb.Value );
	}

	// Derive the thumb's end positions from the track's real size, so the ON
	// position sits flush with the right end even when the button does not
	// render at the width the layout authored.
	//
	// GetSize (NOT GetScreenSize) is required here: it reports in the widget's
	// own flag space, which is the same space SetPos writes in. GetScreenSize
	// returns monitor pixels, a different space at any workspace scale other
	// than 1:1, and feeding those back into SetPos would overshoot.
	//
	// GetSize returns 0 until the widget has been laid out, hence the bail-out;
	// the width check makes this a no-op on every frame but the first and any
	// subsequent resize.
	private void Measure()
	{
		if ( !m_Button || !m_Thumb )
			return;

		float trackW, trackH, thumbW, thumbH;
		m_Button.GetSize( trackW, trackH );
		m_Thumb.GetSize( thumbW, thumbH );

		if ( trackW <= 0 || thumbW <= 0 )
			return;
		if ( trackW == m_MeasuredWidth )
			return;

		m_MeasuredWidth = trackW;

		// The vertical gap above/below the thumb is the same inset the design
		// uses horizontally, and it is already in the right space.
		float inset = ( trackH - thumbH ) * 0.5;
		if ( inset < 0 )
			inset = 0;

		m_ThumbXOff = inset;
		m_ThumbXOn  = trackW - thumbW - inset;
		if ( m_ThumbXOn < m_ThumbXOff )
			m_ThumbXOn = m_ThumbXOff;

		ApplyImmediate();
	}

	private void RetargetAnim()
	{
		int trackColor = COLOR_TRACK_OFF;
		int thumbColor = COLOR_THUMB_OFF;
		float px       = m_ThumbXOff;
		if ( m_Checked )
		{
			trackColor = COLOR_TRACK_ON;
			thumbColor = COLOR_THUMB_ON;
			px         = m_ThumbXOn;
		}

		m_AnimPos.SetTarget(   px,         ANIM_SPEED );
		m_AnimTrack.SetTarget( trackColor, ANIM_SPEED );
		m_AnimThumb.SetTarget( thumbColor, ANIM_SPEED );
	}

	// Snap visuals to the current state without animating (initial paint only).
	private void ApplyImmediate()
	{
		int trackColor = COLOR_TRACK_OFF;
		int thumbColor = COLOR_THUMB_OFF;
		float px       = m_ThumbXOff;
		if ( m_Checked )
		{
			trackColor = COLOR_TRACK_ON;
			thumbColor = COLOR_THUMB_ON;
			px         = m_ThumbXOn;
		}

		m_AnimPos.Set(   px );
		m_AnimTrack.Set( trackColor );
		m_AnimThumb.Set( thumbColor );

		if ( m_Track ) m_Track.SetColor( trackColor );
		if ( m_Thumb )
		{
			m_Thumb.SetColor( thumbColor );
			float dummy, h;
			m_Thumb.GetPos( dummy, h );
			m_Thumb.SetPos( px, h );
		}
	}
}
