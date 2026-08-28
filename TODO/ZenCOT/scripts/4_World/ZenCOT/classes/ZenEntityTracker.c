class ZenEntityTracker
{
	static int m_EntityCount = 0;
	
	// Store entities by classname to save time on for() loop lookups 
	// There can easily be 30,000+ items tracked - not sure how much scanning 30k items in a for loop affects performance
	// so this is probably overkill optimization to do it this way, but server performance optimization is never a bad thing in DayZ lol.
	static ref map<string, ref array<EntityAI>> m_EntitiesByType = new map<string, ref array<EntityAI>>;

	static void Register(EntityAI item)
	{
		string t = item.GetType();
		
		array<EntityAI> bucket;
		if (!m_EntitiesByType.Find(t, bucket)) 
		{
			bucket = new array<EntityAI>;
			m_EntitiesByType.Insert(t, bucket);
		}
		
		bucket.Insert(item);
		m_EntityCount++;
	}

	static void Unregister(EntityAI item)
	{
		string t = item.GetType();
		array<EntityAI> bucket;
		
		if (m_EntitiesByType.Find(t, bucket)) 
		{
			int idx = bucket.Find(item);
			
			if (idx > -1) 
				bucket.Remove(idx);
			
			if (bucket.Count() == 0) 
				m_EntitiesByType.Remove(t);
		}
		
		m_EntityCount--;
	}

	static void GetByClassname(string className, out array<EntityAI> outItems, bool clearExistingArray = true)
	{
		float t0 = g_Game.GetTime();
		
		if (!outItems)
		{
			outItems = new array<EntityAI>;
		}
		else 
		if (clearExistingArray)
		{
			outItems.Clear();
		}
		
		if (className == "") 
			return;

		array<EntityAI> bucket;
		if (m_EntitiesByType.Find(className, bucket)) 
		{
			outItems.Copy(bucket); // return a copy so callers can't mutate our buckets
		}
		
		float ms = (g_Game.GetTime() - t0);
		Print("[ZenCOT] Built list for " + className + " - count=" + outItems.Count() + " / total tracked entities=" + m_EntityCount + " took " + ms + "ms");
	}
	
	static string ZenCOT_GetObjectName(Object objTarget)
	{
		if (!objTarget)
			return "null object";
		
		string objType = objTarget.GetType();
		if (objType != "")
			return objType;

		objType = objTarget.GetDebugName();
		if (objType != "")
			return objType;
		
		objType = objTarget.GetDebugNameNative();
		if (objType != "")
			return objType;
		
		objType = objTarget.GetModelName();
		if (objType != "")
			return objType;
		
		objType = objTarget.ClassName();
		if (objType != "")
			return objType;
		
		return "unknown entity";
	}

	static int DeleteAllEntities(string className)
	{
		int deleteCount = 0;
		array<EntityAI> tempArray;
		GetByClassname(className, tempArray);

		foreach (EntityAI entity : tempArray)
		{
			if (!entity)
				continue;

			entity.DeleteSafe();
			deleteCount++;
		}

		return deleteCount;
	}
}
