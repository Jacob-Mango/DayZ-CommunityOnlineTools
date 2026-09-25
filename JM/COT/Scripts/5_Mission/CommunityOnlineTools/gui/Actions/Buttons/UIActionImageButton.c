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
	protected bool   m_PulseActive;
	protected float  m_PulseTimer;
	protected float  m_PulseDuration;

	//! Double click to leave the button firing its own callback once a
	//! second, double click again to stop. Opt-in per button (see
	//! UIActionManager.CreateRefreshButton) rather than on by default: a
	//! second click on a delete or apply button must never arm anything.
	static const int   AUTO_REPEAT_DOUBLE_CLICK_MS = 400;
	static const float AUTO_REPEAT_INTERVAL        = 1.0;
	protected bool  m_AutoRepeatEnabled;

	//! Spin the icon on every CLICK, including the ones auto-repeat fires, so
	//! a refresh handler does not have to animate its own button.
	protected bool  m_SpinOnClick;
	protected bool  m_AutoRepeatActive;
	protected float m_AutoRepeatTimer;
	protected int   m_LastClickTime;

	bool IsAutoRepeating()
	{
		return m_AutoRepeatActive;
	}

	bool IsShowingFeedback()
	{
		return m_FeedbackActive;
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

	//! Glyph shown while the feedback face is up. Defaults to a check mark.
	void SetFeedbackIcon( string image )
	{
		m_FeedbackImage = image;
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

	void SetSpinOnClick( bool enable )
	{
		m_SpinOnClick = enable;
	}

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

		m_PulseActive   = false;
		m_PulseTimer    = 0;
		m_PulseDuration = 2.0;
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

	void TriggerPulse( float duration = 2.0 )
	{
		m_PulseDuration  = Math.Max( 0.2, duration );
		m_PulseTimer     = 0;
		m_SavedFillColor = m_FillColor;
		m_PulseActive    = true;
	}

	override void AnimatePulse( float duration = 2.0 )
	{
		TriggerPulse( duration );
	}

	void StopPulse()
	{
		if ( !m_PulseActive )
			return;

		m_PulseActive = false;
		m_PulseTimer  = 0;
		SetColor( m_SavedFillColor );
	}

	override void AnimateSpin( float revolutions = 1.0 )
	{
		TriggerSpin( Math.Max( 1, revolutions ) );
	}

	override bool CallEvent( UIEvent eid )
	{
		if ( m_SpinOnClick && eid == UIEvent.CLICK )
			TriggerSpin( 2 );

		return super.CallEvent( eid );
	}

	//! Allow this button to be armed by a double click. Only buttons whose
	//! action is safe to repeat unattended should get this.
	void EnableAutoRepeat( bool enable )
	{
		m_AutoRepeatEnabled = enable;

		if ( !enable )
			StopAutoRepeat();
	}

	void StopAutoRepeat()
	{
		if ( !m_AutoRepeatActive )
			return;

		m_AutoRepeatActive = false;
		m_AutoRepeatTimer  = 0;

		SetColor( m_SavedFillColor );
	}

	protected void StartAutoRepeat()
	{
		m_AutoRepeatActive = true;

		//! Counts a full interval before the first repeat - the click that
		//! armed it has already fired one.
		m_AutoRepeatTimer = 0;

		//! Tinted for as long as it is armed, because a button that keeps
		//! firing on its own has to look different from one that does not.
		SetColor( JMTheme.ACCENT );
	}

	//! Arming needs a double click (a single click on a refresh button has
	//! to stay a single refresh, not a trap that starts looping). Stopping
	//! does not - once it is already firing on its own, any click is
	//! obviously meant to stop that, not to start counting toward a second
	//! double click.
	override bool OnClick( Widget w, int x, int y, int button )
	{
		bool ret = super.OnClick( w, x, y, button );

		if ( !m_AutoRepeatEnabled || w != m_Button )
			return ret;

		if ( m_AutoRepeatActive )
		{
			StopAutoRepeat();
			m_LastClickTime = 0;
			return ret;
		}

		int now = g_Game.GetTime();

		if ( now - m_LastClickTime <= AUTO_REPEAT_DOUBLE_CLICK_MS )
		{
			m_LastClickTime = 0;
			StartAutoRepeat();
			return ret;
		}

		m_LastClickTime = now;

		return ret;
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
		StopPulse();

		//! Closing the panel/menu/module that owns the button is what ends
		//! an armed refresh - it must not keep firing at whatever is behind
		//! it, nor come back still armed the next time it is opened.
		StopAutoRepeat();
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		if ( m_Image && m_SpinActive )
			m_Image.SetRotation( 0, 0, m_SpinAngle );

		if ( m_AutoRepeatActive )
		{
			//! Stop rather than pause when the owner goes away. A button
			//! whose widget is gone or hidden has no panel left to refresh,
			//! and OnHide does not fire for every way that can happen.
			if ( !layoutRoot || !layoutRoot.IsVisibleHierarchy() )
			{
				StopAutoRepeat();
			}
			else
			{
				m_AutoRepeatTimer += timeSlice;

				if ( m_AutoRepeatTimer >= AUTO_REPEAT_INTERVAL )
				{
					m_AutoRepeatTimer = 0;

					AnimateSpin();
					CallEvent( UIEvent.CLICK );
				}
			}
		}

		if ( m_PulseActive )
		{
			m_PulseTimer += timeSlice;
			if ( m_PulseTimer >= m_PulseDuration )
			{
				StopPulse();
			}
			else
			{
				float factor = ( Math.Sin( ( m_PulseTimer / m_PulseDuration ) * Math.PI * 4.0 ) + 1.0 ) * 0.5;
				int r = Math.Lerp( 30, 73, factor );
				int g = Math.Lerp( 36, 184, factor );
				int b = Math.Lerp( 48, 117, factor );
				SetColor( ARGB( 255, r, g, b ) );
			}
		}

		if ( !m_FeedbackActive )
			return;

		m_FeedbackTimer += timeSlice;

		if ( m_FeedbackTimer >= m_FeedbackHold )
			ResetFeedback();
	}
}
