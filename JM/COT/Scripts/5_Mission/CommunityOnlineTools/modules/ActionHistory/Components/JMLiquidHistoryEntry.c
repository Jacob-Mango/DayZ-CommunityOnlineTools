// =============================================================================
//  JMLiquidHistoryEntry
//
//  Undo/redo for ESP's Set Liquid Type on an item. The type is an int (a
//  single-bit liquid id); it rides in the float slot of JMPropertyHistoryEntry
//  and is cast back on the way out - every liquid id is far below 2^24, so it
//  survives the round trip exactly.
// =============================================================================
class JMLiquidHistoryEntry: JMPropertyHistoryEntry
{
	void JMLiquidHistoryEntry( Object target, int before, int after )
	{
		Init( target, JMConstants.PERM_ESP_OBJECT_SETLIQUID, before, after );
	}

	override protected bool Apply( float value )
	{
		ItemBase item;
		if ( !Class.CastTo( item, m_Target ) )
			return false;

		item.SetLiquidType( (int) value );

		return true;
	}

	override protected string GetPropertyName()
	{
		return "liquid type";
	}
}
