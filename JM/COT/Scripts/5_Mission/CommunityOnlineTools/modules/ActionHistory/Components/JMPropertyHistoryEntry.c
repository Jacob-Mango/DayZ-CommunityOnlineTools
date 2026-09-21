// =============================================================================
//  JMPropertyHistoryEntry
//
//  Base for "one number on one object was changed": health, quantity, liquid
//  type, fuel. A subclass says how to write one value (Apply) and what to
//  call it; this class supplies the before/after bookkeeping, the Undo/Redo
//  pair, and the merge that folds a burst of edits to the same property (a
//  slider dragged through ten values) into one step.
//
//  Construct it AFTER the change with both values - `after` should be read
//  back from the object, not taken from the request, so any clamping the
//  action applied is what redo replays:
//
//      float before = target.GetHealth( "", "" );
//      ... apply the change ...
//      JMActionHistory.Push( new JMHealthHistoryEntry( target, before, target.GetHealth( "", "" ) ), owner );
// =============================================================================
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
