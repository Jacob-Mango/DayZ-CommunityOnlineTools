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

	//! Widget has no IsEnabled(), and 11 layouts have no disable
	//! overlay to read the state back off, so it is tracked here.
	protected bool m_Enabled = true;

	protected Class m_Instance;
	protected string m_FuncName;

	protected bool m_IsShown;

	protected bool m_HasCallback;

	protected bool m_WasFocused;

	protected bool m_LeftMouseDown;

	protected ref UIActionData m_Data;

	// ??? Flex layout spec (opt-in) ??????????????????????????????????????????
	// Set via SetFlex(). Read by UIActionFlexRow during layout. When
	// m_HasFlex is false the element uses whatever width SetWidth/SetFixedSize
	// gave it and is ignored by the flex pass - default behaviour unchanged.
	protected bool  m_HasFlex;
	protected float m_FlexGrow;    // proportional share of leftover space (0 = none)
	protected float m_FlexBasisPx; // fixed pixel floor before growing (min width)
	protected float m_FlexMaxPx;   // pixel ceiling; <= 0 means unbounded

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

	void GetUserData( out Class data )
	{
		layoutRoot.GetUserData( data );
	}

	void SetUserData( Class data )
	{
		layoutRoot.SetUserData( data );
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
		Print( "+" + this + "::Show" );
		#endif

		if (m_IsShown)
			return;

		layoutRoot.Show( true );
		OnShow();

		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( Update );

		m_IsShown = true;

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Show" );
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
			CommunityOnlineTools.ForceDisableInputs(true);
		}
		else if (!isFocused && m_WasFocused)
		{
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

	bool IsFocused()
	{
		return m_WasFocused;
	}

	bool IsFocusWidget( Widget widget )
	{
		return false;
	}

	override void UpdatePermission( string permission )
	{
		SetEnabled( GetPermissionsManager().HasPermission( permission ) );
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

	bool IsEnabled()
	{
		if ( !m_Disable )
			return m_Enabled;

		return !m_Disable.IsVisible();
	}

	void Show( bool show )
	{
		if ( show )
			Show();
		else
			Hide();
	}

	void SetDisableColor( int color )
	{
		if ( m_Disable )
			m_Disable.SetColor(color);
	}

	void SetDisableAlpha( float alpha )
	{
		if ( m_Disable )
			m_Disable.SetAlpha(alpha);
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

	void SetSize( float width, float height )
	{
		layoutRoot.SetSize( width, height );
		layoutRoot.Update();
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

	void SetHeight( float height )
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		layoutRoot.SetSize( w, height );
		layoutRoot.Update();
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

	float GetHeight()
	{
		float w;
		float h;
		layoutRoot.GetSize( w, h );
		return h;
	}

	// ??? Flex API (opt-in) ??????????????????????????????????????????????????
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

	bool  HasFlex()      { return m_HasFlex; }
	float GetFlexGrow()  { return m_FlexGrow; }
	float GetFlexBasis() { return m_FlexBasisPx; }
	float GetFlexMax()   { return m_FlexMaxPx; }

	void SetYPosition( float ypos )
	{
		float w;
		float h;
		layoutRoot.GetPos( w, h );
		layoutRoot.SetPos( w, ypos );
		layoutRoot.Update();
	}

	void SetFixedSize( float width, float height )
	{
		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE, true );
		layoutRoot.SetFlags( WidgetFlags.HEXACTSIZE, true );
		layoutRoot.SetSize( width, height );
		layoutRoot.Update();
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

	void SetWidgetPosition( Widget widget, float xpos )
	{
		float x;
		float y;
		widget.GetPos( x, y );
		widget.SetPos( xpos, y );
		layoutRoot.Update();
		widget.Update();
	}

	Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	bool IsVisible()
	{
		if ( !layoutRoot )
			return false;
			
		return layoutRoot.IsVisible();
	}

	void SetCallback( Class instance, string funcname )
	{
		if ( instance == NULL || funcname == "" ) return;

		m_Instance = instance;
		m_FuncName = funcname;

		m_HasCallback = true;
	}

	bool CallEvent( UIEvent eid )
	{
		if ( !m_HasCallback )
			return false;

		auto params = new Param2< UIEvent, UIActionBase >( eid, this );
		g_Game.GameScript.CallFunctionParams( m_Instance, m_FuncName, NULL, params );

		return false;
	}

	void SetData( UIActionData data )
	{
		m_Data = data;
	}

	UIActionData GetData()
	{
		return m_Data;
	}

	void SetButton( string text )
	{
	}

	string GetButton()
	{
		return "";
	}

	void SetLabel( string text )
	{
	}

	void SetChecked( bool checked )
	{
	}
	
	bool IsChecked()
	{
		return false;
	}
	
	void SetText( string text )
	{
	}

	string GetText()
	{
		return "";
	}

	float GetCurrent()
	{
		return 0.0;
	}

	void SetCurrent( float value )
	{
		
	}

	void SetValue( vector v )
	{
	}

	vector GetValue()
	{
		return "0 0 0";
	}

	void SetSelection( int i, bool sendEvent = true )
	{
	}

	int GetSelection()
	{
		return -1;
	}

	void SetIcon( string imagePath )
	{
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

	void AnimateSuccess()
	{
		AnimateFeedback();
	}

	void AnimateError()
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

	void RefreshTooltip()
	{
		if ( m_TooltipText != "" )
			UIActionTooltip.Show( m_TooltipText, m_TooltipIcon, m_TooltipSwatchColor, m_TooltipTextColor );
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		m_ChromeHovered = true;
		ApplyChrome();

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

	protected string m_TooltipText;
	protected string m_TooltipIcon;
	protected int    m_TooltipSwatchColor;
	protected int    m_TooltipTextColor;
}
