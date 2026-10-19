class JMLoadoutItem: JMLoadoutItem_Base
{
	vector m_LocalPosition;
	vector m_LocalRotation;
	autoptr TStringArray m_ConstructionParts;

	override void Acquire(Object obj, int depth = 3, bool includeCargo = true)
	{
		super.Acquire(obj, depth, includeCargo);

		m_LocalPosition = obj.GetPosition();
		m_LocalRotation = obj.GetOrientation();

		// We check the partname instead of the part ID for better readability for the users
		// if its causing too much perf issues we might switch to the int ID instead
		BaseBuildingBase bb;
		if (Class.CastTo(bb, obj))
		{
			Construction construction = bb.GetConstruction();
			if (!construction)
				return;

			m_ConstructionParts = {};
			map<string, ref ConstructionPart> parts = construction.GetConstructionParts();
			foreach (string partName, ConstructionPart part: parts)
			{
				if (part.m_IsBuilt)
				{
					m_ConstructionParts.Insert(part.GetPartName());
				}
			}
		}
	}
}
