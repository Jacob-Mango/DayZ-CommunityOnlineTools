// =============================================================================
//  UIActionFeedbackButton
//
//  A standard pill button that answers its own click: the resting label and
//  icon fade out, a feedback pair - "Copied!" plus a check mark - fades in,
//  holds for a moment, then fades back. The widget tree never changes, only
//  the text, the image and their alpha, so the button keeps its width and the
//  row it lives in never jumps.
//
//  Meant for actions whose result is otherwise invisible: copy a Steam ID,
//  copy coordinates, save a preset.
//
//  Usage:
//      m_CopyBtn = UIActionManager.CreateFeedbackButton( parent,
//          "Copy Steam ID", "Copied!", JMConstants.ICON_CHECK_MARK,
//          this, "OnClick_CopySteamID" );
//
//      void OnClick_CopySteamID( UIEvent eid, UIActionBase action )
//      {
//          if ( eid != UIEvent.CLICK )
//              return;
//          g_Game.CopyToClipboard( m_SteamID );
//      }
//
//  The swap runs on click by default. When the action can fail, turn that off
//  with SetAutoFeedback( false ) and call ShowFeedback() yourself once it
//  actually worked.
// =============================================================================
class UIActionFeedbackButton: UIActionButton
{
	override void AnimateFeedback()
	{
		ShowFeedback();
	}

	//! Idle - the resting label is showing and nothing is animating.
	static const int PHASE_IDLE        = 0;
	//! Resting face fading out, feedback face about to be swapped in.
	static const int PHASE_OUT_TO_FEED = 1;
	//! Feedback face fading in.
	static const int PHASE_IN_FEED     = 2;
	//! Feedback face fully visible, waiting out m_HoldSeconds.
	static const int PHASE_HOLD        = 3;
	//! Feedback face fading out, resting face about to be swapped back.
	static const int PHASE_OUT_TO_REST = 4;
	//! Resting face fading in - the last phase before IDLE.
	static const int PHASE_IN_REST     = 5;

	//! One fade leg. Short on purpose: this is a confirmation, not a transition.
	static const float FADE_SECONDS = 0.10;
	static const float DEFAULT_HOLD = 1.40;

	protected string m_RestText;
	protected string m_RestIcon;

	protected string m_FeedbackText;
	protected string m_FeedbackIcon;

	//! Optional retint of the pill while the feedback face is up.
	protected bool m_HasFeedbackColor;
	protected int  m_FeedbackColor;
	protected int  m_SavedFillColor;

	protected bool  m_AutoFeedback;
	protected float m_HoldSeconds;

	protected int   m_Phase;
	protected float m_PhaseTimer;

	override void OnInit()
	{
		super.OnInit();

		m_RestText         = "";
		m_RestIcon         = "";
		m_FeedbackText     = "";
		m_FeedbackIcon     = JMConstants.ICON_CHECK_MARK;
		m_HasFeedbackColor = false;
		m_FeedbackColor    = JMTheme.SUCCESS_DIM;
		m_SavedFillColor   = m_FillColor;
		m_AutoFeedback     = true;
		m_HoldSeconds      = DEFAULT_HOLD;
		m_Phase            = PHASE_IDLE;
		m_PhaseTimer       = 0;
	}

	// -- Resting face -----------------------------------------------------------

	//! Deliberately does not call super mid-swap: the base would paint the
	//! resting label straight over the feedback face. Forms relabel these
	//! buttons on every refresh, and a refresh must not eat the "Copied!".
	override void SetButton( string text )
	{
		string previous   = m_RestText;
		string translated = Widget.TranslateString( text );

		// The name is where GetButton() reads from, so it always carries the
		// resting value - callers copy it to the clipboard mid-swap.
		if ( m_Button )
			m_Button.SetName( translated );

		m_RestText = translated;

		if ( m_Phase == PHASE_IDLE )
		{
			if ( m_Text )
				m_Text.SetText( translated );

			return;
		}

		// A genuinely new label replaces the face the swap would fade back to,
		// so cut to it. Being relabelled with the same text changes nothing.
		if ( m_RestText != previous )
			ResetFace();
	}

	override void SetIcon( string imagePath )
	{
		string previousIcon = m_RestIcon;
		m_RestIcon = imagePath;

		if ( m_Phase == PHASE_IDLE )
		{
			super.SetIcon( imagePath );
			return;
		}

		if ( imagePath != previousIcon )
			ResetFace();
	}

	// -- Feedback face ----------------------------------------------------------

	//! The label shown after the click. Pass "" to keep the resting label.
	//! imagePath "" leaves the feedback icon alone (a check mark by default);
	//! use SetFeedbackIcon( "" ) for a feedback face with no icon at all.
	void SetFeedback( string text, string imagePath = "" )
	{
		m_FeedbackText = Widget.TranslateString( text );

		if ( imagePath != "" )
			m_FeedbackIcon = imagePath;
	}

	//! Feedback icon, verbatim - "" means the feedback face shows text only.
	void SetFeedbackIcon( string imagePath )
	{
		m_FeedbackIcon = imagePath;
	}

	//! Retint the pill while the feedback face is up. The resting colour comes
	//! back when it fades out again.
	void SetFeedbackColor( int color )
	{
		m_HasFeedbackColor = true;
		m_FeedbackColor    = color;
	}

	void ClearFeedbackColor()
	{
		m_HasFeedbackColor = false;
	}

