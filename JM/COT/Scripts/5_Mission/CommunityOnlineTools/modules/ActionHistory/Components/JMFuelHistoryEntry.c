// =============================================================================
//  JMFuelHistoryEntry
//
//  Undo/redo for a vehicle's fuel level - both ESP's Set Fuel and Refuel.
//  Values are the tank fraction (0..1) CommunityOnlineToolsBase.GetFuel01 /
//  SetFuel01 already speak. A vehicle with no readable tank (GetFuel01 gives
//  -1) does not get an entry.
// =============================================================================
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
