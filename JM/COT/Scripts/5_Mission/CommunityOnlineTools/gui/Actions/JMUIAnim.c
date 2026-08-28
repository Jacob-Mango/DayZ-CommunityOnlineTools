// =============================================================================
//  JMUIAnim - Unified animation utilities for COT UI.
//
//  Color interpolation:
//      int c = JMUIAnim.LerpColor( from, to, t );   // t in [0,1]
//
//  Animated float (drives a single value toward a target each frame):
//      JMAnimFloat anim;
//      anim.Set( 0 );             // initial value
//      anim.SetTarget( 1, 8.0 ); // target, speed (units/sec)
//      float v = anim.Step( timeSlice );  // call in Update(), reads .Value
//
//  Easing:
//      float e = JMUIAnim.EaseOut( t );   // smooth deceleration
//      float e = JMUIAnim.EaseInOut( t ); // smooth both ends
// =============================================================================

class JMUIAnim
{
	static int LerpColor( int from, int to, float t )
	{
		int fa = ( from >> 24 ) & 0xFF;
		int fr = ( from >> 16 ) & 0xFF;
		int fg = ( from >>  8 ) & 0xFF;
		int fb =   from         & 0xFF;

		int ta = ( to >> 24 ) & 0xFF;
		int tr = ( to >> 16 ) & 0xFF;
		int tg = ( to >>  8 ) & 0xFF;
		int tb =   to         & 0xFF;

		int a = fa + (int)( ( ta - fa ) * t );
		int r = fr + (int)( ( tr - fr ) * t );
		int g = fg + (int)( ( tg - fg ) * t );
		int b = fb + (int)( ( tb - fb ) * t );

		return ARGB( a, r, g, b );
	}

	static float EaseOut( float t )
	{
		float inv = 1.0 - t;
		return 1.0 - inv * inv;
	}

	static float EaseInOut( float t )
	{
		return t * t * ( 3.0 - 2.0 * t );
	}

	static float Lerp( float a, float b, float t )
	{
		return a + ( b - a ) * t;
	}
}

// ---------------------------------------------------------------------------
//  JMAnimFloat - a self-contained animated scalar.
//  Declare as a field, call Step() every Update(), read .Value.
// ---------------------------------------------------------------------------
class JMAnimFloat
{
	float Value;
	float Target;
	float Speed;
	bool  Dirty;

	void JMAnimFloat()
	{
		Value  = 0;
		Target = 0;
		Speed  = 8.0;
		Dirty  = false;
	}

	void Set( float v )
	{
		Value  = v;
		Target = v;
		Dirty  = false;
	}

	void SetTarget( float target, float speed = 8.0 )
	{
		Target = target;
		Speed  = speed;
		Dirty  = ( Value != Target );
	}

	bool Step( float timeSlice )
	{
		if ( !Dirty )
			return false;

		float delta = Target - Value;
		float step  = delta * Speed * timeSlice;

		if ( Math.AbsFloat( delta ) < 0.001 )
		{
			Value = Target;
			Dirty = false;
		}
		else
		{
			Value = Value + step;
		}

		return true;
	}

	bool IsAnimating()
	{
		return Dirty;
	}
}

// ---------------------------------------------------------------------------
//  JMAnimColor - animated color transition between two ARGB values.
// ---------------------------------------------------------------------------
class JMAnimColor
{
	int   Value;
	int   From;
	int   Target;
	float T;
	float Speed;

	void JMAnimColor()
	{
		Value  = 0xFFFFFFFF;
		From   = 0xFFFFFFFF;
		Target = 0xFFFFFFFF;
		T      = 1.0;
		Speed  = 6.0;
	}

	void Set( int color )
	{
		Value  = color;
		From   = color;
		Target = color;
		T      = 1.0;
	}

	void SetTarget( int target, float speed = 6.0 )
	{
		if ( target == Target )
			return;

		From   = Value;
		Target = target;
		T      = 0.0;
		Speed  = speed;
	}

	bool Step( float timeSlice )
	{
		if ( T >= 1.0 )
			return false;

		T = T + timeSlice * Speed;
		if ( T > 1.0 )
			T = 1.0;

		Value = JMUIAnim.LerpColor( From, Target, JMUIAnim.EaseOut( T ) );
		return true;
	}

	bool IsAnimating()
	{
		return T < 1.0;
	}
}
