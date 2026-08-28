// =============================================================================
//  Airdrop tracking - modded class keeps a global list of active airdrop
//  container objects so the Events Manager can enumerate them.
//
//  ExpansionAirdropContainerBase is the base class for every airdrop loot box.
//  We hook EEInit / EEDelete and push/pop ourselves in a static array.
// =============================================================================

#ifdef EXPANSIONMODMISSIONS

modded class ExpansionAirdropContainerBase
{
	static ref array<ExpansionAirdropContainerBase> s_JM_COT_Active = new array<ExpansionAirdropContainerBase>();

	override void EEInit()
	{
		super.EEInit();
		if ( GetGame().IsServer() && s_JM_COT_Active.Find( this ) == -1 )
			s_JM_COT_Active.Insert( this );
	}

	override void EEDelete( EntityAI parent )
	{
		s_JM_COT_Active.RemoveItem( this );
		super.EEDelete( parent );
	}
}

#endif
