//  UIActionConfirmInline - a button that morphs into Confirm + Cancel for a timeout (default
//  4 s) and reverts if not confirmed. Fires UIEvent.CLICK on the initial press, UIEvent.CHANGE
//  on confirm, nothing on cancel/timeout: run the action only when eid == UIEvent.CHANGE.
//  Labels default to "Confirm" / "Cancel"; override with SetConfirmLabel() / SetCancelLabel().
class UIActionConfirmInline: UIActionBase
{
	protected ButtonWidget m_BtnAction;
	protected ButtonWidget m_BtnConfirm;
	protected ButtonWidget m_BtnCancel;
	protected TextWidget   m_ActionText;
	protected TextWidget   m_ConfirmText;
	protected TextWidget   m_CancelText;
	protected ImageWidget  m_Icon;

	//! The rounded fill and outline inside action_button. SetColor paints
	//! these, not the ButtonWidget: the button itself is a plain rectangle,
	//! and painting it showed a square block behind the rounded corners.
	protected Widget       m_Fill;
	protected Widget       m_Outline;
	protected int          m_FillColor;
	protected int          m_OutlineColor;
	protected bool  m_Pending;
	protected float m_Timeout;
	protected float m_Timer;
	protected float m_TimeoutSeconds;

	//! Set by CenterIcon(); re-applied every frame from ApplyIconCenter() because
	//! action_button's real pixel size is only known after the engine has laid it out
	//! at least once, not on the frame CenterIcon() is called.
	protected bool m_CenterIconRequested;
	protected int  m_CenterIconPx;

	//! Icon strip reserved on the left: 10 (position) + 16 (size) + 8 (gap).
	//! Half of it is how far the centred label moves right to clear the icon.
	static const int ICON_RIGHT_EDGE   = 26;
	static const int ICON_TEXT_GAP     = 8;
	static const int LABEL_OFFSET_ICON = ( ICON_RIGHT_EDGE + ICON_TEXT_GAP ) / 2;

	bool IsPending()
	{
		return m_Pending;
	}

	void SetCancelLabel( string text )
	{
		if ( m_CancelText )
			m_CancelText.SetText( text );
	}

	void SetConfirmLabel( string text )
	{
		if ( m_ConfirmText )
			m_ConfirmText.SetText( text );
	}

	protected void SetState( bool pending )
	{
		m_Pending = pending;

		if ( m_BtnAction  ) m_BtnAction.Show(  !pending );
		if ( m_BtnConfirm ) m_BtnConfirm.Show(  pending );
		if ( m_BtnCancel  ) m_BtnCancel.Show(   pending );
	}

