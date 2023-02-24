modded class Weapon_Base
{
	override void EEFired( int muzzleType, int mode, string ammoType )
	{
		super.EEFired( muzzleType, mode, ammoType );

		PlayerBase player;
		if ( Class.CastTo( player, GetHierarchyRootPlayer() ) && player.COTHasUnlimitedAmmo() )
		{
			Magazine magazine = GetMagazine( GetCurrentMuzzle() );
			if ( magazine )
			{
				if ( GetGame().IsServer() )
				{
					CommunityOnlineToolsBase.HealEntityRecursive(this, true, false);

					if ( GetGame().IsMultiplayer() && magazine )
						GetGame().RemoteObjectDelete( magazine );

					magazine.ServerSetAmmoMax();

					if ( GetGame().IsMultiplayer() && magazine )
						GetGame().RemoteObjectCreate( magazine );
				} else
				{
					magazine.LocalSetAmmoMax();
				}
			}
		}
	}

	TStringArray COTGetMagazineTypesValidated()
	{
		auto magazines = new TStringArray;
		ConfigGetTextArray("magazines", magazines);
		auto magazinesValidated = new TStringArray;
		foreach (string magazine: magazines)
		{
			if (GetGame().ConfigIsExisting("CfgMagazines " + magazine))
				magazinesValidated.Insert(magazine);
		}
		return magazinesValidated;
	}

	string COTGetMaxMagazineTypeName(int muzzleIndex)
	{
		auto magazines = COTGetMagazineTypesValidated();
		int max;
		string magazineType;
		foreach (string magazine: magazines)
		{
			int count = GetGame().ConfigGetInt("CfgMagazines " + magazine + " count");
			if (count > max)
				magazineType = magazine;
		}
		return magazineType;
	}

	//! @note this is a verbatim copy of the vanilla SpawnAttachedMagazine EXCEPT it doesn't use the vanilla GetMaxMagazineTypeName
	//! because those CTD if there are entries in `magazines[]` whose rvConfig classes don't actually exist
	override Magazine SpawnAttachedMagazine( string magazineType = "", int flags = WeaponWithAmmoFlags.CHAMBER )
	{
		// Check if the gun has any magazines registered in config
		if ( GetMagazineTypeCount(0) == 0 )
		{
			ErrorEx(string.Format("No 'magazines' config entry for %1.", this));
			return null;
		}
		
		// Randomize when no specific one is given
		if ( magazineType == "" )
		{
			if ( flags & WeaponWithAmmoFlags.MAX_CAPACITY_MAG)
				magazineType = COTGetMaxMagazineTypeName(0);
			else
				magazineType = GetRandomMagazineTypeName(0);
		}
		
		EntityAI magAI = GetInventory().CreateAttachment(magazineType);
		if (!magAI)
		{
			ErrorEx(string.Format("Failed to create and attach %1 to %2", GetDebugName(magAI), this));
			return null;
		}
		
		Magazine mag;
		if (!CastTo(mag, magAI))
		{
			ErrorEx(string.Format("Expected magazine, created: %1", GetDebugName(magAI)));
			return null;
		}
		
		// Decide random quantity when enabled
		if (flags & WeaponWithAmmoFlags.QUANTITY_RNG)
			mag.ServerSetAmmoCount(Math.RandomIntInclusive(0, mag.GetAmmoMax()));
		
		// Fill chamber when flagged
		bool chamberRng = (flags & WeaponWithAmmoFlags.CHAMBER_RNG);
		bool chamber = (flags & WeaponWithAmmoFlags.CHAMBER) || chamberRng;		
		if (chamber || chamberRng)
		{
			FillChamber(magazineType, flags);
		}

		// FSM cares about magazine state
		RandomizeFSMState();
		Synchronize();

		return mag;
	}
};