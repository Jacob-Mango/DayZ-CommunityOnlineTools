#ifndef DAYZ_1_29
//! Motorbike itself is the abstract vanilla base (3_Game); MotorbikeScript is
//! the concrete vehicle actually placed/spawned in world (4_World), same
//! relationship as Car -> CarScript and Boat -> BoatScript - so this mods
//! MotorbikeScript, not Motorbike, matching CarScript.c/BoatScript.c.
modded class MotorbikeScript
{
	static ref CF_DoublyLinkedNodes_WeakRef<MotorbikeScript> s_JM_AllBikes = new CF_DoublyLinkedNodes_WeakRef<MotorbikeScript>();
	ref CF_DoublyLinkedNode_WeakRef<MotorbikeScript> s_JM_Node;
	protected bool m_JM_WasDestroyed = false;

	// Tracks the last player who entered the driver seat (non-Expansion fallback)
	string m_JM_LastDriverUID;

	void MotorbikeScript()
	{
		s_JM_Node = s_JM_AllBikes.Add(this);
	}

	void ~MotorbikeScript()
	{
		if (s_JM_AllBikes)
			s_JM_AllBikes.Remove(s_JM_Node);
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

	protected void CheckAndCreateCompensation()
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

	void COT_FillBikeFluid(MotorbikeFluid fluid)
	{
		float fluidCap = GetFluidCapacity(fluid);
		float fluidFraction = GetFluidFraction(fluid);
		if (fluidCap > 0.0 && fluidFraction < 1.0)
			Fill(fluid, fluidCap * (1.0 - fluidFraction));
	}

	//! Move one fluid to an exact fraction of its capacity - see
	//! CarScript::COT_SetCarFluid01 for why both Fill and Leak are needed.
	void COT_SetBikeFluid01( MotorbikeFluid fluid, float fraction )
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

	//! FUEL only - unlike Car, Motorbike has no oil/brake/coolant tanks.
	//! Vanilla's own motorbike_01.c/motorbike_02.c never touch anything but
	//! MotorbikeFluid.FUEL; asking for the others throws "Can't find
	//! variable" at runtime (confirmed on a live server, not caught by a
	//! static compile check).
	void COT_Refuel()
	{
		COT_FillBikeFluid( MotorbikeFluid.FUEL );
	}

	//! MotorbikeWheel extends CarWheel, so the same attachment-slot scan
	//! CarScript uses works unmodified here.
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
}
#endif
