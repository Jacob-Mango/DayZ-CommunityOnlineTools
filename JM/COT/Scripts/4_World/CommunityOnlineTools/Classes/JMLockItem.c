class JMLockItem
{
	static ItemBase Get( Object target, int doorIndex = -1)
	{
		ItemBase item;

		if ( Class.CastTo( item, target ) && item.COT_IsLock() )
			return item;

		Fence fence;
		if ( Class.CastTo( fence, target ) )
		{
			if ( Class.CastTo( item, fence.GetCombinationLock() ) || Class.CastTo( item, fence.GetCodeLock() ) )
				return item;
		}

		//! A bunker has the one lock, on its entrance, not one per door
		Bunker bunker;
		if ( Class.CastTo( bunker, target ) )
		{
			if ( Class.CastTo( item, bunker.GetCodeLock() ) )
				return item;
		}

		BuildingBase building;
		if ( Class.CastTo( building, target ) && doorIndex >= 0 )
		{
			AdditionalDoorInfo doorInfo = building.GetDoorInfo( doorIndex );

			if ( doorInfo  && doorInfo.m_RelatedInventorySlotNames )
			{
				foreach ( string slotName : doorInfo.m_RelatedInventorySlotNames )
				{
					if ( Class.CastTo( item, building.GetCombinationLock( slotName ) ) || Class.CastTo( item, building.GetCodeLock( slotName ) ) )
						return item;
				}

				return null;
			}
		}

		EntityAI entity = EntityAI.Cast( target );

		if ( !entity )
			return null;

		GameInventory inventory = entity.GetInventory();

		if ( !inventory )
			return null;

		for ( int i = 0; i < inventory.AttachmentCount(); ++i )
		{
			if ( Class.CastTo( item, inventory.GetAttachmentFromIndex( i ) ) && item.COT_IsLock() )
				return item;
		}

		return null;
	}
}
