class JMLoadoutItem_Base
{
	string m_Classname;
	ref JMLoadoutItemData m_Data;
	ref array< ref JMLoadoutSubItem > m_Attachments = {};

	[NonSerialized()]
	EntityAI m_Entity;

	void JMLoadoutItem_Base(Object obj = null, int depth = 3, bool includeCargo = true)
	{
		if (obj)
			Acquire(obj, depth, includeCargo);
	}

	void Acquire(Object obj, int depth = 3, bool includeCargo = true)
	{
		m_Classname = obj.GetType();

		if (Class.CastTo(m_Entity, obj))
		{
			m_Data = new JMLoadoutItemData(m_Entity);
			if (depth > 0)
				ProcessInventory(depth, includeCargo);
		}
	}

	void ProcessInventory(int depth = 3, bool includeCargo = true)
	{
		GameInventory inventory = m_Entity.GetInventory();

		if (inventory)
		{
			EntityAI child;
			for (int i = 0; i < inventory.AttachmentCount(); ++i)
			{
				child = inventory.GetAttachmentFromIndex( i );
				AcquireSubItem(child, depth, includeCargo);
			}

			if (includeCargo)
			{
				CargoBase cargo = inventory.GetCargo();
				if (cargo)
				{
					for (int j = 0; j < cargo.GetItemCount(); ++j)
					{
						child = cargo.GetItem(j);
						AcquireSubItem(child, depth, true);
					}
				}
			}
		}
	}

	void AcquireSubItem(EntityAI child, int depth = 3, bool includeCargo = true)
	{
		JMLoadoutSubItem item = new JMLoadoutSubItem(child, depth - 1, includeCargo);
		m_Attachments.Insert(item);
	}
}
