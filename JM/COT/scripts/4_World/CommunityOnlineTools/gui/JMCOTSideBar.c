class JMCOTSideBar 
{
	protected Widget layoutRoot;

	protected Widget m_ButtonsContainer;
	protected Widget m_Windows;

	protected bool m_GameFocus;

	private const float m_HideX = -300.0;
	private const float m_ShowX = 0.0;

	private const float m_TotalAnimateTime = 1.0;

	private float m_StartX;
	private float m_EndX;
	private float m_X;

	private bool m_IsAnimating;
	private float m_AnimateTime;

	void JMCOTSideBar()
	{
		m_StartX = m_ShowX;
		m_EndX = m_HideX;
	}
	
	void ~JMCOTSideBar()
	{
		Hide();
	}

	bool IsVisible()
	{
		return layoutRoot.IsVisible();
	}
	
	Widget Init()
	{
		if ( layoutRoot )
		{
			layoutRoot.Unlink();
		}

		layoutRoot = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_menu.layout" );
		layoutRoot.Show( false );

		TextWidget.Cast( layoutRoot.FindAnyWidget( "Version_Text" ) ).SetText( "PayPal.Me/JacobMango" );

		array< ref JMRenderableModuleBase > modules = GetModuleManager().GetCOTModules();

		for ( int i = 0; i < modules.Count(); i++ )
		{
			JMRenderableModuleBase module = modules.Get( i );

			if ( module.HasButton() )
			{
				module.InitButton( GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_button.layout", layoutRoot.FindAnyWidget( "Buttons" ) ) );
			}
		}

		return layoutRoot;
	}

	void Show()
	{
		SetFocus( NULL );

		if ( !IsMissionClient() ) 
			return;

		if ( !layoutRoot )
			return;

		SetInputFocus( false );

		GetGame().GetMission().GetHud().Show( false );
		
		m_StartX = m_HideX;
		m_EndX = m_ShowX;
	}

	void Hide()
	{
		SetFocus( NULL );

		SetInputFocus( true );

		GetGame().GetMission().GetHud().Show( true );
		
		m_StartX = m_ShowX;
		m_EndX = m_HideX;
	}

	void ToggleInputFocus()
	{
		m_GameFocus = !m_GameFocus;
	}

	void SetInputFocus( bool focus )
	{
		m_GameFocus = focus;
	}

	void OnUpdate( float timeslice )
	{
		if ( m_IsAnimating )
		{
			m_AnimateTime += timeslice;

			m_X = m_StartX + ( Easing.EaseInCubic( m_AnimateTime ) * ( m_EndX - m_StartX ) );

			if ( m_AnimateTime > m_TotalAnimateTime )
			{
				m_IsAnimating = false;
			}
		} else
		{
			m_X = m_EndX;
		}

		if ( m_X == m_HideX )
		{
			layoutRoot.Show( false );
		} else
		{
			layoutRoot.Show( true );
			layoutRoot.SetPos( m_X, 0 );
		}

		if ( !IsVisible() )
			return;

		if ( m_IsAnimating )
			return;

		if ( m_GameFocus )
		{
			GetGame().GetInput().ResetGameFocus();
			GetGame().GetUIManager().ShowUICursor( false );

			GetGame().GetMission().GetHud().Show( true );
		} else
		{
			GetGame().GetInput().ChangeGameFocus( 1 );
			GetGame().GetUIManager().ShowUICursor( true );

			GetGame().GetMission().GetHud().Show( false );
		}
	}

	bool OnClick( Widget w, int x, int y, int button )
	{
		if ( !IsMissionClient() ) 
			return false;
		
		array< ref JMRenderableModuleBase > modules = GetModuleManager().GetCOTModules();

		for ( int i = 0; i < modules.Count(); i++ )
		{
			JMRenderableModuleBase module = modules[i];

			if ( w == module.GetMenuButton() )
			{
				module.ToggleShow();
				break;
			}
		}

		return false;
	}
}

