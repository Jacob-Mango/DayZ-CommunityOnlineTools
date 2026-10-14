//! One stat slider on the General tab: the JMPlayerStat it edits, the slider that
//! shows it, and Dirty: dragged since the last refresh. Until Apply sends it the
//! value is the admin's, so a refresh must not overwrite it.
class JMPlayerStatRow
{
	JMPlayerStat Stat;
	UIActionSlider Slider;
	bool Dirty;

	void JMPlayerStatRow( JMPlayerStat stat, UIActionSlider slider )
	{
		Stat = stat;
		Slider = slider;
	}

	//! The value to send: the slider's, converted back to stat units.
	float GetValue()
	{
		return Stat.FromSlider( Slider.GetCurrent() );
	}

	//! Repaint from the selected player, unless the admin's own edit is pending.
	void Refresh( JMPlayerInstance instance )
	{
		if ( !Dirty )
			Slider.SetCurrent( Stat.ToSlider( Stat.Read( instance ) ) );

		Paint();
	}

	//! Colour (and, for the heat buffer, the stage text) for wherever the slider is now.
	void Paint()
	{
		float value = Slider.GetCurrent();

		Slider.SetColor( Stat.GetColor( value ) );
		Slider.SetAlpha( 1.0 );

		string text = Stat.GetText( value );

		if ( text != "" )
			Slider.SetText( text );
	}
}
