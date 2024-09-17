/*#ifdef VPPADMINTOOLS
modded class AdminHudSubMenu
{
	private JMFormBase m_COTForm;
	private JMWindowBase m_COTWindow;

	private Widget m_VPPRoot;

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );
	}

	override void SetWindowPos( int x, int y )
	{
		if ( m_COTWindow )
		{
			m_COTWindow.SetPosition( x, y );
		} else
		{
			super.SetWindowPos( x, y );
		}
	}

	protected override Widget CreateWidgets( string path )
	{
		if ( m_COTForm )
		{
			m_VPPRoot = GetGame().GetWorkspace().CreateWidgets( path, m_COTForm.GetLayoutRoot() );
		} else
		{
			m_VPPRoot = super.CreateWidgets( path );
		}

		return m_VPPRoot;
	}

	Widget InitializeCOT( JMVPPFormBase form, JMWindowBase window )
	{
		m_COTForm = form;
		m_COTWindow = window;

		OnCreate( NULL );

		float ww;
		float wh;
		m_VPPRoot.GetSize( ww, wh );

		int sw;
		int sh;
		GetScreenSize( sw, sh );

		m_COTWindow.SetSize( ww * sw, wh * sh );

		m_VPPRoot.SetFlags( WidgetFlags.EXACTSIZE | WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE, true );
		m_VPPRoot.SetSize( ww * sw, wh * sh, true );
		m_VPPRoot.Update();

		m_COTForm.GetLayoutRoot().Update();

		return m_VPPRoot;
	}
};
#endif*/