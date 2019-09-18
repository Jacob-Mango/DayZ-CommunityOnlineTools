class JMCOTSideBar 
{
	protected Widget layoutRoot;

	protected Widget m_ButtonsContainer;
	protected Widget m_Windows;

	protected bool m_GameFocus;

	void JMCOTSideBar()
	{
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
		
		layoutRoot.Show( true );

		OnShow();
	}

	void Hide()
	{
		SetFocus( NULL );

		OnHide();
		
		if ( !layoutRoot )
			return;

		layoutRoot.Show( false );
	}

	void OnShow()
	{
		SetInputFocus( false );

		GetGame().GetInput().ChangeGameFocus( 1 );
		GetGame().GetUIManager().ShowUICursor( true );

		GetGame().GetMission().GetHud().Show( false );
	}

	void OnHide()
	{
		SetInputFocus( true );

		GetGame().GetInput().ResetGameFocus();
		GetGame().GetUIManager().ShowUICursor( false );

		GetGame().GetMission().GetHud().Show( true );
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
		if ( !IsVisible() )
			return;

		if ( m_GameFocus )
		{
			GetGame().GetInput().ResetGameFocus();
			GetGame().GetUIManager().ShowUICursor( false );
		} else
		{
			GetGame().GetInput().ChangeGameFocus( 1 );
			GetGame().GetUIManager().ShowUICursor( true );
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

