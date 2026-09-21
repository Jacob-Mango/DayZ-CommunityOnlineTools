// =============================================================================
//  JMHealthHistoryEntry
//
//  Undo/redo for ESP's Set Health (the global health value - the GUI has no
//  per-zone control). Not the Heal action: that is JMHealHistoryEntry, which
//  also handles players' stamina/water.
// =============================================================================
class JMHealthHistoryEntry: JMPropertyHistoryEntry
{
	void JMHealthHistoryEntry( Object target, float before, float after )
	{
		Init( target, JMConstants.PERM_ESP_OBJECT_SETHEALTH, before, after );
	}

	override protected bool Apply( float value )
	{
		if ( !m_Target )
			return false;

		//! Exec_SetHealth refuses a dead man or creature; so does its inverse.
		if ( ( m_Target.IsInherited( Man ) || m_Target.IsInherited( DayZCreature ) ) && !m_Target.IsAlive() )
			return false;

		m_Target.SetHealth( "", "", value );

		return true;
	}

	override protected string GetPropertyName()
	{
		return "health";
	}
}
