//! #define scope in Enforce is per-file, NOT per compiled module - see COTModule.c.
#define COT_DEBUGLOGS

class UIActionBase: COT_ScriptedWidgetEventHandler 
{
#ifdef DIAG
	static int s_UIActionBaseCount;
#endif

	protected Widget layoutRoot;
	protected Widget m_Disable;

	//! Focus and hover chrome.
	//!
	//! 19 controls carry a fill/ring pair and 19 call sites detect focus, but
	//! nothing painted a focus ring anywhere in the UI. Rather than hand-roll
	//! that in every component, the ring is driven from here: a control
	//! registers its ring widget(s) and the base class paints idle / hover /
	//! focus on them. The resting colour is read back off the widget, so a
	//! control keeps whatever its layout specified and only the states are new.
	protected ref array<Widget> m_ChromeRing;
	protected ref array<int>    m_ChromeRingRest;
	protected bool m_ChromeHovered;
	protected bool m_ChromeFocused;

	//! A control whose current value is legal but has a consequence worth
	//! stopping for. The rings pulse red and an "action_warn" icon, where the
	//! layout provides one, fades in step with them and carries its own
	//! tooltip explaining what the value will do.
	//!
	//! This is not an error state: the value is accepted and applied either
	//! way, so nothing is disabled and nothing is blocked.
	protected ImageWidget m_WarnIcon;
	protected bool        m_Warning;
	protected string      m_WarnTooltip;
	protected float       m_WarnPhase;

	//! The resting ring colours from before the warning took them over. The
	//! pulse writes m_ChromeRingRest so hover and focus keep working normally
	//! on top of it, which means the layout's own colours have to be kept
	//! somewhere to be given back.
	protected ref array<int> m_WarnRingRest;

	//! Widget has no IsEnabled(), and 11 layouts have no disable
	//! overlay to read the state back off, so it is tracked here.
	protected bool m_Enabled = true;
	protected Class m_Instance;
	protected string m_FuncName;
	protected bool m_IsShown;
	protected bool m_HasCallback;

	//! Single-event callbacks - see SetOnClick / SetOnChange.
	protected Class m_OnClickInstance;
	protected string m_OnClickFunc;
	protected Class m_OnChangeInstance;
	protected string m_OnChangeFunc;
	protected bool m_WasFocused;
	protected bool m_LeftMouseDown;
	protected ref UIActionData m_Data;

	// --- Flex layout spec (opt-in) ------------------------------------------
	// Set via SetFlex(). Read by UIActionFlexRow during layout. When
	// m_HasFlex is false the element uses whatever width SetWidth/SetFixedSize
	// gave it and is ignored by the flex pass - default behaviour unchanged.
	protected bool  m_HasFlex;
	protected float m_FlexGrow;    // proportional share of leftover space (0 = none)
	protected float m_FlexBasisPx; // fixed pixel floor before growing (min width)
	protected float m_FlexMaxPx;   // pixel ceiling; <= 0 means unbounded

	//! Seconds for one full dim-to-bright-to-dim cycle. Slow enough to read as
	//! a warning rather than a flicker.
	static const float WARN_PULSE_PERIOD = 1.6;
	protected string m_TooltipText;
	protected string m_TooltipIcon;
	protected int    m_TooltipSwatchColor;
	protected int    m_TooltipTextColor;

	void UIActionBase()
	{
	#ifdef DIAG
		s_UIActionBaseCount++;
	#ifdef COT_DEBUGLOGS
		CF_Log.Info("UIActionBase count: " + s_UIActionBaseCount);
	#endif
	#endif
	}

	void ~UIActionBase()
	{
		if (!g_Game)
			return;

	#ifdef DIAG
		auto trace = CF_Trace_0(this);
	#endif

		Deactivate();

		//! @note this should not be necessary since if the JMWindowBase handling this UIAction is destroyed,
		//! it'll unlink its own layoutRoot and all its children with it. This is just here as a safety.
	#ifdef DAYZ_1_28
		DestroyWidget(layoutRoot);
	#endif

	#ifdef DIAG
		s_UIActionBaseCount--;
		if (s_UIActionBaseCount <= 0)
			CF_Log.Info("UIActionBase count: " + s_UIActionBaseCount);
	#endif
	}

