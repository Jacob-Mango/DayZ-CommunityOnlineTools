#ifndef CF_WINDOWS
class JMWindowManager
{
	protected ref array< JMWindowBase > m_Windows;
	protected ref array< JMWindowBase > m_WindowsPendingDeletion;
	protected ImageWidget m_MainCursorWidget;
	protected ref JMWindowLayoutStore m_LayoutStore;

	void JMWindowManager()
	{
		m_Windows = new array< JMWindowBase >;
		m_WindowsPendingDeletion = new array< JMWindowBase >;
		m_LayoutStore = JMWindowLayoutStore.Load();
	}

	//! Saved rect for a module's window, if it was ever moved/resized. Returns
	//! false (leaving x/y/w/h untouched) when the module has no saved entry.
	bool TryGetSavedLayout( string moduleName, out float x, out float y, out float w, out float h )
	{
		JMWindowLayoutEntry entry = m_LayoutStore.FindEntry( moduleName );
		if ( !entry )
			return false;

		x = entry.X;
		y = entry.Y;
		w = entry.W;
		h = entry.H;
		return true;
	}

	//! Persists the window's current rect under its module's name.
	void SaveWindowLayout( JMWindowBase window )
	{
		if ( !window || !window.GetModule() || !window.GetLayoutRoot() )
			return;

		float x, y, w, h;
		window.GetLayoutRoot().GetPos( x, y );
		window.GetLayoutRoot().GetSize( w, h );

		m_LayoutStore.SetEntry( window.GetModule().GetModuleName(), x, y, w, h );
		JMWindowLayoutStore.Save( m_LayoutStore );
	}

	JMWindowBase Get( int index )
	{
		return m_Windows[index];
	}

	//! Escape priority tier 2 (module menu): the frontmost visible window, if
	//! any. BringFront() keeps m_Windows ordered front-to-back (index 0 is
	//! last-focused), so the first visible entry is the one on top.
	JMWindowBase GetTopActive()
	{
		foreach (JMWindowBase window: m_Windows)
		{
			if ( window.IsVisible() )
				return window;
		}
		return NULL;
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

	bool HasAnyActive()
	{
		foreach(JMWindowBase window: m_Windows)
		{
			if ( window.IsVisible() )
				return true;
		}
		return false;
	}

	bool HasAnyUnpinnedActive()
	{
		foreach(JMWindowBase window: m_Windows)
		{
			if ( window.IsVisible() && !window.IsPinned() )
				return true;
		}
		return false;
	}

	JMWindowBase Create()
	{
		JMWindowBase window;
		g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/windowbase.layout", JMStatics.WINDOWS_CONTAINER ).GetScript( window );
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
		{
			string title = "?";
			if (window.GetModule())
				title = window.GetModule().GetTitle();
			window.Show();
		}
	}

	void HideAllActive()
	{
		foreach(JMWindowBase window: m_Windows)
		{
			string title = "?";
			if (window.GetModule())
				title = window.GetModule().GetTitle();

			if ( !window.IsPinned() )
			{
				window.Hide();
			}
			else
			{
			}
		}
	}

	int Count()
	{
		return m_Windows.Count();
	}

	int PendingDeletionCount()
	{
		return m_WindowsPendingDeletion.Count();
	}

	void RemoveWindow( JMWindowBase window )
	{
		if ( window.GetModule() )
		{

			window.GetModule().SetMenuButtonColor( JMTheme.TRANSPARENT );
			window.GetModule().OnWindowDestroyed();
		}

		int cIdx = m_Windows.Find( window );

		if ( cIdx >= 0 )
		{
			m_Windows.RemoveOrdered( cIdx );
		}

		m_WindowsPendingDeletion.Insert( window );

		//! Purge after a delay so we can still block certain actions while closing a window
		g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(PurgeDeletedWindows, 250);
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
			window.GetModule().SetMenuButtonColor( JMTheme.MENU_ACTIVE );
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
			//! Offset above JMUILayout.SORT_WINDOW, not just 1..N, so an open
			//! window clears DayZ-Expansion's map marker sort (999) too. Spaced
			//! by SORT_WINDOW_STEP rather than 1, so the just-focused window
			//! (index 0, the highest number here) sits a clean 10 above the
			//! next one down rather than by a margin nothing else respects.
			m_Windows[i].GetLayoutRoot().SetSort( JMUILayout.SORT_WINDOW + ( m_Windows.Count() - i ) * JMUILayout.SORT_WINDOW_STEP );

			m_Windows[i].Unfocus();
		}

		window.Focus();
	}

	void DestroyAllWindows()
	{
		for (int i = m_Windows.Count() - 1; i >= 0; i--)
		{
			m_Windows[i].Destroy();
		}
	}
}


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