modded class CarScript
{
	static ref CF_DoublyLinkedNodes_WeakRef<CarScript> s_JM_AllCars = new CF_DoublyLinkedNodes_WeakRef<CarScript>();

	ref CF_DoublyLinkedNode_WeakRef<CarScript> s_JM_Node;
	private bool m_JM_WasDestroyed = false;

	// Tracks the last player who entered the driver seat (non-Expansion fallback)
	string m_JM_LastDriverUID;

	void CarScript()
	{
		s_JM_Node = s_JM_AllCars.Add(this);
	}

	void ~CarScript()
	{
		if (s_JM_AllCars)
			s_JM_AllCars.Remove(s_JM_Node);
	}

	#ifndef EXPANSIONMODVEHICLE
	override void OnDriverEnter( Human player )
	{
		super.OnDriverEnter( player );

		if ( g_Game.IsServer() )
		{
			PlayerBase driver;
			if ( Class.CastTo( driver, player ) && driver.GetIdentity() )
				m_JM_LastDriverUID = driver.GetIdentity().GetId();
		}
	}
	#endif

	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		super.EEHealthLevelChanged(oldLevel, newLevel, zone);

		if (g_Game.IsServer() && !m_JM_WasDestroyed && newLevel == GameConstants.STATE_RUINED)
		{
			m_JM_WasDestroyed = true;
			CheckAndCreateCompensation();
		}
	}

	private void CheckAndCreateCompensation()
	{
		string ownerSteamID = "";

		// Try to get driver's steam ID
		Human driver = CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER);
		if (driver)
		{
			PlayerBase driverPlayer;
			if (Class.CastTo(driverPlayer, driver))
			{
				PlayerIdentity identity = driverPlayer.GetIdentity();
				if (identity)
					ownerSteamID = identity.GetPlainId();
			}
		}

		// If no driver, try Expansion owner UID
		#ifdef EXPANSIONMODVEHICLE
		if (ownerSteamID == "")
		{
			ExpansionVehicle vehicle;
			if (ExpansionVehicle.Get(vehicle, this))
			{
				ownerSteamID = vehicle.GetOwnerUID();
			}
		}
		#endif

		if (ownerSteamID != "")
		{
			JMCompensationHelper.CreateVehicleCompensationBackup(this, ownerSteamID);
		}
	}

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef))
			return false;

		PlayerBase sourcePlayer;
		if (source && Class.CastTo(sourcePlayer, source.GetHierarchyRootPlayer()) && sourcePlayer.COTGetReceiveDamageDealt())
		{
			sourcePlayer.ProcessDirectDamage(damageType, source, "Torso", ammo, "0 0 0", speedCoef);
			return false;
		}

		Human driver = CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER);
		if (driver && !driver.GetAllowDamage())
			return false;

		return true;
	}

	//! Prevent calling vanilla EntityAI::OnDebugSpawn
	//! Logic will use COT to spawn attachments, so it'll work like vanilla EXCEPT not have possibility of crashes due to bad items
	override void OnDebugSpawn()
	{
		GetCommunityOnlineToolsBase().SpawnCompatibleAttachments(this, null, 0);
	}

	void COT_OnDebugSpawn(PlayerBase player)
	{
		OnDebugSpawnEx(DebugSpawnParams.WithPlayer(player));

		COT_Refuel();
	}

	void COT_FillCarFluid(CarFluid fluid)
	{
		float fluidCap = GetFluidCapacity(fluid);
		float fluidFraction = GetFluidFraction(fluid);
		if (fluidCap > 0.0 && fluidFraction < 1.0)
			Fill(fluid, fluidCap * (1.0 - fluidFraction));
	}

	//! Move one fluid to an exact fraction of its capacity.
	//!
	//! Fill only ever adds and Leak only ever takes away, so which of the two
	//! runs depends on where the tank already is - a "set to 25%" that only
	//! filled would do nothing to a full tank.
	void COT_SetCarFluid01( CarFluid fluid, float fraction )
	{
		float fluidCap = GetFluidCapacity( fluid );
		if ( fluidCap <= 0.0 )
			return;

		float target = Math.Clamp( fraction, 0.0, 1.0 );
		float delta = ( target - GetFluidFraction( fluid ) ) * fluidCap;

		if ( delta > 0.0 )
			Fill( fluid, delta );
		else if ( delta < 0.0 )
			Leak( fluid, -delta );
	}

	void COT_Refuel()
	{
		COT_FillCarFluid( CarFluid.FUEL );
		COT_FillCarFluid( CarFluid.OIL );
		COT_FillCarFluid( CarFluid.BRAKE );
		COT_FillCarFluid( CarFluid.COOLANT );
	}

	void COT_RefillCoolant()
	{
		COT_FillCarFluid( CarFluid.COOLANT );
	}

	void COT_SetLockWheels( bool lockState )
	{
		if ( !GetInventory() )
			return;

		int count = GetInventory().GetAttachmentSlotsCount();
		for ( int i = 0; i < count; ++i )
		{
			int slotId = GetInventory().GetAttachmentSlotId( i );
			if ( slotId != InventorySlots.INVALID )
			{
				string slotName = InventorySlots.GetSlotName( slotId );
				slotName.ToLower();
				EntityAI att = GetInventory().FindAttachment( slotId );
				bool isWheel = slotName.Contains( "wheel" );
				if ( !isWheel && att )
				{
					if ( att.IsInherited( CarWheel ) )
						isWheel = true;
				}

				if ( isWheel )
				{
					GetInventory().SetSlotLock( slotId, lockState );
				}
			}
		}
	}

	bool COT_AreWheelsLocked()
	{
		if ( !GetInventory() )
			return false;

		int count = GetInventory().GetAttachmentSlotsCount();
		for ( int i = 0; i < count; ++i )
		{
			int slotId = GetInventory().GetAttachmentSlotId( i );
			if ( slotId != InventorySlots.INVALID )
			{
				string slotName = InventorySlots.GetSlotName( slotId );
				slotName.ToLower();
				EntityAI att = GetInventory().FindAttachment( slotId );
				bool isWheel = slotName.Contains( "wheel" );
				if ( !isWheel && att )
				{
					if ( att.IsInherited( CarWheel ) )
						isWheel = true;
				}

				if ( isWheel && GetInventory().GetSlotLock( slotId ) )
				{
					return true;
				}
			}
		}

		return false;
	}

	//! Car doors are NOT separate openable inventory items - CarDoor
	//! attachments are hitzone/health parts, and ItemBase.Open()/Close()/
	//! IsOpen() are stub declarations for lidded CONTAINERS (cans, barrels)
	//! that CarDoor never overrides, so calling them here was always a no-op,
	//! and IsOpen()'s base default unconditionally returns true - which is why
	//! detection always read every door as "open" too. Doors open/close by
	//! setting animation phase on a model selection (see vanilla
	//! ActionCarDoors.OnStartServer), and which selections exist is
	//! per-vehicle (Van_01 has "doors_cargo3", a sedan doesn't, etc.) - so
	//! this brute-forces every selection the model actually has, the same way
	//! Transport.GetSelectionFromAnimSource does, instead of guessing names.
	protected void COT_ForEachDoorAnimSource( out array<string> animSources )
	{
		animSources = new array<string>();

		array<string> allSelections = new array<string>();
		GetSelectionList( allSelections );

		foreach ( string selection : allSelections )
		{
			string selectionLower = selection;
			selectionLower.ToLower();

			if ( !selectionLower.Contains( "door" ) && !selectionLower.Contains( "hood" ) && !selectionLower.Contains( "trunk" ) )
				continue;

			string animSource = GetAnimSourceFromSelection( selection );
			if ( animSource != "" && animSources.Find( animSource ) == -1 )
				animSources.Insert( animSource );
		}
	}

	void COT_SetCarDoors( bool openState )
	{
		array<string> animSources;
		COT_ForEachDoorAnimSource( animSources );

		float phase = 0.0;
		if ( openState )
			phase = 1.0;

		foreach ( string animSource : animSources )
		{
			SetAnimationPhase( animSource, phase );
		}
	}

	bool COT_AreCarDoorsOpen()
	{
		array<string> animSources;
		COT_ForEachDoorAnimSource( animSources );

		foreach ( string animSource : animSources )
		{
			if ( GetAnimationPhase( animSource ) > 0.5 )
				return true;
		}

		return false;
	}
}

