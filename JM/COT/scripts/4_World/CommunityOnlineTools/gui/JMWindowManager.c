class JMWindowManager
{
    private autoptr array< JMWindowBase > m_Windows;

	private ref Widget m_WindowContainer;

    void JMWindowManager()
    {
        m_Windows = new array< JMWindowBase >;
    }

    void ~JMWindowManager()
    {
    }

    void Init()
    {
        m_WindowContainer = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/screen.layout", NULL );
    }

	JMWindowBase Create()
	{
		JMWindowBase window;
		GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/windowbase.layout", m_WindowContainer ).GetScript( window );
		return window;
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

            m_Windows[i].SetBackgroundColour( 0.95, 0.042, 0.056, 0.077 );
        }

        window.SetBackgroundColour( 0.93, 0.06, 0.08, 0.11 );
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
	if( !g_cot_WindowManager )
	{
		g_cot_WindowManager = new JMWindowManager();
        g_cot_WindowManager.Init();
	}

	return g_cot_WindowManager;
}