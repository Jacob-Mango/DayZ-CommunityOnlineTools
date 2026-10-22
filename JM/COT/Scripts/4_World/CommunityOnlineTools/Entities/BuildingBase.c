modded class BuildingBase
{
	override void EEItemAttached(EntityAI item, string slot_name)
	{
		super.EEItemAttached(item, slot_name);

		GetCOTBase().OnItemAttached(this, slot_name, item);
	}

	override void EEItemDetached(EntityAI item, string slot_name)
	{
		super.EEItemDetached(item, slot_name);

		GetCOTBase().OnItemDetached(this, slot_name, item);
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		GetCOTBase().OnVariablesSynchronized(this);
	}
}
