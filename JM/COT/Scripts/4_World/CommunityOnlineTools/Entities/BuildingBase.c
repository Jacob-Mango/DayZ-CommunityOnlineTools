modded class BuildingBase
{
	override void EEItemAttached(EntityAI item, string slot_name)
	{
		super.EEItemAttached(item, slot_name);

		if (g_cotBase)
			g_cotBase.OnItemAttached(this, slot_name, item);
	}

	override void EEItemDetached(EntityAI item, string slot_name)
	{
		super.EEItemDetached(item, slot_name);

		if (g_cotBase)
			g_cotBase.OnItemDetached(this, slot_name, item);
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if (g_cotBase)
			g_cotBase.OnVariablesSynchronized(this);
	}
}
