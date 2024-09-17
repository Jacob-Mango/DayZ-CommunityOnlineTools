class JMCOTSideBar: COT_ScriptedWidgetEventHandler
{
	private Widget m_LayoutRoot;
	private TextWidget m_TitleBarText;

	private float m_WidthFull;
	private float m_WidthIcon;
	private float m_CurrentWidth;

	private bool m_IsAnimatingIn;
	private bool m_IsAnimatingOut;
	
	private bool m_IsTargetCompact;
	private bool m_IsCompact;
	private bool m_WasCompact;

	private float m_TotalAnimateTime;
	private float m_AnimateTime;

	void JMCOTSideBar()
	{
		m_TotalAnimateTime = 0.35;
	}
	
	void ~JMCOTSideBar()
	{
		if (!GetGame())
			return;

	#ifdef DIAG
		auto trace = CF_Trace_0(this);
	#endif

		Hide();

		DestroyWidget(m_LayoutRoot);
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
		Class.CastTo(m_TitleBarText, m_LayoutRoot.FindAnyWidget( "TitleBarText" ));

		TextWidget.Cast( m_LayoutRoot.FindAnyWidget( "CreditsText" ) ).SetText("");
		TextWidget.Cast( m_LayoutRoot.FindAnyWidget( "Version_Text" ) ).SetText("");

		array< JMRenderableModuleBase > modules = new array< JMRenderableModuleBase >;
		SortModuleArray( GetModuleManager().GetCOTModules(), modules );

		for ( int i = 0; i < modules.Count(); i++ )
		{
			JMRenderableModuleBase module = modules.Get( i );

			if ( modules[i] && modules[i].HasButton() )
			{
				modules[i].InitButton( GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_button.layout", m_LayoutRoot.FindAnyWidget( "Buttons" ) ) );
			}
		}

		float h;
		m_LayoutRoot.GetSize( m_WidthFull, h );
		m_WidthIcon = m_WidthFull * 0.8;

		Hide();
		m_LayoutRoot.Show( false );
	}

	private void SortModuleArray( array< JMRenderableModuleBase > modules, out array< JMRenderableModuleBase > sorted )
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
		
		ShowAllWidgets();

		m_IsAnimatingIn = true;
		m_AnimateTime = 0.0;
	}

	void Hide()
	{
		if ( m_IsAnimatingIn )
			return;

		SetFocus( NULL );

		if ( !IsMissionClient() ) 
			return;

		m_IsAnimatingOut = true;
		m_AnimateTime = 0.0;

		HideAllWidgets();
	}

	void ShowAllWidgets()
	{
		#ifndef CF_WINDOWS
		GetCOTWindowManager().ShowAllActive();
		GetGame().GetInput().ChangeGameFocus( 1 );
		GetGame().GetUIManager().ShowUICursor( true );
		#endif
	}

	void HideAllWidgets()
	{
		m_WasCompact = m_IsCompact;

		#ifndef CF_WINDOWS
		GetCOTWindowManager().HideAllActive();
		if ( !GetCOTWindowManager().HasAnyActive() )
		{
			GetGame().GetInput().ResetGameFocus();
			GetGame().GetUIManager().ShowUICursor( false );
		}
		#endif
	}

	void OnUpdate( float timeslice )
	{
		CheckForVisibleModules();
		
		if (m_IsAnimatingIn || m_IsAnimatingOut || (m_IsTargetCompact != m_IsCompact))
		{
			if (m_AnimateTime == 0)
			{
				if (m_IsTargetCompact)
					m_TitleBarText.SetText("COT");
			}

			m_AnimateTime += timeslice;
			float percent = m_AnimateTime / m_TotalAnimateTime;
			if (percent > 1.0)
				percent = 1.0;

			if (m_IsAnimatingIn)
			{
				m_LayoutRoot.Show( true );
				if (m_IsTargetCompact)
				{
					m_CurrentWidth = EaseIn(percent, -m_WidthIcon, -m_WidthFull);
				} else {
					m_CurrentWidth = EaseIn(percent, 0, -m_WidthFull);
				}
			}
			else if (m_IsAnimatingOut)
			{
				if (m_WasCompact)
				{
					m_CurrentWidth = EaseOut(percent, -m_WidthFull, -m_WidthIcon);
				} else {
					m_CurrentWidth = EaseOut(percent, -m_WidthFull, 0);
				}
			}
			else
			{
				if (m_IsTargetCompact)
				{
					m_CurrentWidth = EaseOut(percent, -m_WidthIcon, 0);
				} else {
					m_CurrentWidth = EaseIn(percent, 0, -m_WidthIcon);
				}
			}

			m_LayoutRoot.SetPos( m_CurrentWidth, 0 );

			if ( m_AnimateTime > m_TotalAnimateTime )
			{
				if ( m_IsAnimatingOut )
					m_LayoutRoot.Show( false );
				else if (!m_IsTargetCompact)
					m_TitleBarText.SetText("Community Online Tools");

				m_IsAnimatingIn = false;
				m_IsAnimatingOut = false;
				m_IsCompact = m_IsTargetCompact;
				m_AnimateTime = 0.0;
			}
		}
	}

	float EaseIn(float percent, float max, float min)
	{
    	float result = Math.Sin((percent * Math.PI) / 2);
		return Math.Lerp(min, max, result);
	}

	float EaseOut(float percent, float max, float min)
	{
		float result = (1 - Math.Cos((percent * Math.PI) / 2));
		return Math.Lerp(min, max, result);
	}

	void CheckForVisibleModules()
	{
		array< JMRenderableModuleBase > modules = GetModuleManager().GetCOTModules();
		foreach(JMRenderableModuleBase module: modules)
		{
			if ( module.IsVisible() )
			{
				m_IsTargetCompact = true;
				return;
			}
		}

		m_IsTargetCompact = false;
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( !IsMissionClient() ) 
			return false;
		
		array< JMRenderableModuleBase > modules = GetModuleManager().GetCOTModules();
		foreach(JMRenderableModuleBase module: modules)
		{
			if ( w == module.GetMenuButton() )
			{
				module.ToggleShow();
				return false;
			}
		}

		return false;
	}
};
