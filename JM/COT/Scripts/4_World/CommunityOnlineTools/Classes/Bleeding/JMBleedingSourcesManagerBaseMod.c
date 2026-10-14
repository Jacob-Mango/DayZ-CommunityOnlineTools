//  JMBleedingSourcesManagerBaseMod - exposes protected zone enumeration for the
//  Player body-part dropdown. The int-index -> bit mapping in
//  DebugActivateBleedingSource(int) is not aligned with m_BleedingSourceZone keys,
//  so we round-trip by selection name instead.

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