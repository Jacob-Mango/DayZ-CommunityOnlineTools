// =============================================================================
//  UIActionTimePicker
//
//  Three edit boxes - HH : MM : SS - for entering a duration or time of day.
//  Overflow carries upward, so 90 typed into seconds becomes 00:01:30 and 90
//  typed into minutes becomes 01:30:00. Fires UIEvent.CHANGE whenever the total
//  changes. GetTotalSeconds() converts to a single integer for easy use in ban
//  durations etc.
//
//  Usage:
//      m_Duration = UIActionManager.CreateTimePicker( parent, "Ban Duration",
//          this, "OnChange_Duration" );
//      m_Duration.SetTotalSeconds( 3600 );   // 01:00:00
//
//      void OnChange_Duration( UIEvent eid, UIActionBase action )
//      {
//          int secs = m_Duration.GetTotalSeconds();
//      }
//
//  -- Why this control polls instead of handling events --------------------
//
//  Two things this widget cannot rely on, both confirmed from client logs
//  rather than assumed:
//
//    * OnMouseButtonDown / OnClick never fire for these fields. A diagnostic
//      build logged every press and produced no lines at all while the fields
//      were being clicked, so nothing that depends on a press event can decide
//      which field the user is working in.
//    * GetScreenPos on the fields returns coordinates relative to their
//      right-aligned parent (measured: x = -234, -152, -69), not screen space,
//      so a mouse-rect hit test can never match an absolute cursor position.
//      That is what used to pin every scroll onto hours.
//
//  What does work is polling in Update: GetWidgetUnderCursor() for what the
//  pointer is over, and reading each box's text to see what the user typed. The
//  whole control is driven from those two signals.
// =============================================================================
class UIActionTimePicker: UIActionBase
{
	protected TextWidget    m_Label;
	protected EditBoxWidget m_Hours;
	protected EditBoxWidget m_Minutes;
	protected EditBoxWidget m_Seconds;

	protected Widget m_HoursChrome;
	protected Widget m_MinutesChrome;
	protected Widget m_SecondsChrome;

	protected int m_H;
	protected int m_M;
	protected int m_S;
	protected int m_MaxHours;

	//! The field the user is working in: the last one they typed in or clicked.
	//! This is what the scroll wheel drives, and it is the field that is left
	//! alone when other fields are repainted.
	protected int m_EditingField;
	//! Field under the pointer, refreshed every frame. Only used to pick a
	//! target when nothing is being edited.
	protected int m_HoveredField;

	//! Per-field text watch driving the deferred commit.
	protected ref array<string> m_LastText;
	protected ref array<float>  m_IdleTime;

	protected bool m_WasPressed;


