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

	private Widget layoutRoot;

	private ButtonWidget m_CloseButton;
	private ButtonWidget m_MinimizeButton;
	private TextWidget   m_MinimizeButtonLabel;
	private Widget m_TitleWrapper;
	private TextWidget m_TitleText;
	private Widget m_TitlePanel;
	private Widget m_Background;
	private Widget m_ContentWidget;
	private Widget m_ConfirmationPanel;

	// Edge drag handles
	private Widget m_ResizeDragUp;
	private Widget m_ResizeDragDown;
	private Widget m_ResizeDragLeft;
	private Widget m_ResizeDragRight;

	// Corner drag handles
	private Widget m_ResizeDragTopLeft;
	private Widget m_ResizeDragTopRight;
	private Widget m_ResizeDragBotLeft;
	private Widget m_ResizeDragBotRight;

	// Edge hover highlights
	private Widget m_HighlightUp;
	private Widget m_HighlightDown;
	private Widget m_HighlightLeft;
	private Widget m_HighlightRight;

	// Corner hover highlights
	private Widget m_HighlightTopLeft;
	private Widget m_HighlightTopRight;
	private Widget m_HighlightBotLeft;
	private Widget m_HighlightBotRight;

	private ref JMFormBase m_Form;
	private JMRenderableModuleBase m_Module;
	private JMConfirmationForm m_Confirmation;

	private float m_OffsetX;
	private float m_OffsetY;

	private EResizeDirection m_ResizeDirection;
	private float m_StartResizeSizeW;
	private float m_StartResizeSizeH;
	private float m_StartResizePositionX;
	private float m_StartResizePositionY;
	private float m_StartWindowPosX;
	private float m_StartWindowPosY;

	//! Minimum content height below the title bar (px)
	static const float RESIZE_MIN_CONTENT_HEIGHT = 55;
	static const float RESIZE_MIN_WIDTH          = 200;

	//! Cached title bar height — read once in Init()
	private float m_TitleBarHeight;

	private bool m_IsShown;
	private bool m_HasBeenCentered;

	//! Minimize state
	private bool m_IsMinimized;
	private float m_RestoreWidth;
	private float m_RestoreHeight;

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
			m_Form.Destroy();

		//! @note unlinking the layout root is ABSOLUTELY necessary since destroying the widget handler will NOT do that automatically!
	#ifdef DAYZ_1_28
		DestroyWidget(layoutRoot);
	#endif

	#ifdef DIAG
		s_JMWindowBaseCount--;
		if (s_JMWindowBaseCount <= 0)
			CF_Log.Info("JMWindowBase count: " + s_JMWindowBaseCount);
	#endif
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
		m_MinimizeButton      = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "minimize_button" ) );
		m_MinimizeButtonLabel = TextWidget.Cast(   layoutRoot.FindAnyWidget( "minimize_button_label" ) );
		m_TitleWrapper        = Widget.Cast( layoutRoot.FindAnyWidget( "title_bar_drag" ) );
		m_TitlePanel          = layoutRoot.FindAnyWidget( "title_wrapper" );
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

		// Cache title bar height — used everywhere instead of repeated GetSize() calls
		float tw, th;
		m_TitleWrapper.GetSize( tw, th );
		m_TitleBarHeight = th;

		// Position content and confirmation panel below the title bar
		if ( m_ContentWidget )
			m_ContentWidget.SetPos( 0, m_TitleBarHeight, true );

		if ( m_ConfirmationPanel )
			m_ConfirmationPanel.SetPos( 0, m_TitleBarHeight, true );

		// Offset top edge/corner handles and their highlights below the title bar.
		// Done here so the layout stays free of the hardcoded title bar height.
		if ( m_ResizeDragUp )       m_ResizeDragUp.SetPos( 10, m_TitleBarHeight, true );
		if ( m_ResizeDragTopLeft )  m_ResizeDragTopLeft.SetPos( 0, m_TitleBarHeight, true );
		if ( m_ResizeDragTopRight ) m_ResizeDragTopRight.SetPos( 0, m_TitleBarHeight, true );
		if ( m_HighlightUp )        m_HighlightUp.SetPos( 0, m_TitleBarHeight, true );
		if ( m_HighlightTopLeft )   m_HighlightTopLeft.SetPos( 0, m_TitleBarHeight, true );
		if ( m_HighlightTopRight )  m_HighlightTopRight.SetPos( 0, m_TitleBarHeight, true );
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

			m_ContentWidget.SetSize( width, height );
			SetSize( width, height );

			menu.GetScript( m_Form );
		}

		if ( !m_Form )
			m_Form = m_Module.InitForm( menu );

		if ( Assert_Null( m_Form, "No valid Form supplied." ) )
			return;

		m_Form.Init( this, m_Module );
		m_TitleText.SetText( m_Module.GetTitle() );
		GetCOTWindowManager().BringFront( this );

		m_ConfirmationPanel.GetScript( m_Confirmation );

		if ( m_Confirmation )
			m_Confirmation.Init( this );
	}

	JMRenderableModuleBase GetModule()
	{
		return m_Module;
	}

	JMFormBase GetForm()
	{
		return m_Form;
	}

	Widget GetLayoutRoot()
	{
		return layoutRoot;
	}

	//! @note kept for callers that use GetWidgetRoot() (e.g. JMMapForm)
	Widget GetWidgetRoot()
	{
		return layoutRoot;
	}

	JMConfirmation CreateConfirmation_One( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, int btnIdOffset = -1 )
	{
		if ( !m_Confirmation )
			return NULL;

		m_Confirmation.CreateConfirmation_One( type, title, message, callBackOneName, callBackOne, btnIdOffset );

		return m_Confirmation;
	}

	JMConfirmation CreateConfirmation_Two( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, int btnIdOffset = -1 )
	{
		if ( !m_Confirmation )
			return NULL;

		m_Confirmation.CreateConfirmation_Two( type, title, message, callBackOneName, callBackTwoName, callBackOne, callBackTwo, btnIdOffset );

		return m_Confirmation;
	}

	JMConfirmation CreateConfirmation_Three( JMConfirmationType type, string title, string message, string callBackOneName, string callBackOne, string callBackTwoName, string callBackTwo, string callBackThreeName, string callBackThree, int btnIdOffset = -1 )
	{
		if ( !m_Confirmation )
			return NULL;

		m_Confirmation.CreateConfirmation_Three( type, title, message, callBackOneName, callBackTwoName, callBackThreeName, callBackOne, callBackTwo, callBackThree, btnIdOffset );

		return m_Confirmation;
	}

	void SetTitleColour( float alpha, float r, float g, float b )
	{
		m_TitlePanel.SetColor( ARGB( alpha * 255, r * 255, g * 255, b * 255 ) );
	}

	void SetBackgroundColour( float alpha, float r, float g, float b )
	{
		m_Background.SetColor( ARGB( alpha * 255, r * 255, g * 255, b * 255 ) );
	}

	bool IsVisible()
	{
		if (layoutRoot && layoutRoot.IsVisible())
			return true;

		return false;
	}

	bool IsMinimized()
	{
		return m_IsMinimized;
	}

	void Show()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this);
		#endif

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

		g_Game.GetInput().ChangeGameFocus( 1 );
		g_Game.GetUIManager().ShowUICursor( true );

		m_IsShown = true;
	}

	void Hide()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "Hide");
		#endif

		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( Update );

		if (!m_Form)
			return;

		m_Form.OnHide();
		m_Form.m_IsShown = false;

		if (!layoutRoot || layoutRoot.ToString() == "INVALID")
			return;

		layoutRoot.Show( false );

		if ( !GetCommunityOnlineToolsBase().IsOpen() && !GetCOTWindowManager().HasAnyActive() )
		{
			g_Game.GetInput().ResetGameFocus();
			g_Game.GetUIManager().ShowUICursor( false );
		}

		m_IsShown = false;
	}

	//! Collapse the window to title-bar only, hiding content and resize handles.
	//! Does nothing if a confirmation dialog is currently open.
	void Minimize()
	{
		if ( m_IsMinimized )
			return;

		if ( m_ConfirmationPanel && m_ConfirmationPanel.IsVisible() )
			return;

		float w, h;
		layoutRoot.GetSize( w, h );
		m_RestoreWidth  = w;
		m_RestoreHeight = h;

		layoutRoot.SetSize( w, m_TitleBarHeight );

		if ( m_ContentWidget )       m_ContentWidget.Show( false );
		SetResizeHandlesVisible( false );

		if ( m_MinimizeButtonLabel ) m_MinimizeButtonLabel.SetText( "^" );

		m_IsMinimized = true;
	}

	//! Restore the window to its pre-minimized size.
	void Restore()
	{
		if ( !m_IsMinimized )
			return;

		if ( m_RestoreWidth > 0 && m_RestoreHeight > 0 )
			layoutRoot.SetSize( m_RestoreWidth, m_RestoreHeight );

		if ( m_ContentWidget )       m_ContentWidget.Show( true );
		SetResizeHandlesVisible( true );

		if ( m_MinimizeButtonLabel ) m_MinimizeButtonLabel.SetText( "_" );

		m_IsMinimized = false;
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
		SetBackgroundColour( 0.9995, 0.06, 0.08, 0.11 );
		SetTitleColour( 1.0, 0.04, 0.04, 0.12 );

		if ( m_Form ) m_Form.OnFocus();
	}

	void Unfocus()
	{
		SetBackgroundColour( 0.95, 0.042, 0.056, 0.077 );
		SetTitleColour( 1.0, 0.02, 0.02, 0.06 );

		if ( m_Form ) m_Form.OnUnfocus();
	}

	void Update( float timeSlice )
	{
		if ( m_Form )
			m_Form.Update();

		if ( !m_TitleWrapper )
			return;

		m_TitleWrapper.SetPos( 0, 0, true );
	}

	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
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

		return false;
	}

	override bool OnMouseEnter( Widget w, int x, int y )
	{
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
		Widget highlight = GetHandleHighlight( w );
		if ( highlight )
		{
			highlight.Show( false );
			return true;
		}

		return false;
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

			// Hide highlight while dragging
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
			// Final position already applied by OnDragging; just reset the handle widget
			w.SetPos( 0, 0, true );

			return true;
		}

		return false;
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

	void SetPosition( float x, float y )
	{
		// Clamp so at least the title bar stays on-screen
		float screenW, screenH;
		g_Game.GetWorkspace().GetScreenSize( screenW, screenH );

		float winW, winH;
		layoutRoot.GetSize( winW, winH );

		x = Math.Clamp( x, 0, Math.Max( 0, screenW - winW ) );
		y = Math.Clamp( y, 0, Math.Max( 0, screenH - m_TitleBarHeight ) );

		layoutRoot.SetPos( x, y, true );

		m_TitleWrapper.SetPos( 0, 0, true );
	}

	// ─── Private helpers ────────────────────────────────────────────────────────

	//! Centre the window on screen, with a small cascade offset per open window
	//! so that multiple windows opened in sequence are not perfectly stacked.
	private void CenterOnScreen()
	{
		float screenW, screenH;
		g_Game.GetWorkspace().GetScreenSize( screenW, screenH );

		float winW, winH;
		layoutRoot.GetSize( winW, winH );

		//! Cascade: shift each extra open window by 20px so they are all visible
		int openCount = GetCOTWindowManager().Count() - 1;
		float cascade = Math.Clamp( openCount * 20, 0, 200 );

		SetPosition( ( screenW - winW ) * 0.5 + cascade, ( screenH - winH ) * 0.5 + cascade );
	}

	private void SetResizeHandlesVisible( bool show )
	{
		if ( m_ResizeDragUp )       m_ResizeDragUp.Show( show );
		if ( m_ResizeDragDown )     m_ResizeDragDown.Show( show );
		if ( m_ResizeDragLeft )     m_ResizeDragLeft.Show( show );
		if ( m_ResizeDragRight )    m_ResizeDragRight.Show( show );
		if ( m_ResizeDragTopLeft )  m_ResizeDragTopLeft.Show( show );
		if ( m_ResizeDragTopRight ) m_ResizeDragTopRight.Show( show );
		if ( m_ResizeDragBotLeft )  m_ResizeDragBotLeft.Show( show );
		if ( m_ResizeDragBotRight ) m_ResizeDragBotRight.Show( show );

		// Always hide highlights when hiding handles
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

	private bool IsResizeHandle( Widget w )
	{
		return w == m_ResizeDragUp      || w == m_ResizeDragDown    || w == m_ResizeDragLeft    || w == m_ResizeDragRight    || w == m_ResizeDragTopLeft || w == m_ResizeDragTopRight || w == m_ResizeDragBotLeft || w == m_ResizeDragBotRight;
	}

	private EResizeDirection GetResizeDirection( Widget w )
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

	//! Returns the highlight widget paired with the given drag handle, or NULL.
	private Widget GetHandleHighlight( Widget w )
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

	private void Resize( float x, float y )
	{
		float deltaX = x - m_StartResizePositionX;
		float deltaY = y - m_StartResizePositionY;

		float minHeight = m_TitleBarHeight + RESIZE_MIN_CONTENT_HEIGHT;

		float newWidth  = m_StartResizeSizeW;
		float newHeight = m_StartResizeSizeH;
		float newPosX   = m_StartWindowPosX;
		float newPosY   = m_StartWindowPosY;

		switch ( m_ResizeDirection )
		{
			case EResizeDirection.UP:
				newHeight = Math.Max( m_StartResizeSizeH - deltaY, minHeight );
				newPosY   = m_StartWindowPosY + ( m_StartResizeSizeH - newHeight );
				break;
			case EResizeDirection.DOWN:
				newHeight = Math.Max( m_StartResizeSizeH + deltaY, minHeight );
				break;
			case EResizeDirection.LEFT:
				newWidth = Math.Max( m_StartResizeSizeW - deltaX, RESIZE_MIN_WIDTH );
				newPosX  = m_StartWindowPosX + ( m_StartResizeSizeW - newWidth );
				break;
			case EResizeDirection.RIGHT:
				newWidth = Math.Max( m_StartResizeSizeW + deltaX, RESIZE_MIN_WIDTH );
				break;
			case EResizeDirection.TOP_LEFT:
				newWidth  = Math.Max( m_StartResizeSizeW - deltaX, RESIZE_MIN_WIDTH );
				newPosX   = m_StartWindowPosX + ( m_StartResizeSizeW - newWidth );
				newHeight = Math.Max( m_StartResizeSizeH - deltaY, minHeight );
				newPosY   = m_StartWindowPosY + ( m_StartResizeSizeH - newHeight );
				break;
			case EResizeDirection.TOP_RIGHT:
				newWidth  = Math.Max( m_StartResizeSizeW + deltaX, RESIZE_MIN_WIDTH );
				newHeight = Math.Max( m_StartResizeSizeH - deltaY, minHeight );
				newPosY   = m_StartWindowPosY + ( m_StartResizeSizeH - newHeight );
				break;
			case EResizeDirection.BOT_LEFT:
				newWidth  = Math.Max( m_StartResizeSizeW - deltaX, RESIZE_MIN_WIDTH );
				newPosX   = m_StartWindowPosX + ( m_StartResizeSizeW - newWidth );
				newHeight = Math.Max( m_StartResizeSizeH + deltaY, minHeight );
				break;
			case EResizeDirection.BOT_RIGHT:
				newWidth  = Math.Max( m_StartResizeSizeW + deltaX, RESIZE_MIN_WIDTH );
				newHeight = Math.Max( m_StartResizeSizeH + deltaY, minHeight );
				break;
		}

		layoutRoot.SetSize( newWidth, newHeight );
		layoutRoot.SetPos( newPosX, newPosY, true );

		float contentH = newHeight - m_TitleBarHeight;

		if ( m_ContentWidget )
			m_ContentWidget.SetSize( newWidth, contentH );

		if ( m_ConfirmationPanel )
			m_ConfirmationPanel.SetSize( newWidth, contentH );

		m_TitleWrapper.SetPos( 0, 0, true );

		if ( m_Form )
			m_Form.OnResize( newWidth, contentH );
	}
}
#endif
