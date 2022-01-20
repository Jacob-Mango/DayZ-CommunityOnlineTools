class COTPrefabObject
{
	// Config class name of the item
	string ClassName = "";

	// Number of items in the stack
	int Quantity = 1;

	// Health of item per damage zone -> if empty string then it is global health
	autoptr map<string, float> Health = new map<string, float>();

	// Inventory attachments -> if taken, won't spawn self and contents
	autoptr map<string, ref COTPrefabObject> InventoryAttachments = new map<string, ref COTPrefabObject>();

	// Inventory Cargo -> if no free spot in cargo, won't spawn self and contents
	autoptr array<ref COTPrefabObject> InventoryCargo = new array<ref COTPrefabObject>();

	// Construction parts built -> only works if object is basebuilding
	autoptr array<string> ConstructionPartsBuilt = new array<string>();

	COTPrefabObject BeginAttachment(string className, string slotName = "")
	{
		COTPrefabObject object = new COTPrefabObject();
		object.ClassName = className;

		COTPrefabFile.s_Begin.Insert(this);

		InventoryAttachments.Insert(slotName, object);
		return object;
	}

	COTPrefabObject BeginCargo(string className)
	{
		COTPrefabObject object = new COTPrefabObject();
		object.ClassName = className;

		COTPrefabFile.s_Begin.Insert(this);

		InventoryCargo.Insert(object);
		return object;
	}

	COTPrefabObject End()
	{
		int index = COTPrefabFile.s_Begin.Count() - 1;

		COTPrefabObject object = COTPrefabFile.s_Begin[index];

		COTPrefabFile.s_Begin.Remove(index);

		return object;
	}

	COTPrefabObject SetQuantity(int quantity)
	{
		Quantity = quantity;

		return this;
	}

	COTPrefabObject SetHealth(string zone, float health)
	{
		Health.Insert(zone, health);

		return this;
	}
	
	COTPrefabObject SetPartConstructed(string part)
	{
		ConstructionPartsBuilt.Insert(part);

		return this;
	}
	
	void Save()
	{

	}

	void Spawn(Object self)
	{
		if (!self)
		{
			return;
		}

		foreach (string zone, float health : Health)
		{
			self.SetHealth(zone, "", health);
		}
		
		EntityAI entity;
		if (Class.CastTo(entity, self))
		{
			Car car;
			if (Class.CastTo(car, entity))
			{
				FillCar(car, CarFluid.FUEL);
				FillCar(car, CarFluid.OIL);
				FillCar(car, CarFluid.BRAKE);
				FillCar(car, CarFluid.COOLANT);
			}

			ItemBase item;
			if (Class.CastTo(item, entity))
			{
				int quantity = Quantity;
	
				if (quantity == -1 && item.HasQuantity())
				{
					quantity = item.GetQuantityInit();
				}
	
				if (quantity > 0)
				{
					item.SetQuantity(quantity);
				}
			}
				
			GameInventory inventory = entity.GetInventory();
			if (inventory)
			{
				foreach (string slotName, COTPrefabObject attachment : InventoryAttachments)
				{
					int slotId = InventorySlots.GetSlotIdFromString(slotName);
					if (!inventory.HasInventorySlot(slotId))
						continue;
	
					attachment.Spawn(inventory.CreateAttachmentEx(attachment.ClassName, slotId));
				}
	
				foreach (COTPrefabObject cargo : InventoryCargo)
				{
					cargo.Spawn(inventory.CreateEntityInCargo(cargo.ClassName));
				}
			}
		}	
	}

	void FillCar(Car car, CarFluid fluid)
	{
		car.Fill(fluid, car.GetFluidCapacity(fluid));
	}
};
