// =============================================================================
//  Exposes a public getter for the protected m_DecayState field so that
//  JMEventsAdapter can skip areas that haven't reached the LIVE stage yet.
// =============================================================================

#ifndef EXPANSIONMODMISSIONS

modded class ContaminatedArea_DynamicBase
{
	int COT_GetDecayState()
	{
		return m_DecayState;
	}
}

#endif
