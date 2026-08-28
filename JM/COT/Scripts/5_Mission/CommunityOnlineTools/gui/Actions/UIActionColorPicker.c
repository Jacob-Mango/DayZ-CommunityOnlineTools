// =============================================================================
//  UIActionColorPicker
//
//  Label + hex input field + live color preview swatch.
//  Accepts hex strings in the forms: "RRGGBB" or "AARRGGBB" (case-insensitive,
//  with or without leading '#').  Fires UIEvent.CHANGE when a valid color
//  is entered.  GetColor() returns the current ARGB int.
//
//  Clicking the swatch opens an ARGB popup with one slider per channel. The
//  popup is created lazily on first use and parented to the top of the form so
//  it floats over the rest of the UI instead of being clipped by the row.
//
//  Usage:
//      m_Color = UIActionManager.CreateColorPicker( parent, "Highlight Color",
//          this, "OnChange_Color" );
//      m_Color.SetColor( ARGB(255, 255, 128, 0) );
//
//      void OnChange_Color( UIEvent eid, UIActionBase action )
//      {
//          int col = m_Color.GetColor();
//      }
// =============================================================================
class UIActionColorPicker: UIActionBase
{
	protected TextWidget    m_Label;
	protected EditBoxWidget m_HexBox;
	protected Widget        m_Preview;
	protected ButtonWidget  m_PreviewButton;

	protected int    m_Color;
	protected string m_PreviousText;

	// -- ARGB popup ------------------------------------------------------------
	protected Widget          m_Anchor;
	protected Widget          m_Popup;
	protected Widget          m_PopupSwatch;
	protected UIActionSlider  m_SliderA;
	protected UIActionSlider  m_SliderR;
	protected UIActionSlider  m_SliderG;
	protected UIActionSlider  m_SliderB;
	protected bool            m_Open;
	//! Swallows the click that opened the popup so it does not immediately
	//! count as an outside click and close it again.
	protected float           m_OpenDelay;

