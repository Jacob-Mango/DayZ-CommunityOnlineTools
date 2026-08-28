class JMCOTSideBar: COT_ScriptedWidgetEventHandler
{
	//! Fraction of the sidebar height taken by the title bar. Must match the
	//! `size 1 0.04` of TitleBar in sidebar_menu.layout - the module scroller is
	//! sized to whatever is left between it and the footer.
	private static const float TITLEBAR_HEIGHT_FRAC = 0.04;

	private Widget m_LayoutRoot;
	private TextWidget m_TitleBarText;
	private ScrollWidget m_ButtonsScroller;

	private ref JMCOTSideBarFooter m_Footer;

	//! Last screen height the scroller was sized against, so the (comparatively
	//! expensive) resize only runs when the resolution actually changes.
	private float m_LastScreenHeight;

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
		m_TotalAnimateTime = JMUIAnimations.SIDEBAR_ANIMATE_TIME;
		m_Categories = new array< ref JMCOTSideBarCategory >;
		m_Footer = new JMCOTSideBarFooter();
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
		Print("[COT-TRACE] JMCOTSideBar.OnWidgetScriptInit begin");
		m_LayoutRoot = w;
		m_LayoutRoot.SetHandler( this );

		// The HUD is a workspace sibling, not a parent, so the only thing keeping
		// it off the sidebar is draw order - see JMUILayout.SORT_SIDEBAR.
		m_LayoutRoot.SetSort( JMUILayout.SORT_SIDEBAR );

		Init();
		Print("[COT-TRACE] JMCOTSideBar.OnWidgetScriptInit end");
	}

	void Init()
	{
		Print("[COT-TRACE] JMCOTSideBar.Init: find TitleBarText");
		Class.CastTo( m_TitleBarText, m_LayoutRoot.FindAnyWidget( "TitleBarText" ) );

		Print("[COT-TRACE] JMCOTSideBar.Init: find Footer");
		Widget footerW = m_LayoutRoot.FindAnyWidget( "Footer" );
		Print("[COT-TRACE] JMCOTSideBar.Init: Footer found=" + (footerW != null).ToString());
		m_Footer.Init( footerW );
		Print("[COT-TRACE] JMCOTSideBar.Init: Footer init done");

		Class.CastTo( m_ButtonsScroller, m_LayoutRoot.FindAnyWidget( "Buttons_Scroller" ) );

		Print("[COT-TRACE] JMCOTSideBar.Init: find Buttons");
		Widget buttonsContainer = m_LayoutRoot.FindAnyWidget( "Buttons" );
		Print("[COT-TRACE] JMCOTSideBar.Init: Buttons found=" + (buttonsContainer != null).ToString());

		array< JMRenderableModuleBase > modules = new array< JMRenderableModuleBase >;
		SortModuleArray( GetModuleManager().GetCOTModules(), modules );
		Print("[COT-TRACE] JMCOTSideBar.Init: modules sorted, count=" + modules.Count());

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
			Print("[COT-TRACE] JMCOTSideBar.Init: examine category '" + catName + "'");
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
			{
				Print("[COT-TRACE] JMCOTSideBar.Init: category '" + catName + "' has no modules, skip");
				continue;
			}

			Print("[COT-TRACE] JMCOTSideBar.Init: create sidebar_category.layout for '" + catName + "'");
			// Create the category tile widget
			Widget catWidget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_category.layout", buttonsContainer );
			Print("[COT-TRACE] JMCOTSideBar.Init: catWidget created=" + (catWidget != null).ToString());
			if ( !catWidget )
				continue;

			// Set the category label and icon
			Print("[COT-TRACE] JMCOTSideBar.Init: find cat_ttl");
			TextWidget catTtl = TextWidget.Cast( catWidget.FindAnyWidget( "cat_ttl" ) );
			if ( catTtl )
				catTtl.SetText( catName );

			Print("[COT-TRACE] JMCOTSideBar.Init: find cat_icon, will load: " + JMSideBarConfig.GetCategoryIcon( catName ));
			ImageWidget catIcon = ImageWidget.Cast( catWidget.FindAnyWidget( "cat_icon" ) );
			if ( catIcon )
				catIcon.LoadImageFile( 0, JMSideBarConfig.GetCategoryIcon( catName ) );
			Print("[COT-TRACE] JMCOTSideBar.Init: cat_icon loaded");

			// Create the category manager (also creates the flyout as a top-level widget)
			Print("[COT-TRACE] JMCOTSideBar.Init: new JMCOTSideBarCategory");
			JMCOTSideBarCategory category = new JMCOTSideBarCategory();
			category.SetSideBar( this );
			Print("[COT-TRACE] JMCOTSideBar.Init: category.Init begin");
			category.Init( catName, catWidget );
			Print("[COT-TRACE] JMCOTSideBar.Init: category.Init end");

			// Add all modules in this category
			foreach ( JMRenderableModuleBase mod2: modules )
			{
				if ( mod2.HasButton() && mod2.GetCategory() == catName )
				{
					Print("[COT-TRACE] JMCOTSideBar.Init: AddModule '" + mod2.GetTitle() + "' to '" + catName + "'");
					category.AddModule( mod2 );
					Print("[COT-TRACE] JMCOTSideBar.Init: AddModule done");
				}
			}

			m_Categories.Insert( category );
			Print("[COT-TRACE] JMCOTSideBar.Init: category '" + catName + "' fully built");
		}
		Print("[COT-TRACE] JMCOTSideBar.Init: all categories built");

		float h;
		m_LayoutRoot.GetSize( m_WidthFull, h );
		m_WidthIcon = m_WidthFull * 0.8;

		LayoutScroller();
		Print("[COT-TRACE] JMCOTSideBar.Init: GetSize done, calling Hide");

		Hide();
		Print("[COT-TRACE] JMCOTSideBar.Init: Hide done");
		m_LayoutRoot.Show( false );
	}

	// Size the module scroller to the gap between the title bar and the footer.
	// Both of those are anchored to their own edge, so the scroller is the only
	// widget that has to be told how tall it is - and it has to be told again
	// whenever the resolution changes under it.
	private void LayoutScroller()
	{
		if ( !m_ButtonsScroller )
			return;

		float scrW, scrH;
		g_Game.GetWorkspace().GetScreenSize( scrW, scrH );

		if ( scrH == m_LastScreenHeight )
			return;

		m_LastScreenHeight = scrH;

		float available = scrH - ( scrH * TITLEBAR_HEIGHT_FRAC ) - JMCOTSideBarFooter.HEIGHT;
		if ( available < 0 )
			available = 0;

		m_ButtonsScroller.SetSize( 1.0, available );
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
		Print("[COT-TRACE] JMCOTSideBar.Show called");
		if ( m_IsAnimatingOut )
		{
			Print("[COT-TRACE] JMCOTSideBar.Show: already animating out, abort");
			return;
		}

		SetFocus( NULL );

		if ( !IsMissionClient() )
		{
			Print("[COT-TRACE] JMCOTSideBar.Show: not mission client, abort");
			return;
		}

		if ( !m_LayoutRoot )
		{
			Print("[COT-TRACE] JMCOTSideBar.Show: m_LayoutRoot null, abort");
			return;
		}

		//! Re-asserted here because the HUD is torn down and rebuilt on respawn,
		//! and the rebuilt one comes back at the default sort.
		m_LayoutRoot.SetSort( JMUILayout.SORT_SIDEBAR );

		Print("[COT-TRACE] JMCOTSideBar.Show: calling ShowAllWidgets");
		ShowAllWidgets();
		Print("[COT-TRACE] JMCOTSideBar.Show: ShowAllWidgets done");

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
		bool animating = m_IsAnimatingIn || m_IsAnimatingOut || (m_IsTargetCompact != m_IsCompact);
		if (animating)
			Print("[COT-TRACE] SideBar.OnUpdate animating in=" + m_IsAnimatingIn + " out=" + m_IsAnimatingOut + " t=" + m_AnimateTime);

		CheckForVisibleModules();

		if ( IsVisible() )
		{
			LayoutScroller();
			m_Footer.OnUpdate( timeslice );
		}

		// Update all categories (for flyout hide delay logic)
		foreach ( JMCOTSideBarCategory cat: m_Categories )
		{
			cat.OnUpdate( timeslice );
		}

		if (animating)
		{
			if (m_AnimateTime == 0)
			{
				if (m_IsTargetCompact)
					m_TitleBarText.SetText("COT");

				// Swap the footer at the START of the slide, not the end: the
				// full strip is 300px wide and would be drawn clipped in half
				// for the whole animation if it were left up until then.
				m_Footer.SetCompact( m_IsTargetCompact );
			}

			m_AnimateTime += timeslice;
			float percent = m_AnimateTime / m_TotalAnimateTime;
			if (percent > 1.0)
				percent = 1.0;

			if (m_IsAnimatingIn)
			{
				Print("[COT-TRACE] SideBar.OnUpdate: animatingIn, m_LayoutRoot.Show(true)");
				m_LayoutRoot.Show( true );
				Print("[COT-TRACE] SideBar.OnUpdate: Show returned");
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

			Print("[COT-TRACE] SideBar.OnUpdate: SetPos " + m_CurrentWidth);
			m_LayoutRoot.SetPos( m_CurrentWidth, 0 );
			Print("[COT-TRACE] SideBar.OnUpdate: SetPos done");

			// Keep any open flyout anchored to its category tile during animation
			foreach ( JMCOTSideBarCategory animCat: m_Categories )
			{
				animCat.RepositionFlyout();
			}
			Print("[COT-TRACE] SideBar.OnUpdate: RepositionFlyout done");

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
