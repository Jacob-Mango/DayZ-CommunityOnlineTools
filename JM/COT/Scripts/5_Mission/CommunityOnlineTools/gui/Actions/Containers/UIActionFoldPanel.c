// =============================================================================
//  UIActionFoldPanel
//
//  A container that opens and closes with a slide. Put rows into GetContent()
//  and call SetExpanded / Toggle; the panel measures its own content every
//  frame and animates its clipped height toward it, so the host never has to
//  know how tall the rows came out.
//
//  This is the piece that a "fold" made out of raw widgets keeps getting wrong.
//  A clipping panel CANNOT size itself to its content - it holds a spacer whose
//  own height is a fraction of the panel, so "panel fits child, child fills
//  panel" resolves circularly and the panel grows down the form. The fix is not
//  to guess the height from a row count, and not to poll and pin it from the
//  host: it is to let the control own the measurement, which it can do because
//  UIActionBase already runs it every frame.
//
//  Folds nest. A fold inside another fold's content changes that content's
//  height as it animates, and the outer one is re-measuring every frame, so it
//  follows along on its own.
//
//  Fires UIEvent.CHANGE on every frame the height moves - a host inside a
//  scroller wants that to re-run UpdateScroller().
//
//  Usage:
//      UIActionFoldPanel fold = UIActionManager.CreateFoldPanel( body, this, "OnChange_Fold" );
//      fold.SetColor( JMTheme.SURFACE_SUNKEN );
//      fold.SetExpanded( false, false );          // start closed, no animation
//
//      UIActionManager.CreateText( fold.GetContent(), "a row" );
//
//      // later, from a button:
//      fold.Toggle();
// =============================================================================

class UIActionFoldPanel: UIActionBase
{
	protected Widget m_Content;
	protected bool  m_Expanded;

	//! Height of the content as last measured. Held across frames because
	//! GetScreenSize answers 0 until the rows have actually been laid out.
	protected float m_FullHeight;

	//! 0 = shut, 1 = fully open. The panel's height is this times m_FullHeight,
	//! so a content change mid-open is picked up without restarting anything.
	protected ref JMAnimFloat m_Anim;
	static const float ANIM_SPEED = 14.0;

	//! The widget rows go into.
	Widget GetContent()
	{
		return m_Content;
	}

	bool IsAnimating()
	{
		return m_Anim.IsAnimating();
	}

	bool IsExpanded()
	{
		return m_Expanded;
	}

	//! `animate` false snaps, for setting up a fold that is being rebuilt in a
	//! state the user already put it in - it should not replay the slide.
	void SetExpanded( bool expanded, bool animate = true )
	{
		if ( m_Expanded == expanded )
			return;

		m_Expanded = expanded;

		float target = 0;
		if ( expanded )
			target = 1.0;

		if ( animate )
		{
			m_Anim.SetTarget( target, ANIM_SPEED );
			return;
		}

		m_Anim.Set( target );
		ApplyHeight();
	}

	override void OnInit()
	{
		super.OnInit();

		m_Content = layoutRoot.FindAnyWidget( "action_content" );

		m_Expanded   = true;
		m_FullHeight = 0;

		m_Anim = new JMAnimFloat();
		m_Anim.Set( 1.0 );
	}

	void Toggle()
	{
		SetExpanded( !m_Expanded );
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		float previous = m_FullHeight;

		MeasureContent();

		bool moved = m_Anim.Step( timeSlice );

		// Nothing to do on a frame where neither the slide nor the rows moved.
		// Folds are cheap to leave lying around open; they should not repaint.
		if ( !moved && m_FullHeight == previous )
			return;

		ApplyHeight();

		CallEvent( UIEvent.CHANGE );
	}

	//! The content keeps its full height whatever the panel is clipped to, so
	//! this is the height the fold is opening toward. Zero is not an answer -
	//! it means "not laid out yet" - so the last real measurement stands.
	protected void MeasureContent()
	{
		if ( !m_Content )
			return;

		float cw, ch;
		m_Content.GetScreenSize( cw, ch );

		if ( ch >= 1 )
			m_FullHeight = ch;
	}

	protected void ApplyHeight()
	{
		if ( !layoutRoot )
			return;

		float height = m_FullHeight * m_Anim.Value;

		float w, h;
		layoutRoot.GetSize( w, h );

		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE );
		layoutRoot.SetSize( w, height );
		layoutRoot.Update();

		// A shut fold is not just zero pixels tall - its rows must not answer
		// the pointer either, or a button under a closed fold is still clickable
		// through the collapsed strip.
		if ( m_Content )
			m_Content.Show( height >= 1 );
	}
}