	static const string POPUP_LAYOUT = "JM/COT/GUI/layouts/uiactions/UIActionColorPicker_Popup.layout";

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,   layoutRoot.FindAnyWidget( "action_label"   ) );
		Class.CastTo( m_HexBox,  layoutRoot.FindAnyWidget( "action"         ) );
		Class.CastTo( m_Preview, layoutRoot.FindAnyWidget( "action_preview" ) );
		Class.CastTo( m_PreviewButton, layoutRoot.FindAnyWidget( "action_preview_button" ) );

		m_Color = ARGB( 255, 255, 255, 255 );
		m_PreviousText = "";
		m_Open = false;
		m_OpenDelay = 0;
		UpdatePreview();
	}

	void ~UIActionColorPicker()
	{
		if ( m_Open )
			CommunityOnlineToolsBase.ForceDisableInputs( false );

		if ( m_Popup )
		{
			JMStatics.UnregisterOverlay( m_Popup );
			m_Popup.Unlink();
		}
	}

	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_Label )
			m_Label.SetText( text );
	}

	//! Set color from ARGB int - updates both preview and hex field.
	override void SetColor( int color )
	{
		m_Color = color;
		UpdatePreview();
		UpdateHexBox();
		SyncSlidersFromColor();
	}

	//! Returns the current ARGB color value.
	int GetColor()
	{
		return m_Color;
	}

	override bool IsFocusWidget( Widget widget )
	{
		if ( widget == m_HexBox )        return true;
		if ( widget == m_PreviewButton ) return true;

		return IsInsidePopup( widget );
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_PreviewButton )
		{
			SetOpen( !m_Open );
			return true;
		}

		if ( w == m_HexBox )
		{
			SetFocus( m_HexBox );
			return false;
		}
		return false;
	}

	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		if ( w != m_HexBox || !finished )
			return false;

		string text = m_HexBox.GetText();
		if ( text == m_PreviousText )
			return false;

		int parsed;
		if ( TryParseHex( text, parsed ) )
		{
			m_Color        = parsed;
			m_PreviousText = text;
			UpdatePreview();
			SyncSlidersFromColor();
			CallEvent( UIEvent.CHANGE );
		}

		return true;
	}

	private bool TryParseHex( string input, out int color )
	{
		string s = input;

		// Strip leading '#'
		if ( s.Length() > 0 && s.Get(0) == "#" )
			s = s.Substring( 1, s.Length() - 1 );

		// Must be 6 (RRGGBB) or 8 (AARRGGBB) hex chars
		if ( s.Length() != 6 && s.Length() != 8 )
			return false;

		s.ToUpper();

		int value = 0;
		int nibble;
		string ch;
		for ( int i = 0; i < s.Length(); i++ )
		{
			ch = s.Get( i );

			if ( ch >= "0" && ch <= "9" )
				nibble = ch.ToInt();
			else if ( ch >= "A" && ch <= "F" )
				nibble = 10 + ( ch.ToInt() - "A".ToInt() );
			else
				return false;

			value = ( value << 4 ) | nibble;
		}

		// Promote RRGGBB -> AARRGGBB with full opacity
		if ( s.Length() == 6 )
			color = 0xFF000000 | value;
		else
			color = value;

		return true;
	}

	private void UpdatePreview()
	{
		if ( m_Preview )
			m_Preview.SetColor( m_Color );
	}

	private void UpdateHexBox()
	{
		if ( !m_HexBox )
			return;

		int a = ( m_Color >> 24 ) & 0xFF;
		int r = ( m_Color >> 16 ) & 0xFF;
		int g = ( m_Color >>  8 ) & 0xFF;
		int b =   m_Color         & 0xFF;

		string hex;
		if ( a == 255 )
			hex = string.Format( "%1%2%3", ToHex2(r), ToHex2(g), ToHex2(b) );
		else
			hex = string.Format( "%1%2%3%4", ToHex2(a), ToHex2(r), ToHex2(g), ToHex2(b) );

		m_HexBox.SetText( hex );
		m_PreviousText = hex;
	}

	private string ToHex2( int v )
	{
		string chars = "0123456789ABCDEF";
		return "" + chars.Get( ( v >> 4 ) & 0xF ) + chars.Get( v & 0xF );
	}

	// =========================================================================
	//  ARGB popup
	// =========================================================================

	//! Channel slider colours, so a glance at the popup says which row is which.
	static const int COLOR_CHANNEL_A = 0xFFAAB0BB;
	static const int COLOR_CHANNEL_R = 0xFFE05555;
	static const int COLOR_CHANNEL_G = 0xFF55C46E;
	static const int COLOR_CHANNEL_B = 0xFF5588FF;

	private void EnsurePopup()
	{
		if ( m_Popup )
			return;

		if ( !layoutRoot )
			return;

		// Float the popup in the same full-screen container the windows live in,
		// so no window can clip it and it always draws over them. Fall back to
		// the largest ancestor that still has a parent (one below the workspace
		// root) if the container does not exist.
		m_Anchor = JMStatics.WINDOWS_CONTAINER;

		if ( !m_Anchor )
		{
			m_Anchor = layoutRoot;
			Widget parent = m_Anchor.GetParent();
			while ( parent && parent.GetParent() )
			{
				m_Anchor = parent;
				parent = m_Anchor.GetParent();
			}
		}

		m_Popup = g_Game.GetWorkspace().CreateWidgets( POPUP_LAYOUT, m_Anchor );
		if ( !m_Popup )
			return;

		m_Popup.Show( false );
		m_Popup.SetHandler( this );

		// Sitting outside the window means COTModule's "was this click on COT
		// UI?" ancestor walk cannot find a window above us; without this the
		// click is treated as a world click and the game takes the mouse back,
		// which turns the camera while a slider is being dragged.
		JMStatics.RegisterOverlay( m_Popup );

		m_PopupSwatch = m_Popup.FindAnyWidget( "popup_swatch" );

		Widget content = m_Popup.FindAnyWidget( "popup_content" );
		if ( !content )
			return;

		m_SliderA = CreateChannelSlider( content, "A", COLOR_CHANNEL_A );
		m_SliderR = CreateChannelSlider( content, "R", COLOR_CHANNEL_R );
		m_SliderG = CreateChannelSlider( content, "G", COLOR_CHANNEL_G );
		m_SliderB = CreateChannelSlider( content, "B", COLOR_CHANNEL_B );

		SyncSlidersFromColor();
		UpdatePopupSwatch();
	}

	private UIActionSlider CreateChannelSlider( notnull Widget content, string label, int color )
	{
		UIActionSlider slider = UIActionManager.CreateSlider( content, label, 0, 255, this, "OnChange_Channel" );
		if ( !slider )
			return null;

		slider.SetStepValue( 1 );
		slider.SetFormat( "%1" );
		slider.SetColor( color );

		return slider;
	}

	bool IsOpen()
	{
		return m_Open;
	}

	override void OnHide()
	{
		if ( m_Open )
			SetOpen( false );
	}

	private void SetOpen( bool open )
	{
		if ( open )
			EnsurePopup();

		if ( !m_Popup )
			return;

		m_Open = open;

		if ( open )
		{
			SyncSlidersFromColor();
			UpdatePopupSwatch();
			m_Popup.Show( true );
			m_Popup.SetSort( 9999, true );
			m_OpenDelay = 0.15;

			// The popup is a modal surface: the sliders never take keyboard
			// focus, so UIActionBase's focus-driven input guard never trips and
			// clicks/drags inside the popup reach the world (the camera turns).
			// Hold the inputs down for as long as the popup is open, the same
			// way JMConfirmation does.
			CommunityOnlineToolsBase.ForceDisableInputs( true );
		}
		else
		{
			m_Popup.Show( false );
			CommunityOnlineToolsBase.ForceDisableInputs( false );
		}
	}

	//! True for the popup itself and anything inside it, so dragging a slider
	//! is not mistaken for a click outside the popup.
	private bool IsInsidePopup( Widget widget )
	{
		if ( !m_Popup || !widget )
			return false;

		Widget cur = widget;
		while ( cur )
		{
			if ( cur == m_Popup )
				return true;
			cur = cur.GetParent();
		}

		return false;
	}

	void OnChange_Channel( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		if ( !m_SliderA || !m_SliderR || !m_SliderG || !m_SliderB )
			return;

		int a = Math.Round( m_SliderA.GetCurrent() );
		int r = Math.Round( m_SliderR.GetCurrent() );
		int g = Math.Round( m_SliderG.GetCurrent() );
		int b = Math.Round( m_SliderB.GetCurrent() );

		m_Color = ARGB( a, r, g, b );

		UpdatePreview();
		UpdateHexBox();
		UpdatePopupSwatch();

		CallEvent( UIEvent.CHANGE );
	}

	private void SyncSlidersFromColor()
	{
		if ( !m_SliderA || !m_SliderR || !m_SliderG || !m_SliderB )
			return;

		// The shifts have to land in int locals first: SetCurrent takes a float,
		// and Enforce resolves '>>' against the expected type, so writing the
		// shift directly as the argument fails to compile with
		// "Unknown operator '>>'".
		int a = ( m_Color >> 24 ) & 0xFF;
		int r = ( m_Color >> 16 ) & 0xFF;
		int g = ( m_Color >>  8 ) & 0xFF;
		int b =   m_Color         & 0xFF;

		// SetCurrent only repaints, it does not fire CHANGE, so this cannot
		// loop back into OnChange_Channel.
		m_SliderA.SetCurrent( a );
		m_SliderR.SetCurrent( r );
		m_SliderG.SetCurrent( g );
		m_SliderB.SetCurrent( b );
	}

	private void UpdatePopupSwatch()
	{
		if ( m_PopupSwatch )
			m_PopupSwatch.SetColor( m_Color );
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( !m_Open || !m_Popup || !m_Anchor || !m_PreviewButton )
			return;

		float ax, ay, anchorW, anchorH;
		m_Anchor.GetScreenPos( ax, ay );
		m_Anchor.GetScreenSize( anchorW, anchorH );

		float sx, sy, sw, sh;
		m_PreviewButton.GetScreenPos( sx, sy );
		m_PreviewButton.GetScreenSize( sw, sh );

		float pw, ph;
		m_Popup.GetScreenSize( pw, ph );

		// Right-align the popup with the swatch, and flip it above the row when
		// there is not enough room below.
		float posX = sx + sw - pw - ax;
		posX = Math.Clamp( posX, 0, Math.Max( 0, anchorW - pw ) );

		float posY = sy - ay + sh + 2;
		if ( sy + sh + ph > ay + anchorH )
			posY = sy - ay - ph - 2;

		m_Popup.SetPos( posX, posY );

		if ( m_OpenDelay > 0 )
		{
			m_OpenDelay -= timeSlice;
			return;
		}

		if ( ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) != 0 )
		{
			Widget under = GetWidgetUnderCursor();
			if ( !IsFocusWidget( under ) )
				SetOpen( false );
		}
	}
}
