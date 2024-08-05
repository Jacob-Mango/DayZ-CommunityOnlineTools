#ifndef CF_WINDOWS
class JMWindowManager
{
	private ref array< JMWindowBase > m_Windows;
	private ref array< JMWindowBase > m_WindowsPendingDeletion;

	private ImageWidget m_MainCursorWidget;

	void JMWindowManager()
	{
		m_Windows = new array< JMWindowBase >;
		m_WindowsPendingDeletion = new array< JMWindowBase >;
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

	void ShowAllActive()
	{
		foreach(JMWindowBase window: m_Windows)
			window.Show();
	}

	void HideAllActive()
	{
		foreach(JMWindowBase window: m_Windows)
			window.Hide();
	}

	bool HasAnyActive()
	{
		foreach(JMWindowBase window: m_Windows)
		{
			if ( window.IsVisible() )
				return true;
		}
		return false;
	}

	int Count()
	{
		return m_Windows.Count();
	}

	int PendingDeletionCount()
	{
		return m_WindowsPendingDeletion.Count();
	}

	JMWindowBase Get( int index )
	{
		return m_Windows[index];
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

		m_WindowsPendingDeletion.Insert( window );

		//! Purge after a delay so we can still block certain actions while closing a window
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(PurgeDeletedWindows, 250);
	}

	void PurgeDeletedWindows()
	{
		for (int i = m_WindowsPendingDeletion.Count() - 1; i >= 0; i--)
		{
			JMWindowBase window = m_WindowsPendingDeletion[i];
			if (!window || !window.GetLayoutRoot())
				m_WindowsPendingDeletion.RemoveOrdered(i);
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
};


ref JMWindowManager g_cot_WindowManager;

JMWindowManager GetCOTWindowManager()
{
	if ( !g_cot_WindowManager )
	{
		g_cot_WindowManager = new JMWindowManager();
	}

	return g_cot_WindowManager;
}
#endif