// =============================================================================
//  JMSnapshotHistoryEntry
//
//  Shared machinery for the two entries that move an object in and out of the
//  world: JMDeleteHistoryEntry (undo brings it back) and JMSpawnHistoryEntry
//  (undo removes it). Both are the same pair of operations run in opposite
//  order, so they share one implementation of each.
//
//  For an EntityAI, the snapshot is the same full recursive capture
//  JMLoadoutModule uses for its own deletion backups - classname, health/
//  quantity/liquid/temperature, and every attachment/cargo item recursively -
//  so a restored object has what was inside or attached to it, not just an
//  empty shell. A plain Object (not an EntityAI) falls back to classname +
//  transform only, since the loadout system has nothing to capture on one.
//
//  A restored object is a NEW object. Every other entry on the stack that
//  pointed at the old one is recorded (m_Dependents) before it goes away and
//  handed the replacement, so an earlier "move this crate" or "heal this
//  vehicle" still works after the crate or vehicle is deleted and un-deleted.
// =============================================================================
class JMSnapshotHistoryEntry: JMActionHistoryEntry
{
	protected ref JMLoadoutItem m_Snapshot;
	protected string m_ClassName;
	protected vector m_Position;
	protected vector m_Orientation;
	protected float  m_Scale = 1.0;

	//! The object currently in the world, when there is one. Not a `ref`: it
	//! is the engine's object, and goes NULL by itself if something else
	//! deletes it.
	protected EntityAI m_Live;

	//! Entries that referenced the object when it was last removed. Elements
	//! are weak - an entry aged off the stack simply reads as NULL here.
	protected ref array<JMActionHistoryEntry> m_Dependents = new array<JMActionHistoryEntry>;

	protected bool HasSnapshot()
	{
		return m_Snapshot || m_ClassName != "";
	}

	//! Record the object's transform and, where possible, everything on it.
	//! Called at construction and again before every removal, so a restore
	//! reflects the object as it was last, not as it was first seen.
	protected void Capture( Object target )
	{
		if ( !target )
			return;

		m_Position    = target.GetPosition();
		m_Orientation = target.GetOrientation();
		m_Scale       = target.GetScale();
		m_ClassName   = target.GetType();

		EntityAI entity;
		if ( !Class.CastTo( entity, target ) )
			return;

		JMLoadoutModule loadoutModule;
		if ( !CF_Modules<JMLoadoutModule>.Get( loadoutModule ) )
			return;

		JMLoadoutItem snapshot = loadoutModule.LoadoutProcessItem( entity, m_Position, m_Orientation );
		if ( snapshot )
			m_Snapshot = snapshot;
	}

	//! Remember every entry that acts on `obj`, to be pointed at its
	//! replacement once the object is restored.
	protected void CollectDependents( Object obj )
	{
		m_Dependents.Clear();
		JMActionHistory.FindEntriesTargeting( obj, m_Dependents );

		int index = m_Dependents.Find( this );
		if ( index != -1 )
			m_Dependents.RemoveOrdered( index );
	}

	//! Bring the object back into the world.
	protected bool Restore()
	{
		if ( m_Live )
			return false;

		Object obj;

		if ( m_Snapshot )
		{
			JMLoadoutModule loadoutModule;
			if ( !CF_Modules<JMLoadoutModule>.Get( loadoutModule ) )
				return false;

			obj = loadoutModule.SpawnItem( m_Snapshot, m_Position, false );
		}
		else if ( m_ClassName != "" )
		{
			obj = g_Game.CreateObjectEx( m_ClassName, m_Position, ECE_PLACE_ON_SURFACE );
			if ( obj )
				obj.SetOrientation( m_Orientation );
		}

		if ( !obj )
			return false;

		obj.SetScale( m_Scale );
		Class.CastTo( m_Live, obj );

		foreach ( JMActionHistoryEntry dependent: m_Dependents )
		{
			if ( dependent )
				dependent.SetTarget( obj );
		}

		return true;
	}

	//! Take the object out of the world again.
	protected bool Remove()
	{
		if ( !m_Live )
			return false;

		Capture( m_Live );
		CollectDependents( m_Live );

		g_Game.ObjectDelete( m_Live );
		m_Live = NULL;

		return true;
	}

	override Object GetTarget()
	{
		return m_Live;
	}

	//! This entry's own object was replaced by a restore of another entry.
	override void SetTarget( Object target )
	{
		Class.CastTo( m_Live, target );
	}

	override string GetDescription()
	{
		return m_ClassName;
	}
}
