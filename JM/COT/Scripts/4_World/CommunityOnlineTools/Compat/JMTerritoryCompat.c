/**
 * JMTerritoryCompat.c
 *
 * Compatibility shims for JMTerritoryModule -> ExpansionTerritoryModule.
 *
 * JMTerritoryModule calls two methods that DayZ-Expansion does not ship:
 *
 *   Exec_AdminSetTerritoryLevel( territoryID, newLevel, sender )
 *   JM_COT_GetTerritories( out territories )
 *
 * Without them the Mission module fails to compile:
 *
 *   jmterritorymodule.c(169): Undefined function
 *   'ExpansionTerritoryModule.Exec_AdminSetTerritoryLevel'
 *
 * Rather than stub them empty, they are implemented here against Expansion's
 * real API via `modded class`.
 *
 * Two details drive the implementation:
 *
 *  - Territories come from GetAllTerritoryFlags() ("Called server side"), NOT
 *    from m_Territories. The latter is client-only ("Contains only territories
 *    which a client is member of"), populated solely by Exec_UpdateClient, so
 *    it is empty on the server where this admin path runs.
 *
 *  - ExpansionTerritory.TerritoryLevel is protected with a getter but no
 *    setter, so JM_COT_SetTerritoryLevel is added alongside. Persistence needs
 *    no explicit save: the level lives on the flag entity and is written by
 *    TerritoryFlag.CF_OnStoreSave -> ExpansionTerritory.OnStoreSave.
 *
 * Guarded on EXPANSIONMODBASEBUILDING to match JMTerritoryModule, so COT still
 * builds standalone (Expansion is not in COT's requiredAddons).
 *
 * LAYER: must live in 4_World, not 5_Mission, because ExpansionTerritory and
 * ExpansionTerritoryModule are both 4_World classes -- a modded class has to be
 * declared in the same layer as its base. Placing this in 5_Mission fails with
 * "Unknown type 'ExpansionTerritory'". JMTerritoryModule (5_Mission) can still
 * call into it: later layers see earlier ones, not the reverse.
 *
 * See docs/systems/mod-compatibility.md.
 */

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

		for ( int i = 0; i < flags.Count(); i++ )
		{
			TerritoryFlag flag = flags.GetElement( i );
			if ( !flag )
				continue;

			ExpansionTerritory territory = flag.GetTerritory();
			if ( !territory )
				continue;

			territories.Insert( territory );
		}
	}

	//! Admin-forced territory level change. Permission is already checked by
	//! JMTerritoryModule ("Expansion.Territory.SetLevel") before this is
	//! called, so unlike Exec_DeleteTerritoryAdmin there is no
	//! IsSenderTerritoryAdmin() gate -- a COT admin is not necessarily a
	//! member of the territory being edited. `sender` is kept for signature
	//! symmetry with Expansion's other Exec_* admin methods and for logging.
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
