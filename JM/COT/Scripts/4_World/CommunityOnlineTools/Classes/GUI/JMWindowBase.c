#ifndef CF_WINDOWS
typedef JMWindowBase CF_Window;

enum EResizeDirection
{
	UP         = 0,
	DOWN       = 1,
	LEFT       = 2,
	RIGHT      = 3,
	TOP_LEFT   = 4,
	TOP_RIGHT  = 5,
	BOT_LEFT   = 6,
	BOT_RIGHT  = 7
};

class JMWindowBase: COT_ScriptedWidgetEventHandler
{
#ifdef DIAG
	static int s_JMWindowBaseCount;
#endif

	protected Widget layoutRoot;
	protected ButtonWidget m_CloseButton;
	protected ButtonWidget m_MinimizeButton;
	protected ImageWidget  m_MinimizeButtonLabel;
	protected ButtonWidget m_PinButton;
	protected ImageWidget  m_PinButtonLabel;

	// Title-bar button chrome: a backdrop behind each icon, shown on hover and
	// deepened while held. The buttons use style Empty, so without these they
	// gave no feedback at all.
	protected Widget m_CloseButtonHover;
	protected Widget m_MinimizeButtonHover;
	protected Widget m_PinButtonHover;
	protected ImageWidget m_CloseButtonLabel;
	protected Widget m_PressedButton;
	protected Widget m_TitleWrapper;
	protected TextWidget m_TitleText;
	protected Widget m_TitlePanel;
	protected Widget m_TitleAccent;
	protected Widget m_Background;
	protected Widget m_ContentWidget;
	protected Widget m_ConfirmationPanel;

	// Edge drag handles
	protected Widget m_ResizeDragUp;
	protected Widget m_ResizeDragDown;
	protected Widget m_ResizeDragLeft;
	protected Widget m_ResizeDragRight;

	// Corner drag handles
	protected Widget m_ResizeDragTopLeft;
	protected Widget m_ResizeDragTopRight;
	protected Widget m_ResizeDragBotLeft;
	protected Widget m_ResizeDragBotRight;

	// Edge hover highlights
	protected Widget m_HighlightUp;
	protected Widget m_HighlightDown;
	protected Widget m_HighlightLeft;
	protected Widget m_HighlightRight;

	// Corner hover highlights
	protected Widget m_HighlightTopLeft;
	protected Widget m_HighlightTopRight;
	protected Widget m_HighlightBotLeft;
	protected Widget m_HighlightBotRight;
	protected ref JMFormBase m_Form;
	protected Widget m_FormRoot;
	protected JMRenderableModuleBase m_Module;
	protected JMConfirmationForm m_Confirmation;
	protected float m_OffsetX;
	protected float m_OffsetY;
	protected EResizeDirection m_ResizeDirection;
	protected float m_StartResizeSizeW;
	protected float m_StartResizeSizeH;
	protected float m_StartResizePositionX;
	protected float m_StartResizePositionY;
	protected float m_StartWindowPosX;
	protected float m_StartWindowPosY;

	//! Minimum content dimensions when resizing
	static const float RESIZE_MIN_CONTENT_HEIGHT = 55;
	static const float RESIZE_MIN_WIDTH          = 200;

	//! Maximum window size as a fraction of screen dimensions
	static const float RESIZE_MAX_SCREEN_FRACTION = 0.9;

	//! Pixels from a screen edge within which the window snaps flush to it
	static const float SNAP_THRESHOLD = 10;

	//! Minimize animation duration in seconds (see also JMUIAnimations.WINDOW_MINIMIZE_TIME)
	static const float MINIMIZE_ANIMATE_TIME = 0.2;

	//! Cached title bar height - read once in Init()
	protected float m_TitleBarHeight;
	protected bool m_IsShown;
	protected bool m_HasBeenCentered;

	//! Pin state - window survives COT close when pinned
	protected bool m_IsPinned;

	//! Minimize state
	protected bool m_IsMinimized;
	protected float m_RestoreWidth;
	protected float m_RestoreHeight;

	//! Minimize animation state
	protected bool m_IsAnimatingMinimize;
	protected bool m_IsAnimatingRestore;
	protected float m_AnimateTime;
	protected float m_AnimateFromH;
	protected float m_AnimateToH;

	// -- Title-bar button chrome ---------------------------------------------

	static const float ICON_ALPHA_REST  = 0.75;
	static const float ICON_ALPHA_HOVER = 1.0;

	//! Held state deepens the same backdrop instead of swapping colour, so the
	//! close button keeps its red and the other two stay neutral. These are set
	//! explicitly rather than via SetAlpha, which clamps at 1.0 and so cannot
	//! express "more opaque than the hover state".
	static const int HOVER_NEUTRAL         = 0x2EFFFFFF;
	static const int HOVER_NEUTRAL_PRESSED = 0x4DFFFFFF;
	static const int HOVER_CLOSE           = 0x59FF627D;
	static const int HOVER_CLOSE_PRESSED   = 0x8CFF627D;

	void JMWindowBase()
	{
		GetCOTWindowManager().AddWindow( this );

		m_RestoreWidth  = -1;
		m_RestoreHeight = -1;

	#ifdef DIAG
		s_JMWindowBaseCount++;
		CF_Log.Info("JMWindowBase count: " + s_JMWindowBaseCount);
	#endif
	}

