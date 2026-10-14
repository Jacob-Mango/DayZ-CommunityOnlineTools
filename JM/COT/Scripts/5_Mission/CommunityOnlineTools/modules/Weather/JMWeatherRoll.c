//! One roll between a low and high bound: how a RANGE preset value becomes the engine's number.
//! The "Hi" twin is a range only when ABOVE the low one (absent/zero/equal = plain value; -1 =
//! "leave alone"). Own class, not a static on JMWeatherBase: payloads derive from that and are
//! written with ctx.Write( this ), and a static member there made every write come out empty.
class JMWeatherRoll
{
	static float Pick( float low, float high )
	{
		if ( low == -1 || high == 0 || high <= low )
			return low;

		return Math.RandomFloat( low, high );
	}
}
