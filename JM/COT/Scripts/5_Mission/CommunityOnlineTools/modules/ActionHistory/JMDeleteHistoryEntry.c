// =============================================================================
//  JMDeleteHistoryEntry
//
//  Undo/redo for ESP's generic object delete. For an EntityAI, the snapshot
//  is the same full recursive capture JMLoadoutModule uses for its own
//  deletion backups - classname, health/quantity/liquid/temperature, and
//  every attachment/cargo item recursively - so undoing a delete brings back
//  what was inside or attached to it, not just an empty shell. A plain
//  Object (not an EntityAI) falls back to classname + transform only, since
//  the loadout system has nothing to capture on one.
//
//  Must be constructed BEFORE the target is actually deleted - it reads the
//  target's own state to know what to bring back.
// =============================================================================
class JMDeleteHistoryEntry: JMActionHistoryEntry
{
	protected ref JMLoadoutItem m_Snapshot;
	protected string m_ClassName;
	protected vector m_Position;
	protected vector m_Orientation;
	protected float  m_Scale;

	//! The stand-in Undo() creates, so Redo() has something to delete again.
	protected EntityAI m_Live;

	void JMDeleteHistoryEntry( Object target )
	{
		RequiredPermission = "ESP.Object.Delete";

		if ( !target )
			return;

		m_Position    = target.GetPosition();
		m_Orientation = target.GetOrientation();
		m_Scale       = target.GetScale();

		EntityAI entity;
		if ( !Class.CastTo( entity, target ) )
		{
			//! Not an EntityAI (a plain world Object) - nothing for the
			//! loadout system to capture, so this is transform-only.
			m_ClassName = target.GetType();
			return;
		}

		JMLoadoutModule loadoutModule;
		if ( CF_Modules<JMLoadoutModule>.Get( loadoutModule ) )
			m_Snapshot = loadoutModule.LoadoutProcessItem( entity, m_Position, m_Orientation );

		if ( !m_Snapshot )
			m_ClassName = target.GetType();
	}

	override void Undo()
	{
		Object obj;

		if ( m_Snapshot )
		{
			JMLoadoutModule loadoutModule;
			if ( !CF_Modules<JMLoadoutModule>.Get( loadoutModule ) )
				return;

			obj = loadoutModule.SpawnItem( m_Snapshot, m_Position, false );
		}
		else if ( m_ClassName != "" )
		{
			obj = g_Game.CreateObjectEx( m_ClassName, m_Position, ECE_PLACE_ON_SURFACE );
			if ( obj )
				obj.SetOrientation( m_Orientation );
		}

		if ( !obj )
			return;

		obj.SetScale( m_Scale );
		Class.CastTo( m_Live, obj );
	}

	override void Redo()
	{
		if ( !m_Live )
			return;

		g_Game.ObjectDelete( m_Live );
		m_Live = NULL;
	}
}
