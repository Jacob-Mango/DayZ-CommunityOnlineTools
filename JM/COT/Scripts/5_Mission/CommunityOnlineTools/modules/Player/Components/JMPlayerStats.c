//! The built-in JMPlayerStat implementations. Only what is specific to each stat is
//! here; its id, permission, range and colour bands are supplied where it is
//! registered, in JMPlayerModule.RegisterStats().
//!
//! Kept together in one file because each is a handful of lines. A stat a mod adds
//! should live in its own file.

class JMPlayerStatHealth: JMPlayerStat
{
	override void Apply( PlayerBase player, float value ) { player.SetHealth( "GlobalHealth", "Health", value ); }
	override float Read( JMPlayerInstance instance ) { return instance.GetHealth(); }
}

class JMPlayerStatBlood: JMPlayerStat
{
	override void Apply( PlayerBase player, float value ) { player.SetHealth( "GlobalHealth", "Blood", value ); }
	override float Read( JMPlayerInstance instance ) { return instance.GetBlood(); }
}

//! Shock is banded by the thresholds that decide whether the player is awake, not by
//! evenly spaced bands.
class JMPlayerStatShock: JMPlayerStat
{
	override void Apply( PlayerBase player, float value ) { player.SetHealth( "GlobalHealth", "Shock", value ); }
	override float Read( JMPlayerInstance instance ) { return instance.GetShock(); }

	override int GetColor( float sliderValue )
	{
		if ( sliderValue >= m_High )
			return Colors.COLOR_PRISTINE;

		if ( sliderValue >= PlayerConstants.CONSCIOUS_THRESHOLD )
			return JMTheme.VALUE_OK;

		if ( sliderValue < PlayerConstants.UNCONSCIOUS_THRESHOLD )
			return JMTheme.VALUE_BAD;

		return JMTheme.VALUE_CAUTION;
	}
}

class JMPlayerStatEnergy: JMPlayerStat
{
	override void Apply( PlayerBase player, float value ) { player.GetStatEnergy().Set( value ); }
	override float Read( JMPlayerInstance instance ) { return instance.GetEnergy(); }
}

class JMPlayerStatWater: JMPlayerStat
{
	override void Apply( PlayerBase player, float value ) { player.GetStatWater().Set( value ); }
	override float Read( JMPlayerInstance instance ) { return instance.GetWater(); }
}

class JMPlayerStatStamina: JMPlayerStat
{
	override void Apply( PlayerBase player, float value ) { player.GetStatStamina().Set( value ); }
	override float Read( JMPlayerInstance instance ) { return instance.GetStamina(); }
}

//! The stat runs 0-30 and the slider 0-3, in whole steps of the four heat-buffer stages.
class JMPlayerStatHeatBuffer: JMPlayerStat
{
	static const float SLIDER_SCALE = 10;

	override void Apply( PlayerBase player, float value ) { player.GetStatHeatBuffer().Set( value ); }
	override float Read( JMPlayerInstance instance ) { return instance.GetHeatBuffer(); }
	override float ToSlider( float value ) { return value / SLIDER_SCALE; }
	override float FromSlider( float value ) { return value * SLIDER_SCALE; }

	override int GetColor( float sliderValue )
	{
		if ( sliderValue >= 3 )
			return Colors.COLOR_PRISTINE;

		if ( sliderValue >= 1 )
			return Colors.COLOR_WORN;

		return 0x00FFFFFF;
	}

	//! One plus per stage: none, +, ++, +++, then the number.
	override string GetText( float sliderValue )
	{
		int stage = Math.Clamp( sliderValue, 0, 3 );
		string plusses;

		for ( int i = 0; i < stage; i++ )
			plusses += "+";

		return plusses + " " + sliderValue;
	}
}