	void ~JMWindowBase()
	{
		if (!g_Game)
			return;

	#ifdef DIAG
		auto trace = CF_Trace_0(this);
	#endif

		GetCOTWindowManager().RemoveWindow( this );

		Hide();

		if (m_Confirmation)
			m_Confirmation.Destroy();

		if (m_Form)
		{
			m_Form.OnDestruct();
			m_Form.Destroy();
		}

		//! @note unlinking the layout root is ABSOLUTELY necessary since destroying the widget handler will NOT do that automatically!
		DestroyWidget(layoutRoot);

	#ifdef DIAG
		s_JMWindowBaseCount--;
		if (s_JMWindowBaseCount <= 0)
			CF_Log.Info("JMWindowBase count: " + s_JMWindowBaseCount);
	#endif
	}

	JMFormBase GetForm()
	{
		return m_Form;
	}

	protected Widget GetHandleHighlight( Widget w )
	{
		if ( w == m_ResizeDragUp )       return m_HighlightUp;
		if ( w == m_ResizeDragDown )     return m_HighlightDown;
		if ( w == m_ResizeDragLeft )     return m_HighlightLeft;
		if ( w == m_ResizeDragRight )    return m_HighlightRight;
		if ( w == m_ResizeDragTopLeft )  return m_HighlightTopLeft;
		if ( w == m_ResizeDragTopRight ) return m_HighlightTopRight;
		if ( w == m_ResizeDragBotLeft )  return m_HighlightBotLeft;
		if ( w == m_ResizeDragBotRight ) return m_HighlightBotRight;
		return NULL;
	}

	Widget GetLayoutRoot()
	{
		return layoutRoot;
	}

	JMRenderableModuleBase GetModule()
	{
		return m_Module;
	}

	protected EResizeDirection GetResizeDirection( Widget w )
	{
		if ( w == m_ResizeDragUp )       return EResizeDirection.UP;
		if ( w == m_ResizeDragDown )     return EResizeDirection.DOWN;
		if ( w == m_ResizeDragLeft )     return EResizeDirection.LEFT;
		if ( w == m_ResizeDragRight )    return EResizeDirection.RIGHT;
		if ( w == m_ResizeDragTopLeft )  return EResizeDirection.TOP_LEFT;
		if ( w == m_ResizeDragTopRight ) return EResizeDirection.TOP_RIGHT;
		if ( w == m_ResizeDragBotLeft )  return EResizeDirection.BOT_LEFT;
		return EResizeDirection.BOT_RIGHT;
	}

	protected Widget GetTitleButtonHover( Widget w )
	{
		if ( w == m_CloseButton    ) return m_CloseButtonHover;
		if ( w == m_MinimizeButton ) return m_MinimizeButtonHover;
		if ( w == m_PinButton      ) return m_PinButtonHover;

		return NULL;
	}

	protected int GetTitleButtonHoverColor( Widget w, bool pressed )
	{
		if ( w == m_CloseButton )
		{
			if ( pressed )
				return HOVER_CLOSE_PRESSED;
			return HOVER_CLOSE;
		}

		if ( pressed )
			return HOVER_NEUTRAL_PRESSED;

		return HOVER_NEUTRAL;
	}

	//! @note kept for callers that use GetWidgetRoot() (e.g. JMMapForm)
	Widget GetWidgetRoot()
	{
		return layoutRoot;
	}

	//! Escape priority tier 1 (popup), this window's share of it: this
	//! window's own confirmation dialog, plus its form's registered overlays
	//! (context menus, dropdowns, value prompts - see JMFormBase.HasOpenOverlay).
	bool HasOpenPopup()
	{
		if ( m_ConfirmationPanel && m_ConfirmationPanel.IsVisible() )
			return true;

		if ( m_Form && m_Form.HasOpenOverlay() )
			return true;

		return false;
	}

	bool IsMinimized()
	{
		return m_IsMinimized;
	}

	bool IsPinned()
	{
		return m_IsPinned;
	}

	protected bool IsResizeHandle( Widget w )
	{
		return w == m_ResizeDragUp      || w == m_ResizeDragDown    || w == m_ResizeDragLeft    || w == m_ResizeDragRight    || w == m_ResizeDragTopLeft || w == m_ResizeDragTopRight || w == m_ResizeDragBotLeft || w == m_ResizeDragBotRight;
	}

	void SetBackgroundColour( float alpha, float r, float g, float b )
	{
		m_Background.SetColor( ARGB( alpha * 255, r * 255, g * 255, b * 255 ) );
	}

