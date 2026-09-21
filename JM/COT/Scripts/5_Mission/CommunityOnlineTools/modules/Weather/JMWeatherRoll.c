//! One roll between a low and a high bound - how a preset value that is a RANGE
//! becomes the single number the engine is given. The high bound is a separate
//! field on each weather class (the "Hi" twin of the value) and is only a range
//! when it lies ABOVE the low one: an absent, zero or equal Hi is a plain value (so a
//! span can never end exactly on 0 - the range control nudges it), which is also
//! what a file written before ranges existed loads as. -1 stays "leave alone".
//!
//! Its own class rather than a static on JMWeatherBase: the payload classes derive
//! from that one and are written to the wire with ctx.Write( this ), and a static
//! member on the base made every one of those writes come out empty.
class JMWeatherRoll
{
	static float Pick( float low, float high )
	{
		if ( low == -1 || high == 0 || high <= low )
			return low;

		return Math.RandomFloat( low, high );
	}
}
