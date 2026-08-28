// =============================================================================
//  UIActionConfirmInline
//
//  A button that, when clicked, morphs into Confirm + Cancel split buttons
//  for a configurable timeout (default 4 s).  If the user does not confirm,
//  it reverts automatically.  Fires UIEvent.CLICK on the initial press,
//  UIEvent.CHANGE on confirmed, and nothing on cancel/timeout.
//
//  The confirm/cancel labels default to "Confirm" / "Cancel" but can be
//  overridden with SetConfirmLabel() / SetCancelLabel().
//
//  Usage:
//      m_DeleteBtn = UIActionManager.CreateConfirmInline( parent, "Delete Vehicle",
//          this, "OnConfirm_Delete" );
//
//      void OnConfirm_Delete( UIEvent eid, UIActionBase action )
//      {
//          if ( eid == UIEvent.CHANGE )
//              Exec_DeleteVehicle();
//      }
// =============================================================================
class UIActionConfirmInline: UIActionBase
{
	protected ButtonWidget m_BtnAction;
	protected ButtonWidget m_BtnConfirm;
	protected ButtonWidget m_BtnCancel;
	protected TextWidget   m_ActionText;
	protected TextWidget   m_ConfirmText;
	protected TextWidget   m_CancelText;
	protected ImageWidget  m_Icon;

	protected bool  m_Pending;
	protected float m_Timeout;
	protected float m_Timer;
	protected float m_TimeoutSeconds;

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

		m_Pending        = false;
		m_TimeoutSeconds = 4.0;
		SetState( false );
	}

	override void SetButton( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_ActionText )
			m_ActionText.SetText( text );
	}

	void SetConfirmLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_ConfirmText )
			m_ConfirmText.SetText( text );
	}

	void SetCancelLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_CancelText )
			m_CancelText.SetText( text );
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

		if ( m_ActionText )
			m_ActionText.SetTextOffset( 30, 0 );
	}

	// Position the icon at the geometric center of the button. Use after
	// SetFixedSize when the action text is empty (icon-only mode); the
	// default layout positions the icon at the left to flank the text.
	// buttonSize = the SetFixedSize value (square buttons).
	void CenterIcon( int buttonSize = 32, int iconSize = 16 )
	{
		if ( !m_Icon )
			return;
		m_Icon.SetSize( iconSize, iconSize );
		// Clear exact-position flags so halign/valign center_ref takes over.
		m_Icon.ClearFlags( WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS );
		m_Icon.SetPos( 0, 0 );
	}

	override void SetColor( int color )
	{
		if ( m_BtnAction )
			m_BtnAction.SetColor( color );
	}

	//! Seconds the confirm/cancel pair stays visible before auto-reverting.
	void SetTimeout( float seconds )
	{
		m_TimeoutSeconds = Math.Max( 0.5, seconds );
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

		if ( !m_Pending )
			return;

		m_Timer += timeSlice;

		if ( m_Timer >= m_TimeoutSeconds )
			Revert();
	}

	private void SetState( bool pending )
	{
		m_Pending = pending;

		if ( m_BtnAction  ) m_BtnAction.Show(  !pending );
		if ( m_BtnConfirm ) m_BtnConfirm.Show(  pending );
		if ( m_BtnCancel  ) m_BtnCancel.Show(   pending );
	}

	private void Revert()
	{
		SetState( false );
	}
}
