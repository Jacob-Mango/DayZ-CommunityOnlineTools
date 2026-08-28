// =============================================================================
//  UIActionTooltip
//
//  Global hover tooltip supporting text, optional icon, and optional color swatch.
//  Text wraps automatically via RichTextWidget sized to content vertically.
//
//  Usage:
//      UIActionManager.CreateTooltip( layoutRoot );   // once per form
//
//      btn.SetTooltip( "Delete vehicle" );
//      btn.SetTooltip( "Warning", JMConstants.ICON_ALERT, 0xFFFF4444 );  // icon
//      btn.SetTooltip( "Health",  "",                    0xFF44FF44 );   // swatch
// =============================================================================

class UIActionTooltip: COT_ScriptedWidgetEventHandler
{
	// Strong ref so lazy-init callers (SetTooltip, JMFormBase hook) don't have
	// to hang onto the returned handle themselves. Without this the instance
	// gets GC'd the moment CreateTooltip() returns and tooltips die everywhere.
	static ref UIActionTooltip s_InstanceRef;
	static UIActionTooltip s_Instance;

	protected Widget          m_Root;
	protected RichTextWidget  m_Text;
	protected Widget          m_TextContainer;
	protected ImageWidget     m_Icon;
	protected Widget          m_Swatch;

	protected bool m_Visible;
	protected int  m_TooltipW;

	// Pending show request - tooltip appears after HOVER_DELAY_S of continuous hover.
	protected bool   m_Pending;
	protected float  m_PendingTimer;
	protected string m_PendingText;
	protected string m_PendingIcon;
	protected int    m_PendingSwatch;
	protected int    m_PendingTextColor;
	// The widget that triggered the current (or pending) tooltip - used to auto-hide
	// when the source widget's form is hidden.
	protected Widget m_SourceWidget;

	static const float HOVER_DELAY_S = 1.0;

	void UIActionTooltip( notnull Widget anchor )
	{
		m_Root = g_Game.GetWorkspace().CreateWidgets(
			"JM/COT/GUI/layouts/uiactions/UIActionTooltip.layout", anchor );

		if ( m_Root )
		{
			Class.CastTo( m_Text,          m_Root.FindAnyWidget( "tooltip_text"           ) );
			Class.CastTo( m_TextContainer, m_Root.FindAnyWidget( "tooltip_text_container" ) );
			Class.CastTo( m_Icon,          m_Root.FindAnyWidget( "tooltip_icon"           ) );
			Class.CastTo( m_Swatch,        m_Root.FindAnyWidget( "tooltip_swatch"         ) );
			m_Root.Show( false );
		}

		s_Instance    = this;
		s_InstanceRef = this;

		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Insert( Update );
	}

	void ~UIActionTooltip()
	{
		g_Game.GetUpdateQueue( CALL_CATEGORY_GUI ).Remove( Update );

		if ( s_Instance == this )
		{
			s_Instance    = null;
			s_InstanceRef = null;
		}

		if ( m_Root )
			m_Root.Unlink();
	}

	static void Show( string text, string iconPath = "", int swatchColor = 0, int textColor = 0, Widget sourceWidget = null )
	{
		if ( !s_Instance )
			return;

		s_Instance.QueueShow( text, iconPath, swatchColor, textColor, sourceWidget );
	}

	static void Hide()
	{
		if ( !s_Instance )
			return;

		s_Instance.HideInternal();
	}

	void QueueShow( string text, string iconPath, int swatchColor, int textColor, Widget sourceWidget = null )
	{
		// Store the deadline in game-time ms. Update() fires ShowInternal once
		// the mouse has hovered continuously for HOVER_DELAY_S.
		m_Pending          = true;
		m_PendingTimer     = g_Game.GetTime() + HOVER_DELAY_S * 1000.0;
		m_PendingText      = text;
		m_PendingIcon      = iconPath;
		m_PendingSwatch    = swatchColor;
		m_PendingTextColor = textColor;
		m_SourceWidget     = sourceWidget;
	}

