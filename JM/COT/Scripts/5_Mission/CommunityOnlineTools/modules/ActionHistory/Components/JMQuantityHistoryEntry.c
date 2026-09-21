// =============================================================================
//  JMQuantityHistoryEntry
//
//  Undo/redo for ESP's Set Quantity on an item.
// =============================================================================
class JMQuantityHistoryEntry: JMPropertyHistoryEntry
{
	void JMQuantityHistoryEntry( Object target, float before, float after )
	{
		Init( target, JMConstants.PERM_ESP_OBJECT_SETQUANTITY, before, after );
	}

	override protected bool Apply( float value )
	{
		ItemBase item;
		if ( !Class.CastTo( item, m_Target ) )
			return false;

		item.SetQuantity( value, false );

		return true;
	}

	override protected string GetPropertyName()
	{
		return "quantity";
	}
}
