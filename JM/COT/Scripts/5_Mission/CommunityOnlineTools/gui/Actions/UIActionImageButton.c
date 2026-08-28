class UIActionImageButton: UIActionButton
{
	//! Icon-only confirmation, the twin of UIActionFeedbackButton's "Copied!".
	//  A 32px pill has no room for a label, so the glyph itself is the receipt:
	//  the resting icon is swapped for a check mark, the pill is retinted, and
	//  both come back once the hold is over. No fade - there is nothing here to
	//  read, and a cut is what makes a fast second click still register.
	static const float FEEDBACK_HOLD = 1.0;

	protected ImageWidget m_Image;

	protected string m_RestImage;
	protected string m_FeedbackImage;
	protected int    m_FeedbackColor;
	protected int    m_SavedFillColor;
	protected float  m_FeedbackHold;
	protected bool   m_FeedbackActive;
	protected float  m_FeedbackTimer;

	override void OnInit()
	{
		// super wires up the pill (fill / outline) and its hover + press
		// colours; this layout just swaps the label for a single image.
		super.OnInit();

		Class.CastTo( m_Image, layoutRoot.FindAnyWidget( "action_image" ) );

		m_RestImage      = "";
		m_FeedbackImage  = JMConstants.ICON_CHECK_MARK;
		m_FeedbackColor  = JMTheme.SUCCESS_DIM;
		m_SavedFillColor = m_FillColor;
		m_FeedbackHold   = FEEDBACK_HOLD;
		m_FeedbackActive = false;
		m_FeedbackTimer  = 0;
	}

	void SetImage( string image )
	{
		m_RestImage = image;

		// Mid-feedback the check mark owns the widget; the new resting icon
		// lands when the swap ends, so a refresh cannot eat the confirmation.
		if ( m_FeedbackActive )
			return;

		if ( m_Image )
			m_Image.LoadImageFile( 0, image );
	}

	//! Glyph shown while the feedback face is up. Defaults to a check mark.
	void SetFeedbackIcon( string image )
	{
		m_FeedbackImage = image;
	}

	//! Pill colour while the feedback face is up.
	void SetFeedbackColor( int color )
	{
		m_FeedbackColor = color;
	}

	//! Seconds the check mark stays up.
	void SetFeedbackDuration( float seconds )
	{
		m_FeedbackHold = Math.Max( 0.1, seconds );
	}

	bool IsShowingFeedback()
	{
		return m_FeedbackActive;
	}

	//! Start the swap. Called again while the check mark is already up it just
	//! restarts the hold, so rapid clicks read as one continuous confirmation.
	void ShowFeedback()
	{
		if ( !m_Image )
			return;

		if ( !m_FeedbackActive )
		{
			m_SavedFillColor = m_FillColor;
			SetColor( m_FeedbackColor );

			if ( m_FeedbackImage != "" )
				m_Image.LoadImageFile( 0, m_FeedbackImage );
		}

		m_FeedbackActive = true;
		m_FeedbackTimer  = 0;
	}

	//! Cut back to the resting glyph immediately.
	void ResetFeedback()
	{
		if ( !m_FeedbackActive )
			return;

		m_FeedbackActive = false;
		m_FeedbackTimer  = 0;

		SetColor( m_SavedFillColor );

		if ( m_Image && m_RestImage != "" )
			m_Image.LoadImageFile( 0, m_RestImage );
	}

	// UIActionButton.TriggerSpin guards on m_Icon, which doesn't exist here.
	// Override both so the image spins instead.
	override void TriggerSpin( int revolutions )
	{
		if ( !m_Image || !m_Image.IsVisible() )
			return;

		m_SpinAngle  = 0;
		m_SpinTarget = revolutions * 360.0;
		m_SpinTotal  = m_SpinTarget;
		m_SpinActive = true;
	}

	override void AnimateFeedback()
	{
		ShowFeedback();
	}

	override void AnimateSpin( float revolutions = 1.0 )
	{
		TriggerSpin( Math.Max( 1, revolutions ) );
	}

	override void AnimateError()
	{
		SetColor( JMTheme.DANGER );
		SetFeedbackColor( JMTheme.DANGER );
		ShowFeedback();
	}

	override void OnHide()
	{
		super.OnHide();

		// A form closed mid-swap would come back still wearing the check mark.
		ResetFeedback();
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( m_Image && m_SpinActive )
			m_Image.SetRotation( 0, 0, m_SpinAngle );

		if ( !m_FeedbackActive )
			return;

		m_FeedbackTimer += timeSlice;

		if ( m_FeedbackTimer >= m_FeedbackHold )
			ResetFeedback();
	}
}
