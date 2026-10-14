//  JMWeatherTransition - LEGACY, MIGRATION ONLY (old flat edge list; now JMWeatherNextState).
//  Kept ALIVE ON PURPOSE: JsonFileLoader only reads fields whose type is still declared and
//  migration needs each old edge's From, so removing it silently discards existing configs.
//  Only JMWeatherSerialize.MigrateDynamic should reference it.
class JMWeatherTransition
{
	string From;
	string To;
	int    Chance;

	void JMWeatherTransition()
	{
		From   = "";
		To     = "";
		Chance = 0;
	}
}