	void ShowInternal( string text, string iconPath, int swatchColor, int textColor = 0 )
	{
		if ( !m_Root || text == "" )
			return;

		bool hasIcon   = ( iconPath != "" );
		bool hasSwatch = ( !hasIcon && swatchColor != 0 );

		// Left offset for text container: icon/swatch = 26px, plain = 6px.
		int leftDecor;
		if ( hasIcon || hasSwatch )
			leftDecor = 26;
		else
			leftDecor = 6;

		if ( m_Icon )
		{
			if ( hasIcon )
			{
				m_Icon.LoadImageFile( 0, iconPath );
				m_Icon.SetImage( 0 );
				int iconColor = ARGB( 255, 255, 255, 255 );
				if ( swatchColor != 0 )
					iconColor = swatchColor;
				m_Icon.SetColor( iconColor );
				m_Icon.Show( true );
			}
			else
			{
				m_Icon.Show( false );
			}
		}

		if ( m_Swatch )
		{
			if ( hasSwatch )
			{
				m_Swatch.SetColor( swatchColor );
				m_Swatch.Show( true );
			}
			else
			{
				m_Swatch.Show( false );
			}
		}

		static const int PAD_R  = 14;
		static const int PAD_Y  = 6;
		static const int MARGIN = 32;
		static const int CHAR_W = 8;

		// Max tooltip width = anchor width minus margins, fallback 500.
		int maxW = 500;
		Widget anchor = m_Root.GetParent();
		if ( anchor )
		{
			float aw, ah;
			anchor.GetScreenSize( aw, ah );
			if ( aw > 100 )
				maxW = (int)aw - MARGIN * 2;
		}

		// Total width: fit single line if possible, otherwise cap at maxW.
		int singleW = ( text.Length() + 1 ) * CHAR_W + leftDecor + PAD_R;
		int w;
		if ( singleW <= maxW )
			w = singleW;
		else
			w = maxW;

		m_TooltipW = w;

		// Container fills available text width; RichTextWidget fills 100% of it and
		// self-sizes vertically via "size to text v" - no manual line counting needed.
		int containerW = w - leftDecor - PAD_R;
		if ( m_TextContainer )
		{
			m_TextContainer.SetFlags( WidgetFlags.EXACTPOS,   true );
			m_TextContainer.SetFlags( WidgetFlags.HEXACTSIZE, true );
			m_TextContainer.SetFlags( WidgetFlags.VEXACTSIZE, true );
			m_TextContainer.SetPos( leftDecor, PAD_Y );
			m_TextContainer.SetSize( containerW, 10 );
		}

		if ( m_Text )
		{
			m_Text.SetText( text );
			if ( textColor != 0 )
				m_Text.SetColor( textColor );
			else
				m_Text.SetColor( JMTheme.TEXT_SECONDARY );
		}

		// Show with a temporary height - Update() will read the real rendered height.
		m_Root.SetFlags( WidgetFlags.HEXACTSIZE, true );
		m_Root.SetFlags( WidgetFlags.VEXACTSIZE, true );
		m_Root.SetSize( w, 40 );
		m_Root.Show( true );

		// Raise above every sibling so nothing else in the form obscures it.
		Widget parent = m_Root.GetParent();
		if ( parent )
		{
			Widget sibling = parent.GetChildren();
			while ( sibling )
			{
				Widget next = sibling.GetSibling();
				if ( sibling != m_Root )
					m_Root.SetSort( sibling.GetSort() + 1, true );
				sibling = next;
			}
		}

		m_Visible = true;
	}

	void HideInternal()
	{
		m_Pending      = false;
		m_SourceWidget = null;

		if ( m_Root )
			m_Root.Show( false );

		m_Visible = false;
	}

	void Update()
	{
		// If the source widget (or any ancestor) is hidden, cancel pending show and hide tooltip.
		// This handles closing the form while a tooltip is pending or visible.
		if ( ( m_Pending || m_Visible ) && m_SourceWidget )
		{
			Widget vis = m_SourceWidget;
			while ( vis )
			{
				if ( !vis.IsVisible() )
				{
					HideInternal();
					return;
				}
				vis = vis.GetParent();
			}
		}

		// Delayed-show: fire ShowInternal once the hover deadline passes.
		if ( m_Pending && g_Game.GetTime() >= m_PendingTimer )
		{
			m_Pending = false;
			ShowInternal( m_PendingText, m_PendingIcon, m_PendingSwatch, m_PendingTextColor );
		}

		if ( !m_Visible || !m_Root )
			return;

		static const int PAD_Y = 6;

		// Read actual rendered text height from the RichTextWidget (size-to-content).
		if ( m_TextContainer && m_Text )
		{
			float tw, th;
			m_Text.GetScreenSize( tw, th );
			if ( th > 1 )
			{
				int newH = (int)th + PAD_Y * 2;
				m_Root.SetSize( m_TooltipW, newH );
				m_TextContainer.SetSize( (float)( m_TooltipW - 14 ), th );
			}
		}

		// Position in anchor-local space, flip if near edges.
		float ax, ay, aw, ah;
		Widget anchor = m_Root.GetParent();
		if ( !anchor )
			return;
		anchor.GetScreenPos( ax, ay );
		anchor.GetScreenSize( aw, ah );

		int mx, my;
		GetMousePos( mx, my );

		float cx = mx - ax;
		float cy = my - ay;

		float rw, rh;
		m_Root.GetScreenSize( rw, rh );

		float px = cx + 14;
		float py = cy + 20;

		if ( px + rw > aw - 4 )
			px = cx - rw - 6;
		if ( py + rh > ah - 4 )
			py = cy - rh - 6;

		if ( px < 0 ) px = 0;
		if ( py < 0 ) py = 0;

		m_Root.SetPos( px, py );
	}
}
