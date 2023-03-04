class UIActionScroller: UIActionBase 
{
	const int WHEEL_STEP = 20;

	protected Widget m_Content;
	protected Widget m_ScrollerContainer;
	protected Widget m_Scroller;

	protected float m_RootHeight;
	protected float m_ContentHeight;
	protected float m_Position;

	protected float m_ScrollStartPos;
	protected int m_MouseStartPos;

	protected bool m_IsDragScrolling;
	protected bool m_IsMouseScrolling;

	protected bool m_IsUpdating;

	override void OnInit() 
	{
		super.OnInit();
		
		m_Content = layoutRoot.FindAnyWidget( "Content" );
		m_ScrollerContainer = layoutRoot.FindAnyWidget( "ScrollerContainer" );
		m_Scroller = layoutRoot.FindAnyWidget( "Scroller" );

		m_Position = 0;

		UpdateScroller();
	}

	Widget GetContentWidget()
	{
		return m_Content;
	}

	override void OnShow()
	{
	}

	override void OnHide() 
	{
	}

	void UpdateScroller()
	{
		m_IsUpdating = true;
		
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(_UpdateScroller, 34);
	}

	void _UpdateScroller()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::UpdateScroller" );
		#endif

		m_Content.Update();
		m_ScrollerContainer.Update();
		m_Scroller.Update();
		layoutRoot.Update();

		float width;
		float height;
		float diff;
		float scroller_height;
		float scroller_width;
	
		m_ScrollerContainer.GetScreenSize( scroller_width, m_RootHeight );
		m_Content.GetScreenSize( width, m_ContentHeight );

		float layoutRootWidth;
		float layoutRootHeight;
		layoutRoot.GetScreenSize( layoutRootWidth, layoutRootHeight );

		diff = m_ContentHeight - m_RootHeight;
		
		if ( diff < 0 )
		{
			m_Content.SetPos( 0, 0 );
			m_ScrollerContainer.Show( false );
			m_Scroller.Show( false );

			m_Content.SetSize( layoutRootWidth, m_ContentHeight );
			m_Position = 0;
			return;
		}
		
		m_Content.SetSize( layoutRootWidth - scroller_width, m_ContentHeight );
		m_ScrollerContainer.SetSize( scroller_width, 1 );
		m_ScrollerContainer.SetPos( layoutRootWidth - scroller_width, 0 );
		m_ScrollerContainer.SetSort( 1 );
		
		scroller_height = ( m_RootHeight / m_ContentHeight ) * m_RootHeight;

		m_ScrollerContainer.Show(true);
		m_Scroller.Show(true);
		m_Scroller.GetSize( width, height );
		m_Scroller.SetSize( width, scroller_height );

		if ( m_Position < 0 )
			m_Position = 0;

		if ( m_Position > 1 )
			m_Position = 1;

		float scrollerPos = m_RootHeight - scroller_height;
		scrollerPos = scrollerPos * m_Position;

		float contentPos = -( m_ContentHeight - m_RootHeight );
		contentPos = contentPos * m_Position;

		m_Scroller.SetPos( 0, scrollerPos );
		m_Content.SetPos( 0, contentPos );

		m_IsUpdating = false;

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::UpdateScroller" );
		#endif
	}

	protected void UpdateDragScroll( int mouse_x, int mouse_y, bool is_dragging )
	{
		layoutRoot.Update();
		m_Content.Update();
		float width;
	
		layoutRoot.GetScreenSize( width, m_RootHeight );
		m_Content.GetScreenSize( width, m_ContentHeight );
		
		if ( m_IsDragScrolling )
		{
			if ( is_dragging )
			{
				float diff = ( mouse_y - m_MouseStartPos );
				float scroller_height = ( m_RootHeight / m_ContentHeight ) * m_RootHeight; 
				m_Position = m_ScrollStartPos + ( diff / ( m_RootHeight - scroller_height ) );
			} else
			{
				m_IsDragScrolling = false;
				StopDragScrolling();
			}
		}
		
		UpdateScroller();
	}

	protected void UpdateMouseScroll()
	{
		float posX;
		float posY;
		int mouse_x;
		int mouse_y;

		m_Scroller.GetScreenPos( posX, posY );
		GetMousePos( mouse_x, mouse_y );

		int wheel = 0;
		if ( posY > mouse_y - WHEEL_STEP )
		{
			wheel = 1;
		} else if ( posY < mouse_y + WHEEL_STEP )
		{
			wheel = -1;
		}

		float step = ( 1.0 / ( m_ContentHeight - m_RootHeight ) ) * WHEEL_STEP;
		m_Position -= wheel * step;
	}

	protected void StopDragScrolling()
	{
		if ( m_IsDragScrolling )
		{
			m_IsDragScrolling = false;
		}
	}

	protected void StopMouseScrolling()
	{
		if ( m_IsMouseScrolling )
		{
			m_IsMouseScrolling = false;
		}
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( m_IsMouseScrolling )
		{
			UpdateMouseScroll();
			UpdateScroller();
		}
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button)
	{
		if ( button != MouseState.LEFT )
		{
			return false;
		}

		if ( w == m_ScrollerContainer && !m_IsMouseScrolling && !m_IsDragScrolling )
		{
			m_IsMouseScrolling = true;
			
			return true;
		}

		if ( w == m_Scroller && !m_IsMouseScrolling && !m_IsDragScrolling )
		{
			m_IsDragScrolling = true;
			m_ScrollStartPos = m_Position;
			int mouse_x;
			GetMousePos( mouse_x, m_MouseStartPos );
			GetGame().GetDragQueue().Call( this, "UpdateDragScroll" );
			return true;
		}
	
		return false;
	}
	
	override bool OnMouseButtonUp( Widget w, int x, int y, int button)
	{
		StopDragScrolling();
		StopMouseScrolling();

		return false;
	}
	
	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		if ( m_IsDragScrolling || m_ContentHeight <= m_RootHeight )
			return false;
	
		float step = ( 1.0 / ( m_ContentHeight - m_RootHeight ) ) * WHEEL_STEP;
		m_Position -= wheel * step;

		UpdateScroller();

		return true;
	}
	
	//override bool OnUpdate(Widget w)
	//{
	//	//if (!m_IsUpdating) 
	//	//{
	//	//	UpdateScroller();
	//	//}
//
	//	return false;
	//}
}