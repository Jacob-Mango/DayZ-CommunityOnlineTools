//! One vital an admin can set on a player: health, blood, shock, energy, water,
//! stamina, heat buffer.
//!
//! The stat owns what differs between them - the permission that gates it, its
//! range, how it is written to the entity, how the client reads it back and how
//! its slider is coloured. Sending, clamping, applying and receiving one is the
//! same code for all of them (JMPlayerModule.SetStat / Exec_SetStat / RPC_SetStat),
//! and the General tab builds one JMPlayerStatRow per stat that owns its slider's
//! dirty flag, repaint and Apply.
//!
//! The id is what travels on the wire. The built-ins use JMStatType; a mod adds a stat
//! with an id above JMStatType.COUNT:
//!
//!   modded class JMPlayerModule
//!   {
//!       override void RegisterStats( array< ref JMPlayerStat > stats )
//!       {
//!           super.RegisterStats( stats );
//!           DefineStat( stats, new MyStat(), MY_STAT_ID, "MyStat", MY_PERM, 0, 100 );
//!       }
//!   }
class JMPlayerStat
{
	protected int m_Type;
	protected string m_Name;
	protected string m_Permission;
	protected float m_Min;
	protected float m_Max;

	//! Slider bands, in slider units: at or above High reads as healthy, then Normal, then
	//! Low; below Low is critical.
	protected float m_High;
	protected float m_Normal;
	protected float m_Low;

	//! Colour of the slider at a value, in slider units.
	int GetColor( float sliderValue )
	{
		if ( sliderValue >= m_High )
			return Colors.COLOR_PRISTINE;

		if ( sliderValue >= m_Normal )
			return JMTheme.VALUE_OK;

		if ( sliderValue >= m_Low )
			return JMTheme.VALUE_CAUTION;

		return JMTheme.VALUE_BAD;
	}

	string GetName()
	{
		return m_Name;
	}

	string GetPermission()
	{
		return m_Permission;
	}

	//! Text shown on the slider in place of its number. "" leaves the number.
	string GetText( float sliderValue )
	{
		return "";
	}

	int GetType()
	{
		return m_Type;
	}

	//! type         wire id (JMStatType for the built-ins)
	//! name         wording for log and webhook lines ("Health")
	//! permission   node the SENDER needs
	//! min, max     what the server clamps a value to before applying it
	JMPlayerStat Define( int type, string name, string permission, float min, float max )
	{
		m_Type = type;
		m_Name = name;
		m_Permission = permission;
		m_Min = min;
		m_Max = max;

		return this;
	}

	JMPlayerStat WithBands( float high, float normal, float low )
	{
		m_High = high;
		m_Normal = normal;
		m_Low = low;

		return this;
	}

	float Clamp( float value )
	{
		return Math.Clamp( value, m_Min, m_Max );
	}

	//! Server: put the value on the player.
	void Apply( PlayerBase player, float value )
	{
	}

	//! Client: what the synced copy of a player says the value is, in stat units.
	float Read( JMPlayerInstance instance )
	{
		return 0;
	}

	//! The slider does not always work in stat units (the heat buffer slider runs 0-3 over a
	//! 0-30 stat).
	float ToSlider( float value )
	{
		return value;
	}

	float FromSlider( float value )
	{
		return value;
	}
}
