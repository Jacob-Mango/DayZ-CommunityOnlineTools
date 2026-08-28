// =============================================================================
//  UIActionToggle
//
//  Custom checkbox replacement. Two visual modes:
//    - Square (default) - multi-choice style, rounded-corner box + checkmark
//    - Round            - single-choice / radio style (ring + filled dot)
//
//  Unchecked: grey box / empty ring. Checked: the box fills with the accent
//  colour and a white tick fades in, or the ring gains a coloured dot.
//
//  Fires UIEvent.CHANGE when the value changes.
//  Fires UIEvent.CLICK on every click (before the state change).
// =============================================================================

class UIActionToggle: UIActionBase
{
	protected ButtonWidget m_Button;
	protected Widget       m_BoxSquare;     // styled panel, COTField4Set
	protected Widget       m_BoxRound;      // styled panel, COTPill6LineSet
	protected ImageWidget  m_CheckSquare;   // a tick glyph, still a bitmap
	protected Widget       m_CheckCircle;   // styled panel, COTField4Set
	protected TextWidget   m_Label;

	protected bool  m_Checked;
	protected bool  m_Round;
	protected int   m_CheckColor;
	//! True once SetColor() was called, which pins the mark colour and stops
	//! ApplyShape from re-deriving it from the shape.
	protected bool  m_CheckColorSet;

	protected ref JMAnimFloat m_AnimCheck;
	protected ref JMAnimColor m_AnimBox;

	// OFF must be visibly distinct from the dark form surface. Lighter grey-blue
	// reads as "empty box" against the COTSurface navy background. ON is a
	// brighter blue to match the form's accent color.
	static const int COLOR_BOX_OFF = JMTheme.INK_100;
	//! Matches UIActionToggleSwitch.COLOR_TRACK_ON so a checked box and an on
	//! switch are the same blue.
	static const int COLOR_BOX_ON  = JMTheme.ACCENT;

