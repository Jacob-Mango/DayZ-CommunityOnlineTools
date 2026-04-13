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

	private ref array< ref JMCOTSideBarCategory > m_Categories;


	void JMCOTSideBar()
	{
		m_TotalAnimateTime = 0.35;
		m_Categories = new array< ref JMCOTSideBarCategory >;
	}

	void ~JMCOTSideBar()
	{
		if (!g_Game)
			return;

	#ifdef DIAG
		auto trace = CF_Trace_0(this);
	#endif

		Hide();

	#ifdef DAYZ_1_28
		DestroyWidget(m_LayoutRoot);
	#endif
	}

	Widget GetLayoutRoot()
	{
		return m_LayoutRoot;
	}

	// Returns true if w is the sidebar layout root, any child of it,
	// or any child of a category flyout (which is a top-level workspace widget).
	bool ContainsWidget( Widget w )
	{
		if ( !w )
			return false;

		// Walk up the widget's parent chain to find the sidebar root
		Widget cur = w;
		while ( cur )
		{
			if ( cur == m_LayoutRoot )
				return true;
			cur = cur.GetParent();
		}

		// Check all category flyouts (they are top-level, not parented to m_LayoutRoot)
		foreach ( JMCOTSideBarCategory cat: m_Categories )
		{
			if ( cat.ContainsWidget( w ) )
				return true;
		}

		return false;
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
		Class.CastTo( m_TitleBarText, m_LayoutRoot.FindAnyWidget( "TitleBarText" ) );

		TextWidget.Cast( m_LayoutRoot.FindAnyWidget( "CreditsText" ) ).SetText("");
		TextWidget.Cast( m_LayoutRoot.FindAnyWidget( "Version_Text" ) ).SetText("");

		Widget buttonsContainer = m_LayoutRoot.FindAnyWidget( "Buttons" );

		array< JMRenderableModuleBase > modules = new array< JMRenderableModuleBase >;
		SortModuleArray( GetModuleManager().GetCOTModules(), modules );

		// Group modules by category, preserving preferred order
		array< string > categoryOrder = new array< string >;
		JMSideBarConfig.GetCategoryOrder( categoryOrder );

		// Collect all unique categories not in preferred order
		foreach ( JMRenderableModuleBase mod: modules )
		{
			if ( !mod.HasButton() )
				continue;

			string cat = mod.GetCategory();
			bool found = false;
			foreach ( string existing: categoryOrder )
			{
				if ( existing == cat )
				{
					found = true;
					break;
				}
			}
			if ( !found )
				categoryOrder.Insert( cat );
		}

		// Create one category tile per non-empty category
		foreach ( string catName: categoryOrder )
		{
			// Check if any module belongs to this category
			bool hasAny = false;
			foreach ( JMRenderableModuleBase checkMod: modules )
			{
				if ( checkMod.HasButton() && checkMod.GetCategory() == catName )
				{
					hasAny = true;
					break;
				}
			}

			if ( !hasAny )
				continue;

			// Create the category tile widget
			Widget catWidget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_category.layout", buttonsContainer );
			if ( !catWidget )
				continue;

			// Set the category label and icon
			TextWidget catTtl = TextWidget.Cast( catWidget.FindAnyWidget( "cat_ttl" ) );
			if ( catTtl )
				catTtl.SetText( catName );

			ImageWidget catIcon = ImageWidget.Cast( catWidget.FindAnyWidget( "cat_icon" ) );
			if ( catIcon )
				catIcon.LoadImageFile( 0, JMSideBarConfig.GetCategoryIcon( catName ) );

			// Create the category manager (also creates the flyout as a top-level widget)
			JMCOTSideBarCategory category = new JMCOTSideBarCategory();
			category.SetSideBar( this );
			category.Init( catName, catWidget );

			// Add all modules in this category
			foreach ( JMRenderableModuleBase mod2: modules )
			{
				if ( mod2.HasButton() && mod2.GetCategory() == catName )
				{
					category.AddModule( mod2 );
				}
			}

			m_Categories.Insert( category );
		}

		float h;
		m_LayoutRoot.GetSize( m_WidthFull, h );
		m_WidthIcon = m_WidthFull * 0.8;

		Hide();
		m_LayoutRoot.Show( false );
	}

	// Close every flyout except the one belonging to the given category.
	// Called by a category when it opens its flyout, so siblings close instantly.
	void CloseOtherFlyouts( JMCOTSideBarCategory except )
	{
		foreach ( JMCOTSideBarCategory cat: m_Categories )
		{
			if ( cat != except )
				cat.ForceHideFlyout();
		}
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

		// Close any open category flyout before hiding
		foreach ( JMCOTSideBarCategory hideCat: m_Categories )
		{
			hideCat.ForceHideFlyout();
		}

		m_IsAnimatingOut = true;
		m_AnimateTime = 0.0;

		HideAllWidgets();
	}

	void ShowAllWidgets()
	{
		#ifndef CF_WINDOWS
		GetCOTWindowManager().ShowAllActive();
		g_Game.GetInput().ChangeGameFocus( 1 );
		g_Game.GetUIManager().ShowUICursor( true );
		#endif
	}

	void HideAllWidgets()
	{
		m_WasCompact = m_IsCompact;

		#ifndef CF_WINDOWS
		GetCOTWindowManager().HideAllActive();
		if ( !GetCOTWindowManager().HasAnyUnpinnedActive() )
		{
			g_Game.GetInput().ResetGameFocus();
			g_Game.GetUIManager().ShowUICursor( false );
		}
		#endif
	}

	void OnUpdate( float timeslice )
	{
		CheckForVisibleModules();

		// Update all categories (for flyout hide delay logic)
		foreach ( JMCOTSideBarCategory cat: m_Categories )
		{
			cat.OnUpdate( timeslice );
		}

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

			// Keep any open flyout anchored to its category tile during animation
			foreach ( JMCOTSideBarCategory animCat: m_Categories )
			{
				animCat.RepositionFlyout();
			}

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
}
