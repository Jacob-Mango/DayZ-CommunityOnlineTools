// =============================================================================
//  JMBleedingSourcesManagerBaseMod.c
//
//  COT-side mod of the vanilla bleeding source manager. Exposes protected
//  zone enumeration so the COT Player module can build a body-part dropdown
//  matching vanilla's actual registration order. Critical: the int-index -> bit
//  mapping in DebugActivateBleedingSource(int) is internal and not aligned
//  with the m_BleedingSourceZone map keys; this mod lets us round-trip by
//  selection name instead, which is guaranteed to match vanilla.
// =============================================================================

modded class BleedingSourcesManagerBase
{
	int COT_GetZoneCount()
	{
		return m_BleedingSourceZone.Count();
	}

	string COT_GetZoneSelectionName( int idx )
	{
		if ( idx < 0 || idx >= m_BleedingSourceZone.Count() )
			return "";
		return m_BleedingSourceZone.GetKey( idx );
	}

	int COT_GetZoneBit( int idx )
	{
		string name = COT_GetZoneSelectionName( idx );
		if ( name == "" )
			return 0;
		BleedingSourceZone zone = m_BleedingSourceZone.Get( name );
		if ( !zone )
			return 0;
		return zone.GetBit();
	}

	// Public wrapper around the protected AddBleedingSource so COT code outside
	// the bleeding subsystem can activate a source by bit. Vanilla checks for
	// DEVELOPER + allowDamage inside AddBleedingSource so this stays safe.
	void COT_ActivateBleedingSource( int bit )
	{
		if ( bit == 0 )
			return;
		AddBleedingSource( bit );
	}
}