	//! Default mark colour per shape. The square box fills with COLOR_BOX_ON
	//! when checked, so its tick has to be white to stay readable. The round
	//! box keeps its unfilled ring, so its dot carries the colour instead.
	static const int COLOR_MARK_SQUARE = JMTheme.TEXT_ON_ACCENT;
	static const int COLOR_MARK_ROUND  = JMTheme.ACCENT;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Button,      layoutRoot.FindAnyWidget( "action_button"      ) );
		Class.CastTo( m_BoxSquare,   layoutRoot.FindAnyWidget( "action_box_square"  ) );
		Class.CastTo( m_BoxRound,    layoutRoot.FindAnyWidget( "action_box_round"   ) );
		Class.CastTo( m_CheckSquare, layoutRoot.FindAnyWidget( "action_check"       ) );
		Class.CastTo( m_CheckCircle, layoutRoot.FindAnyWidget( "action_check_image" ) );
		Class.CastTo( m_Label,       layoutRoot.FindAnyWidget( "action_label"       ) );

		m_Checked       = false;
		m_Round         = false;
		m_CheckColorSet = false;
		m_CheckColor    = COLOR_MARK_SQUARE;

		m_AnimCheck = new JMAnimFloat();
		m_AnimCheck.Set( 0 );

		m_AnimBox = new JMAnimColor();
		m_AnimBox.Set( COLOR_BOX_OFF );

		// The box and the radio dot are nine-sliced panels now, so their shape
		// comes from the style and holds at any size; only the tick is still a
		// bitmap, because it is a glyph rather than a shape.
		if ( m_CheckSquare )
		{
			m_CheckSquare.LoadImageFile( 0, JMConstants.SURFACE_CHECK );
			m_CheckSquare.SetImage( 0 );
		}

		ApplyShape();
	}

	override void SetLabel( string text )
	{
		if ( m_Label )
			m_Label.SetText( Widget.TranslateString( text ) );
	}

	void SetRound( bool round )
	{
		m_Round = round;
		ApplyShape();
	}

	bool IsRound() { return m_Round; }

	override bool IsChecked() { return m_Checked; }

	override void SetChecked( bool checked )
	{
		if ( checked == m_Checked )
			return;

		m_Checked = checked;
		ApplyVisuals();
		CallEvent( UIEvent.CHANGE );
	}

	void SetCheckedSilent( bool checked )
	{
		if ( checked == m_Checked )
			return;

		m_Checked = checked;
		SnapVisuals();
	}

	override void SetColor( int color )
	{
		m_CheckColor    = color;
		m_CheckColorSet = true;
		if ( m_CheckSquare ) m_CheckSquare.SetColor( color );
		if ( m_CheckCircle ) m_CheckCircle.SetColor( color );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w != m_Button )
			return false;

		// Order matters: SetChecked first so IsChecked() returns the NEW state
		// when handlers fire on CLICK. Reversed order was causing handlers to
		// store the previous state instead of the freshly-toggled one.
		SetChecked( !m_Checked );
		CallEvent( UIEvent.CLICK );
		return true;
	}

	override bool IsFocusWidget( Widget widget )
	{
		return widget == m_Button;
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		// Guard: Update() can be invoked by the engine before OnInit completes
		// (or after destruction in pathological cases). Dereferencing null here
		// is a confirmed crash path.
		if ( !m_AnimCheck || !m_AnimBox )
			return;

		if ( m_AnimCheck.Step( timeSlice ) )
		{
			if ( m_Round )
			{
				if ( m_CheckCircle ) m_CheckCircle.SetAlpha( m_AnimCheck.Value );
			}
			else
			{
				if ( m_CheckSquare ) m_CheckSquare.SetAlpha( m_AnimCheck.Value );
			}
		}

		if ( m_AnimBox.Step( timeSlice ) && !m_Round )
		{
			if ( m_BoxSquare ) m_BoxSquare.SetColor( m_AnimBox.Value );
		}
	}

	private void ApplyShape()
	{
		// Shape picks the mark colour unless a caller overrode it.
		if ( !m_CheckColorSet )
		{
			if ( m_Round )
				m_CheckColor = COLOR_MARK_ROUND;
			else
				m_CheckColor = COLOR_MARK_SQUARE;
		}

		if ( m_Round )
		{
			if ( m_BoxSquare   ) m_BoxSquare.Show( false );
			if ( m_CheckSquare ) m_CheckSquare.Show( false );
			if ( m_BoxRound    ) { m_BoxRound.Show( true ); m_BoxRound.SetColor( COLOR_BOX_OFF ); m_BoxRound.SetAlpha( 1.0 ); }
			if ( m_CheckCircle ) { m_CheckCircle.Show( true ); m_CheckCircle.SetColor( m_CheckColor ); m_CheckCircle.SetAlpha( 0.0 ); }
		}
		else
		{
			if ( m_BoxRound    ) m_BoxRound.Show( false );
			if ( m_CheckCircle ) m_CheckCircle.Show( false );
			if ( m_BoxSquare   ) { m_BoxSquare.Show( true ); m_BoxSquare.SetColor( COLOR_BOX_OFF ); m_BoxSquare.SetAlpha( 1.0 ); }
			if ( m_CheckSquare ) { m_CheckSquare.Show( true ); m_CheckSquare.SetColor( m_CheckColor ); m_CheckSquare.SetAlpha( 0.0 ); }
		}

		// Snap visuals to the current checked state immediately (no animation), so
		// shape changes don't leave the check widget at a stale alpha.
		SnapVisuals();
	}

	// Set visuals to their final state immediately, bypassing animation.
	// Used during shape change / initial construction where animation would
	// either glitch (stale Value) or be invisible (zero timeSlice).
	private void SnapVisuals()
	{
		float finalAlpha    = 0.0;
		int   finalBoxColor = COLOR_BOX_OFF;

		if ( m_Checked )
		{
			finalAlpha    = 1.0;
			finalBoxColor = COLOR_BOX_ON;
		}

		if ( m_AnimCheck ) m_AnimCheck.Set( finalAlpha );
		if ( m_AnimBox )   m_AnimBox.Set(   finalBoxColor );

		if ( m_Round )
		{
			if ( m_CheckCircle ) m_CheckCircle.SetAlpha( finalAlpha );
			if ( m_BoxRound    ) m_BoxRound.SetColor( COLOR_BOX_OFF );
		}
		else
		{
			if ( m_CheckSquare ) m_CheckSquare.SetAlpha( finalAlpha );
			if ( m_BoxSquare   ) m_BoxSquare.SetColor( finalBoxColor );
		}
	}

	private void ApplyVisuals()
	{
		static const float SPEED = 20.0;

		float targetAlpha    = 0.0;
		int   targetBoxColor = COLOR_BOX_OFF;

		if ( m_Checked )
		{
			targetAlpha    = 1.0;
			targetBoxColor = COLOR_BOX_ON;
		}

		// Hand off to the animation system. Update() will drive SetAlpha / SetColor
		// each tick as long as the anim is dirty. We do NOT set the final values
		// explicitly here -- doing so caused a flicker when the next Step()
		// snapped the widget back to the interpolated value.
		if ( m_AnimCheck ) m_AnimCheck.SetTarget( targetAlpha,    SPEED );
		if ( m_AnimBox )   m_AnimBox.SetTarget(   targetBoxColor, SPEED );
	}
}
