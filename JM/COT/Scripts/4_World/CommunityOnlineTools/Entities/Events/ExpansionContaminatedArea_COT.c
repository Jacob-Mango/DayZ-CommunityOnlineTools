// =============================================================================
//  Server-side tracking for contaminated areas. Expansion's vanilla
//  s_ContaminatedAreas list is client-only, so we keep our own array and
//  push/pop ourselves on EEInit/EEDelete on every host.
// =============================================================================

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
