/**
 * JMTerritoryCompat.c - shims for JMTerritoryModule -> ExpansionTerritoryModule. Guarded on
 * EXPANSIONMODBASEBUILDING; must be 4_World (a modded class needs its base's layer). Territories
 * come from GetAllTerritoryFlags() (server), not m_Territories. See docs/systems/mod-compatibility.md. */

#ifdef EXPANSIONMODBASEBUILDING

modded class ExpansionTerritory
{
	//! Expansion exposes GetTerritoryLevel() but no setter. Prefixed JM_COT_ to
	//! avoid colliding with any setter Expansion may add later.
	void JM_COT_SetTerritoryLevel( int level )
	{
		TerritoryLevel = level;
	}
}

//! Expansion has no configurable territory level list. Levels are the hardcoded
//! ExpansionTerritoryModule.m_TerritorySize_Level_1/2/3 constants (50/100/150m
//! radius), so 3 is the ceiling. If Expansion ever adds Level_4, update this.
const int JM_COT_MAX_TERRITORY_LEVEL = 3;

modded class ExpansionTerritoryModule
{
	//! Max level an admin may set. Exists so callers do not have to hardcode the
	//! bound or invent a settings field for it (there isn't one).
	int JM_COT_GetMaxTerritoryLevel()
	{
		return JM_COT_MAX_TERRITORY_LEVEL;
	}

	//! Collect every territory known to the server. Uses GetAllTerritoryFlags()
	//! because m_Territories is client-side only -- see file header.
	void JM_COT_GetTerritories( out array< ref ExpansionTerritory > territories )
	{
		if ( !territories )
			return;

		territories.Clear();

		map< int, TerritoryFlag > flags = GetAllTerritoryFlags();
		if ( !flags )
			return;

		foreach ( int territoryID, TerritoryFlag flag: flags )
		{
			if ( !flag )
				continue;

			ExpansionTerritory territory = flag.GetTerritory();
			if ( !territory )
				continue;

			territories.Insert( territory );
		}
	}

	//! Admin-forced territory level change. Permission is already checked by
	//! JMTerritoryModule, so no IsSenderTerritoryAdmin() gate (a COT admin need not be a
	//! member). `sender` is kept for signature symmetry with Expansion's Exec_* methods
	//! and for logging.
	void Exec_AdminSetTerritoryLevel( int territoryID, int newLevel, PlayerIdentity sender )
	{
		map< int, TerritoryFlag > flags = GetAllTerritoryFlags();
		if ( !flags )
			return;

		TerritoryFlag flag = flags.Get( territoryID );
		if ( !flag )
			return;

		ExpansionTerritory territory = flag.GetTerritory();
		if ( !territory )
			return;

		//! Clamp: a level outside Expansion's range would desync the flag size
		//! and mesh lookups. Expansion has no configurable level list -- the
		//! levels are the hardcoded m_TerritorySize_Level_1..3 constants on
		//! this module, so JM_COT_MAX_TERRITORY_LEVEL tracks them.
		if ( newLevel < 1 )
			newLevel = 1;

		if ( newLevel > JM_COT_MAX_TERRITORY_LEVEL )
			newLevel = JM_COT_MAX_TERRITORY_LEVEL;

		territory.JM_COT_SetTerritoryLevel( newLevel );

		//! Refresh the flag entity so its size/mesh follow the new level, and
		//! so CF_OnStoreSave persists it.
		flag.SetSynchDirty();

		//! Push the new level to every member's client copy. UpdateClient
		//! guards on IsMissionHost() and walks the member list itself.
		UpdateClient( territoryID );
	}
}

#endif
