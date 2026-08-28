// =============================================================================
//  Server-side tracking for vanilla contaminated / effect areas.
//  EffectArea has no built-in static registry, so we maintain our own.
//  EEInit runs on every host (server + singleplayer).
// =============================================================================

modded class EffectArea
{
	static ref array<EffectArea> s_JM_COT_AllAreas = new array<EffectArea>();

	override void EEInit()
	{
		super.EEInit();
		if ( GetGame().IsServer() && s_JM_COT_AllAreas.Find( this ) == -1 )
			s_JM_COT_AllAreas.Insert( this );
	}

	override void EEDelete( EntityAI parent )
	{
		s_JM_COT_AllAreas.RemoveItem( this );
		super.EEDelete( parent );
	}
}
