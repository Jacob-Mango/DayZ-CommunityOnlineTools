// JMPropertyHistoryEntry - base for "one number on one object was changed" (health,
// quantity, fuel). A subclass writes one value (Apply) and names it; this supplies
// before/after bookkeeping, Undo/Redo and the merge of bursts of edits (a dragged slider).
// Construct AFTER the change, reading `after` back from the object so clamping is replayed.
class JMPropertyHistoryEntry: JMActionHistoryEntry
{
	//! Edits to the same property of the same object closer together than this
	//! (seconds) fold into a single step.
	static const float MERGE_WINDOW = 3.0;
	protected Object m_Target;
	protected float m_Before;
	protected float m_After;

	protected string GetPropertyName()
	{
		return "property";
	}

	//! A base constructor cannot dispatch to a subclass's override, so the
	//! subclass hands its inputs over through this instead.
	protected void Init( Object target, string permission, float before, float after )
	{
		RequiredPermission = permission;

		m_Target = target;
		m_Before = before;
		m_After  = after;
	}

	//! Write `value` to the property. Return false when it cannot be applied.
	protected bool Apply( float value )
	{
		return false;
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
		return Apply( m_Before );
	}

	override bool Redo()
	{
		return Apply( m_After );
	}

	override Object GetTarget()
	{
		return m_Target;
	}

	override void SetTarget( Object target )
	{
		m_Target = target;
	}

	override string GetDescription()
	{
		return "Set " + GetPropertyName() + " of " + DescribeObject( m_Target );
	}

	//! Absorb a later edit of the same property on the same object: the
	//! original `before` stays, `after` becomes the latest value.
	override bool TryMerge( JMActionHistoryEntry incoming )
	{
		JMPropertyHistoryEntry other;
		if ( !Class.CastTo( other, incoming ) )
			return false;

		if ( !m_Target || other.Type() != Type() || other.m_Target != m_Target )
			return false;

		if ( other.Time - Time > MERGE_WINDOW )
			return false;

		m_After = other.m_After;
		Time    = other.Time;

		return true;
	}
}
