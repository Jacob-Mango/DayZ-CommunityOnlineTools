//  JMFuelHistoryEntry - undo/redo for vehicle fuel (ESP Set Fuel and Refuel).
//  Values are the tank fraction 0..1 of GetFuel01/SetFuel01; no entry if
//  GetFuel01 gives -1.
class JMFuelHistoryEntry: JMPropertyHistoryEntry
{
	void JMFuelHistoryEntry( Object target, float before, float after )
	{
		Init( target, JMConstants.PERM_ESP_OBJECT_CAR_REFUEL, before, after );
	}

	override protected bool Apply( float value )
	{
		if ( !m_Target )
			return false;

		COT.SetFuel01( m_Target, value );

		return true;
	}

	override protected string GetPropertyName()
	{
		return "fuel";
	}
}
