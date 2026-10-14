//  JMCompositeHistoryEntry - one undo step of several (bulk delete = one Ctrl+Z).
//  ONE RequiredPermission for the group, children not re-checked. Undo runs
//  newest-first, redo oldest-first; succeeds if any child did, children that
//  lost their target are skipped.
class JMCompositeHistoryEntry: JMActionHistoryEntry
{
	protected string m_Description;
	protected ref array<ref JMActionHistoryEntry> m_Children = new array<ref JMActionHistoryEntry>;

	void JMCompositeHistoryEntry( string permission, string description )
	{
		RequiredPermission = permission;
		m_Description = description;
	}

	void Add( JMActionHistoryEntry child )
	{
		if ( child )
			m_Children.Insert( child );
	}

	int Count()
	{
		return m_Children.Count();
	}

	override bool CanUndo()
	{
		foreach ( JMActionHistoryEntry child: m_Children )
		{
			if ( child.CanUndo() )
				return true;
		}

		return false;
	}

	override bool CanRedo()
	{
		foreach ( JMActionHistoryEntry child: m_Children )
		{
			if ( child.CanRedo() )
				return true;
		}

		return false;
	}

	override bool Undo()
	{
		bool any;

		for ( int i = m_Children.Count() - 1; i >= 0; i-- )
		{
			if ( m_Children[i].CanUndo() && m_Children[i].Undo() )
				any = true;
		}

		return any;
	}

	override bool Redo()
	{
		bool any;

		for ( int i = 0; i < m_Children.Count(); i++ )
		{
			if ( m_Children[i].CanRedo() && m_Children[i].Redo() )
				any = true;
		}

		return any;
	}

	//! The group has no single target; the children answer for themselves.
	override void CollectTargeting( Object target, array<JMActionHistoryEntry> results )
	{
		foreach ( JMActionHistoryEntry child: m_Children )
			child.CollectTargeting( target, results );
	}

	override string GetDescription()
	{
		return m_Description + " (" + m_Children.Count() + ")";
	}
}
