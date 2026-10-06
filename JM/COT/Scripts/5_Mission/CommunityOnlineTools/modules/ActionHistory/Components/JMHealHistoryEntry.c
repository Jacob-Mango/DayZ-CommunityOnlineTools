// =============================================================================
//  JMHealHistoryEntry
//
//  Undo/redo for ESP's Heal action. Restores the health value Heal wiped out
//  (and, for a player, stamina/water) - bleeding sources, broken legs and
//  diseases are not restored, since Heal clears several independent systems
//  at once and only the health value is worth an inverse here.
//
//  Must be constructed BEFORE the target is actually healed.
// =============================================================================
class JMHealHistoryEntry: JMActionHistoryEntry
{
	protected EntityAI m_Target;
	protected float m_Health;
	protected bool m_IsPlayer;
	protected bool m_IncludeAttachments;
	protected bool m_IncludeCargo;
	protected float m_Energy;
	protected float m_Water;

	void JMHealHistoryEntry( EntityAI target, bool includeAttachments, bool includeCargo )
	{
		RequiredPermission = JMConstants.PERM_ESP_OBJECT_HEAL;

		m_Target = target;
		if ( !target )
			return;

		//! TODO/FIXME: In case of including atts/cargo, would need to record the states of the whole hierarchy
		//! TODO/FIXME: Not recording the individual damage zones, but HealEntityRecursive heals ALL damage zones
		m_Health = target.GetHealth( "", "" );

		m_IncludeAttachments = includeAttachments;
		m_IncludeCargo = includeCargo;

		PlayerBase player;
		if ( Class.CastTo( player, target ) )
		{
			m_IsPlayer = true;
			m_Energy = player.GetStatEnergy().Get();
			m_Water  = player.GetStatWater().Get();
		}
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
		if ( !m_Target )
			return false;

		COT.SetHealth( m_Target, "", "", m_Health );

		if ( !m_IsPlayer )
			return true;

		PlayerBase player;
		if ( Class.CastTo( player, m_Target ) )
		{
			player.GetStatEnergy().Set( m_Energy );
			player.GetStatWater().Set( m_Water );
		}

		return true;
	}

	//! Heal again - the entity's own max is still the truth for "healed", so
	//! this replays the same call the action itself made rather than
	//! remembering a number.
	override bool Redo()
	{
		if ( !m_Target )
			return false;

		COT.HealEntityRecursive( m_Target, m_IncludeAttachments, m_IncludeCargo );

		if ( !m_IsPlayer )
			return true;

		PlayerBase player;
		if ( Class.CastTo( player, m_Target ) )
		{
			player.GetStatEnergy().Set( player.GetStatEnergy().GetMax() );
			player.GetStatWater().Set( player.GetStatWater().GetMax() );
		}

		return true;
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
		return "Heal " + DescribeObject( m_Target );
	}
}
