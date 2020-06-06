class JMWindowManager
{
	private autoptr array< JMWindowBase > m_Windows;

	private ImageWidget m_MainCursorWidget;

	void JMWindowManager()
	{
		m_Windows = new array< JMWindowBase >;
	}

	void ~JMWindowManager()
	{
	}

	JMWindowBase Create()
	{
		JMWindowBase window;
		GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/windowbase.layout", JMStatics.WINDOWS_CONTAINER ).GetScript( window );
		return window;
	}

	void Update( float pDt )
	{
		Widget widget_under_mouse = GetWidgetUnderCursor();
		if ( widget_under_mouse.GetName().IndexOf( "resize_drag" ) )
		{
			
		}
	}

	int Count()
	{
		return m_Windows.Count();
	}

	void RemoveWindow( JMWindowBase window )
	{
		if ( window.GetModule() )
		{
			window.GetModule().SetMenuButtonColour( 1, 1, 0, 0 );
		}

		int cIdx = m_Windows.Find( window );

		if ( cIdx >= 0 )
		{
			m_Windows.RemoveOrdered( cIdx );
		}
	}

	void AddWindow( JMWindowBase window )
	{
		if ( window.GetModule() )
		{
			window.GetModule().SetMenuButtonColour( 1, 0.5, 0, 0 );
		}

		m_Windows.Insert( window );
	}

	void BringFront( JMWindowBase window )
	{
		int cIdx = m_Windows.Find( window );

		if ( cIdx >= 0 )
		{
			m_Windows.RemoveOrdered( cIdx );

			if ( m_Windows.Count() > 0 )
			{
				m_Windows.InsertAt( window, 0 );
			} else
			{
				m_Windows.Insert( window );
			}
		} else
		{
			return;
		}

		for ( int i = 0; i < m_Windows.Count(); i++ )
		{
			m_Windows[i].GetLayoutRoot().SetSort( m_Windows.Count() - i );

			m_Windows[i].Unfocus();
		}

		window.Focus();
	}

	JMWindowBase GetWindowFromWidget( notnull Widget w )
	{  
		for ( int i = 0; i < m_Windows.Count(); i++ )
		{
			if ( w == m_Windows[i].GetLayoutRoot() )
			{
				return m_Windows[i];
			}
		}

		return NULL;
	}
}

ref JMWindowManager g_cot_WindowManager;

ref JMWindowManager GetCOTWindowManager()
{
	if ( !g_cot_WindowManager )
	{
		g_cot_WindowManager = new JMWindowManager();
	}

	return g_cot_WindowManager;
}