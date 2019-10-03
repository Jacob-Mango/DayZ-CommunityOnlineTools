class UIActionScroller extends UIActionBase 
{
	protected Widget m_Content;
	protected Widget m_ScrollerContainer;
	protected Widget m_Scroller;

	const int WHEEL_STEP = 20;
	protected float m_root_height;
	protected float m_content_height;
	protected float m_position;
	protected bool m_scrolling;
	protected float m_scrolling_start_pos;
	protected int m_scrolling_mouse_pos;

	override void OnInit() 
	{
		super.OnInit();
		
		m_Content = layoutRoot.FindAnyWidget( "Content" );
		m_ScrollerContainer = layoutRoot.FindAnyWidget( "ScrollerContainer" );
		m_Scroller = layoutRoot.FindAnyWidget( "Scroller" );

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

    protected void UpdateScroller()
    {
        Print( "UpdateScroller" );
		m_Content.Update();
		m_ScrollerContainer.Update();
		m_Scroller.Update();
        layoutRoot.Update();

		float width;
		float height;
		float diff;
		float scroller_height;
	
		m_ScrollerContainer.GetScreenSize( width, m_root_height );
		m_Content.GetScreenSize( width, m_content_height );

        Print( m_root_height );
        Print( m_content_height );
	
		diff = m_content_height - m_root_height;
        
        Print( diff );
		if ( diff <= 0 )
		{
			m_Content.SetPos( 0, 0 );
			m_ScrollerContainer.Show( false );
			m_Scroller.Show( false );
			m_position = 0;
			return;
		}
		
		scroller_height = ( m_root_height / m_content_height ) * m_root_height;

        Print( scroller_height );

		m_ScrollerContainer.Show(true);
		m_Scroller.Show(true);
		m_Scroller.GetSize( width, height );
		m_Scroller.SetSize( width, scroller_height );

        Print( height );
		float pos = ( -m_content_height * m_position );
		
        Print( pos );
		if ( pos <= -diff )
        {
			pos = -diff;
        }
		
        Print( pos );
		m_Scroller.SetPos( 0, -pos );
		m_Content.SetPos( 0, pos );
    }

	protected void UpdateScroll( int mouse_x, int mouse_y, bool is_dragging )
	{
		layoutRoot.Update();
		m_Content.Update();
		float width;
	
		layoutRoot.GetScreenSize( width, m_root_height );
		m_Content.GetScreenSize( width, m_content_height );
		
		if ( m_scrolling )
		{
			if ( is_dragging )
			{
				float diff = (mouse_y - m_scrolling_mouse_pos);
				float scroller_height = ( m_root_height / m_content_height ) * m_root_height; 
				m_position = m_scrolling_start_pos + ( diff / ( m_root_height - scroller_height ) );

				if ( m_position < 0 )
                    m_position = 0;
				if ( m_position > 1 )
                    m_position = 1;
			} else
			{
				m_scrolling = false;
				StopScrolling();
			}
		}
		
		UpdateScroller();
	}

	protected void StopScrolling()
	{
		if ( m_scrolling )
		{
			GetGame().GetDragQueue().RemoveCalls(this);
			m_scrolling = false;
		}
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button)
	{
		if ( button == MouseState.LEFT && w == m_Scroller && !m_scrolling )
		{
			m_scrolling = true;
			m_scrolling_start_pos = m_position;
			int mouse_x;
			GetMousePos( mouse_x, m_scrolling_mouse_pos );
			GetGame().GetDragQueue().Call( this, "UpdateScroll" );
			return true;
		}
	
		return false;
	}
	
	override bool OnMouseButtonUp( Widget w, int x, int y, int button)
	{
		StopScrolling();

		return false;
	}
	
	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		if ( m_scrolling || m_content_height <= m_root_height )
            return false;
	
		float step = ( 1.0 / ( m_content_height - m_root_height ) ) * WHEEL_STEP;
		m_position -= wheel * step;
	
		if ( m_position < 0 )
            m_position = 0;

		if ( m_position > 1 )
            m_position = 1;

		UpdateScroller();
		return true;
	}
	
	override bool OnResize( Widget w, int x, int y )
    {
		if ( w == layoutRoot || w == m_Content ) 
		{
			// UpdateScroller();
		}

		return false;
	}
}