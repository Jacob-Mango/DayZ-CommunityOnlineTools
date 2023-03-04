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

		return true;
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
		for ( int j = 0; j < GetInventory().AttachmentCount(); j++ )
		{
			ItemBase attachment = ItemBase.Cast(GetInventory().GetAttachmentFromIndex(j));
			if ( attachment )
			{
				attachment.SetHealthMax("","");
				if ( attachment.IsInherited(CarWheel_Ruined) )
				{
					string att_str = attachment.GetType();
					string new_attachment = att_str.Substring(0, att_str.Length() - 7);

					if ( GetGame().IsKindOf(new_attachment, "CarWheel") )
					{
						if ( attachment.IsLockedInSlot() )
						{
							attachment.UnlockFromParent();
						}

						ReplaceWheelLambda lambda = new ReplaceWheelLambda(attachment, new_attachment, null);
						lambda.SetTransferParams(true, true, true);
						GetInventory().ReplaceItemWithNew(InventoryMode.SERVER, lambda);
					}
				}

				if ( attachment.IsInherited(CarDoor) )
				{
					array<string> att_dmgZones = new array<string>;
					attachment.GetDamageZones(att_dmgZones);
					foreach (string att_dmgZone: att_dmgZones)
					{
						attachment.SetHealthMax( att_dmgZone, "Health" );
					}
				}
			}
		}

		array<string> dmgZones = new array<string>;
		GetDamageZones(dmgZones);
		foreach (string dmgZone: dmgZones)
		{
			SetHealthMax( dmgZone, "Health" );
		}
		SetHealthMax( "", "" );
	}

	void COT_Refuel()
	{
		Fill( CarFluid.FUEL, GetFluidCapacity( CarFluid.FUEL ) );
		Fill( CarFluid.OIL, GetFluidCapacity( CarFluid.OIL ) );
		Fill( CarFluid.BRAKE, GetFluidCapacity( CarFluid.BRAKE ) );
		Fill( CarFluid.COOLANT, GetFluidCapacity( CarFluid.COOLANT ) );
	}
};

