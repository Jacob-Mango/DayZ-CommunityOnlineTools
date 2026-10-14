//  JMLiquidHistoryEntry - undo/redo for ESP's Set Liquid Type. The int liquid id rides in the
//  float slot of JMPropertyHistoryEntry and is cast back; ids are far below 2^24, so the
//  round trip is exact.
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
