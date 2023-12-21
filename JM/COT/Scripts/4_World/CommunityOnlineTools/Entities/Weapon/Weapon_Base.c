modded class Weapon_Base
{
	override void EEFired( int muzzleType, int mode, string ammoType )
	{
		super.EEFired( muzzleType, mode, ammoType );

		PlayerBase player;
		if ( Class.CastTo( player, GetHierarchyRootPlayer() ) )
		{
			if (GetGame().IsServer() && player.COTHasGodMode())
			{
				CommunityOnlineToolsBase.HealEntityRecursive(this, true, false);
			}

			if (player.COTHasUnlimitedAmmo())
			{
				Magazine magazine = GetMagazine( GetCurrentMuzzle() );
				if ( magazine )
				{
					if ( GetGame().IsServer() )
					{
						magazine.ServerSetAmmoMax();
					}
					else
					{
						magazine.LocalSetAmmoMax();
					}
				}
			}
		}
	}

	TStringArray COTGetMagazineTypesValidated(string name = "magazines")
	{
		auto magazines = new TStringArray;
		ConfigGetTextArray(name, magazines);
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
			{
				max = count;
				magazineType = magazine;
			}
		}
		return magazineType;
	}

	//! @note this is a verbatim copy of the vanilla SpawnAttachedMagazine EXCEPT it doesn't use the vanilla GetMaxMagazineTypeName/GetRandomMagazineTypeName
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
				magazineType = COTGetMagazineTypesValidated().GetRandomElement();
		}
		
		if (!magazineType)
			return null;

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
		if ((flags & WeaponWithAmmoFlags.CHAMBER) || (flags & WeaponWithAmmoFlags.CHAMBER_RNG))
		{
			FillChamber(magazineType, flags);
		}

		// FSM cares about magazine state
		RandomizeFSMState();
		Synchronize();

		return mag;
	}

	//! @note this is a verbatim copy of the vanilla FillInnerMagazine EXCEPT it doesn't use the vanilla GetRandomChamberableAmmoTypeName
	//! because that CTDs if there are entries in `chamberableFrom[]` whose rvConfig classes don't actually exist
	override bool FillInnerMagazine( string ammoType = "", int flags = WeaponWithAmmoFlags.CHAMBER )
	{
		// Don't try to fill it when there are none
		if (!HasInternalMagazine(-1))
			return false;
	
		// Make sure the given ammoType is actually useable
		if (ammoType != "")
		{
			if (!AmmoTypesAPI.MagazineTypeToAmmoType(ammoType, ammoType))
				return false;
		}
		
		
		bool didSomething = false;		
		int muzzCount = GetMuzzleCount();
		
		bool ammoRng = ammoType == "";
		bool ammoFullRng = ammoRng && (flags & WeaponWithAmmoFlags.AMMO_MAG_RNG);
		
		// No full RNG flag, so pick one random and use only this one
		if (ammoRng && !ammoFullRng)
			ammoType = COTGetMagazineTypesValidated("chamberableFrom").GetRandomElement();
		
		// Fill the internal magazine
		for (int i = 0; i < muzzCount; ++i)
		{
			int ammoCount = GetInternalMagazineMaxCartridgeCount(i);
			
			// Decide random quantity when enabled
			if ( flags & WeaponWithAmmoFlags.QUANTITY_RNG )
				ammoCount = Math.RandomIntInclusive(0, ammoCount);
			
			// Only do the things when there is actually ammo to fill
			if (ammoCount > 0)
			{
				// Push in the cartridges
				for (int j = 0; j < ammoCount; ++j)
				{
					// Full random, decide a new one for every cartridge
					if ( ammoFullRng )
						ammoType = COTGetMagazineTypesValidated("chamberableFrom").GetRandomElement();
					
					if (!ammoType)
						continue;

					PushCartridgeToInternalMagazine(i, 0, ammoType);
					didSomething = true;
				}
			}
		}
		
		// Call the chamber method if asked for
		bool chamber = (flags & WeaponWithAmmoFlags.CHAMBER) || (flags & WeaponWithAmmoFlags.CHAMBER_RNG);
		if (chamber && FillChamber(ammoType, flags))
		{
			didSomething = true;
		}
		
		// Does not need any FSM fixing, FSM does not care about inner magazines
		
		return didSomething;
	}

	//! @note this is a verbatim copy of the vanilla FillChamber EXCEPT it doesn't use the vanilla GetRandomChamberableAmmoTypeName
	//! because that CTDs if there are entries in `chamberableFrom[]` whose rvConfig classes don't actually exist
	override bool FillChamber( string ammoType = "", int flags = WeaponWithAmmoFlags.CHAMBER )
	{
		// Quickly check if there are any chambers we can fill
		int muzzCount = GetMuzzleCount();
		bool anyEmpty = false;
		
		for (int m = 0; m < muzzCount; ++m)
		{
			if (IsChamberEmpty(m))
			{
				anyEmpty = true;
				break;
			}
		}

		if (!anyEmpty)
			return false;
		
		// Make sure the given ammoType is actually useable
		if (ammoType != "")
			if (!AmmoTypesAPI.MagazineTypeToAmmoType(ammoType, ammoType))
				return false;
		
		// Just so we don't '&' wastefully in a loop
		bool didSomething = false;		
		bool chamberFullRng = (flags & WeaponWithAmmoFlags.CHAMBER_RNG_SPORADIC);
		bool chamberRng = (flags & WeaponWithAmmoFlags.CHAMBER_RNG);
		bool chamber = (flags & WeaponWithAmmoFlags.CHAMBER);
		
		if (chamber || chamberRng || chamberFullRng)
		{
			int amountToChamber = muzzCount;
			
			// No need to do this for full rng, as that will roll for every muzzle
			if (chamberRng)
				amountToChamber = Math.RandomIntInclusive(0, muzzCount);
			
			bool chamberAmmoRng = (ammoType == "");
			bool chamberAmmoFullRng = chamberAmmoRng && (flags & WeaponWithAmmoFlags.AMMO_CHAMBER_RNG);
			
			// No full RNG flag, so pick one random and use only this one
			if (chamberAmmoRng && !chamberAmmoFullRng)
				ammoType = COTGetMagazineTypesValidated("chamberableFrom").GetRandomElement();
			
			for (int i = 0; i < muzzCount; ++i)
			{
				// Skip when there's already something in the chamber
				if (!IsChamberEmpty(i))
					continue;
				
				// Roll the rng when enabled
				if (chamberFullRng)
					chamber = Math.RandomIntInclusive(0, 1);
				
				// We chambering
				if (chamber)
				{
					// Full random, decide a new one for every muzzle
					if ( chamberAmmoFullRng )
						ammoType = COTGetMagazineTypesValidated("chamberableFrom").GetRandomElement();
					
					if (!ammoType)
						continue;

					// Push it
					PushCartridgeToChamber(i, 0, ammoType);
					didSomething = true;
					
					// Stop chambering when we hit the desired amount
					--amountToChamber;				
					if (amountToChamber <= 0)
						break;
				}
			}
		}
		
		// Only fix the FSM and Synchronize when absolutely needed
		if (!didSomething)
			return false;
		
		// FSM cares about chamber state
		RandomizeFSMState();		
		Synchronize();
		
		return true;
	}
};