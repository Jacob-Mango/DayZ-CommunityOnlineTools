class JMEntityTracker
{
	static int m_EntityCount = 0;

	// Store entities by classname for fast lookup
	static ref map<string, ref array<EntityAI>> m_EntitiesByType = new map<string, ref array<EntityAI>>;

	static void Register(EntityAI item)
	{
		if (!item)
			return;

		string t = item.GetType();

		array<EntityAI> bucket;
		if (!m_EntitiesByType.Find(t, bucket))
		{
			bucket = new array<EntityAI>;
			m_EntitiesByType.Insert(t, bucket);
		}

		//! EEInit can fire more than once for the same entity (e.g. on
		//! re-init after storage load), so guard against double-counting.
		if (bucket.Find(item) > -1)
			return;

		bucket.Insert(item);
		m_EntityCount++;
	}

	static void Unregister(EntityAI item)
	{
		if (!item)
			return;

		string t = item.GetType();
		array<EntityAI> bucket;

		if (!m_EntitiesByType.Find(t, bucket))
			return;

		int idx = bucket.Find(item);

		//! Only decrement when the entity was actually in the bucket.
		//! Decrementing unconditionally drifts m_EntityCount permanently
		//! negative on any unregister of an untracked entity.
		if (idx == -1)
			return;

		bucket.Remove(idx);
		m_EntityCount--;

		if (bucket.Count() == 0)
			m_EntitiesByType.Remove(t);
	}

	static void GetByClassname(string className, out array<EntityAI> outItems, bool clearExistingArray = true)
	{
		if (!outItems)
		{
			outItems = new array<EntityAI>;
		}
		else if (clearExistingArray)
		{
			outItems.Clear();
		}

		if (className == "")
			return;

		array<EntityAI> bucket;
		if (m_EntitiesByType.Find(className, bucket))
		{
			outItems.Copy(bucket);
		}
	}

	static int DeleteAllEntities(string className)
	{
		int deleteCount = 0;

		//! GetByClassname copies into tempArray, so this iterates a snapshot --
		//! DeleteSafe -> EEDelete -> Unregister mutates the live bucket
		//! underneath us, which would invalidate a direct iteration.
		array<EntityAI> tempArray;
		GetByClassname(className, tempArray);

		foreach (EntityAI entity : tempArray)
		{
			if (!entity)
				continue;

			//! Already queued for deletion by something else this frame.
			if (entity.IsSetForDeletion())
				continue;

			entity.DeleteSafe();
			deleteCount++;
		}

		return deleteCount;
	}

	static int GetTotalEntityCount()
	{
		return m_EntityCount;
	}

	static int GetClassnameCount(string className)
	{
		array<EntityAI> bucket;
		if (m_EntitiesByType.Find(className, bucket))
			return bucket.Count();
		return 0;
	}

	static void GetAllClassnames(out array<string> outClassnames)
	{
		if (!outClassnames)
			outClassnames = new array<string>;
		else
			outClassnames.Clear();

		for (int i = 0; i < m_EntitiesByType.Count(); i++)
		{
			outClassnames.Insert(m_EntitiesByType.GetKey(i));
		}
	}
}
