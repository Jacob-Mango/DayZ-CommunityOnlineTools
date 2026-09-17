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
	protected float m_Energy;
	protected float m_Water;

	void JMHealHistoryEntry( EntityAI target )
	{
		RequiredPermission = "ESP.Object.Heal";

		m_Target = target;
		if ( !target )
			return;

		m_Health = target.GetHealth( "", "" );

		PlayerBase player;
		if ( Class.CastTo( player, target ) )
		{
			m_IsPlayer = true;
			m_Energy = player.GetStatEnergy().Get();
			m_Water  = player.GetStatWater().Get();
		}
	}

	override void Undo()
	{
		if ( !m_Target )
			return;

		m_Target.SetHealth( "", "", m_Health );

		if ( !m_IsPlayer )
			return;

		PlayerBase player;
		if ( Class.CastTo( player, m_Target ) )
		{
			player.GetStatEnergy().Set( m_Energy );
			player.GetStatWater().Set( m_Water );
		}
	}

	//! Heal again - the entity's own max is still the truth for "healed", so
	//! this replays the same call the action itself made rather than
	//! remembering a number.
	override void Redo()
	{
		if ( !m_Target )
			return;

		CommunityOnlineToolsBase.HealEntityRecursive( m_Target );

		if ( !m_IsPlayer )
			return;

		PlayerBase player;
		if ( Class.CastTo( player, m_Target ) )
		{
			player.GetStatEnergy().Set( player.GetStatEnergy().GetMax() );
			player.GetStatWater().Set( player.GetStatWater().GetMax() );
		}
	}
}
