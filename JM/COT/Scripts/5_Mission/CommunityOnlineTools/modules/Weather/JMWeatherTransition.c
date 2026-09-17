// =============================================================================
//  JMWeatherTransition - LEGACY, MIGRATION ONLY
//
//  The shape the dynamic chain's edges used to be stored in: one flat list on
//  the settings, every entry naming the preset it left. Edges now live on the
//  preset they leave, as JMWeatherNextState, where the From field is implicit.
//
//  This class is kept ALIVE ON PURPOSE. JsonFileLoader can only read a field
//  whose type the class still declares, and the migration cannot tell which
//  preset an old edge belongs to without reading its From - so removing this
//  would not delete dead code, it would silently discard an existing
//  configuration on the first load after the upgrade.
//
//  Nothing but JMWeatherSerialize.MigrateDynamic should reference it. Once
//  every server has loaded once, it can go.
// =============================================================================
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
