// =============================================================================
//  UIActionScrollerH
//
//  Horizontal-scrolling variant of UIActionScroller. Same widget pattern
//  (Content GridSpacer + ScrollerContainer + Scroller thumb) but the geometry
//  is flipped to the X axis: Content grows horizontally, thumb sits at the
//  bottom edge, mouse wheel scrolls left/right.
//
//  Used by the Object Spawner category strip; could be reused for any
//  horizontally-scrolling row of chips/buttons.
// =============================================================================
class UIActionScrollerH: UIActionBase
{
	const int WHEEL_STEP = 40;

	protected Widget m_Content;
	protected Widget m_ScrollerContainer;
	protected Widget m_Scroller;

	protected float m_RootWidth;
	protected float m_ContentWidth;
	protected float m_Position;

	protected float m_ScrollStartPos;
	protected int   m_MouseStartPos;

	protected bool m_IsDragScrolling;
	protected bool m_IsMouseScrolling;

	protected bool m_IsUpdating;

	override void OnInit()
	{
		super.OnInit();

		m_Content           = layoutRoot.FindAnyWidget( "Content" );
		m_ScrollerContainer = layoutRoot.FindAnyWidget( "ScrollerContainer" );
		m_Scroller          = layoutRoot.FindAnyWidget( "Scroller" );

		m_Position = 0;

		UpdateScroller();
	}

	Widget GetContentWidget()
	{
		return m_Content;
	}

	void UpdateScroller()
	{
		if ( m_IsUpdating )
			return;

		m_IsUpdating = true;
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( _UpdateScroller, 34 );
	}

	void _UpdateScroller()
	{
		m_IsUpdating = false;

		m_Content.Update();
		m_ScrollerContainer.Update();
		m_Scroller.Update();
		layoutRoot.Update();

		float dummy;
		float scrollerHeight;
		float scrollerWidth;
		float layoutW;
		float layoutH;

		m_ScrollerContainer.GetScreenSize( dummy, scrollerHeight );
		m_Content.GetScreenSize( m_ContentWidth, dummy );
		layoutRoot.GetScreenSize( layoutW, layoutH );

		m_RootWidth = layoutW;

		float diff = m_ContentWidth - m_RootWidth;

		if ( diff < 0 )
		{
			// Content fits within the visible area: hide the bar entirely.
			m_Content.SetPos( 0, 0 );
			m_ScrollerContainer.Show( false );
			m_Scroller.Show( false );

			// Do NOT set the content height here. The Content widget has
			// vexactsize 0 (fractional) + "Size To Content H" in the layout, so
			// it already fills the parent's height and sizes its width to the
			// chips. Passing GetScreenSize values (screen px) into SetSize
			// (layout px) shrinks the row on scaled displays - the "fixed short
			// height" bug. Screen measurements are only for scroll math below.
			m_Position = 0;
			return;
		}

		// Reserve scrollerHeight pixels at the bottom for the bar.
		m_ScrollerContainer.SetSize( 1, scrollerHeight );
		m_ScrollerContainer.SetPos( 0, layoutH - scrollerHeight );
		m_ScrollerContainer.SetSort( 1 );

		float thumbWidth = ( m_RootWidth / m_ContentWidth ) * m_RootWidth;

		m_ScrollerContainer.Show( true );
		m_Scroller.Show( true );

		float thumbW;
		float thumbH;
		m_Scroller.GetSize( thumbW, thumbH );
		m_Scroller.SetSize( thumbWidth, thumbH );

		if ( m_Position < 0 )
			m_Position = 0;
		if ( m_Position > 1 )
			m_Position = 1;

		float thumbPos    = ( m_RootWidth - thumbWidth ) * m_Position;
		float contentPos  = -( m_ContentWidth - m_RootWidth ) * m_Position;

		m_Scroller.SetPos( thumbPos, 0 );
		m_Content.SetPos( contentPos, 0 );
	}

	protected void UpdateDragScroll( int mouse_x, int mouse_y, bool is_dragging )
	{
		layoutRoot.Update();
		m_Content.Update();

		float dummy;
		layoutRoot.GetScreenSize( m_RootWidth, dummy );
		m_Content.GetScreenSize( m_ContentWidth, dummy );

		if ( m_IsDragScrolling )
		{
			if ( is_dragging )
			{
				float deltaPx = ( mouse_x - m_MouseStartPos );
				float thumbWidth = ( m_RootWidth / m_ContentWidth ) * m_RootWidth;
				m_Position = m_ScrollStartPos + ( deltaPx / ( m_RootWidth - thumbWidth ) );
			}
			else
			{
				m_IsDragScrolling = false;
				StopDragScrolling();
			}
		}

		UpdateScroller();
	}

	protected void StopDragScrolling()
	{
		if ( m_IsDragScrolling )
			m_IsDragScrolling = false;
	}

	protected void StopMouseScrolling()
	{
		if ( m_IsMouseScrolling )
			m_IsMouseScrolling = false;
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		// Re-check scroll state when the container resizes (e.g. window resize).
		if ( !m_IsUpdating && layoutRoot )
		{
			float w, h;
			layoutRoot.GetScreenSize( w, h );
			if ( w != m_RootWidth )
				UpdateScroller();
		}
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( button != MouseState.LEFT )
			return false;

		if ( w == m_Scroller && !m_IsMouseScrolling && !m_IsDragScrolling )
		{
			m_IsDragScrolling = true;
			m_ScrollStartPos = m_Position;
			int dummyY;
			GetMousePos( m_MouseStartPos, dummyY );
			g_Game.GetDragQueue().Call( this, "UpdateDragScroll" );
			return true;
		}

		return false;
	}

	override bool OnMouseButtonUp( Widget w, int x, int y, int button )
	{
		StopDragScrolling();
		StopMouseScrolling();
		return false;
	}

	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		if ( m_IsDragScrolling || m_ContentWidth <= m_RootWidth )
			return false;

		float step = ( 1.0 / ( m_ContentWidth - m_RootWidth ) ) * WHEEL_STEP;
		m_Position -= wheel * step;

		UpdateScroller();
		return true;
	}
}
