// =============================================================================
//  JMWeatherNextState
//
//  One candidate the dynamic chain can move to, and the chance it is picked.
//
//  These live on the preset they lead OUT of, so there is no "from" to store -
//  the owning preset is the from. That is the whole point of holding them here
//  rather than in one flat list on the settings: a preset arrives already
//  knowing where it can go next, and a rename or a delete only ever disturbs
//  the presets that actually name the one that changed.
// =============================================================================
class JMWeatherNextState
{
	string To;

	//! Whole percent. The form keeps a preset's candidates summing to 100, but
	//! nothing here depends on it - the roll draws against whatever total the
	//! list happens to add up to.
	int Chance;

	void JMWeatherNextState()
	{
		To     = "";
		Chance = 0;
	}
}
