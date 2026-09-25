//! Degrees-to-8-point-compass, shared by anything that shows a heading -
//! wind direction (JMWeatherFormTabWind) and player facing (the sidebar
//! footer position row). Lives in 3_Game so both a 5_Mission form and a
//! 4_World widget can reach it.
class JMCompass
{
	static autoptr TStringArray CARDINAL_DIRECTIONS = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

	//! Compass point for a heading in degrees, 0 = N, 90 = E.
	static string CardinalFor( float degrees )
	{
		int index = Math.Floor( ( ( degrees + 22.5 ) / 45 ) );

		if ( index > CARDINAL_DIRECTIONS.Count() - 1 )
			index -= CARDINAL_DIRECTIONS.Count();
		else if ( index < 0 )
			index = 0;

		return CARDINAL_DIRECTIONS[index];
	}
}