	//! Seconds the confirm/cancel pair stays visible before auto-reverting.
	void SetTimeout( float seconds )
	{
		m_TimeoutSeconds = Math.Max( 0.5, seconds );
	}

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_BtnAction,  layoutRoot.FindAnyWidget( "action_button"         ) );
		Class.CastTo( m_BtnConfirm, layoutRoot.FindAnyWidget( "action_button_confirm" ) );
		Class.CastTo( m_BtnCancel,  layoutRoot.FindAnyWidget( "action_button_cancel"  ) );
		Class.CastTo( m_ActionText, layoutRoot.FindAnyWidget( "action"                ) );
		Class.CastTo( m_ConfirmText,layoutRoot.FindAnyWidget( "confirm_text"          ) );
		Class.CastTo( m_CancelText, layoutRoot.FindAnyWidget( "cancel_text"           ) );
		Class.CastTo( m_Icon,       layoutRoot.FindAnyWidget( "action_icon"           ) );

		if ( m_BtnAction )
		{
			m_Fill    = m_BtnAction.FindAnyWidget( "fill" );
			m_Outline = m_BtnAction.FindAnyWidget( "outline" );
		}

		if ( m_Fill )
			m_FillColor = m_Fill.GetColor();

		if ( m_Outline )
			m_OutlineColor = m_Outline.GetColor();

		m_Pending        = false;
		m_TimeoutSeconds = 4.0;
		SetState( false );
	}

	override void SetButton( string text )
	{
		if ( m_ActionText )
			m_ActionText.SetText( text );
	}

	// For icon-only / fixed-size buttons whose confirm+cancel pair would otherwise
	// be too narrow to fit "Confirm"/"Cancel" text legibly. Uses ASCII glyphs
	// that the SDF font is guaranteed to have. Multi-byte Unicode in script
	// source caused a CTD on COT open.
	void UseIconConfirmLabels()
	{
		SetConfirmLabel( "OK" );
		SetCancelLabel( "X" );
	}

	override void SetIcon( string imagePath )
	{
		if ( !m_Icon )
			return;

		if ( imagePath == "" )
		{
			m_Icon.Show( false );
			if ( m_ActionText )
				m_ActionText.SetTextOffset( 0, 0 );
			return;
		}

		m_Icon.LoadImageFile( 0, imagePath );
		m_Icon.Show( true );

		// The label is centred across the WHOLE button while the icon sits at a
		// fixed x=10, so a centred label grows leftwards straight into the icon.
		// Shifting the glyphs right by half the reserved strip re-centres them in
		// the space right of the icon instead - same math as UIActionTabs.
		if ( m_ActionText )
			m_ActionText.SetTextOffset( LABEL_OFFSET_ICON, 0 );
	}

	// Center the icon in the button; use after SetFixedSize when the action text is empty
	// (icon-only mode). iconSize is the icon's square; the button size is read back live
	// every frame (see ApplyIconCenter) since a just-called SetFixedSize() is not yet laid out.
	void CenterIcon( int buttonSizeUnused = 32, int iconSize = 16 )
	{
		if ( !m_Icon )
			return;

		m_Icon.SetSize( iconSize, iconSize );

		//! action_icon is halign left_ref in the layout: clearing the exact-position flags would
		//! turn its position into a FRACTION of the button, not center it. left_ref plus an
		//! exact pixel offset centers it without needing a center_ref anchor.
		m_Icon.SetFlags( WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS, true );

		m_CenterIconPx = iconSize;
		m_CenterIconRequested = true;
		ApplyIconCenter();
	}

	//! Re-centers the icon against action_button's current pixel size. Called from CenterIcon()
	//! and every frame from Update() until it can measure: GetScreenSize() reads 0 on the frame
	//! a widget is first sized (same reason UIActionCard seeds tall).
	protected void ApplyIconCenter()
	{
		if ( !m_CenterIconRequested || !m_Icon || !m_BtnAction )
			return;

		float bw, bh;
		m_BtnAction.GetScreenSize( bw, bh );

		if ( bw < m_CenterIconPx || bh < m_CenterIconPx )
			return;

		//! Only X needs an offset. action_icon is declared valign center_ref,
		//! so Y is already measured from the button's vertical centre - adding
		//! half the slack on top of that pushed the icon down by that much,
		//! which is why trash icons sat on the bottom edge of their buttons.
		float offsetX = ( bw - m_CenterIconPx ) / 2.0;
		m_Icon.SetPos( offsetX, 0 );
	}

	override void SetColor( int color )
	{
		m_FillColor = color;

		if ( m_Fill )
			m_Fill.SetColor( color );
	}

	//! Hover feedback on the rounded shapes - the button widget has an empty
	//! style, so it draws no hover of its own.
	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_BtnAction )
		{
			if ( m_Fill )
				m_Fill.SetColor( JMTheme.Lighten( m_FillColor, 0.18 ) );

			if ( m_Outline )
				m_Outline.SetColor( JMTheme.Lighten( m_OutlineColor, 0.35 ) );
		}

		return super.OnMouseEnter( w, x, y );
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_BtnAction )
		{
			if ( m_Fill )
				m_Fill.SetColor( m_FillColor );

			if ( m_Outline )
				m_Outline.SetColor( m_OutlineColor );
		}

		return super.OnMouseLeave( w, enterW, x, y );
	}

	//! Programmatic equivalent of the first click: arms the confirm/cancel pair
	//! and (re)starts the timeout. Used by keybinds that need the same
	//! "press once to arm, press again to confirm" flow as a mouse click,
	//! instead of the callback firing straight away.
	void Arm()
	{
		SetState( true );
		m_Timer = 0;
		CallEvent( UIEvent.CLICK );
	}

	//! Programmatic equivalent of clicking Confirm while armed.
	void ConfirmPending()
	{
		if ( !m_Pending )
			return;

		Revert();
		CallEvent( UIEvent.CHANGE );
	}

	//! Programmatic equivalent of clicking Cancel while armed.
	void CancelPending()
	{
		if ( !m_Pending )
			return;

		Revert();
	}

	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( w == m_BtnAction && !m_Pending )
		{
			SetState( true );
			m_Timer = 0;
			CallEvent( UIEvent.CLICK );
			return true;
		}

		if ( w == m_BtnConfirm && m_Pending )
		{
			Revert();
			CallEvent( UIEvent.CHANGE );
			return true;
		}

		if ( w == m_BtnCancel && m_Pending )
		{
			Revert();
			return true;
		}

		return false;
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( m_CenterIconRequested )
			ApplyIconCenter();

		if ( !m_Pending )
			return;

		m_Timer += timeSlice;

		if ( m_Timer >= m_TimeoutSeconds )
			Revert();
	}

	protected void Revert()
	{
		SetState( false );
	}
}
