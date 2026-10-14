modded class CarScript
{
	static ref CF_DoublyLinkedNodes_WeakRef<CarScript> s_JM_AllCars = new CF_DoublyLinkedNodes_WeakRef<CarScript>();
	ref CF_DoublyLinkedNode_WeakRef<CarScript> s_JM_Node;
	protected bool m_JM_WasDestroyed = false;

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
			JMCompensationHelper.CreateCompensationBackup(this, ownerSteamID);
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
	//! Fill only adds and Leak only removes, so pick by current level; a fill-only
	//! "set to 25%" would do nothing to a full tank.
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

	//! Car doors are not openable items (ItemBase.Open()/IsOpen() are unoverridden
	//! stubs, IsOpen() is always true). They move by animation phase on a model
	//! selection (vanilla ActionCarDoors), which is per-vehicle, so brute-force every
	//! selection like Transport.GetSelectionFromAnimSource.
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

