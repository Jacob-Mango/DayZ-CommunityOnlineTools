//  Server-side contaminated area tracking: Expansion's s_ContaminatedAreas is
//  client-only, so we keep our own array, updated on EEInit/EEDelete.

#ifdef EXPANSIONMODMISSIONS

modded class ExpansionContaminatedArea
{
	static ref array<ExpansionContaminatedArea> s_JM_COT_AllAreas = new array<ExpansionContaminatedArea>();

	override void EEInit()
	{
		super.EEInit();
		if ( s_JM_COT_AllAreas.Find( this ) == -1 )
			s_JM_COT_AllAreas.Insert( this );
	}

	override void EEDelete( EntityAI parent )
	{
		s_JM_COT_AllAreas.RemoveItem( this );
		super.EEDelete( parent );
	}
}

#endif
