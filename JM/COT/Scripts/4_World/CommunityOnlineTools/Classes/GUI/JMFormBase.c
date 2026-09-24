class JMFormBase: COT_ScriptedWidgetEventHandler
{
#ifdef DIAG
	static int s_JMFormBaseCount;
#endif

	protected Widget layoutRoot;
	protected CF_Window m_Window;

	#ifndef CF_WINDOWS
	protected JMWindowBase window;
	#endif

	bool m_IsShown;

	// ---------------------------------------------------------------------------
	//  Shared design tokens
	//
	//  Every form measures its chrome from these, so a tab strip is the same
	//  height in the player manager and in the ESP manager. Do NOT redeclare
	//  them in a subclass: a same-named constant in a derived class is a
	//  redefinition, not an override.
	// ---------------------------------------------------------------------------

	//! Drawn height of every control in a panel's header block - the toolbar
	//! pills, the sort toggle, the filter box. Their layouts default to a 32px
	//! square, which is three rows of dead height above a list that wants all
	//! of it. 24 rather than the search box's own 22, because UIActionSearchBox
	//! centres a 22px chrome inside its root and a root of exactly 22 clips the
	//! chrome's top and bottom border.
	static const float HEADER_CONTROL_HEIGHT = 30;

	//! Drawn height of a tab strip. The strip's own layout is a fixed 30px row;
	//! 34 leaves it 2px of breathing room top and bottom.
	static const int TAB_STRIP_HEIGHT = 34;

	//! Side of an icon-only button. A glyph pill carries a 16px icon, so 32
	//! square is the whole control - matching UIActionImageButton.layout's own
	//! declared size and the 30px word-button row it sits next to. Anything
	//! that puts one of these in a grid stretches it to the cell, so icon
	//! strips belong in a wrap / flex row instead.
	static const int ICON_BUTTON_PX = 32;

	//! Side of a button living in a card's title bar. The glyph inside stays
	//! 16px whatever this is - UIActionImageButton.layout fixes the image - so
	//! this only grows the pill and its hit area. 28 is as far as it goes
	//! before the 30px header strip has no edge left.
	static const int HEADER_ACTION_PX = 28;

	//! Drawn height of one identity / property row, in layout pixels.
	static const int IDENTITY_ROW_HEIGHT = 32;

	// ---------------------------------------------------------------------------
	//  Structural panels
	//
	//  Resolved by the subclass in OnInit() with FindAnyWidget(). A form whose
	//  archetype does not use a given panel leaves it null - everything below
	//  null-guards.
	// ---------------------------------------------------------------------------
	protected Widget m_LeftPanel;
	protected Widget m_RightPanel;
	protected Widget m_RightPanelDisable;
	protected Widget m_RightTabStrip;
	protected Widget m_RightContent;

	//! The top/bottom archetype's counterpart of m_RightTabStrip / m_RightContent: the tab strip and
	//! the panel it switches over, in the bottom half under the form's toolbar.
	protected Widget m_BottomTabStrip;
	protected Widget m_BottomContent;

	// ---------------------------------------------------------------------------
	//  Registries
	//
	//  Typed to COT_ScriptedWidgetEventHandler rather than UIActionBase on
	//  purpose: this class lives in 4_World and cannot see 5_Mission types.
	//  COT_ScriptedWidgetEventHandler is the common ancestor of both this class
	//  and UIActionBase, and declares the Close()/UpdatePermission() contract
	//  these walks rely on. Call sites still pass UIActionBase - the upcast is
	//  implicit.
	// ---------------------------------------------------------------------------

	//! Overlays that anchor their popup to the window root rather than to their
	//! own parent, so hiding the tab they live on does not hide them. Register
	//! at creation; they are dismissed together on tab change and on hide.
	protected ref array<COT_ScriptedWidgetEventHandler> m_FloatingOverlays;

	//! Control -> permission key. Walked on every OnClientPermissionsUpdated().
	protected ref map<COT_ScriptedWidgetEventHandler, string> m_PermissionControls;

	//! Per-tab content containers and their built flags, for lazy tab building.
	protected ref array<Widget> m_TabContainers;
	protected ref array<bool> m_TabBuilt;
	protected ref map<int, JMFormTab> m_TabControllers;
	protected int m_FocusedTab = -1;

	//! Method names this form has scheduled through DeferCall, so they can all be
	//! cancelled together.
	protected ref array<string> m_DeferredCalls;

	//! Set once the base OnCreate() has forwarded to OnInit(), so Init() does not
	//! run a legacy OnInit() override a second time.
	protected bool m_LegacyInitRan;

	//! Tabs added at runtime through AddTab(): build callback per tab index.
	protected ref map<int, ref JMCustomTabCallback> m_CustomTabCallbacks;

	//! Tabs whose panel this form created (AddTab) - unlinked again by RemoveTabById.
	protected ref array<int> m_OwnedTabs;

	// -------------------------------------------------------------------------
	//  Player targeting. An action on players used to need a Multi / Single /
	//  Self trio of methods, each repeating the same call with a different GUID
	//  list, plus a forwarding stub on the form when the trio lived in a tab.
	//  ConfirmPlayerAction asks the same question as CreateAdvancedPlayerConfirm
	//  but resolves the answer to a GUID list itself, so the action is written
	//  once:
	//
	//      m_Form.ConfirmPlayerAction( "Heal", this, "HealTargets", false );
	//
	//      void HealTargets( array<string> targets, JMConfirmation confirmation = NULL )
	//      {
	//          m_Form.m_Module.Heal( targets );
	//      }
	//
	//  `handler` is the object that owns the callback (a tab, or the form
	//  itself). `confirmSelf` / `executeOnNoConfirmation` are as for
	//  CreateAdvancedPlayerConfirm.
	// -------------------------------------------------------------------------
	protected Class m_PlayerTargetHandler;
	protected string m_PlayerTargetCallback;

	//! The module this form was created for, as the base type. Set by Init() once SetModule()
	//! accepts it; a form keeps its own typed reference for its module-specific calls.
	protected JMRenderableModuleBase m_ModuleBase;

	void JMFormBase() 
	{
	#ifdef DIAG
		s_JMFormBaseCount++;
		CF_Log.Info("JMFormBase count: " + s_JMFormBaseCount);
	#endif
	}

	void ~JMFormBase()
	{
		if (!g_Game)
			return;

		CancelDeferredCalls();

	#ifdef DIAG
		auto trace = CF_Trace_0(this);
	#endif

		OnHide();

		//! @note this should not be necessary since if the JMWindow handling this JMForm is destroyed,
		//! it'll unlink its own layoutRoot and all its children with it. This is just here as a safety.
	#ifdef DAYZ_1_28
		DestroyWidget(layoutRoot);
	#endif

	#ifdef DIAG
		s_JMFormBaseCount--;
		if (s_JMFormBaseCount <= 0)
			CF_Log.Info("JMFormBase count: " + s_JMFormBaseCount);
	#endif
	}

	//! Index of the tab currently on screen, -1 when the form has no strip.
	int GetActiveTabIndex()
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( !strip )
			return -1;

		return strip.COT_TabSelection();
	}

	Widget GetLayoutRoot()
	{
		return layoutRoot;
	}

	//! Parent widget AddTab() creates its panels in - the scroller content or
	//! panel the form's other tab containers live in. Null disables AddTab().
	protected Widget GetTabContentHost()
	{
		return null;
	}

	protected JMFormTab GetTabController( int tab )
	{
		JMFormTab controller;

		if ( m_TabControllers && m_TabControllers.Find( tab, controller ) )
			return controller;

		return null;
	}

	//! The panel of tab `tab` (the widget passed to OnTabCreate), or null.
	protected Widget GetTabPanel( int tab )
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( !strip )
			return null;

		return strip.COT_TabContent( tab );
	}

	//! The form's tab strip, or null when it has none. UIActionTabs is a
	//! 5_Mission type and cannot be named from 4_World, so it is held as its
	//! 3_Game ancestor - a form with tabs overrides this with
	//! "return m_Tabs;" and everything below works from that one hook.
	protected COT_ScriptedWidgetEventHandler GetTabStrip()
	{
		return null;
	}

	//! Public: a per-tab class anchoring a floating overlay (e.g. a context
	//! menu) to the window root needs this the same way EnsureContextMenu()
	//! bodies already did before they lived on the form itself.
	CF_Window GetWindow()
	{
		return m_Window;
	}

	//! Escape priority tier 1 (popup). Whether any registered overlay -
	//! context menu, dropdown list, value prompt - is currently open on this
	//! form, as opposed to merely registered. Used by COTModule's Escape
	//! handler to decide whether Escape closes a popup here or falls through
	//! to closing the window/sidebar instead.
	bool HasOpenOverlay()
	{
		if ( !m_FloatingOverlays )
			return false;

		foreach ( COT_ScriptedWidgetEventHandler overlay : m_FloatingOverlays )
		{
			if ( overlay && overlay.IsOpen() )
				return true;
		}

		return false;
	}

	//! Gate background refreshes on the tab being visible. A form with no tabs
	//! (or one asked before its strip exists) reports every index active, so an
	//! unconverted caller keeps its old behaviour.
	bool IsTabActive( int tabIdx )
	{
		int active = GetActiveTabIndex();

		if ( active < 0 )
			return true;

		return active == tabIdx;
	}

	bool IsTabCreated( int tab )
	{
		return m_TabBuilt && tab >= 0 && tab < m_TabBuilt.Count() && m_TabBuilt[tab];
	}

	bool IsTabEnabled( int tab )
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( !strip )
			return false;

		return strip.COT_IsTabEnabled( tab );
	}

	bool IsTabFocused( int tab )
	{
		return m_FocusedTab == tab;
	}

	protected bool SetModule( JMRenderableModuleBase mdl )
	{
		return false;
	}

	// ---------------------------------------------------------------------------
	//  Selection / disable overlay
	//
	//  UpdateUI() stays in the subclass - deciding whether anything is selected
	//  is domain logic. These two only own the panel and the overlays.
	// ---------------------------------------------------------------------------

	//! Enable or disable the right-hand panel: disabled shows the "nothing
	//! selected" overlay over it and dismisses popups. Same grammar as
	//! SetTabEnabled / SetEnabled.
	void SetPanelEnabled( bool enabled )
	{
		if ( m_RightPanelDisable )
			m_RightPanelDisable.Show( !enabled );

		if ( !enabled )
			CloseAllOverlays();
	}

	void SetSize( float w, float h )
	{
		if (!IsVisible()) return;
		
		m_Window.SetSize(w, h);
	}

	// -- Tab state ---------------------------------------------------------------

	void SetTabEnabled( int tab, bool enabled )
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( strip )
			strip.COT_SetTabEnabled( tab, enabled );

		JMFormTab controller = GetTabController( tab );
		if ( controller )
			controller.SetEnabled( enabled );
	}

	//! `true` selects the tab (running the change sequence); `false` on the
	//! focused tab selects the first other visible one; `false` on any other tab
	//! does nothing.
	void SetTabFocused( int tab, bool focused )
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( strip )
			strip.COT_SetTabFocused( tab, focused );
	}

	void SetTabVisible( int tab, bool visible )
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( strip )
			strip.COT_SetTabVisible( tab, visible );
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
	}

	//! Run `method` (a method of this form) after `delayMs` on the GUI queue,
	//! once or repeating. Scheduling a name again replaces the pending call
	//! instead of stacking a second one, so a repaint or re-measure that fires
	//! on every resize event runs once, after the last.
	//!
	//! Prefer this to g_Game.GetCallQueue( ... ).CallLater( Method, ... ): the
	//! queue does not know when its target dies, and only the form knows which
	//! calls it has pending - so it cancels them all on destruction. `params`
	//! carries the method's arguments, e.g. new Param2< bool, vector >( true, vector.Zero ).
	void DeferCall( string method, int delayMs = 0, bool repeat = false, Param params = null )
	{
		if ( !m_DeferredCalls )
			m_DeferredCalls = new array<string>();

		if ( m_DeferredCalls.Find( method ) == -1 )
			m_DeferredCalls.Insert( method );

		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).RemoveByName( this, method );
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLaterByName( this, method, delayMs, repeat, params );
	}

	//! Cancel one pending DeferCall - the way to stop a repeating one.
	void CancelDeferredCall( string method )
	{
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).RemoveByName( this, method );
	}

	//! Cancel every pending DeferCall. Runs on destruction; call it yourself
	//! only to stop everything without destroying the form.
	void CancelDeferredCalls()
	{
		if ( !m_DeferredCalls )
			return;

		foreach ( string method : m_DeferredCalls )
			CancelDeferredCall( method );

		m_DeferredCalls.Clear();
	}

	void Init( CF_Window wdw, JMRenderableModuleBase mdl )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "Init").Add(wdw.ToString()).Add(mdl.ToString());
		#endif

		m_Window = wdw;
		
		#ifndef CF_WINDOWS
		window = wdw;
		#endif

		if ( SetModule( mdl ) )
		{
			m_ModuleBase = mdl;
			mdl.SetForm(this);

			OnCreate();

			//! A form that overrides OnCreate() without calling super never reaches the
			//! legacy OnInit(), so a third-party `modded class` `override void OnInit()`
			//! (ZenCOT on JMObjectSpawnerForm, ...) would silently stop running. Run it
			//! here instead, after the form's own widgets exist, which is the order
			//! those overrides were written for.
			if ( !m_LegacyInitRan )
				OnInit();

			OnClientPermissionsUpdated();

			OnShow();
			m_IsShown = true;
		}
	}

	/**
	 * @brief Called once when the form is created.
	 *
	 * Recommended structure for subclass overrides:
	 *
	 *   override void OnInit()
	 *   {
	 *       // 1. Create widgets (UIActionManager calls, FindAnyWidget, etc.)
	 *       InitWidgets();          // optional helper(s) - split by panel if large
	 *
	 *       // 2. Bind permissions to widgets (after widgets exist).
	 *       //    BindPermission() calls here; evaluated now and on every
	 *       //    subsequent OnClientPermissionsUpdated().
	 *       //    Only needed if you override OnClientPermissionsUpdated() with
	 *       //    bindings NOT covered by InitWidgets().
	 *
	 *       // 3. Perform any one-time data fetches / state initialisation.
	 *   }
	 *
	 * Do NOT call super.OnInit() - the base implementation is intentionally empty.
	 *
	 * NOTE: OnClientPermissionsUpdated() is called by Init() immediately after
	 * OnInit(), so widgets must exist before OnInit() returns.
	 */
	void OnInit()
	{
	}

	//! Called once when the form is created (widgets, bindings, first data
	//! requests). The base calls the legacy OnInit(), so a form or third-party
	//! modded form that still overrides OnInit() keeps working unchanged - new
	//! code overrides OnCreate() and does not call super.OnInit().
	void OnCreate()
	{
		m_LegacyInitRan = true;
		OnInit();
	}

	override bool IsVisible()
	{
		return m_Window != null;
	}

	//! Asks the module for the data the form displays - see JMRenderableModuleBase.RequestData().
	//! A form that overrides OnShow() calls super so the request is not lost.
	void OnShow()
	{
		if ( m_ModuleBase )
			m_ModuleBase.RequestData();
	}

	void OnHide() 
	{
		CloseAllOverlays();
	}

	void OnFocus()
	{
	}

	void OnUnfocus()
	{
	}

	void OnSettingsUpdated()
	{
	}

	//! @note a subclass with bindings of its own must call
	//! super.OnClientPermissionsUpdated() (or ApplyRegisteredPermissions())
	//! or its registered controls stop tracking permission changes.
	void OnClientPermissionsUpdated()
	{
		ApplyRegisteredPermissions();
	}

	void Update()
	{

	}

	//! Called whenever the window is resized. w/h are the new content dimensions (excluding title bar).
	void OnResize( float w, float h )
	{
	}

	// ---------------------------------------------------------------------------
	//  Floating overlay management
	// ---------------------------------------------------------------------------

	//! Public (not protected): a per-tab helper class holding a back-reference
	//! to its owning form (same shape as JMPlayerRowWidget.Menu) needs to call
	//! this from outside the JMFormBase family.
	void AddOverlay( COT_ScriptedWidgetEventHandler control )
	{
		if ( !control )
			return;

		if ( !m_FloatingOverlays )
			m_FloatingOverlays = new array<COT_ScriptedWidgetEventHandler>;

		if ( m_FloatingOverlays.Find( control ) == -1 )
			m_FloatingOverlays.Insert( control );
	}

	void RemoveOverlay( COT_ScriptedWidgetEventHandler control )
	{
		if ( !control || !m_FloatingOverlays )
			return;

		int idx = m_FloatingOverlays.Find( control );

		if ( idx != -1 )
			m_FloatingOverlays.Remove( idx );
	}

	//! DEPRECATED - use AddOverlay
	void RegisterOverlay( COT_ScriptedWidgetEventHandler control )
	{
		JMDeprecated.WarnOnce( this, "RegisterOverlay() is deprecated. Please use AddOverlay()." );

		AddOverlay( control );
	}

	//! DEPRECATED - use RemoveOverlay
	void UnregisterOverlay( COT_ScriptedWidgetEventHandler control )
	{
		JMDeprecated.WarnOnce( this, "UnregisterOverlay() is deprecated. Please use RemoveOverlay()." );

		RemoveOverlay( control );
	}

	//! Dismiss every registered overlay. Safe to call when nothing is open.
	void CloseAllOverlays()
	{
		if ( !m_FloatingOverlays )
			return;

		COT_ScriptedWidgetEventHandler overlay;

		for ( int i = m_FloatingOverlays.Count() - 1; i >= 0; i-- )
		{
			overlay = m_FloatingOverlays[i];

			if ( overlay && overlay.ToString() != "INVALID" )
				overlay.Close();
		}
	}

	//! Closes only the overlay(s) that are actually open right now (unlike
	//! CloseAllOverlays(), which is a blanket "tear everything down" used on
	//! hide/tab-change). Returns whether it closed anything, so a caller can
	//! tell "consumed the Escape press" from "nothing to do here".
	bool CloseOpenOverlays()
	{
		if ( !m_FloatingOverlays )
			return false;

		bool closedAny = false;

		for ( int i = m_FloatingOverlays.Count() - 1; i >= 0; i-- )
		{
			COT_ScriptedWidgetEventHandler overlay = m_FloatingOverlays[i];

			if ( overlay && overlay.IsOpen() )
			{
				overlay.Close();
				closedAny = true;
			}
		}

		return closedAny;
	}

	// ---------------------------------------------------------------------------
	//  Declarative permission binding
	// ---------------------------------------------------------------------------

	//! Bind a control to a permission key and evaluate it immediately. From
	//! here on the control is kept in step by OnClientPermissionsUpdated().
	void BindPermission( COT_ScriptedWidgetEventHandler control, string permissionKey )
	{
		if ( !control || permissionKey == "" )
			return;

		if ( !m_PermissionControls )
			m_PermissionControls = new map<COT_ScriptedWidgetEventHandler, string>;

		m_PermissionControls.Set( control, permissionKey );

		control.COT_ApplyPermission( permissionKey );
	}

	//! DEPRECATED - use BindPermission
	void BindPanelPermission( COT_ScriptedWidgetEventHandler panel, string permissionKey )
	{
		JMDeprecated.WarnOnce( this, "BindPanelPermission() is deprecated. Please use BindPermission()." );

		BindPermission( panel, permissionKey );
	}

	//! Evaluate a control against a permission ONCE, without tracking it.
	//!
	//! BindPermission() is the normal choice: it evaluates now and keeps the
	//! control in step on every permission change. Use this only for controls
	//! that are rebuilt on every refresh (a row list) and so must not sit in the
	//! binding map, or for a third-party form that re-applies its own controls
	//! from an OnClientPermissionsUpdated() override - which is exactly what
	//! DayZ-Expansion's PersonalStorage, AI and Hardline modules do, so this
	//! name stays. Takes the 3_Game base type because JMFormBase lives in
	//! 4_World and cannot see UIActionBase.
	void UpdatePermission( COT_ScriptedWidgetEventHandler control, string permission )
	{
		if ( !control )
			return;

		control.COT_ApplyPermission( permission );
	}

	//! Drop a binding whose control is about to be destroyed. Forms that
	//! rebuild part of their UI on every selection change need this, or the
	//! map fills up with dead keys.
	void UnregisterPermission( COT_ScriptedWidgetEventHandler control )
	{
		if ( !control || !m_PermissionControls )
			return;

		m_PermissionControls.Remove( control );
	}

	void ApplyRegisteredPermissions()
	{
		if ( !m_PermissionControls )
			return;

		COT_ScriptedWidgetEventHandler control;

		for ( int i = 0; i < m_PermissionControls.Count(); i++ )
		{
			control = m_PermissionControls.GetKey( i );

			if ( control )
				control.COT_ApplyPermission( m_PermissionControls.GetElement( i ) );
		}
	}

	// ---------------------------------------------------------------------------
	//  Lazy tab construction
	// ---------------------------------------------------------------------------

	//! Declare `count` more tabs, all unbuilt. Call once after the tab
	//! containers exist; call again to append tabs (a modded form adding its own,
	//! or AddTab()). One verb for both first declaration and extension.
	void DeclareTabs( int count )
	{
		if ( !m_TabBuilt )
			m_TabBuilt = new array<bool>;

		for ( int i = 0; i < count; i++ )
		{
			m_TabBuilt.Insert( false );
		}
	}

	//! DEPRECATED - use DeclareTabs
	void InitTabState( int tabCount )
	{
		JMDeprecated.WarnOnce( this, "InitTabState() is deprecated. Please use DeclareTabs()." );

		m_TabBuilt = null;
		DeclareTabs( tabCount );
	}

	//! DEPRECATED - use DeclareTabs
	void ExtendTabState( int count )
	{
		JMDeprecated.WarnOnce( this, "ExtendTabState() is deprecated. Please use DeclareTabs()." );

		DeclareTabs( count );
	}

	//! Create a tab the first time it is needed, and never again: builds its
	//! panel through OnTabCreate() (which forwards to a registered JMFormTab).
	void EnsureTabCreated( int tabIdx )
	{
		if ( !ShouldBuildTab( tabIdx ) )
			return;

		//! A tab added with AddTab() has its own builder; the form's
		//! OnTabCreate() never needs a case for it.
		if ( m_CustomTabCallbacks && m_CustomTabCallbacks.Contains( tabIdx ) )
		{
			BuildCustomTab( tabIdx );
			return;
		}

		OnTabCreate( tabIdx, GetTabPanel( tabIdx ) );
	}

	//! Initial selection: create tab `tab` and mark it focused WITHOUT running the
	//! focus/update hooks, for a form's OnCreate() after
	//! `m_Tabs.SetSelection( tab, false )`. Later changes go through HandleTabChange().
	void InitTabFocus( int tab )
	{
		m_FocusedTab = tab;

		EnsureTabCreated( tab );

		JMFormTab controller = GetTabController( tab );
		if ( controller )
			controller.SetFocused( true );
	}

	// -- Tab lifecycle hooks -----------------------------------------------------
	//  Same grammar as the form's own OnCreate / OnShow / OnHide / OnResize /
	//  OnDestruct, with the tab index added. Each forwards to the JMFormTab
	//  registered for the index (if any), so a form with tab classes needs no
	//  per-tab switch; a form without them overrides these and switches on `tab`.

	//! First time the tab is selected: build its widgets into `panel`.
	protected void OnTabCreate( int tab, Widget panel )
	{
		JMFormTab controller = GetTabController( tab );
		if ( controller )
			controller.OnCreate( panel );
	}

	//! Repaint the tab from current state. Runs after create and focus, and
	//! whenever UpdateTab() / UpdateActiveTab() is called - only for the focused tab.
	protected void OnTabUpdate( int tab )
	{
		JMFormTab controller = GetTabController( tab );
		if ( controller )
			controller.OnUpdate();
	}

	//! The tab became the selected one.
	protected void OnTabFocus( int tab )
	{
		JMFormTab controller = GetTabController( tab );
		if ( controller )
			controller.SetFocused( true );

		//! Legacy extension hook - see OnTabChanged().
		OnTabChanged( tab );
	}

	//! Another tab was selected (or the form is closing): hide hover panels,
	//! tooltips, stop timers that belong to this tab.
	protected void OnTabUnfocus( int tab )
	{
		JMFormTab controller = GetTabController( tab );
		if ( controller )
			controller.SetFocused( false );
	}

	//! The form is being destroyed: release what the tab's widgets do not own.
	protected void OnTabDestruct( int tab )
	{
		JMFormTab controller = GetTabController( tab );
		if ( controller )
			controller.OnDestruct();
	}

	//! The form was resized; only created tabs are told.
	protected void OnTabResize( int tab, float w, float h )
	{
		JMFormTab controller = GetTabController( tab );
		if ( controller )
			controller.OnResize( w, h );
	}

	//! DEPRECATED - override OnTabFocus() / OnTabUpdate(). Still called at the end
	//! of OnTabFocus() so third-party overrides of the old hook keep running.
	protected void OnTabChanged( int tabIdx )
	{
	}

	//! Attach the controller object of a tab to its index - the id UIActionTabs.AddTab() (or the
	//! form's AddTab) returned when the tab was added, never a number written by hand, so a mod appending tabs simply gets the next one whatever else is loaded. The form
	//! keeps its own typed reference; this is what lets the lifecycle reach it, and from here
	//! on the tab reports its index itself (JMFormTab.GetTabId).
	void RegisterTab( int tab, JMFormTab controller )
	{
		if ( !controller || tab < 0 )
			return;

		if ( !m_TabControllers )
			m_TabControllers = new map<int, JMFormTab>;

		controller.AttachToForm( this, tab );
		m_TabControllers.Set( tab, controller );
	}

	//! Repaint tab `tab` if - and only if - it is the focused one. Background
	//! refreshers call this instead of polling IsTabActive( TAB_X ).
	void UpdateTab( int tab )
	{
		if ( tab != m_FocusedTab || !IsTabCreated( tab ) )
			return;

		OnTabUpdate( tab );
	}

	void UpdateActiveTab()
	{
		UpdateTab( m_FocusedTab );
	}

	//! Forward a resize to every created tab. Call from the form's OnResize().
	protected void ResizeTabs( float w, float h )
	{
		m_LastTabsWidth = w;
		m_LastTabsHeight = h;

		if ( !m_TabBuilt )
			return;

		for ( int i = 0; i < m_TabBuilt.Count(); i++ )
		{
			if ( m_TabBuilt[i] )
				OnTabResize( i, w, h );
		}
	}

	//! Called by the window right before the form is freed.
	void OnDestruct()
	{
		if ( !m_TabBuilt )
			return;

		for ( int i = m_TabBuilt.Count() - 1; i >= 0; i-- )
		{
			if ( m_TabBuilt[i] )
				OnTabDestruct( i );
		}
	}

	//! True the FIRST time a tab index is asked about, false every time after.
	//! Prefer EnsureTabCreated(), which calls this and then OnTabCreate(); use this
	//! directly only for the older per-form BuildTabIfNeeded() shape:
	//!
	//!     private void BuildTabIfNeeded( int tabIdx )
	//!     {
	//!         if ( !ShouldBuildTab( tabIdx ) )
	//!             return;
	//!
	//!         switch ( tabIdx ) { ... }
	//!     }
	bool ShouldBuildTab( int tabIdx )
	{
		if ( !m_TabBuilt )
			return false;

		if ( tabIdx < 0 || tabIdx >= m_TabBuilt.Count() )
			return false;

		if ( m_TabBuilt[tabIdx] )
			return false;

		m_TabBuilt[tabIdx] = true;

		return true;
	}

	//! Append a tab and return its id (-1 if the form has no strip / content host). The id is
	//! assigned by the strip - keep it, never write a number down: another mod may append tabs to
	//! the same form, and removing a tab never renumbers the others.
	//!
	//! The tab is built lazily, the first time it is shown, by whichever of these is given:
	//!  - `buildCallbackFn`, called as `void Fn( Widget parentPanel )` on `instance` (default: the
	//!    form itself) - build the tab's widgets into that panel;
	//!  - `controller`, a JMFormTab: its OnCreate( panel ) / OnFocus / OnUpdate / ... are driven.
	//!
	//!     AddTab( "Extras", JMConstants.Lucide( "star" ), "BuildExtrasTab" );
	//!     AddTab( "Extras", JMConstants.Lucide( "star" ), "", null, new MyTab() );
	int AddTab( string label, string icon, string buildCallbackFn = "", Class instance = null, JMFormTab controller = null )
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		Widget host = GetTabContentHost();

		if ( !strip || !host )
			return -1;

		if ( !instance )
			instance = this;

		int tabIdx = strip.COT_AddTab( label, icon, host );
		if ( tabIdx < 0 )
			return -1;

		DeclareTabs( 1 );

		if ( !m_OwnedTabs )
			m_OwnedTabs = new array<int>;

		m_OwnedTabs.Insert( tabIdx );

		if ( buildCallbackFn != "" )
		{
			if ( !m_CustomTabCallbacks )
				m_CustomTabCallbacks = new map<int, ref JMCustomTabCallback>;

			m_CustomTabCallbacks.Insert( tabIdx, new JMCustomTabCallback( instance, buildCallbackFn ) );
		}

		if ( controller )
			RegisterTab( tabIdx, controller );

		return tabIdx;
	}

	//! Remove a tab: its controller is told (unfocus, destruct), its panel is unlinked when this form
	//! created it, and the strip drops the button. Other tabs keep their ids; if it was the selected
	//! tab the strip selects another (the usual change sequence runs).
	void RemoveTabById( int tab )
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( !strip )
			return;

		Widget panel = strip.COT_TabContent( tab );

		if ( m_FocusedTab == tab )
		{
			OnTabUnfocus( tab );
			m_FocusedTab = -1;
		}

		if ( IsTabCreated( tab ) )
			OnTabDestruct( tab );

		if ( m_TabControllers )
			m_TabControllers.Remove( tab );

		if ( m_CustomTabCallbacks )
			m_CustomTabCallbacks.Remove( tab );

		//! A removed tab must never be built afterwards.
		if ( m_TabBuilt && tab >= 0 && tab < m_TabBuilt.Count() )
			m_TabBuilt[tab] = true;

		strip.COT_RemoveTabById( tab );

		if ( m_OwnedTabs && panel )
		{
			int ownedIndex = m_OwnedTabs.Find( tab );
			if ( ownedIndex >= 0 )
			{
				m_OwnedTabs.Remove( ownedIndex );
				panel.Unlink();
			}
		}
	}

	//! Remove the tab of a JMFormTab controller.
	void RemoveTab( JMFormTab controller )
	{
		if ( controller && controller.GetTabId() >= 0 )
			RemoveTabById( controller.GetTabId() );
	}

	//! Attach an existing panel to a tab (shown while the tab is selected). Returns the panel's id.
	int AddPanel( int tab, Widget panel )
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( !strip )
			return -1;

		return strip.COT_AddPanel( tab, panel );
	}

	void RemovePanelById( int panelId )
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( strip )
			strip.COT_RemovePanelById( panelId );
	}

	void RemovePanel( Widget panel )
	{
		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( strip )
			strip.COT_RemovePanel( panel );
	}

	//! Run the build callback of a tab added with AddTab(). The base OnTabCreate()
	//! calls this for any tab without a registered JMFormTab, so injected tabs need
	//! no case of their own (a form overriding OnTabCreate() calls super in its
	//! default branch).
	protected void BuildCustomTab( int tabIdx )
	{
		if ( !m_CustomTabCallbacks || !m_CustomTabCallbacks.Contains( tabIdx ) )
			return;

		COT_ScriptedWidgetEventHandler strip = GetTabStrip();
		if ( !strip )
			return;

		JMCustomTabCallback entry = m_CustomTabCallbacks.Get( tabIdx );
		g_Game.GameScript.CallFunctionParams( entry.m_Instance, entry.m_FuncName, null, new Param1<Widget>( strip.COT_TabContent( tabIdx ) ) );
	}

	//! Everything a tab-strip change handler has to do, in one call: dismiss
	//! popups, unfocus the tab being left, create the new one on first visit,
	//! focus it and update it. The strip's own callback still lives in the form
	//! (its signature needs 5_Mission types), so the whole handler is:
	//!
	//!     void OnChange_Tab( UIEvent eid, UIActionBase action )
	//!     {
	//!         if ( eid == UIEvent.CHANGE )
	//!             HandleTabChange();
	//!     }
	//!
	//! Order: OnTabUnfocus( previous ), OnTabCreate( new, first time only ),
	//! OnTabFocus( new ), OnTabUpdate( new ).
	void HandleTabChange()
	{
		CloseAllOverlays();

		int sel = GetActiveTabIndex();

		if ( m_FocusedTab >= 0 && m_FocusedTab != sel )
			OnTabUnfocus( m_FocusedTab );

		m_FocusedTab = sel;

		//! Every tab was removed: nothing left to create or focus.
		if ( sel < 0 )
			return;

		EnsureTabCreated( sel );
		OnTabFocus( sel );
		OnTabUpdate( sel );
	}

	// ---------------------------------------------------------------------------
	//  Geometry pinning
	// ---------------------------------------------------------------------------

	//! Pin a fixed-height strip above a content pane that takes the remainder.
	//! Used for tab strips and for filter/toolbar headers alike, which is why it
	//! takes its widgets as parameters rather than reading the members.
	//!
	//! `contentHeight` is the form content height in LAYOUT PIXELS, as handed to
	//! OnResize by the window. Do not try to derive it from the panel itself: a
	//! panel declared with hexactsize/vexactsize 0 returns its FRACTION of the
	//! parent from GetSize(), and feeding those into an exact-size SetSize
	//! collapses the strip to sub-pixel width and gives the content a negative
	//! height - which renders as an empty panel.
	//!
	//! Only the vertical axis is pinned. Width stays fractional at 1, so both
	//! frames keep filling their parent on their own and there is no second
	//! unit to get wrong.
	//!
	//! A tab strip inside `strip` that does not fit on one row wraps onto more, and the strip is
	//! pinned taller by the rows it gained (see FitTabStrips), so the content moves down instead of
	//! being overlapped. `stripHeight` is the single-row height.
	protected void PinStripGeometry( Widget strip, Widget content, float contentHeight, int stripHeight, Widget band = null, float bandHeight = 0 )
	{
		if ( !strip || !content )
			return;

		JMPinnedStrip pinned = RememberPinnedStrip( strip, content, contentHeight, stripHeight, band, bandHeight );

		pinned.PinnedStripHeight = stripHeight + FitTabStrips( strip );

		// Below this the split is meaningless, and before the first real resize
		// the value can arrive as a fraction. The layout's own fractional split
		// is a reasonable stand-in until a true height shows up.
		if ( contentHeight <= pinned.PinnedStripHeight + bandHeight )
			return;

		strip.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		strip.SetPos( 0, 0 );
		strip.SetSize( 1, pinned.PinnedStripHeight );

		if ( band )
		{
			band.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
			band.SetPos( 0, pinned.PinnedStripHeight );
			band.SetSize( 1, bandHeight );
		}

		float contentTop = pinned.PinnedStripHeight + bandHeight;

		content.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		content.SetPos( 0, contentTop );
		content.SetSize( 1, contentHeight - contentTop );
	}

	//! Every strip this form has pinned, with what it was pinned with.
	protected ref array<ref JMPinnedStrip> m_PinnedStrips;

	//! The size ResizeTabs() last forwarded, so a strip that wraps later can forward it again.
	protected float m_LastTabsWidth;
	protected float m_LastTabsHeight;

	protected JMPinnedStrip RememberPinnedStrip( Widget strip, Widget content, float contentHeight, int stripHeight, Widget band, float bandHeight )
	{
		if ( !m_PinnedStrips )
			m_PinnedStrips = new array<ref JMPinnedStrip>;

		JMPinnedStrip pinned = FindPinnedStrip( strip );
		if ( !pinned )
		{
			pinned = new JMPinnedStrip();
			pinned.Strip = strip;
			m_PinnedStrips.Insert( pinned );
		}

		pinned.Content = content;
		pinned.Band = band;
		pinned.ContentHeight = contentHeight;
		pinned.StripHeight = stripHeight;
		pinned.BandHeight = bandHeight;

		return pinned;
	}

	protected JMPinnedStrip FindPinnedStrip( Widget strip )
	{
		if ( !m_PinnedStrips )
			return null;

		foreach ( JMPinnedStrip pinned : m_PinnedStrips )
		{
			if ( pinned.Strip == strip )
				return pinned;
		}

		return null;
	}

	//! A tab strip in this form changed its number of rows on its own (the window's width settled
	//! after OnResize measured it). Pins every strip again with the height it now needs, then hands
	//! the tabs their size again - the content is a row shorter (or taller) than they were laid out for.
	void OnTabStripFitted()
	{
		if ( !m_PinnedStrips )
			return;

		foreach ( JMPinnedStrip pinned : m_PinnedStrips )
			PinStripGeometry( pinned.Strip, pinned.Content, pinned.ContentHeight, pinned.StripHeight, pinned.Band, pinned.BandHeight );

		if ( m_LastTabsHeight > 0 )
			ResizeTabs( m_LastTabsWidth, m_LastTabsHeight );
	}

	//! The height `strip` was last pinned at, extra tab rows included. A form that sizes something
	//! from the space under a tab strip uses this instead of TAB_STRIP_HEIGHT, which is only the
	//! height of a single row of tabs. Falls back to TAB_STRIP_HEIGHT for a strip that was never pinned.
	float GetPinnedStripHeight( Widget strip )
	{
		JMPinnedStrip pinned = FindPinnedStrip( strip );
		if ( !pinned )
			return TAB_STRIP_HEIGHT;

		return pinned.PinnedStripHeight;
	}

	//! Fit every tab strip directly inside `strip` to the strip's width and return the extra height
	//! (over one row) the tallest of them needs - 0 when they all fit on one row, or there is none.
	//! A form that pins a strip by hand adds this to the height it pins.
	protected float FitTabStrips( Widget strip )
	{
		if ( !strip )
			return 0;

		float width;
		float height;
		strip.GetScreenSize( width, height );

		float extra;

		Widget child = strip.GetChildren();
		while ( child )
		{
			COT_ScriptedWidgetEventHandler handler = null;
			child.GetScript( handler );
			if ( handler )
				extra = Math.Max( extra, handler.COT_FitTabs( width ) );

			child = child.GetSibling();
		}

		return extra;
	}

	//! Pin the right panel's tab strip over its content (the split-pane archetype). `band` is an
	//! optional widget of `bandHeight` pixels between the two.
	protected void PinRightPanelGeometry( float contentHeight, Widget band = null, float bandHeight = 0 )
	{
		PinStripGeometry( m_RightTabStrip, m_RightContent, contentHeight, TAB_STRIP_HEIGHT, band, bandHeight );
	}

	//! The same for the bottom panel's tab strip (the top/bottom archetype). `contentHeight` is the
	//! height of the bottom panel, i.e. the form's height less its toolbar.
	protected void PinBottomPanelGeometry( float contentHeight, Widget band = null, float bandHeight = 0 )
	{
		PinStripGeometry( m_BottomTabStrip, m_BottomContent, contentHeight, TAB_STRIP_HEIGHT, band, bandHeight );
	}

	Widget GetBottomTabStrip()
	{
		return m_BottomTabStrip;
	}

	Widget GetRightTabStrip()
	{
		return m_RightTabStrip;
	}
	//! DEPRECATED - use SetPanelEnabled( true )
	void ShowUI()
	{
		JMDeprecated.WarnOnce( this, "ShowUI() is deprecated. Please use SetPanelEnabled( true )." );

		SetPanelEnabled( true );
	}

	//! DEPRECATED - use SetPanelEnabled( false )
	void HideUI()
	{
		JMDeprecated.WarnOnce( this, "HideUI() is deprecated. Please use SetPanelEnabled( false )." );

		SetPanelEnabled( false );
	}

	JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return window.CreateConfirmation_One( type, title, message, callBackOneName, callBackOne );
		#endif
	}

	JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return CreateConfirmation_One( type, title, message, callBackOneName, "" );
		#endif
	}

	JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return window.CreateConfirmation_Two( type, title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo );
		#endif
	}

	JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackTwoName )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return CreateConfirmation_Two( type, title, message, callBackOneName, "", callBackTwoName, "" );
		#endif
	}

	JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, string callBackThreeName, string callBackThree )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return window.CreateConfirmation_Three( type, title, message, callBackOneName, callBackOne, callBackTwoName, callBackTwo, callBackThreeName, callBackThree );
		#endif
	}

	JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackTwoName, string callBackThreeName )
	{
		#ifdef CF_WINDOWS
		return null;
		#else
		return CreateConfirmation_Three( type, title, message, callBackOneName, "", callBackTwoName, "", callBackThreeName, "" );
		#endif
	}

	// -------------------------------------------------------------------------
	//  Standard dialogs. The CreateConfirmation_* family above is the raw form
	//  (button titles and callbacks in a fixed, easy-to-misorder argument list);
	//  nearly every caller wants one of these three shapes, so use these.
	//
	//  The callback names a method ON THIS FORM taking ( JMConfirmation confirmation = NULL ),
	//  the same as before - read the edit box value with confirmation.GetEditBoxValue().
	//  Labels default to the localized Confirm / Cancel / OK.
	// -------------------------------------------------------------------------

	//! Cancel / Confirm. `confirmCallback` runs only if the admin confirms.
	JMConfirmation ConfirmAction( string title, string message, string confirmCallback, string confirmLabel = "", string cancelLabel = "" )
	{
		if ( confirmLabel == "" )
			confirmLabel = "#STR_COT_GENERIC_CONFIRM";

		if ( cancelLabel == "" )
			cancelLabel = "#STR_COT_GENERIC_CANCEL";

		return CreateConfirmation_Two( JMConfirmationType.INFO, title, message, cancelLabel, "", confirmLabel, confirmCallback );
	}

	//! Cancel / Confirm with a text box for the admin to fill in.
	JMConfirmation PromptInput( string title, string message, string confirmCallback, string confirmLabel = "", string cancelLabel = "" )
	{
		if ( confirmLabel == "" )
			confirmLabel = "#STR_COT_GENERIC_CONFIRM";

		if ( cancelLabel == "" )
			cancelLabel = "#STR_COT_GENERIC_CANCEL";

		return CreateConfirmation_Two( JMConfirmationType.EDIT, title, message, cancelLabel, "", confirmLabel, confirmCallback );
	}

	//! Message with a single OK button, for "you can't do that" style feedback.
	JMConfirmation ShowNotice( string title, string message, string okLabel = "" )
	{
		if ( okLabel == "" )
			okLabel = "#STR_COT_GENERIC_OK";

		return CreateConfirmation_One( JMConfirmationType.INFO, title, message, okLabel, "" );
	}

	JMConfirmation ConfirmPlayerAction( string title, Class handler, string targetsCallback, bool confirmSelf = true, bool executeOnNoConfirmation = true )
	{
		m_PlayerTargetHandler = handler;
		m_PlayerTargetCallback = targetsCallback;

		return CreateAdvancedPlayerConfirm( title, "OnPlayerTargets_Multi", "OnPlayerTargets_Single", "OnPlayerTargets_Self", confirmSelf, executeOnNoConfirmation );
	}

	//! Every selected player.
	void OnPlayerTargets_Multi( JMConfirmation confirmation = NULL )
	{
		RunPlayerTargets( JM_GetSelected().GetPlayers(), confirmation );
	}

	//! The first selected player, or the admin if nobody is selected.
	void OnPlayerTargets_Single( JMConfirmation confirmation = NULL )
	{
		array<string> targets = new array<string>();
		targets.Insert( JM_GetSelected().GetPlayersOrSelf()[0] );

		RunPlayerTargets( targets, confirmation );
	}

	//! The admin.
	void OnPlayerTargets_Self( JMConfirmation confirmation = NULL )
	{
		array<string> targets = new array<string>();
		targets.Insert( GetPermissionsManager().GetClientPlayer().GetGUID() );

		RunPlayerTargets( targets, confirmation );
	}

	protected void RunPlayerTargets( array<string> targets, JMConfirmation confirmation )
	{
		if ( !m_PlayerTargetHandler || m_PlayerTargetCallback == "" )
			return;

		g_Game.GameScript.CallFunctionParams( m_PlayerTargetHandler, m_PlayerTargetCallback, NULL, new Param2< array<string>, JMConfirmation >( targets, confirmation ) );
	}

	/**
	 * @brief Create confirmation asking whether to act on multiple selected players, single (first) selected player or self
	 *
	 * @param callbackSelectedPlayersMulti
	 * @param callbackSelectedPlayerSingle
	 * @param callbackSelf
	 * @param confirmSelf  If the selected player is the client player (aka self), whether to show confirmation or not
	 * @param executeCallbackOnNoConfirmation  If the selected player is the client player (aka self) and confirmSelf is false, whether to execute the respective callback w/o confirmation
	 * 
	 * @return confirmation if created, else null
	 */
	JMConfirmation CreateAdvancedPlayerConfirm(string title, string callbackSelectedPlayersMulti, string callbackSelectedPlayerSingle, string callbackSelf, bool confirmSelf = true, bool executeCallbackOnNoConfirmation = true)
	{
		auto selected = JM_GetSelected();

		if (!selected)
			return null;

		auto players = selected.GetPlayers();

		JMPlayerInstance inst = GetPermissionsManager().GetPlayer( players[0] );

		if (!inst)
			return null;

		int count = players.Count();
		if (count > 1)
		{
			if (inst != GetPermissionsManager().GetClientPlayer() && callbackSelectedPlayerSingle)
				return CreateConfirmation_Three( JMConfirmationType.INFO, title, TranslateStringEx("#STR_COT_WARNING_PLAYERS_MESSAGE_BODY", count.ToString()), "#STR_COT_GENERIC_CANCEL", "", inst.GetName(), callbackSelectedPlayerSingle, "#STR_COT_GENERIC_CONFIRM", callbackSelectedPlayersMulti );
			else if (callbackSelf)
				return CreateConfirmation_Three( JMConfirmationType.INFO, title, TranslateStringEx("#STR_COT_WARNING_PLAYERS_MESSAGE_BODY", count.ToString()), "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_SELF", callbackSelf, "#STR_COT_GENERIC_CONFIRM", callbackSelectedPlayersMulti );
			else
				return CreateConfirmation_Two( JMConfirmationType.INFO, title, "#STR_COT_WARNING_PLAYERS_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", callbackSelectedPlayersMulti );
		}
		else
		{
			if (inst != GetPermissionsManager().GetClientPlayer() && callbackSelectedPlayerSingle)
			{
				return CreateConfirmation_Three( JMConfirmationType.INFO, title, TranslateStringEx("#STR_COT_WARNING_SELECTEDPLAYER_MESSAGE_BODY", inst.GetName()), "#STR_COT_GENERIC_CANCEL", "", inst.GetName(), callbackSelectedPlayerSingle, "#STR_COT_GENERIC_SELF", callbackSelf );
			}
			else if (callbackSelf)
			{
				if (confirmSelf)
					return CreateConfirmation_Two( JMConfirmationType.INFO, title, "#STR_COT_WARNING_SELECTEDSELF_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_SELF", callbackSelf );
				else if (executeCallbackOnNoConfirmation)
					g_Game.GameScript.CallFunctionParams( this, callbackSelf, NULL, new Param1<JMConfirmation>( NULL ) );
			}
		}

		return null;
	}

	/**
	 * @brief Create confirmation if acting on multiple selected objects or no confirmation if single selected object
	 *
	 * @param callbackSelectedObjectsMulti
	 * @param callbackSelectedObjectSingle
	 * @param executeCallbackOnNoConfirmation  If single selected object, whether to execute the respective callback w/o confirmation
	 * 
	 * @return confirmation if created, else null
	 */
	JMConfirmation CreateAdvancedObjectConfirm(string callbackSelectedObjectsMulti, string callbackSelectedObjectSingle, bool executeCallbackOnNoConfirmation = true)
	{
		auto selected = JM_GetSelected();

		if (!selected)
			return null;

		auto objects = selected.GetObjects();

		int count = objects.Count();
		if (count > 1)
		{
			return CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_WARNING_OBJECTS_MESSAGE_HEADER", TranslateStringEx("#STR_COT_WARNING_OBJECTS_MESSAGE_BODY", count.ToString()), "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", callbackSelectedObjectsMulti );
		}

		if ( executeCallbackOnNoConfirmation && callbackSelectedObjectSingle != string.Empty )
			g_Game.GameScript.CallFunctionParams( this, callbackSelectedObjectSingle, NULL, new Param1<JMConfirmation>( NULL ) );

		return null;
	}
}

