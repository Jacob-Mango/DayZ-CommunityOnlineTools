// Represente un "lot" de constructions regroupees geographiquement (une zone).
class JMBuildGroup
{
	vector m_Center;
	ref array<ref JMBuildMetaData> m_Items;

	void JMBuildGroup()
	{
		m_Items = new array<ref JMBuildMetaData>;
		m_Center = vector.Zero;
	}

	void AddItem(JMBuildMetaData meta)
	{
		m_Items.Insert(meta);
	}

	int Count()
	{
		return m_Items.Count();
	}

	// Duree de vie restante (max configuree) la plus elevee parmi les elements, en jours.
	float GetMaxLifetimeDays()
	{
		float maxDays = 0;
		foreach (JMBuildMetaData m: m_Items)
		{
			float d = m.GetLifetimeDays();
			if (d > maxDays)
				maxDays = d;
		}
		return maxDays;
	}

	// Sante moyenne (%) de tous les elements de la zone.
	float GetAvgHealthPercent()
	{
		if (m_Items.Count() == 0)
			return 0;

		float sum = 0;
		foreach (JMBuildMetaData m: m_Items)
			sum += m.GetHealthPercent();

		return sum / m_Items.Count();
	}

	// Duree de vie restante MOYENNE de la zone, en jours.
	float GetAvgLifetimeDays()
	{
		if (m_Items.Count() == 0)
			return 0;

		float sum = 0;
		foreach (JMBuildMetaData m: m_Items)
			sum += m.GetLifetimeDays();

		return sum / m_Items.Count();
	}

	// Recalcule le centre (barycentre horizontal) du groupe.
	void RecomputeCenter()
	{
		if (m_Items.Count() == 0)
			return;

		float sx = 0;
		float sz = 0;
		float sy = 0;
		foreach (JMBuildMetaData m: m_Items)
		{
			sx += m.m_Position[0];
			sy += m.m_Position[1];
			sz += m.m_Position[2];
		}
		float n = m_Items.Count();
		m_Center = Vector(sx / n, sy / n, sz / n);
	}

	// ------------------------------------------------------------------------
	//  Regroupement glouton par distance horizontale.
	//  Tout element a moins de "radius" metres du germe rejoint le meme lot.
	// ------------------------------------------------------------------------
	static array<ref JMBuildGroup> BuildGroups(array<ref JMBuildMetaData> items, float radius)
	{
		array<ref JMBuildGroup> groups = new array<ref JMBuildGroup>;

		if (!items || items.Count() == 0)
			return groups;

		if (radius <= 0)
			radius = 150.0;

		int total = items.Count();
		array<bool> assigned = new array<bool>;
		for (int a = 0; a < total; a++)
			assigned.Insert(false);

		for (int i = 0; i < total; i++)
		{
			if (assigned.Get(i))
				continue;

			JMBuildGroup grp = new JMBuildGroup();
			JMBuildMetaData seed = items.Get(i);
			vector seedPos = seed.m_Position;

			grp.AddItem(seed);
			assigned.Set(i, true);

			for (int j = i + 1; j < total; j++)
			{
				if (assigned.Get(j))
					continue;

				JMBuildMetaData other = items.Get(j);
				vector d = other.m_Position - seedPos;
				d[1] = 0; // distance horizontale uniquement
				if (d.Length() <= radius)
				{
					grp.AddItem(other);
					assigned.Set(j, true);
				}
			}

			grp.RecomputeCenter();
			groups.Insert(grp);
		}

		return groups;
	}
}
