//! #define scope in Enforce is per-file, NOT per compiled module - a file in
//! the same 4_World module (e.g. CommunityOnlineToolsBase.c) defining this
//! does NOT make it visible here. Every file that wants COT_DBG output needs
//! its own copy of this line.
#define COT_DEBUGLOGS

class JMCOTSideBar: COT_ScriptedWidgetEventHandler
{
	//! Fraction of the sidebar height taken by the title bar. Must match the
	//! `size 1 0.04` of TitleBar in sidebar_menu.layout - the module scroller is
	//! sized to whatever is left between it and the footer.
	protected static const float TITLEBAR_HEIGHT_FRAC = 0.04;
	protected Widget m_LayoutRoot;
	protected TextWidget m_TitleBarText;
	protected ScrollWidget m_ButtonsScroller;
	protected ref JMCOTSideBarFooter m_Footer;

	//! Last screen height the scroller was sized against, so the (comparatively
	//! expensive) resize only runs when the resolution actually changes.
	protected float m_LastScreenHeight;
	protected float m_WidthFull;
	protected float m_WidthIcon;
	protected float m_CurrentWidth;
	protected bool m_IsAnimatingIn;
	protected bool m_IsAnimatingOut;
	protected bool m_IsTargetCompact;
	protected bool m_IsCompact;
	protected bool m_WasCompact;
	protected float m_TotalAnimateTime;
	protected float m_AnimateTime;
	protected ref array< ref JMCOTSideBarCategory > m_Categories;

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

	//! Where the bar is heading, not where it currently is. The root stays visible
	//! for the whole slide-out, so IsVisible() answers "open" while the bar is on
	//! its way off screen - and a re-open arriving in that window would be dropped.
	bool IsShown()
	{
		if ( m_IsAnimatingOut )
			return false;

		if ( m_IsAnimatingIn )
			return true;

		return IsVisible();
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

	override bool IsVisible()
	{
		return m_LayoutRoot && m_LayoutRoot.IsVisible();
	}

	void OnWidgetScriptInit( Widget w )
	{
		m_LayoutRoot = w;
		m_LayoutRoot.SetHandler( this );

		// The HUD is a workspace sibling, not a parent, so the only thing keeping
		// it off the sidebar is draw order - see JMUILayout.SORT_SIDEBAR.
		m_LayoutRoot.SetSort( JMUILayout.SORT_SIDEBAR );

		Init();
	}

	void Init()
	{
		Class.CastTo( m_TitleBarText, m_LayoutRoot.FindAnyWidget( "TitleBarText" ) );

		Widget footerW = m_LayoutRoot.FindAnyWidget( "Footer" );
		m_Footer.Init( footerW );

		Class.CastTo( m_ButtonsScroller, m_LayoutRoot.FindAnyWidget( "Buttons_Scroller" ) );

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
			{
				continue;
			}

			// Create the category tile widget
			Widget catWidget = g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/sidebar_category.layout", buttonsContainer );
			if ( !catWidget )
				continue;

			// Set the category label and icon
			TextWidget catTtl = TextWidget.Cast( catWidget.FindAnyWidget( "cat_ttl" ) );
			if ( catTtl )
				catTtl.SetText( JMSideBarConfig.GetCategoryDisplayName( catName ) );

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

		UpdatePermissions();

		float h;
		m_LayoutRoot.GetSize( m_WidthFull, h );
		m_WidthIcon = m_WidthFull * 0.8;

		LayoutScroller();

		Hide();
		m_LayoutRoot.Show( false );
	}

	// Size the module scroller to the gap between the title bar and the footer.
	// Both of those are anchored to their own edge, so the scroller is the only
	// widget that has to be told how tall it is - and it has to be told again
	// whenever the resolution changes under it.
	protected void LayoutScroller()
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

	void UpdatePermissions()
	{
		foreach ( JMCOTSideBarCategory cat: m_Categories )
		{
			if ( cat )
			{
				cat.UpdateModuleVisibility();
			}
		}
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

	protected void SortModuleArray( array< JMRenderableModuleBase > modules, out array< JMRenderableModuleBase > sorted )
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
		#ifdef COT_DEBUGLOGS
		Print("[COT_DBG] JMCOTSideBar.Show()");
		#endif

		// Reverse a slide that is still running rather than dropping the request.
		// EaseIn at p and EaseOut at 1-p resolve to the same offset, so resuming
		// the opposite animation from the mirrored time is seamless. Bailing out
		// instead left m_IsOpen saying "open" while the bar finished sliding away.
		float resume = 0.0;
		if ( m_IsAnimatingOut )
			resume = m_TotalAnimateTime - m_AnimateTime;

		SetFocus( NULL );

		if ( !IsMissionClient() )
		{
			return;
		}

		if ( !m_LayoutRoot )
		{
			return;
		}

		//! Re-asserted here because the HUD is torn down and rebuilt on respawn,
		//! and the rebuilt one comes back at the default sort.
		m_LayoutRoot.SetSort( JMUILayout.SORT_SIDEBAR );

		UpdatePermissions();

		ShowAllWidgets();

		m_IsAnimatingOut = false;
		m_IsAnimatingIn = true;
		m_AnimateTime = resume;
	}

	void Hide()
	{
		#ifdef COT_DEBUGLOGS
		string cotDbgHideStack;
		DumpStackString(cotDbgHideStack);
		Print("[COT_DBG] JMCOTSideBar.Hide() call stack:\n" + cotDbgHideStack);
		#endif

		float resume = 0.0;
		if ( m_IsAnimatingIn )
			resume = m_TotalAnimateTime - m_AnimateTime;

		SetFocus( NULL );

		if ( !IsMissionClient() )
			return;

		// Close any open category flyout before hiding
		foreach ( JMCOTSideBarCategory hideCat: m_Categories )
		{
			hideCat.ForceHideFlyout();
		}

		m_IsAnimatingIn = false;
		m_IsAnimatingOut = true;
		m_AnimateTime = resume;

		HideAllWidgets();
	}

	void ShowAllWidgets()
	{
		#ifndef CF_WINDOWS
		GetCOTWindowManager().ShowAllActive();
		g_Game.GetMission().AddActiveInputExcludes({"menu"});
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
			g_Game.GetMission().RemoveActiveInputExcludes({"menu"});
			g_Game.GetUIManager().ShowUICursor( false );
		}
		#endif
	}

	void OnUpdate( float timeslice )
	{
		bool animating = m_IsAnimatingIn || m_IsAnimatingOut || (m_IsTargetCompact != m_IsCompact);

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
		string visibleTitle = "";

		array< JMRenderableModuleBase > modules = GetModuleManager().GetCOTModules();
		foreach(JMRenderableModuleBase module: modules)
		{
			if ( module.IsVisible() )
			{
				m_IsTargetCompact = true;
				visibleTitle = module.GetTitle();
				break;
			}
		}

		if (visibleTitle == "")
			m_IsTargetCompact = false;
	}
}
