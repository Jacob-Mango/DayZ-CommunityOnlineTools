//! Couples a UIActionSlider with a numeric UIActionEditableText so an admin can
//! either drag or type an exact value.
//!
//! The slider stays the source of truth and OWNS this object (see
//! UIActionSlider.SetSync), so:
//!   - every existing consumer keeps working unchanged: SetCurrent/GetCurrent,
//!     the SetCallback( instance, "OnChange_X" ) handler and SetOnChange all fire
//!     exactly as before, whether the value came from a drag or from the box;
//!   - a programmatic SetCurrent (loading values from the server) refreshes the
//!     box too, without firing CHANGE.
//!
//! Use UIActionManager.CreateSyncedSlider to build the pair in one call.
class UIActionSliderSync
{
	protected UIActionSlider m_Slider;
	protected UIActionEditableText m_EditBox;
	protected Widget m_Row;

	//! Share of the pair's width the box takes; the slider gets the rest less a gap.
	protected float m_EditShare = 0.16;
	protected static const float GAP = 0.01;

	void UIActionSliderSync( UIActionSlider slider, UIActionEditableText editBox, float min = 0.0, float max = 1.0, float step = 0.01 )
	{
		m_Slider = slider;
		m_EditBox = editBox;

		if ( m_Slider )
		{
			if ( max > min )
				m_Slider.SetMinMax( min, max );

			if ( step > 0 )
				m_Slider.SetStepValue( step );

			m_Slider.SetSync( this );
		}

		if ( m_EditBox )
		{
			m_EditBox.SetOnlyNumbers( true );
			m_EditBox.SetCallback( this, "OnEditBoxChanged" );
		}

		Refresh();
	}

	UIActionEditableText GetEditBox()
	{
		return m_EditBox;
	}

	UIActionSlider GetSlider()
	{
		return m_Slider;
	}

	float GetValue()
	{
		if ( !m_Slider )
			return 0;

		return m_Slider.GetCurrent();
	}

	//! Handler with the usual ( UIEvent, UIActionBase ) signature; called with
	//! the slider as the action.
	void SetCallback( Class instance, string functionName )
	{
		if ( m_Slider )
			m_Slider.SetCallback( instance, functionName );
	}

	void SetEditShare( float share )
	{
		m_EditShare = Math.Clamp( share, 0.05, 0.5 );
	}

	void SetMinMax( float min, float max )
	{
		if ( m_Slider )
			m_Slider.SetMinMax( min, max );
	}

	//! The container holding slider + box. Hiding the slider hides this instead,
	//! otherwise the box would be left floating on its own.
	void SetRow( Widget row )
	{
		m_Row = row;
	}

	void SetStepValue( float step )
	{
		if ( m_Slider )
			m_Slider.SetStepValue( step );
	}

	void SetValue( float val, bool triggerCallback = true )
	{
		if ( !m_Slider )
			return;

		m_Slider.SetCurrent( val );

		if ( triggerCallback )
			m_Slider.CallEvent( UIEvent.CHANGE );
	}

	//! Size the pair to `width` of its row.
	void ApplyWidth( float width )
	{
		if ( m_Slider )
			m_Slider.SetOwnWidth( width * ( 1.0 - m_EditShare - GAP ) );

		if ( m_EditBox )
			m_EditBox.SetWidth( width * m_EditShare );
	}

	//! Called by the slider whenever its value or visuals change.
	void Refresh()
	{
		if ( !m_Slider || !m_EditBox )
			return;

		//! Rounded so step-snapping float noise (0.30000001) never reaches the
		//! box. UIActionEditableText.SetText ignores the call while the box is
		//! focused, so this can never overwrite what the admin is typing.
		float shown = Math.Round( m_Slider.GetCurrent() * 10000 ) / 10000;
		m_EditBox.SetText( shown );
	}

	void OnSliderEnabled( bool enabled )
	{
		if ( m_EditBox )
			m_EditBox.SetEnabled( enabled );
	}

	void OnSliderShown( bool shown )
	{
		if ( m_Row )
			m_Row.Show( shown );
	}

	protected void OnEditBoxChanged( UIEvent eid, UIActionBase action )
	{
		if ( !m_Slider || !m_EditBox )
			return;

		if ( eid != UIEvent.CHANGE && eid != UIEvent.MOUSEWHEEL )
			return;

		string txt = m_EditBox.GetText();

		//! Half-typed input; wait for a number.
		if ( txt == "" || txt == "-" || txt == "." || txt == "-." )
			return;

		float parsed = txt.ToFloat();

		//! Refresh() writing the box back echoes a CHANGE here; that is not an
		//! edit and must not mark the owning section dirty.
		if ( Math.AbsFloat( parsed - m_Slider.GetCurrent() ) < 0.0001 )
			return;

		m_Slider.SetCurrent( parsed );
		m_Slider.CallEvent( UIEvent.CHANGE );
	}
}
