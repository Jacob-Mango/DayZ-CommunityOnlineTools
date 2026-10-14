//  Airdrop tracking: hooks EEInit / EEDelete on ExpansionAirdropContainerBase (base of every
//  airdrop loot box) to keep a static list of active containers for the Events Manager.

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
