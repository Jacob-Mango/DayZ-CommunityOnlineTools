modded class CarScript
{
	static ref CF_DoublyLinkedNodes_WeakRef<CarScript> s_JM_AllCars = new CF_DoublyLinkedNodes_WeakRef<CarScript>();

	ref CF_DoublyLinkedNode_WeakRef<CarScript> s_JM_Node;

	void CarScript()
	{
		s_JM_Node = s_JM_AllCars.Add(this);
	}

	void ~CarScript()
	{
		if (s_JM_AllCars)
			s_JM_AllCars.Remove(s_JM_Node);
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

	void COT_ForcePositionAndOrientation(vector position, vector orientation)
	{
		Error("DEPRECATED, use CommunityOnlineToolsBase::ForceTransportPositionAndOrientation");
		CommunityOnlineToolsBase.ForceTransportPositionAndOrientation(this, position, orientation);
	}

	void COT_PlaceOnSurfaceAtPosition(vector position)
	{
		Error("DEPRECATED, use CommunityOnlineToolsBase::PlaceOnSurfaceAtPosition");
		CommunityOnlineToolsBase.PlaceOnSurfaceAtPosition(this, position);
	}

	void COT_Repair()
	{
		Error("DEPRECATED, use CommunityOnlineToolsBase::HealEntityRecursive");
		CommunityOnlineToolsBase.HealEntityRecursive(this);
	}

	void COT_Refuel()
	{
		COT_FillCarFluid( CarFluid.FUEL );
		COT_FillCarFluid( CarFluid.OIL );
		COT_FillCarFluid( CarFluid.BRAKE );
		COT_FillCarFluid( CarFluid.COOLANT );
	}
};