	string GetButton()
	{
		return "";
	}

	float GetCurrent()
	{
		return 0.0;
	}

	UIActionData GetData()
	{
		return m_Data;
	}

	float GetFlexBasis() { return m_FlexBasisPx; }

	float GetFlexGrow()  { return m_FlexGrow; }

	float GetFlexMax()   { return m_FlexMaxPx; }

	float GetHeight()
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		return h;
	}

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	int GetSelection()
	{
		return -1;
	}

	string GetText()
	{
		return "";
	}

	void GetUserData( out Class data )
	{
		layoutRoot.GetUserData( data );
	}

	vector GetValue()
	{
		return "0 0 0";
	}

	bool  HasFlex()      { return m_HasFlex; }

	bool IsChecked()
	{
		return false;
	}

	bool IsEnabled()
	{
		if ( !m_Disable )
			return m_Enabled;

		return !m_Disable.IsVisible();
	}

	bool IsFocused()
	{
		return m_WasFocused;
	}

	bool IsFocusWidget( Widget widget )
	{
		return false;
	}

	bool IsWarning()
	{
		return m_Warning;
	}

	void SetButton( string text )
	{
	}

	void SetCallback( Class instance, string funcname )
	{
		if ( instance == NULL || funcname == "" ) return;

		m_Instance = instance;
		m_FuncName = funcname;

		m_HasCallback = true;
	}

	void SetChecked( bool checked )
	{
	}

	void SetColor( int color )
	{
		layoutRoot.SetColor( color );
	}

	void SetColorAlpha( int rgb, float alpha )
	{
		int color = (rgb & 0x00FFFFFF) | ((int)(alpha * 255) << 24);
		SetColor( color );
	}

	void SetCurrent( float value )
	{
		
	}

	void SetData( UIActionData data )
	{
		m_Data = data;
	}

	void SetDisableAlpha( float alpha )
	{
		if ( m_Disable )
			m_Disable.SetAlpha(alpha);
	}

	void SetDisableColor( int color )
	{
		if ( m_Disable )
			m_Disable.SetColor(color);
	}

	void SetEnabled( bool enable )
	{
		if ( enable )
		{
			Enable();
		} else
		{
			Disable();
		}
	}

	void SetEnabledIf( bool condition )
	{
		SetEnabled( condition );
	}

	void SetFixedHeight( float height )
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE, true );
		layoutRoot.SetSize( w, height );
		layoutRoot.Update();
	}

	void SetFixedSize( float width, float height )
	{
		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE, true );
		layoutRoot.SetFlags( WidgetFlags.HEXACTSIZE, true );
		layoutRoot.SetSize( width, height );
		layoutRoot.Update();
	}

	//! Mark this element as a flex child of a UIActionFlexRow.
	//!   grow   - proportional share of the row's leftover space (after every
	//!            child's basis floor is reserved). 0 = never grows past basis.
	//!   minPx  - pixel floor; the element never renders narrower than this,
	//!            even on a very narrow form. This is the "min size" guarantee
	//!            the raw fractional SetWidth could not express.
	//!   maxPx  - pixel ceiling; <= 0 means unbounded.
	//! Elements that never call SetFlex are untouched by the flex pass, so
	//! existing forms keep their current behaviour.
	void SetFlex( float grow, float minPx, float maxPx = -1 )
	{
		m_HasFlex     = true;
		m_FlexGrow    = grow;
		m_FlexBasisPx = minPx;
		m_FlexMaxPx   = maxPx;
	}

	void SetHeight( float height )
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		layoutRoot.SetSize( w, height );
		layoutRoot.Update();
	}

	void SetIcon( string imagePath )
	{
	}

	void SetLabel( string text )
	{
	}

	//! Same as SetOnClick, for UIEvent.CHANGE (slider moved, box edited, tab or
	//! selection changed, toggle flipped).
	void SetOnChange( Class instance, string funcname )
	{
		m_OnChangeInstance = instance;
		m_OnChangeFunc = funcname;
	}

	//! Single-event callbacks: the handler is `void Fn( UIActionBase action )`
	//! and runs only for that event, so it never needs the
	//! `if ( eid != UIEvent.CLICK ) return;` guard the general (UIEvent,
	//! UIActionBase) callback requires. Both kinds can be set on one control.
	//!
	//!     UIActionManager.CreateButton( parent, "Go", null, "" ).SetOnClick( this, "OnGo" );
	//!     void OnGo( UIActionBase action ) { ... }
	void SetOnClick( Class instance, string funcname )
	{
		m_OnClickInstance = instance;
		m_OnClickFunc = funcname;
	}

	void SetPosition( float xpos )
	{
		float w;
		float h;
		layoutRoot.GetPos( w, h );
		layoutRoot.SetPos( xpos, h );
		layoutRoot.Update();
	}

	void SetPosition( float xpos, float ypos )
	{
		layoutRoot.SetPos( xpos, ypos );
		layoutRoot.Update();
	}

	void SetSelection( int i, bool sendEvent = true )
	{
	}

	void SetSize( float width, float height )
	{
		layoutRoot.SetSize( width, height );
		layoutRoot.Update();
	}

	void SetText( string text )
	{
	}

	void SetTooltip( string text, string iconPath = "", int swatchColor = 0, int textColor = 0 )
	{
		m_TooltipText        = text;
		m_TooltipIcon        = iconPath;
		m_TooltipSwatchColor = swatchColor;
		m_TooltipTextColor   = textColor;

		// Lazy-create the global tooltip overlay if the owning form didn't do
		// it explicitly. Walk up to the largest ancestor that still has a
		// parent (i.e. one below the workspace root) so the tooltip floats
		// above the whole form but stays inside the event-routing tree.
		if ( text != "" && !UIActionTooltip.s_Instance && layoutRoot )
		{
			Widget anchor = layoutRoot;
			Widget parent = anchor.GetParent();
			while ( parent && parent.GetParent() )
			{
				anchor = parent;
				parent = anchor.GetParent();
			}
			UIActionManager.CreateTooltip( anchor );
		}
	}

	void SetUserData( Class data )
	{
		layoutRoot.SetUserData( data );
	}

	void SetValue( vector v )
	{
	}

	//! Show or hide from a condition, without the caller writing the branch.
	//!
	//! NOT an overload of Show(). Enforce has no method overloading: a second
	//! Show taking a bool made the two indistinguishable to the call resolver,
	//! and resolving a call to it crashed the script compiler outright - a
	//! native access violation while the Mission module was still compiling,
	//! so no script error was ever printed and the server died before boot.
	//! Keep this name distinct from Show()/Hide().
	void SetVisible( bool show )
	{
		if ( show )
			Show();
		else
			Hide();
	}

	//! Flag or clear the warning.
	//!
	//! `tooltip` is what the icon says on hover - the CONSEQUENCE of the
	//! current value, not a restatement of the value itself. Passing an empty
	//! tooltip leaves the icon with nothing to explain, so pass one.
	void SetWarning( bool warning, string tooltip = "" )
	{
		if ( tooltip != "" )
			m_WarnTooltip = tooltip;

		if ( warning == m_Warning )
			return;

		m_Warning   = warning;
		m_WarnPhase = 0;

		if ( m_WarnIcon )
			m_WarnIcon.Show( warning );

		//! Hand the layout's own ring colours back the moment the warning
		//! clears, so a control that warned once does not stay red-ish.
		if ( !warning )
		{
			for ( int i = 0; i < m_ChromeRingRest.Count(); i++ )
			{
				if ( i < m_WarnRingRest.Count() )
					m_ChromeRingRest[i] = m_WarnRingRest[i];
			}

			ApplyChrome();
		}
	}

	void SetWidgetPosition( Widget widget, float xpos )
	{
		float x;
		float y;
		widget.GetPos( x, y );
		widget.SetPos( xpos, y );
		layoutRoot.Update();
		widget.Update();
	}

	void SetWidgetWidth( Widget widget, float width )
	{
		float w;
		float h;
		widget.GetSize( w, h );
		widget.ClearFlags( WidgetFlags.HEXACTSIZE );
		widget.SetSize( width, h );
		layoutRoot.Update();
		widget.Update();
	}

	void SetWidth( float width )
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		layoutRoot.ClearFlags( WidgetFlags.HEXACTSIZE );
		layoutRoot.SetSize( width, h );
		layoutRoot.Update();
	}

	void SetYPosition( float ypos )
	{
		float w;
		float h;
		layoutRoot.GetPos( w, h );
		layoutRoot.SetPos( w, ypos );
		layoutRoot.Update();
	}

	void Deactivate()
	{
		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( Update );

		// If this action currently owns keyboard focus, release it - otherwise
		// the focused widget gets destroyed while still marked as focused,
		// which leaves world inputs permanently disabled (look around breaks).
		Widget currentFocus = GetFocus();
		if ( currentFocus && IsFocusWidget( currentFocus ) )
			SetFocus( NULL );

		// Unconditionally re-enable inputs. Previously only gated on m_WasFocused,
		// but if Update() stopped running while we thought we had focus, the flag
		// goes out of sync and world input stays locked.
		if (m_WasFocused)
		{
			m_WasFocused = false;
		}
		CommunityOnlineTools.ForceDisableInputs(false);
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		OnInit();

		m_Disable = layoutRoot.FindAnyWidget( "action_wrapper_disable" );

		m_ChromeRing     = new array<Widget>;
		m_ChromeRingRest = new array<int>;
		RegisterChromeRing( "action_ring" );
		RegisterChromeRing( "outline" );
		OnRegisterChrome();

		//! Optional: only the layouts that have somewhere to put it declare one.
		Class.CastTo( m_WarnIcon, layoutRoot.FindAnyWidget( "action_warn" ) );

		if ( m_WarnIcon )
		{
			m_WarnIcon.LoadImageFile( 0, JMConstants.ICON_WARNING );
			m_WarnIcon.SetImage( 0 );
			m_WarnIcon.Show( false );
		}

		//! Captured before anything can repaint them.
		m_WarnRingRest = new array<int>;
		for ( int ringIdx = 0; ringIdx < m_ChromeRingRest.Count(); ringIdx++ )
			m_WarnRingRest.Insert( m_ChromeRingRest[ringIdx] );

		Show();
	}

	void OnInit()
	{
	}

	//! Controls with more than one field -- a vector's x/y/z, a time picker's
	//! h/m/s -- register the rest of their rings here.
	void OnRegisterChrome()
	{
	}

	void RegisterChromeRing( string widgetName )
	{
		if ( !layoutRoot || !m_ChromeRing )
			return;

		Widget ring = layoutRoot.FindAnyWidget( widgetName );
		if ( !ring )
			return;

		m_ChromeRing.Insert( ring );
		m_ChromeRingRest.Insert( ring.GetColor() );
	}

	protected void ApplyChrome()
	{
		if ( !m_ChromeRing )
			return;

		int i;
		for ( i = 0; i < m_ChromeRing.Count(); i++ )
		{
			if ( !m_ChromeRing[i] )
				continue;

			int color = m_ChromeRingRest[i];
			if ( m_ChromeFocused )
				color = JMTheme.BORDER_RING;
			else if ( m_ChromeHovered )
				color = JMTheme.BORDER_RING_HOVER;

			m_ChromeRing[i].SetColor( color );
		}
	}

	void Show()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+UIActionBase::Show" );
		#endif

		if (m_IsShown)
			return;

		layoutRoot.Show( true );
		OnShow();

		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( Update );

		m_IsShown = true;

		#ifdef COT_DEBUGLOGS
		Print( "-UIActionBase::Show" );
		#endif
	}

	void Hide()
	{
		Deactivate();

		OnHide();
		
		if ( layoutRoot )
			layoutRoot.Show( false );

		m_IsShown = false;
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	void Update( float timeSlice )
	{
		UpdateWarning( timeSlice );

		// "Focused" means this action owns the currently keyboard-focused widget
		// AND the mouse is currently over one of our widgets. Without the second
		// clause, clicking and holding LMB anywhere in the 3D world keeps inputs
		// disabled because the last-used EditBox still has keyboard focus.
		Widget focused   = GetFocus();
		Widget underMouse = GetWidgetUnderCursor();

		bool hasOurFocus = focused && IsFocusWidget( focused );
		bool mouseOnUs   = underMouse && IsFocusWidget( underMouse );
		bool isFocused   = hasOurFocus && mouseOnUs;

		// The ring follows keyboard focus alone. The mouse clause below exists
		// to stop a stale EditBox focus from eating world input; it is not what
		// "this control is focused" means visually.
		if ( hasOurFocus != m_ChromeFocused )
		{
			m_ChromeFocused = hasOurFocus;
			ApplyChrome();
		}

		if (isFocused && !m_WasFocused)
		{
			#ifdef COT_DEBUGLOGS
			Print("[COT_DBG] UIActionBase::Update isFocused true->ForceDisableInputs(true)");
			#endif
			CommunityOnlineTools.ForceDisableInputs(true);
		}
		else if (!isFocused && m_WasFocused)
		{
			#ifdef COT_DEBUGLOGS
			Print("[COT_DBG] UIActionBase::Update isFocused false->ForceDisableInputs(false)");
			#endif
			CommunityOnlineTools.ForceDisableInputs(false);
		}

		m_WasFocused = isFocused;
		if ( m_LeftMouseDown )
		{
			if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) == 0 )
			{
				m_LeftMouseDown = false;

				// On release outside our widgets, drop keyboard focus so the
				// next Update cycle fully re-enables world inputs.
				if ( hasOurFocus && !mouseOnUs )
				{
					SetFocus( NULL );
				}
			}
		} else
		{
			if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) != 0 )
			{
				m_LeftMouseDown = true;
			}
		}
	}

	//! The name every caller uses on a control. Kept as its own declaration
	//! rather than an override so JMFormBase is free to expose the unrelated
	//! two-argument UpdatePermission( control, permission ) helper without the
	//! two colliding on a shared ancestor - see COT_ScriptedWidgetEventHandler.
	//!
	//! Hidden rather than shown-disabled without the permission: a control
	//! that only ever sits there grayed out teaches nothing about what the
	//! tool can do, and every caller already re-runs this on permission
	//! changes (OnClientPermissionsUpdated), so a later grant reveals it same
	//! as before. SetVisible, not layoutRoot.Show(bool) - see SetVisible's own
	//! comment for why a second Show(bool) is not safe to add here.
	void UpdatePermission( string permission )
	{
		bool allowed = JMPermissions.Has( permission );

		SetEnabled( allowed );
		SetVisible( allowed );
	}

	//! Polymorphic entry point used by JMFormBase's permission registry, which
	//! lives in 4_World and only ever sees the 3_Game base type.
	override void COT_ApplyPermission( string permission )
	{
		UpdatePermission( permission );
	}

	void Disable()
	{
		layoutRoot.Enable( false );
		m_Enabled = false;

		// 11 row and cell layouts carry no disable overlay, so this is a real
		// null, not a defensive flourish.
		if ( m_Disable )
			m_Disable.Show( true );
	}

	void Enable()
	{
		layoutRoot.Enable( true );
		m_Enabled = true;

		if ( m_Disable )
			m_Disable.Show( false );
	}

	//! Apply an exact pixel width while preserving the current height. Used by
	//! UIActionFlexRow to lay out flex children deterministically. The value is
	//! in the engine's exact-size space (same space as GetScreenSize / SetFixedSize),
	//! so callers measuring a container with GetScreenSize can pass those pixels
	//! directly - no unit mismatch, unlike mixing fractional-fill widgets (the
	//! classic DPI trap). Height is left untouched (fractional or exact as-is).
	void ApplyFlexWidthPx( float px )
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		layoutRoot.SetFlags( WidgetFlags.HEXACTSIZE, true );
		layoutRoot.SetSize( px, h );
		layoutRoot.Update();
	}

	// --- Flex API (opt-in) --------------------------------------------------

	override bool IsVisible()
	{
		if ( !layoutRoot )
			return false;

		return layoutRoot.IsVisible();
	}

	bool CallEvent( UIEvent eid )
	{
		if ( eid == UIEvent.CLICK && m_OnClickInstance && m_OnClickFunc != "" )
			g_Game.GameScript.CallFunctionParams( m_OnClickInstance, m_OnClickFunc, NULL, new Param1< UIActionBase >( this ) );
		else if ( eid == UIEvent.CHANGE && m_OnChangeInstance && m_OnChangeFunc != "" )
			g_Game.GameScript.CallFunctionParams( m_OnChangeInstance, m_OnChangeFunc, NULL, new Param1< UIActionBase >( this ) );

		if ( !m_HasCallback )
			return false;

		auto params = new Param2< UIEvent, UIActionBase >( eid, this );
		g_Game.GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, params );

		return false;
	}

	// ---------------------------------------------------------------------------
	//  Universal Animation & Feedback Triggers
	//  Enables calling action.AnimateFeedback() or action.AnimateSpin() on any
	//  UIActionBase reference without casting to a specific button subclass.
	// ---------------------------------------------------------------------------
	void AnimateFeedback()
	{
	}

	void AnimateSpin( float revolutions = 1.0 )
	{
	}

	void AnimatePulse( float duration = 2.0 )
	{
	}

	void AnimateSuccess()
	{
		AnimateFeedback();
	}

	void AnimateError()
	{
	}

	//! One step of the red pulse, shared by the rings and the icon so they
	//! brighten together rather than beating against each other.
	protected void UpdateWarning( float timeSlice )
	{
		if ( !m_Warning )
			return;

		m_WarnPhase += timeSlice;

		float factor = ( Math.Sin( ( m_WarnPhase / WARN_PULSE_PERIOD ) * Math.PI * 2.0 ) + 1.0 ) * 0.5;
		int color = JMTheme.Mix( JMTheme.DANGER_DIM, JMTheme.DANGER, factor );

		if ( m_WarnIcon )
			m_WarnIcon.SetColor( color );

		//! Written as the RESTING colour rather than straight onto the widget,
		//! so hovering or focusing the control still shows its normal ring and
		//! the pulse resumes underneath when the pointer leaves.
		for ( int i = 0; i < m_ChromeRingRest.Count(); i++ )
			m_ChromeRingRest[i] = color;

		ApplyChrome();
	}

	void RefreshTooltip()
	{
		if ( m_TooltipText != "" )
			UIActionTooltip.Show( m_TooltipText, m_TooltipIcon, m_TooltipSwatchColor, m_TooltipTextColor );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		m_ChromeHovered = true;
		ApplyChrome();

		//! The icon explains the warning; the rest of the control explains what
		//! it is for. Hovering the icon should answer the question the icon
		//! itself raised, so its tooltip wins over the control's.
		if ( w == m_WarnIcon && m_Warning && m_WarnTooltip != "" )
		{
			UIActionTooltip.Show( m_WarnTooltip, JMConstants.ICON_WARNING, JMTheme.DANGER, JMTheme.DANGER, w );
			return false;
		}

		if ( m_TooltipText != "" )
			UIActionTooltip.Show( m_TooltipText, m_TooltipIcon, m_TooltipSwatchColor, m_TooltipTextColor, w );

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		m_ChromeHovered = false;
		ApplyChrome();

		UIActionTooltip.Hide();

		return false;
	}
}
