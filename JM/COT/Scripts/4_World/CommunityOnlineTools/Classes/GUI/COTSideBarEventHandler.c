class COTSideBarEventHandler extends ScriptedWidgetEventHandler
{
	private Widget m_LayoutRoot;

	private float m_WidthX;
	private float m_PosX;

	private bool m_IsAnimatingIn;
	private bool m_IsAnimatingOut;

	private float m_TotalAnimateTime;
	private float m_AnimateTime;

	void COTSideBarEventHandler()
	{
		m_TotalAnimateTime = 0.05; //0.35;
	}
	
	void ~COTSideBarEventHandler()
	{
		Hide();
	}

	Widget GetLayoutRoot()
	{
		return m_LayoutRoot;
	}

	bool IsVisible()
	{
		return m_LayoutRoot && m_LayoutRoot.IsVisible();
	}

	void OnWidgetScriptInit( Widget w )
	{
		m_LayoutRoot = w;
		m_LayoutRoot.SetHandler( this );

		Init();
	}
	
	void Init()
	{
		TextWidget.Cast( m_LayoutRoot.FindAnyWidget( "Version_Text" ) ).SetText( "PayPal.Me/JacobMango" );

		array<COTModule> modules();
		SortModuleArray( COTModule.s_All, modules );

		foreach (auto module : modules)
		{
			if (!module || !module.HasButton())
			{
				continue;
			}

			module.InitButton(GetGame().GetWorkspace().CreateWidgets("JM/COT/GUI/layouts/sidebar_button.layout", m_LayoutRoot.FindAnyWidget("Buttons")));
		}

		float h;
		m_LayoutRoot.GetSize( m_WidthX, h );

		Hide();
	}

	private void SortModuleArray( array<COTModule> modules, out array<COTModule> sorted )
	{
		string pNames[ 1000 ];
		int pIndices[ 1000 ];

		for ( int i = 0; i < modules.Count(); i++ )
		{
			pNames[ i ] = modules[ i ].GetLocalisedTitle();
		}

		Sort( pNames, modules.Count() );

		for ( i = 0; i < modules.Count(); i++ )
		{
			for ( int j = 0; j < modules.Count(); j++ )
			{
				if ( pNames[ j ] == modules[ i ].GetLocalisedTitle() )
				{
					pIndices[ i ] = j;
				}
			}
		}

		for ( i = 0; i < modules.Count(); i++ )
		{
			sorted.Insert( NULL );
		}

		for ( i = 0; i < modules.Count(); i++ )
		{
			sorted.Set( pIndices[ i ], modules[ i ] );
		}
	}

	void Show()
	{
		if ( m_IsAnimatingOut )
			return;

		SetFocus( NULL );

		if ( !IsMissionClient() ) 
			return;

		if ( !m_LayoutRoot )
			return;

		m_IsAnimatingIn = true;
		m_AnimateTime = 0.0;
		
		#ifndef CF_WINDOWS
		GetGame().GetInput().ChangeGameFocus( 1 );
		GetGame().GetUIManager().ShowUICursor( true );
		#endif
	}

	void Hide()
	{
		if ( m_IsAnimatingIn )
			return;

		SetFocus( NULL );

		m_IsAnimatingOut = true;
		m_AnimateTime = 0.0;
		
		#ifndef CF_WINDOWS
		if ( GetCOTWindowManager().Count() == 0 )
		{
			GetGame().GetInput().ResetGameFocus();
			GetGame().GetUIManager().ShowUICursor( false );
		}
		#endif
	}

	void OnUpdate( float timeslice )
	{
		if ( m_IsAnimatingIn || m_IsAnimatingOut )
		{
			m_AnimateTime += timeslice;

			float scaledTime = m_TotalAnimateTime / m_AnimateTime;

			if ( m_IsAnimatingIn )
			{
				//Print( "+" + this + "::OnUpdate m_IsAnimatingIn" );
				
				m_LayoutRoot.Show( true );

				//Print( "-" + this + "::OnUpdate m_IsAnimatingIn" );

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
					//Print( "+" + this + "::OnUpdate m_IsAnimatingOut" );

					m_LayoutRoot.Show( false );

					//Print( "-" + this + "::OnUpdate m_IsAnimatingOut" );
				}

				m_IsAnimatingIn = false;
				m_IsAnimatingOut = false;
			}
			
			m_LayoutRoot.SetPos( m_PosX, 0 );
			return;
		}
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( !IsMissionClient() ) 
			return false;
		
		foreach (auto module : COTModule.s_All)
		{
			if ( w == module.GetMenuButton() )
			{
				module.ToggleShow();
				break;
			}
		}

		return false;
	}
}

