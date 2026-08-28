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
	static const float HEADER_CONTROL_HEIGHT = 24;

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

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );
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
			mdl.SetForm(this);

			OnInit();

			OnClientPermissionsUpdated();

			OnShow();
			m_IsShown = true;
		}
	}

	protected bool SetModule( JMRenderableModuleBase mdl )
	{
		return false;
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
	 *       // 2. Register permission bindings (after widgets exist).
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

	bool IsVisible()
	{
		return m_Window != null;
	}

	void SetSize( float w, float h )
	{
		if (!IsVisible()) return;
		
		m_Window.SetSize(w, h);
	}

	void OnShow()
	{
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

	protected void RegisterOverlay( COT_ScriptedWidgetEventHandler control )
	{
		if ( !control )
			return;

		if ( !m_FloatingOverlays )
			m_FloatingOverlays = new array<COT_ScriptedWidgetEventHandler>;

		if ( m_FloatingOverlays.Find( control ) == -1 )
			m_FloatingOverlays.Insert( control );
	}

	protected void UnregisterOverlay( COT_ScriptedWidgetEventHandler control )
	{
		if ( !control || !m_FloatingOverlays )
			return;

		int idx = m_FloatingOverlays.Find( control );

		if ( idx != -1 )
			m_FloatingOverlays.Remove( idx );
	}

	//! Dismiss every registered overlay. Safe to call when nothing is open.
	protected void CloseAllOverlays()
	{
		if ( !m_FloatingOverlays )
			return;

		COT_ScriptedWidgetEventHandler overlay;

		for ( int i = m_FloatingOverlays.Count() - 1; i >= 0; i-- )
		{
			overlay = m_FloatingOverlays[i];

			if ( overlay )
				overlay.Close();
		}
	}

	// ---------------------------------------------------------------------------
	//  Declarative permission binding
	// ---------------------------------------------------------------------------

	//! Bind a control to a permission key and evaluate it immediately. From
	//! here on the control is kept in step by OnClientPermissionsUpdated().
	protected void RegisterPermission( COT_ScriptedWidgetEventHandler control, string permissionKey )
	{
		if ( !control || permissionKey == "" )
			return;

		if ( !m_PermissionControls )
			m_PermissionControls = new map<COT_ScriptedWidgetEventHandler, string>;

		m_PermissionControls.Set( control, permissionKey );

		control.UpdatePermission( permissionKey );
	}

	//! Drop a binding whose control is about to be destroyed. Forms that
	//! rebuild part of their UI on every selection change need this, or the
	//! map fills up with dead keys.
	protected void UnregisterPermission( COT_ScriptedWidgetEventHandler control )
	{
		if ( !control || !m_PermissionControls )
			return;

		m_PermissionControls.Remove( control );
	}

	protected void ApplyRegisteredPermissions()
	{
		if ( !m_PermissionControls )
			return;

		COT_ScriptedWidgetEventHandler control;

		for ( int i = 0; i < m_PermissionControls.Count(); i++ )
		{
			control = m_PermissionControls.GetKey( i );

			if ( control )
				control.UpdatePermission( m_PermissionControls.GetElement( i ) );
		}
	}

	// ---------------------------------------------------------------------------
	//  Lazy tab construction
	// ---------------------------------------------------------------------------

	//! Sized to the number of tabs, all false. Call once, after the tab
	//! containers exist.
	protected void InitTabState( int tabCount )
	{
		m_TabBuilt = new array<bool>;

		for ( int i = 0; i < tabCount; i++ )
		{
			m_TabBuilt.Insert( false );
		}
	}

	//! True the FIRST time a tab index is asked about, false every time after.
	//! Only the bookkeeping is shared - the switch over tab constants stays in
	//! the subclass, since the cases are form-specific:
	//!
	//!     private void BuildTabIfNeeded( int tabIdx )
	//!     {
	//!         if ( !ShouldBuildTab( tabIdx ) )
	//!             return;
	//!
	//!         switch ( tabIdx ) { ... }
	//!     }
	protected bool ShouldBuildTab( int tabIdx )
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

	//! Index of the tab currently on screen. UIActionTabs is a 5_Mission type
	//! and cannot be named from 4_World, so a subclass that has tabs overrides
	//! this with "return m_Tabs.GetSelection();".
	protected int GetActiveTabIndex()
	{
		return -1;
	}

	//! Gate background refreshes on the tab being visible. A form with no tabs
	//! (or one asked before its strip exists) reports every index active, so an
	//! unconverted caller keeps its old behaviour.
	protected bool IsTabActive( int tabIdx )
	{
		int active = GetActiveTabIndex();

		if ( active < 0 )
			return true;

		return active == tabIdx;
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
	protected void PinStripGeometry( Widget strip, Widget content, float contentHeight, int stripHeight )
	{
		if ( !strip || !content )
			return;

		// Below this the split is meaningless, and before the first real resize
		// the value can arrive as a fraction. The layout's own fractional split
		// is a reasonable stand-in until a true height shows up.
		if ( contentHeight <= stripHeight )
			return;

		strip.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		strip.SetPos( 0, 0 );
		strip.SetSize( 1, stripHeight );

		content.SetFlags( WidgetFlags.VEXACTPOS | WidgetFlags.VEXACTSIZE, true );
		content.SetPos( 0, stripHeight );
		content.SetSize( 1, contentHeight - stripHeight );
	}

	//! Split-pane convenience: pin the right panel's tab strip over its content.
	protected void PinRightPanelGeometry( float contentHeight )
	{
		PinStripGeometry( m_RightTabStrip, m_RightContent, contentHeight, TAB_STRIP_HEIGHT );
	}

	// ---------------------------------------------------------------------------
	//  Selection / disable overlay
	//
	//  UpdateUI() stays in the subclass - deciding whether anything is selected
	//  is domain logic. These two only own the panel and the overlays.
	// ---------------------------------------------------------------------------

	void ShowUI()
	{
		if ( m_RightPanelDisable )
			m_RightPanelDisable.Show( false );
	}

	void HideUI()
	{
		if ( m_RightPanelDisable )
			m_RightPanelDisable.Show( true );

		CloseAllOverlays();
	}

	Widget GetLayoutRoot()
	{
		return layoutRoot;
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
				return CreateConfirmation_Three( JMConfirmationType.INFO, title, string.Format(Widget.TranslateString("#STR_COT_WARNING_PLAYERS_MESSAGE_BODY"), count.ToString()), "#STR_COT_GENERIC_CANCEL", "", inst.GetName(), callbackSelectedPlayerSingle, "#STR_COT_GENERIC_CONFIRM", callbackSelectedPlayersMulti );
			else if (callbackSelf)
				return CreateConfirmation_Three( JMConfirmationType.INFO, title, string.Format(Widget.TranslateString("#STR_COT_WARNING_PLAYERS_MESSAGE_BODY"), count.ToString()), "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_SELF", callbackSelf, "#STR_COT_GENERIC_CONFIRM", callbackSelectedPlayersMulti );
			else
				return CreateConfirmation_Two( JMConfirmationType.INFO, title, "#STR_COT_WARNING_PLAYERS_MESSAGE_BODY", "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", callbackSelectedPlayersMulti );
		}
		else
		{
			if (inst != GetPermissionsManager().GetClientPlayer() && callbackSelectedPlayerSingle)
			{
				return CreateConfirmation_Three( JMConfirmationType.INFO, title, string.Format(Widget.TranslateString("#STR_COT_WARNING_SELECTEDPLAYER_MESSAGE_BODY"), inst.GetName()), "#STR_COT_GENERIC_CANCEL", "", inst.GetName(), callbackSelectedPlayerSingle, "#STR_COT_GENERIC_SELF", callbackSelf );
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
			return CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_COT_WARNING_OBJECTS_MESSAGE_HEADER", string.Format(Widget.TranslateString("#STR_COT_WARNING_OBJECTS_MESSAGE_BODY"), count.ToString()), "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_GENERIC_CONFIRM", callbackSelectedObjectsMulti );
		}

		if ( executeCallbackOnNoConfirmation && callbackSelectedObjectSingle != string.Empty )
			g_Game.GameScript.CallFunctionParams( this, callbackSelectedObjectSingle, NULL, new Param1<JMConfirmation>( NULL ) );

		return null;
	}
}