	//! Seconds a field must sit unchanged before it is normalised. Committing on
	//! every keystroke rewrites the box mid-entry, which is how "20" typed into
	//! seconds used to collapse to 0.
	static const float COMMIT_DELAY = 0.6;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Label,   layoutRoot.FindAnyWidget( "action_label"   ) );
		Class.CastTo( m_Hours,   layoutRoot.FindAnyWidget( "action_hours"   ) );
		Class.CastTo( m_Minutes, layoutRoot.FindAnyWidget( "action_minutes" ) );
		Class.CastTo( m_Seconds, layoutRoot.FindAnyWidget( "action_seconds" ) );

		m_HoursChrome   = layoutRoot.FindAnyWidget( "action_hours_chrome"   );
		m_MinutesChrome = layoutRoot.FindAnyWidget( "action_minutes_chrome" );
		m_SecondsChrome = layoutRoot.FindAnyWidget( "action_seconds_chrome" );

		m_MaxHours     = 9999;
		m_EditingField = -1;
		m_HoveredField = -1;

		m_LastText = new array<string>;
		m_LastText.Insert( "" );
		m_LastText.Insert( "" );
		m_LastText.Insert( "" );

		m_IdleTime = new array<float>;
		m_IdleTime.Insert( 0 );
		m_IdleTime.Insert( 0 );
		m_IdleTime.Insert( 0 );

		SetTotalSeconds( 0 );

	}


	override void SetLabel( string text )
	{
		text = Widget.TranslateString( text );
		if ( m_Label )
			m_Label.SetText( text );
	}

	//! Set max hours (e.g. 24 for time-of-day, 9999 for duration).
	void SetMaxHours( int max )
	{
		m_MaxHours = Math.Max( 0, max );
		SetFields( m_H, m_M, m_S, true );
	}

	void SetTotalSeconds( int totalSeconds )
	{
		SetFields( 0, 0, Math.Max( 0, totalSeconds ), true );
	}

	int GetTotalSeconds()
	{
		return m_H * 3600 + m_M * 60 + m_S;
	}

	int GetHours()   { return m_H; }
	int GetMinutes() { return m_M; }
	int GetSeconds() { return m_S; }

	// -------------------------------------------------------------------------
	//  Polling
	// -------------------------------------------------------------------------

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		UpdateHovered();
		UpdatePress();

		WatchField( 0, timeSlice );
		WatchField( 1, timeSlice );
		WatchField( 2, timeSlice );

	}

	protected void UpdateHovered()
	{
		m_HoveredField = FieldForWidget( GetWidgetUnderCursor() );
	}

	//! A press selects the field under the pointer, or ends the edit when it
	//! lands anywhere else.
	protected void UpdatePress()
	{
		bool pressed = ( GetMouseState( MouseState.LEFT ) & MB_PRESSED_MASK ) != 0;

		if ( pressed && !m_WasPressed )
		{
			if ( m_HoveredField >= 0 )
				SelectField( m_HoveredField );
			else
				FinishEditing();
		}

		m_WasPressed = pressed;
	}

	//! Watch one field's text. A change means the user is typing in it; going
	//! quiet with a value that disagrees with the model means it is ready to be
	//! normalised.
	protected void WatchField( int field, float timeSlice )
	{
		EditBoxWidget box = FieldBox( field );
		if ( !box )
			return;

		string text = box.GetText();

		if ( text != m_LastText.Get( field ) )
		{
			SelectField( field );
			m_LastText.Set( field, text );
			m_IdleTime.Set( field, 0 );
			return;
		}

		if ( text.ToInt() == FieldValue( field ) )
			return;

		m_IdleTime.Set( field, m_IdleTime.Get( field ) + timeSlice );
		if ( m_IdleTime.Get( field ) < COMMIT_DELAY )
			return;


		CommitField( field );
	}

	//! Mark a field as the one being worked in. Leaving a field normalises how it
	//! is written (7 -> 07) now that the user is done with it.
	protected void SelectField( int field )
	{
		if ( field == m_EditingField )
			return;

		int previous = m_EditingField;
		m_EditingField = field;


		if ( previous >= 0 )
		{
			CommitField( previous );
			RepaintField( previous );
		}

		FocusField( field );
	}

	protected void FinishEditing()
	{
		if ( m_EditingField < 0 )
			return;

		int field = m_EditingField;
		m_EditingField = -1;

		CommitField( field );
		RepaintField( field );
	}

	// -------------------------------------------------------------------------
	//  Commit
	// -------------------------------------------------------------------------

	//! Read back one field and re-normalise the whole duration from it. The other
	//! two keep their last-known good values - reading all three is not safe,
	//! since a field can hold a part-typed number.
	protected void CommitField( int field )
	{
		EditBoxWidget box = FieldBox( field );
		if ( !box )
			return;

		m_IdleTime.Set( field, 0 );

		int typed = Math.Max( 0, box.GetText().ToInt() );
		if ( typed == FieldValue( field ) )
			return;

		int h = m_H;
		int m = m_M;
		int s = m_S;

		if ( field == 0 )      h = typed;
		else if ( field == 1 ) m = typed;
		else if ( field == 2 ) s = typed;

		int oldTotal = GetTotalSeconds();
		SetFields( h, m, s, false );
		int newTotal = GetTotalSeconds();

		m_LastText.Set( field, box.GetText() );

		if ( newTotal != oldTotal )
			CallEvent( UIEvent.CHANGE );
	}

	protected void RepaintField( int field )
	{
		EditBoxWidget box = FieldBox( field );
		if ( !box )
			return;

		int digits = 2;
		if ( field == 0 && m_H >= 100 )
			digits = 3;

		WriteField( box, FieldValue( field ), digits );
		m_LastText.Set( field, box.GetText() );
	}

	private void SetFields( int h, int m, int s, bool repaintEditing )
	{
		// Carry overflow/underflow through a single total.
		int total = h * 3600 + m * 60 + s;
		total = Math.Max( 0, total );

		// Enforce's '/' is floating point even for two ints, so the usual
		// "total / 3600 * 3600" trick does not truncate and the remainders come
		// out as zero - 100 seconds normalised to 00:00:00 instead of 00:01:40.
		// Integer division has to be spelled out.
		int hours = Math.Floor( total / 3600 );
		int rest  = total - hours * 3600;
		int mins  = Math.Floor( rest / 60 );
		int secs  = rest - mins * 60;

		m_H = Math.Clamp( hours, 0, m_MaxHours );
		m_M = Math.Clamp( mins,  0, 59 );
		m_S = Math.Clamp( secs,  0, 59 );

		int hDigits = 2;
		if ( m_H >= 100 )
			hDigits = 3;

		PadField( 0, m_Hours,   m_H, hDigits, repaintEditing );
		PadField( 1, m_Minutes, m_M, 2,       repaintEditing );
		PadField( 2, m_Seconds, m_S, 2,       repaintEditing );
	}

	private void PadField( int field, EditBoxWidget box, int value, int digits, bool force )
	{
		if ( !box )
			return;

		// Never rewrite the field being typed in while what it shows already
		// means the right number: "2" and "02" are the same value, and replacing
		// the text moves the caret out from under the user.
		if ( !force && field == m_EditingField && box.GetText().ToInt() == value )
		{
			m_LastText.Set( field, box.GetText() );
			return;
		}

		WriteField( box, value, digits );
		m_LastText.Set( field, box.GetText() );
	}

	private void WriteField( EditBoxWidget box, int value, int digits )
	{
		string s = "" + value;
		while ( s.Length() < digits )
			s = "0" + s;

		box.SetText( s );
	}

	// -------------------------------------------------------------------------
	//  Wheel
	// -------------------------------------------------------------------------

	//! The field being worked in takes the wheel. The pointer only decides when
	//! nothing is selected. The widget the event arrives on is deliberately
	//! ignored - it reports hours regardless of which field is in use.
	override bool OnMouseWheel( Widget w, int x, int y, int wheel )
	{
		int field = m_EditingField;
		if ( field < 0 )
			field = m_HoveredField;


		if ( field < 0 )
			return false;

		// Fold in anything typed but not yet committed, so the step moves from
		// what is on screen rather than from a stale model.
		CommitField( field );

		// One step per event, direction only. The raw delta is ~120.
		int step = 1;
		if ( wheel < 0 )
			step = -1;

		int h = m_H;
		int m = m_M;
		int s = m_S;

		if ( field == 0 )      h = m_H + step;
		else if ( field == 1 ) m = m_M + step;
		else if ( field == 2 ) s = m_S + step;

		int oldTotal = GetTotalSeconds();
		SetFields( h, m, s, true );

		if ( GetTotalSeconds() != oldTotal )
			CallEvent( UIEvent.CHANGE );

		return true;
	}

	// -------------------------------------------------------------------------
	//  Widget helpers
	// -------------------------------------------------------------------------

	//! Anything inside this control counts as ours, so UIActionBase does not
	//! treat the pointer as being outside while a field is in use.
	override bool IsFocusWidget( Widget widget )
	{
		return OwnsWidget( widget );
	}

	protected bool OwnsWidget( Widget widget )
	{
		Widget cur = widget;
		while ( cur )
		{
			if ( cur == layoutRoot )
				return true;

			cur = cur.GetParent();
		}

		return false;
	}

	//! 0/1/2 for the field the widget belongs to (box, fill, ring or chrome), -1
	//! if it is not part of a field.
	protected int FieldForWidget( Widget widget )
	{
		Widget cur = widget;
		while ( cur )
		{
			if ( cur == m_Hours   || cur == m_HoursChrome   ) return 0;
			if ( cur == m_Minutes || cur == m_MinutesChrome ) return 1;
			if ( cur == m_Seconds || cur == m_SecondsChrome ) return 2;

			cur = cur.GetParent();
		}

		return -1;
	}

	protected EditBoxWidget FieldBox( int field )
	{
		if ( field == 0 ) return m_Hours;
		if ( field == 1 ) return m_Minutes;
		if ( field == 2 ) return m_Seconds;

		return NULL;
	}

	protected int FieldValue( int field )
	{
		if ( field == 0 ) return m_H;
		if ( field == 1 ) return m_M;
		if ( field == 2 ) return m_S;

		return 0;
	}

	protected void FocusField( int field )
	{
		EditBoxWidget box = FieldBox( field );
		if ( box )
			SetFocus( box );
	}

	override void OnRegisterChrome()
	{
		RegisterChromeRing( "action_hours_ring" );
		RegisterChromeRing( "action_minutes_ring" );
		RegisterChromeRing( "action_seconds_ring" );
	}
}