	void SetModule( JMRenderableModuleBase module )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "SetModule").Add(module.ToString());
		#endif

		m_Module = module;
		if ( Assert_Null( m_Module, "No valid RenderableModule supplied." ) )
			return;

		Widget menu = m_ContentWidget;

		if ( m_Module.GetLayoutRoot() != "" )
		{
			menu = g_Game.GetWorkspace().CreateWidgets( m_Module.GetLayoutRoot(), m_ContentWidget );
			if ( Assert_Null( menu, "No valid widget supplied." ) )
				return;

			float width = -1;
			float height = -1;
			menu.GetSize( width, height );

			float screenW, screenH;
			g_Game.GetWorkspace().GetScreenSize( screenW, screenH );

			if ( width > screenW )
				width = screenW;

			if ( height + m_TitleBarHeight > screenH )
				height = screenH - m_TitleBarHeight;

			m_ContentWidget.SetSize( width, height );
			SetSize( width, height );

			menu.GetScript( m_Form );
		}

		m_FormRoot = menu;

		if ( !m_Form )
		{
			m_Form = m_Module.InitForm( menu );
		}

		if ( Assert_Null( m_Form, "No valid Form supplied." ) )
			return;

		m_Form.Init( this, m_Module );

		m_TitleText.SetText( m_Module.GetTitle() );
		GetCOTWindowManager().BringFront( this );

		m_ConfirmationPanel.GetScript( m_Confirmation );

		if ( m_Confirmation )
			m_Confirmation.Init( this );

		// Notify form of its initial size so it can lay out immediately
		float winW, winH;
		layoutRoot.GetSize( winW, winH );
		m_Form.OnResize( winW, winH - m_TitleBarHeight );
	}

	void SetPosition( float x, float y )
	{
		float screenW, screenH;
		g_Game.GetWorkspace().GetScreenSize( screenW, screenH );

		float winW, winH;
		layoutRoot.GetSize( winW, winH );

		// Snap flush to screen edges when within threshold
		if ( x < SNAP_THRESHOLD )                    x = 0;
		if ( y < SNAP_THRESHOLD )                    y = 0;
		if ( screenW - ( x + winW ) < SNAP_THRESHOLD ) x = screenW - winW;
		if ( screenH - ( y + winH ) < SNAP_THRESHOLD ) y = screenH - winH;

		// Hard clamp: keep at least the title bar on screen
		x = Math.Clamp( x, 0, Math.Max( 0, screenW - winW ) );
		y = Math.Clamp( y, 0, Math.Max( 0, screenH - m_TitleBarHeight ) );

		layoutRoot.SetPos( x, y, true );

		m_TitleWrapper.SetPos( 0, 0, true );
	}

	protected void SetResizeHandlesVisible( bool show )
	{
		if ( m_ResizeDragUp )       m_ResizeDragUp.Show( show );
		if ( m_ResizeDragDown )     m_ResizeDragDown.Show( show );
		if ( m_ResizeDragLeft )     m_ResizeDragLeft.Show( show );
		if ( m_ResizeDragRight )    m_ResizeDragRight.Show( show );
		if ( m_ResizeDragTopLeft )  m_ResizeDragTopLeft.Show( show );
		if ( m_ResizeDragTopRight ) m_ResizeDragTopRight.Show( show );
		if ( m_ResizeDragBotLeft )  m_ResizeDragBotLeft.Show( show );
		if ( m_ResizeDragBotRight ) m_ResizeDragBotRight.Show( show );

		if ( !show )
		{
			if ( m_HighlightUp )       m_HighlightUp.Show( false );
			if ( m_HighlightDown )     m_HighlightDown.Show( false );
			if ( m_HighlightLeft )     m_HighlightLeft.Show( false );
			if ( m_HighlightRight )    m_HighlightRight.Show( false );
			if ( m_HighlightTopLeft )  m_HighlightTopLeft.Show( false );
			if ( m_HighlightTopRight ) m_HighlightTopRight.Show( false );
			if ( m_HighlightBotLeft )  m_HighlightBotLeft.Show( false );
			if ( m_HighlightBotRight ) m_HighlightBotRight.Show( false );
		}
	}

	//! Set the window size from content dimensions (content height excludes the title bar).
	//! Keeps m_RestoreWidth/Height in sync when not minimized.
	void SetSize( float w, float h )
	{
		float totalH = h + m_TitleBarHeight;
		layoutRoot.SetSize( w, totalH );

		if ( !m_IsMinimized )
		{
			m_RestoreWidth  = w;
			m_RestoreHeight = totalH;
		}
	}

	protected void SetTitleButtonIconAlpha( Widget w, float alpha )
	{
		if ( w == m_CloseButton && m_CloseButtonLabel )
			m_CloseButtonLabel.SetAlpha( alpha );
		else if ( w == m_MinimizeButton && m_MinimizeButtonLabel )
			m_MinimizeButtonLabel.SetAlpha( alpha );
		else if ( w == m_PinButton && m_PinButtonLabel )
			m_PinButtonLabel.SetAlpha( alpha );
	}

	protected void SetTitleButtonPressed( Widget w, bool pressed )
	{
		Widget hover = GetTitleButtonHover( w );
		if ( !hover )
			return;

		hover.SetColor( GetTitleButtonHoverColor( w, pressed ) );

		if ( pressed )
		{
			hover.Show( true );
			m_PressedButton = w;
		}
		else
		{
			m_PressedButton = NULL;
		}
	}

	void SetTitleColour( float alpha, float r, float g, float b )
	{
		m_TitlePanel.SetColor( ARGB( alpha * 255, r * 255, g * 255, b * 255 ) );
	}

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}

	void Init()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "Init");
		#endif

		m_CloseButton         = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "close_button" ) );
		m_MinimizeButton      = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "minimize_button"       ) );
		m_MinimizeButtonLabel = ImageWidget.Cast(  layoutRoot.FindAnyWidget( "minimize_button_label" ) );
		m_PinButton           = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "pin_button"             ) );
		m_PinButtonLabel      = ImageWidget.Cast(  layoutRoot.FindAnyWidget( "pin_button_label"       ) );
		m_CloseButtonLabel    = ImageWidget.Cast(  layoutRoot.FindAnyWidget( "close_button_image"     ) );
		m_CloseButtonHover    = layoutRoot.FindAnyWidget( "close_button_hover"    );
		m_MinimizeButtonHover = layoutRoot.FindAnyWidget( "minimize_button_hover" );
		m_PinButtonHover      = layoutRoot.FindAnyWidget( "pin_button_hover"      );

		ResetTitleButtonChrome();
		SyncPinIcon();
		SyncMinimizeIcon( m_IsMinimized );
		m_TitleWrapper        = Widget.Cast( layoutRoot.FindAnyWidget( "title_bar_drag" ) );
		m_TitlePanel          = layoutRoot.FindAnyWidget( "title_wrapper" );
		m_TitleAccent         = Widget.Cast( layoutRoot.FindAnyWidget( "title_accent" ) );
		m_TitleText           = TextWidget.Cast( layoutRoot.FindAnyWidget( "title_text" ) );
		m_Background          = Widget.Cast( layoutRoot.FindAnyWidget( "background" ) );
		m_ContentWidget       = Widget.Cast( layoutRoot.FindAnyWidget( "content" ) );
		m_ConfirmationPanel   = Widget.Cast( layoutRoot.FindAnyWidget( "confirmation_panel" ) );

		// Edge drag handles
		m_ResizeDragUp    = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_up" ) );
		m_ResizeDragDown  = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_down" ) );
		m_ResizeDragLeft  = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_left" ) );
		m_ResizeDragRight = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_right" ) );

		// Corner drag handles
		m_ResizeDragTopLeft  = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_top_left" ) );
		m_ResizeDragTopRight = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_top_right" ) );
		m_ResizeDragBotLeft  = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_bot_left" ) );
		m_ResizeDragBotRight = Widget.Cast( layoutRoot.FindAnyWidget( "resize_drag_bot_right" ) );

		// Edge highlights
		m_HighlightUp    = Widget.Cast( layoutRoot.FindAnyWidget( "resize_highlight_up" ) );
		m_HighlightDown  = Widget.Cast( layoutRoot.FindAnyWidget( "resize_highlight_down" ) );
		m_HighlightLeft  = Widget.Cast( layoutRoot.FindAnyWidget( "resize_highlight_left" ) );
		m_HighlightRight = Widget.Cast( layoutRoot.FindAnyWidget( "resize_highlight_right" ) );

		// Corner highlights
		m_HighlightTopLeft  = Widget.Cast( layoutRoot.FindAnyWidget( "resize_highlight_top_left" ) );
		m_HighlightTopRight = Widget.Cast( layoutRoot.FindAnyWidget( "resize_highlight_top_right" ) );
		m_HighlightBotLeft  = Widget.Cast( layoutRoot.FindAnyWidget( "resize_highlight_bot_left" ) );
		m_HighlightBotRight = Widget.Cast( layoutRoot.FindAnyWidget( "resize_highlight_bot_right" ) );

		// Guarantee highlights render above all content
		int highlightSort = 32000;
		if ( m_HighlightUp )        m_HighlightUp.SetSort( highlightSort );
		if ( m_HighlightDown )      m_HighlightDown.SetSort( highlightSort );
		if ( m_HighlightLeft )      m_HighlightLeft.SetSort( highlightSort );
		if ( m_HighlightRight )     m_HighlightRight.SetSort( highlightSort );
		if ( m_HighlightTopLeft )   m_HighlightTopLeft.SetSort( highlightSort );
		if ( m_HighlightTopRight )  m_HighlightTopRight.SetSort( highlightSort );
		if ( m_HighlightBotLeft )   m_HighlightBotLeft.SetSort( highlightSort );
		if ( m_HighlightBotRight )  m_HighlightBotRight.SetSort( highlightSort );

		// Also raise drag handles above content so they stay hittable
		int dragSort = 32001;
		if ( m_ResizeDragUp )       m_ResizeDragUp.SetSort( dragSort );
		if ( m_ResizeDragDown )     m_ResizeDragDown.SetSort( dragSort );
		if ( m_ResizeDragLeft )     m_ResizeDragLeft.SetSort( dragSort );
		if ( m_ResizeDragRight )    m_ResizeDragRight.SetSort( dragSort );
		if ( m_ResizeDragTopLeft )  m_ResizeDragTopLeft.SetSort( dragSort );
		if ( m_ResizeDragTopRight ) m_ResizeDragTopRight.SetSort( dragSort );
		if ( m_ResizeDragBotLeft )  m_ResizeDragBotLeft.SetSort( dragSort );
		if ( m_ResizeDragBotRight ) m_ResizeDragBotRight.SetSort( dragSort );

		// Cache title bar height - used everywhere instead of repeated GetSize() calls
		if ( m_TitleWrapper )
		{
			float tw, th;
			m_TitleWrapper.GetSize( tw, th );
			m_TitleBarHeight = th;
		}
		else
		{
			m_TitleBarHeight = 25;
		}

		// Position content and confirmation panel below the title bar
		if ( m_ContentWidget )
			m_ContentWidget.SetPos( 0, m_TitleBarHeight, true );

		if ( m_ConfirmationPanel )
			m_ConfirmationPanel.SetPos( 0, m_TitleBarHeight, true );

		// Offset top edge/corner handles and their highlights below the title bar
		if ( m_ResizeDragUp )       m_ResizeDragUp.SetPos( 10, m_TitleBarHeight, true );
		if ( m_ResizeDragTopLeft )  m_ResizeDragTopLeft.SetPos( 0, m_TitleBarHeight, true );
		if ( m_ResizeDragTopRight ) m_ResizeDragTopRight.SetPos( 0, m_TitleBarHeight, true );
		if ( m_HighlightUp )        m_HighlightUp.SetPos( 0, m_TitleBarHeight, true );
		if ( m_HighlightTopLeft )   m_HighlightTopLeft.SetPos( 0, m_TitleBarHeight, true );
		if ( m_HighlightTopRight )  m_HighlightTopRight.SetPos( 0, m_TitleBarHeight, true );
	}

	//! The confirmation scrim is sized in pixels by the open/resize animations,
	//! so a dialog raised before either has run would still be at the layout's
	//! default size - and the inner panel centres itself inside whatever this
	//! is, so getting it wrong puts the dialog off-screen with only the scrim
	//! visible. Cheap enough to just re-derive on every raise.
	protected void SyncConfirmationPanel()
	{
		if ( !m_ConfirmationPanel )
			return;

		float w, h;
		layoutRoot.GetSize( w, h );

		m_ConfirmationPanel.SetPos( 0, m_TitleBarHeight, true );
		m_ConfirmationPanel.SetSize( w, Math.Max( 0, h - m_TitleBarHeight ), true );
	}

	//! Closes whatever popup(s) are open on this window - confirmation first,
	//! then any open form overlay. Returns whether it closed anything.
	bool CloseOpenPopup()
	{
		bool closedAny = false;

		if ( m_ConfirmationPanel && m_ConfirmationPanel.IsVisible() && m_Confirmation )
		{
			m_Confirmation.Close();
			closedAny = true;
		}

		if ( m_Form && m_Form.CloseOpenOverlays() )
			closedAny = true;

		return closedAny;
	}

	JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne )
	{
		if ( !m_Confirmation )
			return NULL;

		SyncConfirmationPanel();

		m_Confirmation.CreateConfirmation_One( type, title, message, callBackOneName, callBackOne );

		return m_Confirmation;
	}

	JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo )
	{
		if ( !m_Confirmation )
			return NULL;

		SyncConfirmationPanel();

		m_Confirmation.CreateConfirmation_Two( type, title, message, callBackOneName, callBackTwoName, callBackOne, callBackTwo );

		return m_Confirmation;
	}

	JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, string callBackThreeName, string callBackThree )
	{
		if ( !m_Confirmation )
			return NULL;

		SyncConfirmationPanel();

		m_Confirmation.CreateConfirmation_Three( type, title, message, callBackOneName, callBackTwoName, callBackThreeName, callBackOne, callBackTwo, callBackThree );

		return m_Confirmation;
	}

	override bool IsVisible()
	{
		if (layoutRoot && layoutRoot.IsVisible())
			return true;

		return false;
	}

	void TogglePin()
	{
		m_IsPinned = !m_IsPinned;

		SyncPinIcon();
	}

	//! Point the pin button at the state it is currently in: a plain pin while
	//! the window is pinned, a struck-through pin while it is not. The colour
	//! swap stays - it is what makes the state readable at a glance - but the
	//! glyph now carries it too, so the button is not two identical pins that
	//! only differ in tint.
	protected void SyncPinIcon()
	{
		if ( !m_PinButtonLabel )
			return;

		if ( m_IsPinned )
		{
			m_PinButtonLabel.LoadImageFile( 0, JMConstants.ICON_PIN );
			m_PinButtonLabel.SetColor( JMUIColors.PIN_ACTIVE );
		}
		else
		{
			m_PinButtonLabel.LoadImageFile( 0, JMConstants.ICON_PIN_OFF );
			m_PinButtonLabel.SetColor( JMUIColors.PIN_INACTIVE );
		}

		m_PinButtonLabel.SetImage( 0 );
	}

	//! Same idea for the collapse button: chevrons pointing at each other while
	//! the window is open (click to collapse), pointing apart while it is
	//! collapsed (click to restore).
	//! @param collapsed the state the window is heading INTO. Minimize() and
	//!        Restore() both start an animation and only flip m_IsMinimized when
	//!        it finishes, so the caller says where it is going rather than
	//!        letting this read a flag that has not moved yet.
	protected void SyncMinimizeIcon( bool collapsed )
	{
		if ( !m_MinimizeButtonLabel )
			return;

		if ( collapsed )
		{
			m_MinimizeButtonLabel.LoadImageFile( 0, JMConstants.ICON_EXPAND_VERTICAL );
			m_MinimizeButtonLabel.SetColor( JMUIColors.PIN_ACTIVE );
		}
		else
		{
			m_MinimizeButtonLabel.LoadImageFile( 0, JMConstants.ICON_COLLAPSE_VERTICAL );
			m_MinimizeButtonLabel.SetColor( JMUIColors.PIN_INACTIVE );
		}

		m_MinimizeButtonLabel.SetImage( 0 );
	}

	void Show()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this);
		#endif

		string flickerTitle = "?";
		if (m_Module)
			flickerTitle = m_Module.GetTitle();

		if ( !layoutRoot )
			return;

		if ( !m_Form )
			return;

		layoutRoot.Show( true );

		if ( !m_HasBeenCentered )
		{
			CenterOnScreen();
			m_HasBeenCentered = true;
		}

		if (!m_Form.m_IsShown)
		{
			m_Form.OnShow();
			m_Form.m_IsShown = true;
		}

		layoutRoot.Update();

		if (m_IsShown)
			return;

		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( Update );

		if ( !GetCommunityOnlineToolsBase().IsOpen() )
		{
			g_Game.GetMission().AddActiveInputExcludes({"menu"});
			g_Game.GetUIManager().ShowUICursor( true );
		}

		m_IsShown = true;
	}

	void Hide()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "Hide");
		#endif

		string flickerTitle = "?";
		if (m_Module)
			flickerTitle = m_Module.GetTitle();

		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( Update );

		if (!m_Form)
			return;

		m_Form.OnHide();
		m_Form.m_IsShown = false;

		if (!layoutRoot || layoutRoot.ToString() == "INVALID")
			return;

		layoutRoot.Show( false );

		if ( !GetCommunityOnlineToolsBase().IsOpen() && !GetCOTWindowManager().HasAnyUnpinnedActive() )
		{
			g_Game.GetMission().RemoveActiveInputExcludes({"menu"});
			g_Game.GetUIManager().ShowUICursor( false );
		}

		m_IsShown = false;
	}

	//! Collapse the window to title-bar only with an animated transition.
	//! Does nothing if a confirmation dialog is currently open.
	void Minimize()
	{
		if ( m_IsMinimized || m_IsAnimatingMinimize || m_IsAnimatingRestore )
			return;

		if ( m_ConfirmationPanel && m_ConfirmationPanel.IsVisible() )
			return;

		float w, h;
		layoutRoot.GetSize( w, h );
		m_RestoreWidth  = w;
		m_RestoreHeight = h;

		m_AnimateFromH = h;
		m_AnimateToH   = m_TitleBarHeight;
		m_AnimateTime  = 0;

		m_IsAnimatingMinimize = true;

		SyncMinimizeIcon( true );
	}

	//! Restore the window to its pre-minimized size with an animated transition.
	void Restore()
	{
		if ( !m_IsMinimized || m_IsAnimatingMinimize || m_IsAnimatingRestore )
			return;

		if ( m_RestoreWidth <= 0 || m_RestoreHeight <= 0 )
			return;

		if ( m_ContentWidget ) m_ContentWidget.Show( true );
		SetResizeHandlesVisible( true );

		m_AnimateFromH = m_TitleBarHeight;
		m_AnimateToH   = m_RestoreHeight;
		m_AnimateTime  = 0;

		m_IsAnimatingRestore = true;

		SyncMinimizeIcon( false );
	}

	//! Toggle between minimized and restored states.
	void ToggleMinimize()
	{
		if ( m_IsMinimized )
			Restore();
		else
			Minimize();
	}

	void Focus()
	{
		SetBackgroundColour( JMUIColors.BG_FOCUSED_A,     JMUIColors.BG_FOCUSED_R,     JMUIColors.BG_FOCUSED_G,     JMUIColors.BG_FOCUSED_B );
		SetTitleColour(      JMUIColors.TITLE_FOCUSED_A,  JMUIColors.TITLE_FOCUSED_R,  JMUIColors.TITLE_FOCUSED_G,  JMUIColors.TITLE_FOCUSED_B );

		if ( m_TitleAccent ) m_TitleAccent.Show( true );

		if ( m_Form ) m_Form.OnFocus();
	}

	void Unfocus()
	{
		SetBackgroundColour( JMUIColors.BG_UNFOCUSED_A,    JMUIColors.BG_UNFOCUSED_R,    JMUIColors.BG_UNFOCUSED_G,    JMUIColors.BG_UNFOCUSED_B );
		SetTitleColour(      JMUIColors.TITLE_UNFOCUSED_A, JMUIColors.TITLE_UNFOCUSED_R, JMUIColors.TITLE_UNFOCUSED_G, JMUIColors.TITLE_UNFOCUSED_B );

		if ( m_TitleAccent ) m_TitleAccent.Show( false );

		if ( m_Form ) m_Form.OnUnfocus();
	}

	void Update( float timeSlice )
	{
		if ( m_Form )
			m_Form.Update();

		if ( !m_TitleWrapper )
			return;

		m_TitleWrapper.SetPos( 0, 0, true );

		UpdateMinimizeAnimation( timeSlice );
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		if ( button == MouseState.LEFT )
			SetTitleButtonPressed( w, true );

		Widget parentWidget = w;
		while ( parentWidget != NULL )
		{
			if ( parentWidget == layoutRoot )
			{
				GetCOTWindowManager().BringFront( this );
				break;
			}
			parentWidget = parentWidget.GetParent();
		}

		return super.OnMouseButtonDown( w, x, y, button );
	}

	override bool OnMouseButtonUp( Widget w, int x, int y, int button )
	{
		if ( m_PressedButton )
			SetTitleButtonPressed( m_PressedButton, false );

		return super.OnMouseButtonUp( w, x, y, button );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_CloseButton )
		{
			m_Module.Close();
			return true;
		}

		if ( w == m_MinimizeButton )
		{
			ToggleMinimize();
			return true;
		}

		if ( w == m_PinButton )
		{
			TogglePin();
			return true;
		}

		return false;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
		Widget buttonHover = GetTitleButtonHover( w );
		if ( buttonHover )
		{
			buttonHover.SetColor( GetTitleButtonHoverColor( w, false ) );
			buttonHover.Show( true );
			SetTitleButtonIconAlpha( w, ICON_ALPHA_HOVER );
			return true;
		}

		if ( m_IsMinimized )
			return false;

		Widget highlight = GetHandleHighlight( w );
		if ( highlight )
		{
			highlight.Show( true );
			return true;
		}

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		Widget buttonHover = GetTitleButtonHover( w );
		if ( buttonHover )
		{
			buttonHover.Show( false );
			SetTitleButtonIconAlpha( w, ICON_ALPHA_REST );

			if ( m_PressedButton == w )
				m_PressedButton = NULL;

			return true;
		}

		Widget highlight = GetHandleHighlight( w );
		if ( highlight )
		{
			highlight.Show( false );
			return true;
		}

		return false;
	}

	protected void ResetTitleButtonChrome()
	{
		if ( m_CloseButtonHover )    m_CloseButtonHover.Show( false );
		if ( m_MinimizeButtonHover ) m_MinimizeButtonHover.Show( false );
		if ( m_PinButtonHover )      m_PinButtonHover.Show( false );

		if ( m_CloseButtonLabel )    m_CloseButtonLabel.SetAlpha( ICON_ALPHA_REST );
		if ( m_MinimizeButtonLabel ) m_MinimizeButtonLabel.SetAlpha( ICON_ALPHA_REST );
		if ( m_PinButtonLabel )      m_PinButtonLabel.SetAlpha( ICON_ALPHA_REST );

		m_PressedButton = NULL;
	}

	override bool OnDrag( Widget w, int x, int y )
	{
		if ( w == m_TitleWrapper )
		{
			layoutRoot.GetPos( m_OffsetX, m_OffsetY );

			m_OffsetX = x - m_OffsetX;
			m_OffsetY = y - m_OffsetY;

			m_TitleWrapper.SetPos( 0, 0, true );
			m_TitleWrapper.SetPos( 0, 0, false );

			return true;
		}

		if ( !m_IsMinimized && IsResizeHandle( w ) )
		{
			w.SetPos( 0, 0, true );
			w.SetPos( 0, 0, false );

			m_ResizeDirection = GetResizeDirection( w );

			m_StartResizePositionX = x;
			m_StartResizePositionY = y;

			layoutRoot.GetSize( m_StartResizeSizeW, m_StartResizeSizeH );
			layoutRoot.GetPos( m_StartWindowPosX, m_StartWindowPosY );

			Widget highlight = GetHandleHighlight( w );
			if ( highlight ) highlight.Show( false );

			return true;
		}

		return false;
	}

	override bool OnDragging( Widget w, int x, int y, Widget reciever )
	{
		if ( w == m_TitleWrapper )
		{
			SetPosition( x - m_OffsetX, y - m_OffsetY );

			return true;
		}

		if ( !m_IsMinimized && IsResizeHandle( w ) )
		{
			Resize( x, y );

			return true;
		}

		return false;
	}

	override bool OnDrop( Widget w, int x, int y, Widget reciever )
	{
		if ( w == m_TitleWrapper )
		{
			SetPosition( x - m_OffsetX, y - m_OffsetY );

			return true;
		}

		if ( !m_IsMinimized && IsResizeHandle( w ) )
		{
			w.SetPos( 0, 0, true );

			return true;
		}

		return false;
	}

	// --- Private helpers --------------------------------------------------------

	protected void CenterOnScreen()
	{
		float screenW, screenH;
		g_Game.GetWorkspace().GetScreenSize( screenW, screenH );

		float winW, winH;
		layoutRoot.GetSize( winW, winH );

		int openCount = GetCOTWindowManager().Count() - 1;
		float cascade = Math.Clamp( openCount * JMUILayout.WINDOW_CASCADE_OFFSET, 0, JMUILayout.WINDOW_CASCADE_MAX );

		SetPosition( ( screenW - winW ) * 0.5 + cascade, ( screenH - winH ) * 0.5 + cascade );
	}

	protected void UpdateMinimizeAnimation( float timeSlice )
	{
		if ( !m_IsAnimatingMinimize && !m_IsAnimatingRestore )
			return;

		m_AnimateTime += timeSlice;
		float percent = Math.Clamp( m_AnimateTime / MINIMIZE_ANIMATE_TIME, 0, 1 );

		// EaseOut: fast start, slow finish
		float eased = 1.0 - Math.Cos( percent * Math.PI * 0.5 );

		float curH = Math.Lerp( m_AnimateFromH, m_AnimateToH, eased );
		float curW, existingH;
		layoutRoot.GetSize( curW, existingH );
		layoutRoot.SetSize( curW, curH );

		float contentH = Math.Max( 0, curH - m_TitleBarHeight );

		if ( m_ContentWidget )
			m_ContentWidget.SetSize( curW, contentH );

		if ( m_FormRoot )
			m_FormRoot.SetSize( curW, contentH );

		if ( m_ConfirmationPanel )
			m_ConfirmationPanel.SetSize( curW, contentH );

		if ( percent >= 1.0 )
		{
			if ( m_IsAnimatingMinimize )
			{
				if ( m_ContentWidget )       m_ContentWidget.Show( false );
				SetResizeHandlesVisible( false );
				m_IsMinimized = true;
				m_IsAnimatingMinimize = false;
			}
			else
			{
				m_IsMinimized = false;
				m_IsAnimatingRestore = false;
			}
		}
	}

	protected void Resize( float x, float y )
	{
		float deltaX = x - m_StartResizePositionX;
		float deltaY = y - m_StartResizePositionY;

		float screenW, screenH;
		g_Game.GetWorkspace().GetScreenSize( screenW, screenH );

		float minHeight = m_TitleBarHeight + RESIZE_MIN_CONTENT_HEIGHT;
		float maxWidth  = screenW * RESIZE_MAX_SCREEN_FRACTION;
		float maxHeight = screenH * RESIZE_MAX_SCREEN_FRACTION;

		float newWidth  = m_StartResizeSizeW;
		float newHeight = m_StartResizeSizeH;
		float newPosX   = m_StartWindowPosX;
		float newPosY   = m_StartWindowPosY;

		switch ( m_ResizeDirection )
		{
			case EResizeDirection.UP:
				newHeight = Math.Clamp( m_StartResizeSizeH - deltaY, minHeight, maxHeight );
				newPosY   = m_StartWindowPosY + ( m_StartResizeSizeH - newHeight );
				break;
			case EResizeDirection.DOWN:
				newHeight = Math.Clamp( m_StartResizeSizeH + deltaY, minHeight, maxHeight );
				break;
			case EResizeDirection.LEFT:
				newWidth = Math.Clamp( m_StartResizeSizeW - deltaX, RESIZE_MIN_WIDTH, maxWidth );
				newPosX  = m_StartWindowPosX + ( m_StartResizeSizeW - newWidth );
				break;
			case EResizeDirection.RIGHT:
				newWidth = Math.Clamp( m_StartResizeSizeW + deltaX, RESIZE_MIN_WIDTH, maxWidth );
				break;
			case EResizeDirection.TOP_LEFT:
				newWidth  = Math.Clamp( m_StartResizeSizeW - deltaX, RESIZE_MIN_WIDTH, maxWidth );
				newPosX   = m_StartWindowPosX + ( m_StartResizeSizeW - newWidth );
				newHeight = Math.Clamp( m_StartResizeSizeH - deltaY, minHeight, maxHeight );
				newPosY   = m_StartWindowPosY + ( m_StartResizeSizeH - newHeight );
				break;
			case EResizeDirection.TOP_RIGHT:
				newWidth  = Math.Clamp( m_StartResizeSizeW + deltaX, RESIZE_MIN_WIDTH, maxWidth );
				newHeight = Math.Clamp( m_StartResizeSizeH - deltaY, minHeight, maxHeight );
				newPosY   = m_StartWindowPosY + ( m_StartResizeSizeH - newHeight );
				break;
			case EResizeDirection.BOT_LEFT:
				newWidth  = Math.Clamp( m_StartResizeSizeW - deltaX, RESIZE_MIN_WIDTH, maxWidth );
				newPosX   = m_StartWindowPosX + ( m_StartResizeSizeW - newWidth );
				newHeight = Math.Clamp( m_StartResizeSizeH + deltaY, minHeight, maxHeight );
				break;
			case EResizeDirection.BOT_RIGHT:
				newWidth  = Math.Clamp( m_StartResizeSizeW + deltaX, RESIZE_MIN_WIDTH, maxWidth );
				newHeight = Math.Clamp( m_StartResizeSizeH + deltaY, minHeight, maxHeight );
				break;
		}

		layoutRoot.SetSize( newWidth, newHeight );
		layoutRoot.SetPos( newPosX, newPosY, true );

		float contentH = newHeight - m_TitleBarHeight;

		if ( m_ContentWidget )
			m_ContentWidget.SetSize( newWidth, contentH );

		if ( m_FormRoot )
			m_FormRoot.SetSize( newWidth, contentH );

		if ( m_ConfirmationPanel )
			m_ConfirmationPanel.SetSize( newWidth, contentH );

		m_TitleWrapper.SetPos( 0, 0, true );

		// Keep restore size in sync with user-driven resizes
		m_RestoreWidth  = newWidth;
		m_RestoreHeight = newHeight;

		if ( m_Form )
			m_Form.OnResize( newWidth, contentH );
	}
}
#endif
