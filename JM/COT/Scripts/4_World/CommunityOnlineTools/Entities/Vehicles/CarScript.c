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
	override void OnDebugSpawn()
	{
	}

	void COT_OnDebugSpawn(PlayerBase player)
	{
		OnDebugSpawn();

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
		if (dBodyIsActive(this))
		{
			vector velocity = GetVelocity(this);
			vector angularVelocity = dBodyGetAngularVelocity(this);

			dBodyActive(this, ActiveState.INACTIVE);
			dBodyDynamic(this, false);

			SetPosition(position);
			SetOrientation(orientation);

			SetVelocity(this, velocity);
			dBodySetAngularVelocity(this, angularVelocity);

			dBodyDynamic(this, true);
		}
		else
		{
			//! Force position/orientation without setting vehicle active (avoids collision)
			for (int i = 0; i < 2; i++)
			{
				SetPosition(position);
				orientation[2] = orientation[2] - 1;
				SetOrientation(orientation);
				orientation[2] = orientation[2] + 1;
				SetOrientation(orientation);
				Synchronize();
			}
		}
	}

	void COT_PlaceOnSurfaceAtPosition(vector position)
	{
		vector surface = Vector(position[0], GetGame().SurfaceY(position[0], position[2]), position[2]);

		vector entityMinMax[2];
		if (!GetCollisionBox(entityMinMax))
			ClippingInfo(entityMinMax);

		float entityOffsetY = entityMinMax[0][1];
		if (entityOffsetY > 0)
			entityOffsetY = 0;

		vector startPos = position;
		startPos[1] = startPos[1] - entityOffsetY;
		if (surface[1] > startPos[1])
			startPos[1] = surface[1];

		PhxInteractionLayers layerMask;
		layerMask |= PhxInteractionLayers.BUILDING;
		layerMask |= PhxInteractionLayers.DOOR;
		layerMask |= PhxInteractionLayers.VEHICLE;
		layerMask |= PhxInteractionLayers.ROADWAY;
		layerMask |= PhxInteractionLayers.TERRAIN;
		layerMask |= PhxInteractionLayers.ITEM_LARGE;
		layerMask |= PhxInteractionLayers.FENCE;
		vector hitPosition;
		vector hitNormal;

		if (DayZPhysics.RayCastBullet(startPos + "0 1 0", surface - "0 1 0", layerMask, this, null, hitPosition, hitNormal, null))
		{
			position = hitPosition;
		} else {
			position = surface;
			hitNormal = GetGame().SurfaceGetNormal(surface[0], surface[2]);
		}

		position[1] = position[1] - entityOffsetY;

		bool isActive = dBodyIsActive(this);
		bool isDynamic = dBodyIsDynamic(this);

		vector velocity = GetVelocity(this);
		vector angularVelocity = dBodyGetAngularVelocity(this);

		if (isActive)
		{
			dBodyActive(this, ActiveState.INACTIVE);
			dBodyDynamic(this, false);
		}

		vector orientation = GetOrientation();
		SetOrientation(Vector(orientation[0], 0, 0));

		vector transform[4];
		GetTransform(transform);
		transform[3] = position;
		PlaceOnSurfaceRotated(transform, position, hitNormal[0] * -1, hitNormal[2] * -1, 0, true);
		SetTransform(transform);

		if (isActive)
		{
			SetVelocity(this, velocity);
			dBodySetAngularVelocity(this, angularVelocity);
			dBodyDynamic(this, true);
		}
		else
		{
			COT_ForcePositionAndOrientation(position, GetOrientation());
		}
	}

	void COT_Repair()
	{
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

