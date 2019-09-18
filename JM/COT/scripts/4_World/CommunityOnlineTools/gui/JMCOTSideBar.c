class JMCOTSideBar extends ScriptedWidgetEventHandler
{
	protected Widget layoutRoot;

	protected Widget m_ButtonsContainer;
	protected Widget m_Windows;

	protected bool m_GameFocus;

	private float m_WidthX;
	private float m_PosX;

	private bool m_IsAnimatingIn;
	private bool m_IsAnimatingOut;

	private float m_TotalAnimateTime;
	private float m_AnimateTime;

	void JMCOTSideBar()
	{
		m_TotalAnimateTime = 0.05; //0.35;
	}
	
	void ~JMCOTSideBar()
	{
		Hide();
	}

	bool IsVisible()
	{
		return layoutRoot.IsVisible();
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}
	
	void Init()
	{
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

		float h;
		layoutRoot.GetSize( m_WidthX, h );

		Hide();
	}

	void Show()
	{
		if ( m_IsAnimatingOut )
			return;

		SetFocus( NULL );

		if ( !IsMissionClient() ) 
			return;

		if ( !layoutRoot )
			return;

		SetInputFocus( false );
		
		if ( !m_GameFocus )
		{
			GetGame().GetInput().ChangeGameFocus( 1 );
			GetGame().GetUIManager().ShowUICursor( true );
		}

		//GetGame().GetMission().GetHud().Show( false );

		m_IsAnimatingIn = true;
		m_AnimateTime = 0.0;
	}

	void Hide()
	{
		if ( m_IsAnimatingIn )
			return;

		SetFocus( NULL );

		SetInputFocus( true );
		
		GetGame().GetInput().ResetGameFocus();
		GetGame().GetUIManager().ShowUICursor( false );

		//GetGame().GetMission().GetHud().Show( true );

		m_IsAnimatingOut = true;
		m_AnimateTime = 0.0;
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
		if ( m_IsAnimatingIn || m_IsAnimatingOut )
		{
			m_AnimateTime += timeslice;

			float scaledTime = m_TotalAnimateTime / m_AnimateTime;

			if ( m_IsAnimatingIn )
			{
				layoutRoot.Show( true );

				m_PosX = 0;

				//m_PosX = /*Easing.EaseInSine*/( 1.0 - scaledTime ) * -1.0;
			}

			if ( m_IsAnimatingOut )
			{
				m_PosX = -m_WidthX;

				//m_PosX = /*Easing.EaseOutSine*/( scaledTime ) * -1.0;
			}

			if ( m_AnimateTime > m_TotalAnimateTime )
			{
				if ( m_IsAnimatingOut )
				{
					layoutRoot.Show( false );
				}

				m_IsAnimatingIn = false;
				m_IsAnimatingOut = false;
			}
			
			layoutRoot.SetPos( m_PosX, 0 );
			return;
		}

		if ( !IsVisible() )
			return;

		if ( m_GameFocus )
		{
			GetGame().GetInput().ResetGameFocus();
			GetGame().GetUIManager().ShowUICursor( false );

			//GetGame().GetMission().GetHud().Show( true );
		} else
		{
			GetGame().GetInput().ChangeGameFocus( 1 );
			GetGame().GetUIManager().ShowUICursor( true );

			//GetGame().GetMission().GetHud().Show( false );
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

