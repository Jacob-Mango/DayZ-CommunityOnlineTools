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

	void COT_VehicleSetPos(vector position)
	{
		vector surface = Vector(position[0],GetGame().SurfaceY(position[0], position[2]),position[2]);

		PhxInteractionLayers layerMask;
		layerMask |= PhxInteractionLayers.BUILDING;
		layerMask |= PhxInteractionLayers.DOOR;
		layerMask |= PhxInteractionLayers.VEHICLE;
		layerMask |= PhxInteractionLayers.ROADWAY;
		layerMask |= PhxInteractionLayers.TERRAIN;
		layerMask |= PhxInteractionLayers.ITEM_LARGE;
		layerMask |= PhxInteractionLayers.FENCE;
		vector hitPosition;

		if (DayZPhysics.RayCastBullet(position, surface, layerMask, this, null, hitPosition, null, null))
		{
			position = hitPosition;
		} else {
			position = surface;
		}

		vector entityMinMax[2];
		if (!GetCollisionBox(entityMinMax))
			ClippingInfo(entityMinMax);

		float entityOffsetY = entityMinMax[0][1];
		if (entityOffsetY > 0)
			entityOffsetY = 0;

		position[1] = position[1] + entityOffsetY;

		SetPosition(position);
		SetOrientation(Vector(GetOrientation()[0],0,0));
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
}
