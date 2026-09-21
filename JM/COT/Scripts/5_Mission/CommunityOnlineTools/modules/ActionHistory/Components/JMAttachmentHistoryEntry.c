// =============================================================================
//  JMAttachmentHistoryEntry
//
//  Undo/redo for ESP's attachment add / swap / remove on one slot. It records
//  the classname before and after ("" = empty slot), so all three are the same
//  entry: undo puts `before` in the slot, redo puts `after` back.
//
//  What is restored is a fresh attachment of that classname, not the exact
//  item that was there - its health, quantity and anything attached to IT are
//  not carried across.
// =============================================================================
class JMAttachmentHistoryEntry: JMActionHistoryEntry
{
	protected EntityAI m_Target;
	protected int m_Slot;
	protected string m_Before;
	protected string m_After;

	void JMAttachmentHistoryEntry( EntityAI target, int slotId, string before, string after )
	{
		RequiredPermission = JMConstants.PERM_ESP_OBJECT_SETATTACHMENT;

		m_Target = target;
		m_Slot   = slotId;
		m_Before = before;
		m_After  = after;
	}

	protected bool SetSlot( string className )
	{
		if ( !m_Target || !m_Target.GetInventory() )
			return false;

		GameInventory inventory = m_Target.GetInventory();

		EntityAI existing = inventory.FindAttachment( m_Slot );
		if ( existing )
			g_Game.ObjectDelete( existing );

		if ( className != "" )
			inventory.CreateAttachmentEx( className, m_Slot );

		return true;
	}

	override bool CanUndo()
	{
		return m_Target != NULL;
	}

	override bool CanRedo()
	{
		return m_Target != NULL;
	}

	override bool Undo()
	{
		return SetSlot( m_Before );
	}

	override bool Redo()
	{
		return SetSlot( m_After );
	}

	override Object GetTarget()
	{
		return m_Target;
	}

	override void SetTarget( Object target )
	{
		Class.CastTo( m_Target, target );
	}

	override string GetDescription()
	{
		return "Attachment change on " + DescribeObject( m_Target );
	}
}