	//! Seconds the feedback face stays up between its two fades.
	void SetFeedbackDuration( float seconds )
	{
		m_HoldSeconds = Math.Max( 0.1, seconds );
	}

	//! Off means the click alone no longer swaps the face - call ShowFeedback()
	//! from the callback once the action has actually succeeded.
	void SetAutoFeedback( bool enabled )
	{
		m_AutoFeedback = enabled;
	}

	bool IsShowingFeedback()
	{
		return m_Phase != PHASE_IDLE;
	}

	//! Start the swap. Called again while the feedback face is already up it
	//! just restarts the hold, so rapid clicks read as one continuous "Copied!".
	void ShowFeedback()
	{
		if ( m_Phase == PHASE_IN_FEED || m_Phase == PHASE_HOLD )
		{
			m_Phase      = PHASE_HOLD;
			m_PhaseTimer = 0;
			SetFaceAlpha( 1.0 );
			return;
		}

		m_Phase      = PHASE_OUT_TO_FEED;
		m_PhaseTimer = 0;
	}

	//! Cut back to the resting face immediately, no fade.
	void ResetFace()
	{
		if ( m_Phase != PHASE_IDLE && m_HasFeedbackColor )
			SetColor( m_SavedFillColor );

		m_Phase      = PHASE_IDLE;
		m_PhaseTimer = 0;
		ApplyFace( m_RestText, m_RestIcon );
		SetFaceAlpha( 1.0 );
	}

	// -- Interaction ------------------------------------------------------------

	override bool OnClick( Widget w, int x, int y, int button )
	{
		bool ret = false;

		if ( w == m_Button )
		{
			ret = CallEvent( UIEvent.CLICK );

			if ( m_AutoFeedback )
				ShowFeedback();
		}

		return ret;
	}

	override void OnHide()
	{
		super.OnHide();

		// A form closed mid-swap would come back still reading "Copied!".
		if ( m_Phase != PHASE_IDLE )
			ResetFace();
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( m_Phase == PHASE_IDLE )
			return;

		m_PhaseTimer += timeSlice;

		// One shared progress value: Enforce has no block scope, so every branch
		// below writes this same local instead of declaring its own.
		float t = m_PhaseTimer / FADE_SECONDS;
		if ( t > 1.0 )
			t = 1.0;

		if ( m_Phase == PHASE_OUT_TO_FEED )
		{
			SetFaceAlpha( 1.0 - t );

			if ( t >= 1.0 )
			{
				EnterFeedbackFace();
				m_Phase      = PHASE_IN_FEED;
				m_PhaseTimer = 0;
			}

			return;
		}

		if ( m_Phase == PHASE_IN_FEED )
		{
			SetFaceAlpha( t );

			if ( t >= 1.0 )
			{
				m_Phase      = PHASE_HOLD;
				m_PhaseTimer = 0;
			}

			return;
		}

		if ( m_Phase == PHASE_HOLD )
		{
			if ( m_PhaseTimer >= m_HoldSeconds )
			{
				m_Phase      = PHASE_OUT_TO_REST;
				m_PhaseTimer = 0;
			}

			return;
		}

		if ( m_Phase == PHASE_OUT_TO_REST )
		{
			SetFaceAlpha( 1.0 - t );

			if ( t >= 1.0 )
			{
				LeaveFeedbackFace();
				m_Phase      = PHASE_IN_REST;
				m_PhaseTimer = 0;
			}

			return;
		}

		if ( m_Phase == PHASE_IN_REST )
		{
			SetFaceAlpha( t );

			if ( t >= 1.0 )
			{
				m_Phase      = PHASE_IDLE;
				m_PhaseTimer = 0;
			}
		}
	}

	// -- Internals --------------------------------------------------------------

	protected void EnterFeedbackFace()
	{
		string shown = m_FeedbackText;
		if ( shown == "" )
			shown = m_RestText;

		ApplyFace( shown, m_FeedbackIcon );

		if ( m_HasFeedbackColor )
		{
			m_SavedFillColor = m_FillColor;
			SetColor( m_FeedbackColor );
		}
	}

	protected void LeaveFeedbackFace()
	{
		ApplyFace( m_RestText, m_RestIcon );

		if ( m_HasFeedbackColor )
			SetColor( m_SavedFillColor );
	}

	//! Swap what the pill shows without touching the stored resting face. The
	//! text offsets match UIActionButton.SetIcon so both faces sit identically.
	protected void ApplyFace( string text, string imagePath )
	{
		if ( m_Text )
			m_Text.SetText( text );

		if ( !m_Icon )
			return;

		if ( imagePath == "" )
		{
			m_Icon.Show( false );

			if ( m_Text )
				m_Text.SetTextOffset( 0, 0 );

			return;
		}

		m_Icon.LoadImageFile( 0, imagePath );
		m_Icon.SetImage( 0 );
		m_Icon.Show( true );

		if ( m_Text )
			m_Text.SetTextOffset( 16, 0 );
	}

	//! Only the label and the icon fade; the pill itself stays put so the
	//! button never looks like it is dropping out of the form.
	protected void SetFaceAlpha( float alpha )
	{
		if ( alpha < 0 )
			alpha = 0;

		if ( m_Text )
			m_Text.SetAlpha( alpha );

		if ( m_Icon )
			m_Icon.SetAlpha( alpha );
	}
}
